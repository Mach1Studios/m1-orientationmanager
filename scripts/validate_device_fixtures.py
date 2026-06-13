from __future__ import annotations

import json
import sys
from pathlib import Path


ALLOWED_TRANSPORTS = {"serial", "ble", "osc", "camera", "emulator", "unknown"}
ALLOWED_ORIENTATION_FORMATS = {
    "quaternion",
    "euler yaw / pitch / roll",
    "accel / gyro / mag that must be fused",
    "unknown",
}


def validate_fixture_directory(fixture_dir: Path) -> list[str]:
    errors: list[str] = []
    manifest_path = fixture_dir / "manifest.json"

    if not manifest_path.exists():
        return [f"{fixture_dir}: missing manifest.json"]

    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return [f"{manifest_path}: invalid JSON ({exc})"]

    required_string_fields = ("deviceName", "company", "transport", "orientationFormat")
    for field in required_string_fields:
        value = manifest.get(field)
        if not isinstance(value, str) or not value.strip():
            errors.append(f"{manifest_path}: field '{field}' must be a non-empty string")

    if not isinstance(manifest.get("schemaVersion"), int):
        errors.append(f"{manifest_path}: field 'schemaVersion' must be an integer")

    if not isinstance(manifest.get("issue"), int):
        errors.append(f"{manifest_path}: field 'issue' must be an integer")

    if manifest.get("transport") not in ALLOWED_TRANSPORTS:
        errors.append(f"{manifest_path}: field 'transport' must be one of {sorted(ALLOWED_TRANSPORTS)}")

    orientation_format = str(manifest.get("orientationFormat", "")).lower()
    if orientation_format not in ALLOWED_ORIENTATION_FORMATS:
        errors.append(
            f"{manifest_path}: field 'orientationFormat' must be one of {sorted(ALLOWED_ORIENTATION_FORMATS)}"
        )

    artifacts = manifest.get("artifacts")
    if not isinstance(artifacts, list) or not artifacts:
        errors.append(f"{manifest_path}: field 'artifacts' must be a non-empty list")
        return errors

    for artifact in artifacts:
        if not isinstance(artifact, dict):
            errors.append(f"{manifest_path}: every artifact entry must be an object")
            continue

        relative_path = artifact.get("path")
        description = artifact.get("description")
        if not isinstance(relative_path, str) or not relative_path.strip():
            errors.append(f"{manifest_path}: every artifact must contain a non-empty 'path'")
            continue
        if not isinstance(description, str) or not description.strip():
            errors.append(f"{manifest_path}: artifact '{relative_path}' is missing a description")

        artifact_path = fixture_dir / relative_path
        if not artifact_path.exists():
            errors.append(f"{manifest_path}: artifact path does not exist: {artifact_path}")

    return errors


def main() -> int:
    repo_root = Path.cwd()
    fixtures_root = repo_root / "test" / "device-fixtures"

    if not fixtures_root.exists():
        print("No device fixture directory found. Skipping fixture validation.")
        return 0

    fixture_dirs = sorted(
        path for path in fixtures_root.iterdir() if path.is_dir() and not path.name.startswith("_")
    )
    if not fixture_dirs:
        print("No concrete device fixtures found. Skipping fixture validation.")
        return 0

    errors: list[str] = []
    for fixture_dir in fixture_dirs:
        errors.extend(validate_fixture_directory(fixture_dir))

    if errors:
        print("Fixture validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1

    print(f"Validated {len(fixture_dirs)} device fixture director{'y' if len(fixture_dirs) == 1 else 'ies'}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
