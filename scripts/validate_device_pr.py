from __future__ import annotations

import json
import os
import re
import subprocess
import sys


DEVICE_PATH_PREFIXES = (
    "Source/Devices/",
    "Source/HardwareSerial.h",
    "Source/HardwareBLE.h",
    "Source/BLEDeviceMap.cpp",
    "Source/BLEDeviceMap.h",
    "Source/CMakeLists.txt",
    "test/device-fixtures/",
)

REQUIRED_CHECKLIST_ITEMS = (
    "I linked protocol or vendor docs in this PR or the linked issue.",
    "I captured or linked representative sample data / logs for this device.",
    "I added or updated fixture metadata under `test/device-fixtures/` for this change.",
    "A human tester smoke tested this build on real hardware.",
    "A human reviewer verified axis order, sign conventions, and recenter/calibration behavior.",
)

ISSUE_LINK_RE = re.compile(r"(?im)^\s*(closes|fixes|relates to)\s+#\d+\s*$")


def get_changed_files(base_sha: str, head_sha: str) -> list[str]:
    result = subprocess.run(
        ["git", "diff", "--name-only", base_sha, head_sha],
        check=True,
        capture_output=True,
        text=True,
    )
    return [line.strip() for line in result.stdout.splitlines() if line.strip()]


def is_device_pr(changed_files: list[str]) -> bool:
    return any(path.startswith(prefix) for path in changed_files for prefix in DEVICE_PATH_PREFIXES)


def has_checked_box(body: str, label: str) -> bool:
    pattern = re.compile(rf"(?im)^\s*-\s*\[[xX]\]\s*{re.escape(label)}\s*$")
    return bool(pattern.search(body))


def main() -> int:
    event_path = os.environ["GITHUB_EVENT_PATH"]
    base_sha = os.environ["GITHUB_BASE_SHA"]
    head_sha = os.environ["GITHUB_HEAD_SHA"]

    with open(event_path, "r", encoding="utf-8") as handle:
        event = json.load(handle)

    pull_request = event["pull_request"]
    body = pull_request.get("body", "") or ""
    is_draft = bool(pull_request.get("draft"))
    changed_files = get_changed_files(base_sha, head_sha)

    if not is_device_pr(changed_files):
        print("PR does not affect device integration paths. Skipping device-specific gates.")
        return 0

    errors: list[str] = []

    if not ISSUE_LINK_RE.search(body):
        errors.append("Device PRs must link an issue with `Closes #...`, `Fixes #...`, or `Relates to #...`.")

    if "## Device Integration Checklist" not in body:
        errors.append("Device PRs must use the device pull request template.")

    fixture_files_changed = any(path.startswith("test/device-fixtures/") for path in changed_files)

    if not is_draft:
        if not fixture_files_changed:
            errors.append("Ready-for-review device PRs must include fixture changes under `test/device-fixtures/`.")

        for item in REQUIRED_CHECKLIST_ITEMS:
            if not has_checked_box(body, item):
                errors.append(f"Unchecked required checklist item: {item}")
    else:
        print("Device PR is still draft; checklist enforcement is deferred until review-ready.")

    if errors:
        print("Device PR validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1

    print("Device PR validation passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
