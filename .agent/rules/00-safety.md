# Safety Rules (T-LoRa Pager)

## Never do (hard stop)
- Never run destructive commands (rm -rf, disk/partition tools, format, dd, mkfs).
- Never flash firmware unless I explicitly approve the exact upload command and port.
- Never change pins, I2C addresses, SPI buses, or baud rates unless you cite the source file in this repo.

## Terminal approval
- Any terminal command must be proposed first with:
  1) purpose
  2) exact command
  3) expected output
  4) rollback/undo (if relevant)

## Change size rule
If a change touches >5 files or >200 lines:
- Produce a plan artifact and request approval before editing.
