# Changelog

All notable changes to `ossim` are documented here.

## [Unreleased]

> **TL;DR:** The combined workspace now gets the registration-source bridge
> from `ossim-autoreg`, reducing the optional registration build to one sibling
> dependency.

### Changed
- Stop adding a separate `ossim-registration-source` repository from the OSSIM
  workspace build; `BUILD_OSSIM_REGISTRATION_SOURCE` now adds only
  `ossim-autoreg`, which owns the registration bridge target (69ec165c).
- Remove `OSSIM_REGISTRATION_SOURCE_ROOT` handling from the workspace configure
  script and CMake summary (69ec165c).

## [2026-04-27]

> **TL;DR:** OSSIM can now optionally include the standalone registration-source
> bridge and its autoreg dependency from the combined workspace build, keeping
> the core repo out of the registration implementation while still letting
> geocell link it when requested.

### Added
- Add `BUILD_OSSIM_REGISTRATION_SOURCE` to the common CMake settings and
  workspace configure script so the optional registration bridge can be enabled
  from the existing OSSIM build flow (b344c9a9).
- Add sibling-root discovery for `ossim-autoreg` and
  `ossim-registration-source`, include those projects before `ossim-gui`, and
  print the registration build settings in the CMake summary (b344c9a9).
