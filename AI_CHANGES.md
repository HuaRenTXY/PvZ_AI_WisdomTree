# AI modification summary

This is a fork of PvZ-Portable. Changes relative to upstream:

## New files

```text
src/Lawn/System/AIClient.cpp
src/Lawn/System/AIClient.h
src/Lawn/System/AIUtil.cpp
src/Lawn/System/AIUtil.h
src/Lawn/System/AIFont.cpp
src/Lawn/System/AIFont.h
src/Lawn/Widget/AITreeDialog.cpp
src/Lawn/Widget/AITreeDialog.h
src/Lawn/Widget/AITreeReplyDialog.cpp   (currently unused; kept for reference)
src/Lawn/Widget/AITreeReplyDialog.h
```

## Modified files

- `CMakeLists.txt`
  - link `winhttp` on Windows.
- `src/ConstEnums.h`
  - add `DIALOG_AI_PROMPT`, `DIALOG_AI_WARNING`, `DIALOG_AI_REPLY`.
- `src/LawnApp.h` / `src/LawnApp.cpp`
  - AI request state, `ShowAITreePrompt`, `BeginAITreeRequest`, `UpdateAITree`.
  - perfect-save initialization when profile is below level 45.
  - `F9` prompt, `F10` API key quick dialog.
- `src/Lawn/Challenge.h` / `src/Lawn/Challenge.cpp`
  - `mAITreeText`; tree bubble displays AI text instead of localized speech.
  - `TreeOfWisdomFertilize` opens the AI prompt.
- `src/Lawn/Widget/NewOptionsDialog.h` / `.cpp`
  - `DeepSeek API Key` edit box in the Options dialog.
- `src/SexyAppFramework/SexyAppBase.cpp`
  - default windowed launch behavior.
- `src/SexyAppFramework/SexyApp.cpp`
  - default windowed regardless of `partner.xml`.

## Licensing

All additions are intended to be distributed under
**LGPL-3.0-or-later**, matching PvZ-Portable. No PopCap/EA game assets are
included. The `PvZ 2 Regular` font is not bundled; it is loaded from the
user's system if present.
