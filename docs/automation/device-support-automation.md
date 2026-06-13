# Device Support Automation

This repository now has a lightweight automation path for new IMU/device support and device bug reports:

1. Structured issue forms collect transport, docs, sample data, and validation availability.
2. `Device Issue Triage` applies transport and readiness labels and leaves a single updatable triage comment.
3. A maintainer can comment `/scaffold-device` on a ready issue to open a draft PR with:
   - a compile-safe `Source/Devices/*Interface.{h,cpp}` scaffold
   - a `test/device-fixtures/<device>/` folder for replay captures
   - a PR body checklist that blocks review-ready device PRs until human validation is recorded
4. `PR Validation` builds the project on macOS and Windows, validates fixture manifests, and enforces the device PR checklist when a PR leaves draft.

## Labels

Automation manages these labels:

- `device-request`
- `device-bug`
- `ready-for-scaffold`
- `needs-docs`
- `needs-sample-data`
- `needs-repro`
- `needs-transport-triage`
- `transport:serial`
- `transport:ble`
- `transport:osc`
- `transport:camera`
- `transport:emulator`
- `transport:unknown`

## Maintainer Flow

1. A reporter opens either the device request or device bug issue form.
2. The triage workflow classifies transport and checks whether the issue has enough information.
3. Once the issue has docs, sample data, and a known transport, a maintainer comments `/scaffold-device`.
4. The scaffold workflow opens a draft PR with a compile-safe device interface stub and fixture placeholder files.
5. Follow-up commits fill in the real transport/parser work, replace placeholder captures, and document real hardware validation.
6. Move the PR out of draft only after the PR checklist is fully checked and the PR passes CI.

## Required GitHub Settings

Enable branch protection on the default branch with these settings:

- Require a pull request before merging.
- Require approval from at least one reviewer.
- Require review from Code Owners.
- Require conversation resolution before merging.
- Require status checks to pass before merging.
- Add these required checks:
  - `device-pr-gates`
  - `build (macos-latest)`
  - `build (windows-2019)`
- Restrict direct pushes to the protected branch.

The repository now includes `.github/CODEOWNERS` for the device and workflow paths, but GitHub only enforces it once branch protection enables code owner review.

## Fixture Layout

Each device fixture folder should contain:

- `manifest.json`
- one or more raw captures referenced by `manifest.json`
- optional notes in `README.md`

The fixture is intentionally transport-agnostic. The raw artifacts can be serial packets, BLE notifications, OSC messages, screenshots, or any other replayable input.

## Ideal End State

The current automation is deliberately conservative. The ideal long-term design is:

1. Keep the current issue-form and draft-PR workflow as the front door.
2. Add a small runtime device registry so transport routing and device aliases live in a single metadata source instead of scattered `if` branches.
3. Add parser/replay harnesses that can consume `test/device-fixtures/` captures without the physical device attached.
4. Add a GitHub App or webhook worker that can:
   - read the issue form
   - fetch linked docs
   - update the draft PR when new captures or logs are added
   - generate replay-test additions instead of only scaffolding files
5. Keep merge authority human-only:
   - no auto-merge for device-affecting PRs
   - code owner review required
   - real hardware smoke test required
   - axis/sign/recenter validation explicitly acknowledged in the PR body

## Future Registry Shape

If you want to make the repo more automation-friendly later, the best next code refactor is a single registry describing:

- device family id
- display name
- aliases / advertisement-name matches
- transport type
- parser class
- special connection hooks
- calibration / recenter notes
- replay fixture directory

That registry can power both runtime routing and automation scaffolding. At that point a bot only needs to modify one structured metadata source plus a device parser, instead of touching multiple transport-specific switch points.
