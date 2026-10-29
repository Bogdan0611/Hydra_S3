# Contributing

How work is organized in this repository: branches, commits, pull requests
and releases. The goal is a history where every commit builds and every
stage can be traced from plan to release.

## Branches

- **`main`** always builds and contains only finished, merged work.
- **One branch per roadmap stage:** `stage/<number>-<name>`, e.g.
  `stage/0-foundation`, `stage/2-subghz`. The full list is in
  [docs/roadmap.md](docs/roadmap.md).
- **Fixes outside a stage:** `fix/<short-description>`, branched from
  `main`.
- Branches are deleted after merging.

## Commits

[Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <summary>

<body: what changed and why, wrapped at 72 characters>
```

- **Types:** `feat`, `fix`, `docs`, `refactor`, `test`, `build`, `ci`,
  `chore`.
- **Scopes:** `core`, `display`, `input`, `storage`, `buzzer`, `menu`,
  `subghz`, `nfc`, `ir`, `ble`, `wifi`, `nrf24`, `usb-hid`, `hw`.
- **Summary:** imperative mood, lower case, no trailing period, at most
  72 characters.
- **Body:** explain *why* when it is not obvious. State the test status,
  e.g. "Tested on hardware: OLED shows the menu" or "Untested on
  hardware".
- **Every commit builds** (`pio run`). Small, focused commits; one logical
  change each.

Examples:

```
feat(input): add edge detection with per-button debounce
fix(display): restore draw color after inverted text
docs(hw): document AP2112K startup test for bulk capacitance
build: pin pioarduino platform to 55.03.39
```

## Pull requests

Every stage branch is merged into `main` through a pull request, using
**"Create a merge commit"** so the individual commits stay visible.

PR checklist:

- [ ] CI build is green.
- [ ] Tested on hardware; the PR description says what was tested and how.
- [ ] Every "Done when" item of the stage is checked in
      [docs/roadmap.md](docs/roadmap.md).
- [ ] Docs updated where behavior or hardware changed.
- [ ] `CHANGELOG.md` updated.

Recommended GitHub setting: protect `main` and require the CI check to
pass before merging.

## Releases

- [Semantic Versioning](https://semver.org/). Before 1.0, each finished
  stage bumps the minor version (Stage 0 → `v0.1.0`, Stage 1 → `v0.2.0`,
  ...).
- Annotated tags on `main` after the merge:

  ```bash
  git tag -a v0.1.0 -m "Stage 0: foundation"
  git push origin v0.1.0
  ```

- A GitHub Release is created from each tag, with the matching
  `CHANGELOG.md` section as its notes.

## Code style

- C++ formatting follows `.clang-format` (format on save in VS Code).
- Naming, comments and error handling: concise, professional, English.
  Comments explain *why* (hardware quirks, register values, timing), not
  what the code already says.
