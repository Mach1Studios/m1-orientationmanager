# Device Fixtures

Device-affecting PRs should add or update a fixture folder here so parser changes can be reproduced without the physical hardware.

Expected layout:

- `test/device-fixtures/<device-slug>/manifest.json`
- one or more raw captures referenced by `manifest.json`
- optional `README.md` notes

Use `_template/` as the starting point for new fixtures.
