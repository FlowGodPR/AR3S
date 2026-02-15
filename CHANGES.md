# Removed per user request

All test files and test helpers were removed or disabled per request. No automated tests remain in the main build.
  - Added debug logging for drag/resize candidate registration/promotion and layout saves.

- Satellite Plugin
  - Verified `AR3S Satellite` target is buildable and the AU was built & installed to `~/Library/Audio/Plug-Ins/Components/AR3S Satellite.component`.

- Tests & Debugging
  - Debug logging added for drag/resize candidate registration/promotion and layout saves.

How to test quickly (recommended):

1) Build & install (already done during this session):

   cd "/Users/untitled folder/plugins/build"
   cmake --build . --target SimpleGain_AU --config Release
   cd ".."
   sh install.sh

2) Restart / reload your DAW and load the plugin `AR3S` and `AR3S Satellite`.

3) Try moving/resizing components:
   - Hold Cmd/Ctrl and drag any component to move it immediately.
   - Hold Alt and drag the bottom-right corner to resize immediately.
   - Or simply click and drag (no modifier) and move 6px to promote to drag/resize.
   - Layout is auto-saved on mouseUp; you can also press `Save Layout` or `Load Layout` in the Settings tab.

Notes / Limitations:
- There is no git repository in this workspace, so I couldn't create a backup branch; please create one locally if you want a restore point.
- Host-side verification (DAW-specific rendering/idiosyncrasies) remains recommended — please reload plugin in your DAW and report exact failures if anything still misbehaves.

If you want, I'll continue adding automated tests (headless GUI harness) and further hardening (edge-case fixes, accessibility, keyboard move/resize), then produce a formal PR and changelog entry.

-- Automated changes made by GitHub Copilot (developer helper)