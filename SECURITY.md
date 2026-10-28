# Security notes

## API key

`PvZAI.ini` stores the DeepSeek API key in plain text next to the executable.
Do not commit it, share it, or include it in release archives. The repository
`.gitignore` excludes `PvZAI.ini`.

Prefer setting the key through the in-game Options dialog, or create
`PvZAI.ini` locally from `PvZAI.ini.example`.

If a key is exposed, revoke it in the DeepSeek console and create a new one.

## Game assets

`main.pak`, `properties/`, images, music, fonts and save files are not part of
this repository. Do not upload them to a public repository.

## Reporting

If you find a security issue in the AI integration, avoid posting real API
keys or personal save data in a public issue.
