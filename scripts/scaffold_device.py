from __future__ import annotations

import json
import os
import sys
from pathlib import Path

from device_issue_utils import (
    classify_issue_type,
    extract_urls,
    get_section,
    normalize_transport,
    parse_issue_sections,
    pascal_identifier,
    slugify,
)


def cpp_string_literal(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def write_file(path: Path, content: str) -> None:
    if path.exists():
        raise FileExistsError(f"Refusing to overwrite existing file: {path}")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def write_output(name: str, value: str) -> None:
    output_path = os.environ.get("GITHUB_OUTPUT")
    if not output_path:
        return

    with open(output_path, "a", encoding="utf-8") as handle:
        handle.write(f"{name}<<__CURSOR_EOF__\n{value}\n__CURSOR_EOF__\n")


def build_header(class_name: str, device_name: str, company: str, transport: str, issue_number: int) -> str:
    escaped_device_name = cpp_string_literal(device_name)
    escaped_company = cpp_string_literal(company)
    escaped_transport = cpp_string_literal(transport)
    return f"""//
//  m1-orientationmanager
//  Generated scaffold for issue #{issue_number}
//

#pragma once

#include <string>

class {class_name} {{
public:
    {class_name}();
    ~{class_name}();

    static const char* getDeviceName();
    static const char* getCompanyName();
    static const char* getTransportName();

    // TODO(issue #{issue_number}): Replace this placeholder matcher with the
    // exact identifier, address filter, or advertisement parsing needed.
    bool matchesDeviceName(const std::string& deviceName) const;
}};

"""


def build_cpp(class_name: str, device_name: str, company: str, transport: str) -> str:
    escaped_device_name = cpp_string_literal(device_name)
    escaped_company = cpp_string_literal(company)
    escaped_transport = cpp_string_literal(transport)
    return f"""//
//  m1-orientationmanager
//  Generated device scaffold
//

#include "{class_name}.h"

{class_name}::{class_name}() = default;
{class_name}::~{class_name}() = default;

const char* {class_name}::getDeviceName() {{
    return "{escaped_device_name}";
}}

const char* {class_name}::getCompanyName() {{
    return "{escaped_company}";
}}

const char* {class_name}::getTransportName() {{
    return "{escaped_transport}";
}}

bool {class_name}::matchesDeviceName(const std::string& deviceName) const {{
    return deviceName.find(getDeviceName()) != std::string::npos;
}}

"""


def build_fixture_readme(issue_number: int, device_name: str, transport: str) -> str:
    return f"""# {device_name}

Generated from issue #{issue_number}.

Replace `capture.txt` with representative raw packets, BLE notifications, serial logs, OSC messages, or any other replayable input for the `{transport}` transport. Keep the payload as close to raw device output as possible so follow-up PRs can turn this into a replay test.
"""


def build_manifest(issue_number: int, device_name: str, company: str, transport: str, orientation_format: str, docs_urls: list[str]) -> str:
    manifest = {
        "schemaVersion": 1,
        "issue": issue_number,
        "deviceName": device_name,
        "company": company,
        "transport": transport,
        "orientationFormat": orientation_format or "unknown",
        "docs": docs_urls,
        "artifacts": [
            {
                "path": "capture.txt",
                "description": "Replace this placeholder with representative sample packets or logs.",
            }
        ],
    }
    return json.dumps(manifest, indent=2) + "\n"


def build_placeholder_capture(device_name: str) -> str:
    return "\n".join(
        [
            "# Replace this placeholder with representative raw device output.",
            f"# Device: {device_name}",
            "# Keep timestamps / delimiters / packet framing intact when possible.",
            "",
        ]
    )


def build_pr_body(issue_number: int, issue_type: str, device_name: str, company: str, transport: str, docs_urls: list[str]) -> str:
    action_line = "Scaffold support for the requested device" if issue_type == "device-request" else "Scaffold investigation and fixture capture for the reported device bug"
    docs_line = docs_urls[0] if docs_urls else "Add vendor or protocol links here."
    return f"""## Summary
- {action_line}
- Generate a compile-safe interface stub under `Source/Devices/`
- Create a fixture folder under `test/device-fixtures/` for replayable sample data

## Linked Issue
Closes #{issue_number}

## Generated Context
- Device: `{device_name}`
- Company: `{company}`
- Transport: `{transport}`
- Primary doc: {docs_line}

Keep this PR in draft until the last two checkboxes are complete.

## Device Integration Checklist
- [ ] I linked protocol or vendor docs in this PR or the linked issue.
- [ ] I captured or linked representative sample data / logs for this device.
- [ ] I added or updated fixture metadata under `test/device-fixtures/` for this change.
- [ ] A human tester smoke tested this build on real hardware.
- [ ] A human reviewer verified axis order, sign conventions, and recenter/calibration behavior.

## Test Notes
- CI:
- Manual:
- Hardware:
"""


def main() -> int:
    event_path = os.environ["GITHUB_EVENT_PATH"]
    repo_root = Path.cwd()

    with open(event_path, "r", encoding="utf-8") as handle:
        event = json.load(handle)

    issue = event["issue"]
    issue_number = issue["number"]
    title = issue.get("title", "")
    body = issue.get("body", "")
    labels = [label["name"] for label in issue.get("labels", [])]
    sections = parse_issue_sections(body)
    issue_type = classify_issue_type(title, labels, sections)
    if issue_type is None:
        raise RuntimeError("Issue is not recognized as a device request or device bug.")
    if "ready-for-scaffold" not in labels:
        raise RuntimeError("Issue is not labeled ready-for-scaffold. Complete triage before scaffolding.")

    device_name = get_section(sections, "device_name") or title
    company = get_section(sections, "company") or "Unknown"
    transport = normalize_transport(get_section(sections, "transport"))
    orientation_format = get_section(sections, "orientation_format") or "unknown"
    docs_urls = extract_urls(get_section(sections, "docs"))

    class_name = pascal_identifier(device_name)
    if not class_name.endswith("Interface"):
        class_name = f"{class_name}Interface"

    device_slug = slugify(device_name)
    branch_name = f"automation/device/{device_slug}-issue-{issue_number}"

    header_path = repo_root / "Source" / "Devices" / f"{class_name}.h"
    cpp_path = repo_root / "Source" / "Devices" / f"{class_name}.cpp"
    fixture_dir = repo_root / "test" / "device-fixtures" / device_slug
    fixture_readme_path = fixture_dir / "README.md"
    fixture_manifest_path = fixture_dir / "manifest.json"
    fixture_capture_path = fixture_dir / "capture.txt"

    write_file(header_path, build_header(class_name, device_name, company, transport, issue_number))
    write_file(cpp_path, build_cpp(class_name, device_name, company, transport))
    write_file(fixture_readme_path, build_fixture_readme(issue_number, device_name, transport))
    write_file(
        fixture_manifest_path,
        build_manifest(issue_number, device_name, company, transport, orientation_format, docs_urls),
    )
    write_file(fixture_capture_path, build_placeholder_capture(device_name))

    pr_title_prefix = "Scaffold support for" if issue_type == "device-request" else "Scaffold bugfix investigation for"
    commit_prefix = "scaffold device support for" if issue_type == "device-request" else "scaffold device bug investigation for"

    write_output("branch_name", branch_name)
    write_output("commit_message", f"{commit_prefix} {device_name}")
    write_output("pr_title", f"[device] {pr_title_prefix} {device_name}")
    write_output("pr_body", build_pr_body(issue_number, issue_type, device_name, company, transport, docs_urls))
    write_output("device_slug", device_slug)
    write_output("class_name", class_name)
    print(f"Generated scaffold for {device_name} in {header_path.parent}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
