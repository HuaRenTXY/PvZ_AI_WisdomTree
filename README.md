# PvZ AI Wisdom Tree (PvZ-Portable fork)

A PvZ-Portable fork that adds a DeepSeek-backed Wisdom Tree to Plants vs. Zombies GOTY.

> **Platform note:** the AI integration is currently Windows-only because it
> uses WinHTTP and GDI fonts. The upstream PvZ-Portable code remains
> cross-platform; building this fork on Linux/macOS would require replacing
> those two components.

This repository contains **source code only**. It does **not** contain
`main.pak`, `properties/`, images, music, fonts, save files, or any other
PopCap/EA game assets. You must supply those from a legally purchased copy of
Plants vs. Zombies: Game of the Year Edition.

## Features

- Original PvZ-Portable main menu, loading screen, achievements screen and save system.
- Windowed 800x600 by default.
- Fertilizing the Zen Garden Wisdom Tree opens a question dialog.
- After pressing OK the tree immediately shows `思考中…` in its speech bubble.
- The DeepSeek reply replaces the tree speech and is not auto-closed.
- Chinese rendering uses `PvZ 2 Regular` when installed, with a Microsoft YaHei fallback.
- DeepSeek API key field is in the in-game Options dialog and is written to `PvZAI.ini`.
- Missing/invalid API key shows a warning dialog.
- Built-in perfect save when the existing profile is below Adventure level 45.

## Controls

| Action | Key / UI |
| --- | --- |
| Ask the tree | Fertilize the Wisdom Tree in Zen Garden |
| Quick prompt | `F9` |
| API key quick prompt | `F10` |
| Submit | `Enter` or `OK` |
| Set API key normally | Options → `DeepSeek API Key` |

## Build

See [BUILDING-WINDOWS.md](BUILDING-WINDOWS.md).

The short version on Windows with MSYS2 MinGW64:

```bash
pacman -S --needed \
  mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libjpeg-turbo \
  mingw-w64-x86_64-libpng mingw-w64-x86_64-libopenmpt \
  mingw-w64-x86_64-ninja

cmake -G Ninja -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=D:/msys64/mingw64 \
  -DCMAKE_MAKE_PROGRAM=D:/msys64/mingw64/bin/ninja.exe
cmake --build build -j 8
```

Copy `build/pvz-portable.exe` next to your own `main.pak` and `properties/`.

## Assets

Place your own legally obtained files next to the executable:

```text
pvz-portable.exe
main.pak
properties\
PvZAI.ini          # optional; can be created from the game Options dialog
```

The code also accepts an extracted resource directory via `-resdir`.

## API key

`PvZAI.ini`:

```ini
api_key=sk-your-key
base_url=https://api.deepseek.com
model=deepseek-chat
```

Do not commit `PvZAI.ini`; it is ignored by `.gitignore`.

## License

The PvZ-Portable code in this repository is licensed under the
**GNU Lesser General Public License v3.0 or later (LGPL-3.0-or-later)**.

- `LICENSE` — LGPL-3.0 text
- `COPYING` — GPL-3.0 text referenced by LGPL-3.0
- Original PvZ-Portable: https://github.com/wszqkzqk/PvZ-Portable

The AI additions are released under the same LGPL-3.0-or-later terms unless a
file states otherwise. Retain the original copyright notices.

Plants vs. Zombies, PopCap and Electronic Arts are trademarks of their
respective owners. This project is not affiliated with or endorsed by them.
No game assets are distributed here.

## Credits

- PvZ-Portable by Zhou Qiankang and contributors.
- PvZ AI Wisdom Tree modifications in this fork.
