from __future__ import annotations

import re
from typing import Iterable


TRIAGE_MARKER = "<!-- device-automation-triage -->"

SECTION_ALIASES = {
    "device_name": {
        "device_name",
        "name_of_device",
    },
    "company": {
        "company",
        "company_vendor",
        "name_of_company",
    },
    "transport": {
        "transport",
    },
    "orientation_format": {
        "orientation_output_format",
    },
    "docs": {
        "product_api_protocol_links",
        "links_and_documentation_to_device",
    },
    "sample_data": {
        "sample_data_or_logs",
        "logs_sample_packets_screenshots",
    },
    "firmware_version": {
        "firmware_app_version",
    },
    "host_platforms": {
        "host_platforms",
    },
    "can_validate": {
        "will_you_be_able_to_validate_a_candidate_integration_on_real_hardware",
        "can_you_test_a_candidate_fix_on_real_hardware",
    },
    "additional_context": {
        "additional_context",
        "additional_information_of_device",
    },
    "device_family": {
        "device_family_interface",
    },
    "actual_behavior": {
        "what_happened",
    },
    "expected_behavior": {
        "what_did_you_expect_to_happen",
    },
    "reproduction_steps": {
        "how_can_we_reproduce_it",
    },
    "regression": {
        "regression",
    },
}

TRANSPORT_LABELS = {
    "serial": "transport:serial",
    "ble": "transport:ble",
    "osc": "transport:osc",
    "camera": "transport:camera",
    "emulator": "transport:emulator",
    "unknown": "transport:unknown",
}

TRANSPORT_CHOICES = set(TRANSPORT_LABELS)

NON_VALUES = {
    "",
    "n/a",
    "na",
    "none",
    "unknown",
    "not sure",
    "unsure",
    "tbd",
    "todo",
}


def normalize_heading(text: str) -> str:
    normalized = re.sub(r"[^a-z0-9]+", "_", text.lower()).strip("_")
    return normalized


def extract_heading(line: str) -> str | None:
    stripped = line.strip()
    if stripped.startswith("### "):
        return stripped[4:].strip()

    bold_match = re.match(r"^\*\*(.+?)\*\*(?:\s*.*)?$", stripped)
    if bold_match:
        return bold_match.group(1).strip()

    return None


def parse_issue_sections(body: str) -> dict[str, str]:
    sections: dict[str, list[str]] = {}
    current_heading: str | None = None

    for raw_line in body.splitlines():
        heading = extract_heading(raw_line)
        if heading:
            current_heading = normalize_heading(heading)
            sections.setdefault(current_heading, [])
            continue

        if current_heading is None:
            continue

        sections[current_heading].append(raw_line)

    return {
        heading: "\n".join(lines).strip()
        for heading, lines in sections.items()
    }


def get_section(sections: dict[str, str], canonical_name: str) -> str:
    aliases = SECTION_ALIASES.get(canonical_name, {canonical_name})
    for alias in aliases:
        value = sections.get(alias, "").strip()
        if value:
            return value
    return ""


def cleanup_value(value: str) -> str:
    return re.sub(r"\s+", " ", value).strip()


def is_meaningful(value: str, *, allow_unknown: bool = False) -> bool:
    cleaned = cleanup_value(value).lower()
    if not cleaned:
        return False
    if allow_unknown:
        return True
    return cleaned not in NON_VALUES


def normalize_transport(value: str) -> str:
    lowered = cleanup_value(value).lower()
    if "serial" in lowered or "usb" in lowered:
        return "serial"
    if "ble" in lowered or "bluetooth" in lowered:
        return "ble"
    if "osc" in lowered or "network" in lowered:
        return "osc"
    if "camera" in lowered:
        return "camera"
    if "emulator" in lowered or "virtual" in lowered:
        return "emulator"
    return "unknown"


def classify_issue_type(title: str, labels: Iterable[str], sections: dict[str, str]) -> str | None:
    label_set = {label.lower() for label in labels}
    title_lower = title.lower()

    if "device-request" in label_set or title_lower.startswith("[device request]"):
        return "device-request"
    if "device-bug" in label_set or title_lower.startswith("[device bug]"):
        return "device-bug"

    if get_section(sections, "reproduction_steps") or get_section(sections, "actual_behavior"):
        return "device-bug"
    if get_section(sections, "docs") or get_section(sections, "company"):
        return "device-request"

    return None


def slugify(value: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")
    return slug or "device"


def pascal_identifier(value: str) -> str:
    parts = re.findall(r"[A-Za-z0-9]+", value)
    identifier = "".join(part[:1].upper() + part[1:] for part in parts)
    if not identifier:
        identifier = "Device"
    if identifier[0].isdigit():
        identifier = f"Device{identifier}"
    return identifier


def extract_urls(text: str) -> list[str]:
    return re.findall(r"https?://[^\s)>\"]+", text)
