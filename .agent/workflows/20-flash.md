# Flash Firmware (Approval-gated)

Pre-check:
- Identify port (macOS: /dev/cu.usbmodemXXX, Windows: COM#)
- Propose upload command but do not run it.

Commands to propose:
- `pio run -t upload`
- or `pio run -t upload --upload-port <port>`

After flash:
- Provide monitor command: `pio device monitor --baud 115200`
