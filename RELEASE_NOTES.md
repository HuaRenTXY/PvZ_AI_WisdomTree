# PvZ AI Wisdom Tree v0.1.0

Source-only release of the PvZ-Portable fork that adds a DeepSeek-backed Wisdom Tree.

## Highlights

- Fertilize the Zen Garden Wisdom Tree to open a question dialog.
- Pressing OK immediately shows `思考中…` in the tree's speech bubble.
- The DeepSeek reply replaces the tree speech and stays open.
- DeepSeek API key field in the in-game Options dialog.
- Missing or invalid API key shows a warning dialog.
- Chinese text via GDI with `PvZ 2 Regular` and Microsoft YaHei fallback.
- Original PvZ-Portable main menu, loading screen, achievements screen and save system.
- Default windowed 800×600.
- Built-in perfect save for testing.

## Requirements

- Windows (the AI part uses WinHTTP and GDI).
- Your own legally purchased Plants vs. Zombies: GOTY data:
  - `main.pak`
  - `properties/`
- MSYS2 MinGW64 with SDL2, libjpeg-turbo, libpng, libopenmpt, CMake and Ninja to build.

## Build

See `BUILDING-WINDOWS.md`.

## Assets and licensing

This release contains **source code only**. No `main.pak`, `properties/`, images, music, fonts or save files are included.

The code is licensed under **LGPL-3.0-or-later**. See `LICENSE`, `COPYING`, `NOTICE.md` and `AI_CHANGES.md`.

Plants vs. Zombies, PopCap and Electronic Arts are trademarks of their respective owners. This project is not affiliated with or endorsed by them.

## Notes

- `PvZAI.ini` stores the API key in plain text locally; never commit it.
- The AI part is Windows-only; upstream PvZ-Portable remains cross-platform.
