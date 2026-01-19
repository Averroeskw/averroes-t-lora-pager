# LVGL / UI Rules

- Any UI change must be:
  1) described as a user-visible behavior change
  2) accompanied by a control mapping update (keyboard/encoder shortcuts)
  3) tested with an on-device checklist step (page switching, encoder scroll, status bar icons)

- Keep UI split:
  - UI composition in src/ui/ui.c
  - declarations in src/ui/ui.h
  - device services remain outside UI where possible
