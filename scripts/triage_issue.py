from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.parse
import urllib.request

from device_issue_utils import (
    TRANSPORT_LABELS,
    TRIAGE_MARKER,
    classify_issue_type,
    cleanup_value,
    get_section,
    is_meaningful,
    normalize_transport,
    parse_issue_sections,
)


LABEL_DEFINITIONS = {
    "bug": {"color": "d73a4a", "description": "Something is not working"},
    "device-request": {"color": "1d76db", "description": "Request for a new device integration"},
    "device-bug": {"color": "b60205", "description": "Problem report for an existing device integration"},
    "ready-for-scaffold": {"color": "0e8a16", "description": "Enough information is present to open a draft scaffold PR"},
    "needs-docs": {"color": "fbca04", "description": "Protocol or vendor documentation is still needed"},
    "needs-sample-data": {"color": "fbca04", "description": "Representative logs, packets, or captures are still needed"},
    "needs-repro": {"color": "fbca04", "description": "Reproduction steps are still needed"},
    "needs-transport-triage": {"color": "fbca04", "description": "A maintainer needs to confirm the transport layer"},
    "transport:serial": {"color": "bfdadc", "description": "Serial or USB transport"},
    "transport:ble": {"color": "bfdadc", "description": "BLE transport"},
    "transport:osc": {"color": "bfdadc", "description": "OSC or network transport"},
    "transport:camera": {"color": "bfdadc", "description": "Camera transport"},
    "transport:emulator": {"color": "bfdadc", "description": "Virtual or emulator transport"},
    "transport:unknown": {"color": "cfd3d7", "description": "Transport still needs maintainer triage"},
}

MANAGED_LABELS = set(LABEL_DEFINITIONS)


class GitHubClient:
    def __init__(self, repository: str, token: str) -> None:
        self.repository = repository
        self.base_url = f"https://api.github.com/repos/{repository}"
        self.token = token

    def request(self, method: str, path: str, payload: dict | list | None = None) -> dict | list | None:
        data = None
        headers = {
            "Accept": "application/vnd.github+json",
            "Authorization": f"Bearer {self.token}",
            "X-GitHub-Api-Version": "2022-11-28",
        }
        if payload is not None:
            data = json.dumps(payload).encode("utf-8")
            headers["Content-Type"] = "application/json"

        request = urllib.request.Request(f"{self.base_url}{path}", data=data, headers=headers, method=method)
        try:
            with urllib.request.urlopen(request) as response:
                if response.length == 0:
                    return None
                body = response.read().decode("utf-8")
                return json.loads(body) if body else None
        except urllib.error.HTTPError as exc:
            body = exc.read().decode("utf-8")
            if exc.code == 404:
                raise
            raise RuntimeError(f"GitHub API {method} {path} failed: {exc.code} {body}") from exc

    def ensure_label(self, name: str) -> None:
        definition = LABEL_DEFINITIONS[name]
        try:
            self.request("GET", f"/labels/{urllib.parse.quote(name, safe='')}")
        except urllib.error.HTTPError as exc:
            if exc.code != 404:
                raise
            self.request(
                "POST",
                "/labels",
                {
                    "name": name,
                    "color": definition["color"],
                    "description": definition["description"],
                },
            )

    def add_labels(self, issue_number: int, labels: list[str]) -> None:
        if labels:
            self.request("POST", f"/issues/{issue_number}/labels", {"labels": labels})

    def remove_label(self, issue_number: int, label: str) -> None:
        self.request("DELETE", f"/issues/{issue_number}/labels/{urllib.parse.quote(label, safe='')}")

    def list_comments(self, issue_number: int) -> list[dict]:
        comments = self.request("GET", f"/issues/{issue_number}/comments")
        return comments if isinstance(comments, list) else []

    def create_comment(self, issue_number: int, body: str) -> None:
        self.request("POST", f"/issues/{issue_number}/comments", {"body": body})

    def update_comment(self, comment_id: int, body: str) -> None:
        self.request("PATCH", f"/issues/comments/{comment_id}", {"body": body})


def build_triage(issue_type: str, sections: dict[str, str]) -> tuple[set[str], str]:
    labels: set[str] = {issue_type}

    if issue_type == "device-bug":
        labels.add("bug")

    transport = normalize_transport(get_section(sections, "transport"))
    labels.add(TRANSPORT_LABELS[transport])

    missing: list[str] = []

    if issue_type == "device-request":
        if not is_meaningful(get_section(sections, "docs")):
            missing.append("protocol or vendor docs")
            labels.add("needs-docs")
        if not is_meaningful(get_section(sections, "sample_data")):
            missing.append("representative sample data or logs")
            labels.add("needs-sample-data")
        if transport == "unknown":
            missing.append("transport classification")
            labels.add("needs-transport-triage")
    else:
        if not is_meaningful(get_section(sections, "reproduction_steps")):
            missing.append("reproduction steps")
            labels.add("needs-repro")
        if not is_meaningful(get_section(sections, "sample_data")):
            missing.append("logs or sample packets")
            labels.add("needs-sample-data")
        if transport == "unknown":
            missing.append("transport classification")
            labels.add("needs-transport-triage")

    if not any(label.startswith("needs-") for label in labels):
        labels.add("ready-for-scaffold")

    status_line = (
        "Ready for `/scaffold-device`."
        if "ready-for-scaffold" in labels
        else "Needs more information before a scaffold PR should be opened."
    )

    missing_lines = "\n".join(f"- missing: {item}" for item in missing) if missing else "- missing: none"

    summary = "\n".join(
        [
            TRIAGE_MARKER,
            "Automation triage summary:",
            f"- type: `{issue_type}`",
            f"- transport: `{transport}`",
            f"- status: {status_line}",
            missing_lines,
            "",
            "Maintainer action:",
            "- Comment `/scaffold-device` to open a draft PR scaffold once this issue is ready.",
        ]
    )

    return labels, summary


def sync_labels(client: GitHubClient, issue_number: int, current_labels: set[str], target_labels: set[str]) -> None:
    for label in sorted(target_labels):
        client.ensure_label(label)

    labels_to_add = sorted(target_labels - current_labels)
    labels_to_remove = sorted((current_labels & MANAGED_LABELS) - target_labels)

    client.add_labels(issue_number, labels_to_add)
    for label in labels_to_remove:
        client.remove_label(issue_number, label)


def upsert_triage_comment(client: GitHubClient, issue_number: int, body: str) -> None:
    for comment in client.list_comments(issue_number):
        current_body = comment.get("body", "")
        if TRIAGE_MARKER in current_body:
            if cleanup_value(current_body) != cleanup_value(body):
                client.update_comment(comment["id"], body)
            return

    client.create_comment(issue_number, body)


def main() -> int:
    event_path = os.environ["GITHUB_EVENT_PATH"]
    repository = os.environ["GITHUB_REPOSITORY"]
    token = os.environ["GITHUB_TOKEN"]

    with open(event_path, "r", encoding="utf-8") as handle:
        event = json.load(handle)

    issue = event["issue"]
    issue_number = issue["number"]
    title = issue.get("title", "")
    body = issue.get("body", "")
    current_labels = {label["name"] for label in issue.get("labels", [])}
    sections = parse_issue_sections(body)

    issue_type = classify_issue_type(title, current_labels, sections)
    if issue_type is None:
        print("Issue is not a device request or device bug. Skipping triage.")
        return 0

    target_labels, summary = build_triage(issue_type, sections)
    client = GitHubClient(repository, token)
    sync_labels(client, issue_number, current_labels, target_labels)
    upsert_triage_comment(client, issue_number, summary)
    print(f"Triaged issue #{issue_number} as {issue_type}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
