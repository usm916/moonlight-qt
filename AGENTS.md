# Custom development

Read `README.custom.md`, `docs/custom/mac-development.md`, and `docs/custom/compatibility.json` first.
Use `custom/main` as the integration branch and `codex/` for new task branches. Preserve existing work.

## Build and verification

- On macOS use Xcode 15+, a Qt 6.11+ macOS SDK, `python3 setup-deps.py`, qmake, and make.
  Use `cmake-build-macos-*` directories for new Mac builds. Follow the checked-in Mac handoff.
- On Windows use the Qt MSVC SDK and `scripts/build-custom.ps1`. Do not use Sunshine's MSYS2 toolchain.
- Keep the common-C submodule pinned to the commit shared with Sunshine. Do not update it with `--remote`.
- Run the common-C clipboard protocol test and the client lifecycle test for related changes. The lifecycle
  test uses assertions, so build it in debug mode. Verify Mac clipboard and streaming on a real Mac;
  Windows test success is not evidence of Mac runtime support.
- Use the existing code style and add/update tests for behavior changes. Document protocol changes in
  `moonlight-common-c/moonlight-common-c/docs/custom/clipboard-v1.md` and coordinate both endpoints.
- Do not commit personal settings, pairing data, private keys, logs, or generated SDK/build files.
