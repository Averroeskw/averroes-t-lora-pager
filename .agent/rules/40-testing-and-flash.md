# Build/Flash Discipline

Before any flash request:
- Must run a build-only step (pio run) or show how to run it.
- Must provide the exact upload command and how to set the port.

Flashing guidance must match README:
- `pio run -t upload`
- Port override: `pio run -t upload --upload-port <port>`
- Monitor: `pio device monitor --baud 115200`
