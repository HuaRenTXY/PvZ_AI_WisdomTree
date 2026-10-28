# Building on Windows

Tested with MSYS2 MinGW64 (GCC 15, CMake 4.2, Ninja) on Windows 11 x64.

## 1. Install MSYS2 packages

Using the MSYS2 MinGW64 shell (or `pacman.exe` directly):

```bash
pacman -S --needed \
  base-devel \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-SDL2 \
  mingw-w64-x86_64-libjpeg-turbo \
  mingw-w64-x86_64-libpng \
  mingw-w64-x86_64-libopenmpt
```

## 2. Configure and build

From the repository root, using Windows paths:

```powershell
D:\msys64\mingw64\bin\cmake.exe -G Ninja -S . -B build `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=D:/msys64/mingw64 `
  -DCMAKE_MAKE_PROGRAM=D:/msys64/mingw64/bin/ninja.exe `
  -DPVZ_DEBUG=OFF

D:\msys64\mingw64\bin\cmake.exe --build build -j 8
```

The executable is `build/pvz-portable.exe`.

## 3. Run

Put the executable beside your own legally purchased game data:

```text
pvz-portable.exe
main.pak
properties\
PvZAI.ini          # optional, can be created in Options
```

## Notes

- The build links `winhttp` for the DeepSeek client.
- `PvZAI.ini` is read from the executable directory.
- AI text uses GDI fonts; `PvZ 2 Regular` is used when present, otherwise
  `Microsoft YaHei`.
- No game assets are part of this repository.
