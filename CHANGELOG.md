# Changelog

All notable changes to `ossim` are documented here.

## [Unreleased]

> **TL;DR:** The combined workspace now gets the registration-source bridge
> from `ossim-autoreg`, reducing the optional registration build to one sibling
> dependency. RPC generation also gets a steadier solver, layered-height
> fitting controls, and a way to compare fitted coefficients against existing
> RPC inputs without quietly copying them. The RPC generator can now fit a
> height slab around the scene automatically, reducing the need for downstream
> users to reproduce the exact same elevation database, and the solver now
> treats invalid samples and unstable intermediate fits as recoverable input
> problems instead of inviting NaNs to the party. PPJ frame sensors also get
> complete adjustable definitions and a local camera orientation adjustment so
> registration has real roll/pitch/yaw controls instead of leaning on offsets
> alone. The generated OSSIM config header also now uses standard fixed-width
> integer types without the old platform size-check dance, and CMake no longer
> runs the unused probes that fed those checks.

### Added
- Add `ossim-rpcgen` controls for force-fitting existing RPC inputs, sampling
  layered height planes, and limiting fit refinement by iteration count or
  minimum max-residual improvement (561862d0).
- Add radius-only auto layered RPC fitting that estimates nominal scene height,
  derives a height-layer delta from sampled elevation variation, and caps it by
  image height sensitivity (f86f398d).
- Add experimental `weighted-svd`, `lm`, and `lm-huber` coefficient optimizer
  selection for comparing linearized and nonlinear RPC fits (f86f398d).

### Changed
- Use standard fixed-width integer typedefs in `ossimConfig.h.in` for both C
  and C++ consumers, removing generated `SIZE_OF_*` checks and the unused
  `ossim_float128` typedef (428261de).
- Remove the obsolete CMake `CheckTypeSize` setup and generated alias-header
  plumbing that previously fed the `ossimConfig.h.in` size checks (cbcc3b11).
- Define all PPJ adjustable parameters from one shared definition path, including
  200-meter lon/lat offsets, 0.1-degree roll/pitch/yaw offsets, altitude, and
  focal length adjustment defaults.
- Stop adding a separate `ossim-registration-source` repository from the OSSIM
  workspace build; `BUILD_OSSIM_REGISTRATION_SOURCE` now adds only
  `ossim-autoreg`, which owns the registration bridge target (69ec165c).
- Remove `OSSIM_REGISTRATION_SOURCE_ROOT` handling from the workspace configure
  script and CMake summary (69ec165c).
- Improve `ossimRpcSolver` coefficient fitting with weighted SVD,
  denominator damping, stricter max-pixel residual convergence, layered height
  observations, input guards, and Doxygen usage notes (561862d0).
- Document elevation-aware layered RPC fitting as the normal generation path
  when elevation is available, with `--disable-elev` reserved for diagnostics
  and flat comparisons (f86f398d).

### Fixed
- Apply PPJ roll, pitch, and yaw adjustable offsets in local camera space for
  both imaging rays and world-to-image projection.
- Harden `ossimRpcSolver` against degenerate bounds, non-finite elevation
  samples, invalid rational-fit residuals, and radius-only layer requests that
  cannot produce a valid positive height delta (d7962caa).

## [2026-05-01]

> **TL;DR:** The combined OSSIM build can now opt into the legacy
> `ossim-planet-gui` target and resolve the planet support libraries from the
> active workspace instead of getting lost in stale cache paths.

### Added
- Add `BUILD_OSSIM_PLANET_GUI` as the common build option for the legacy
  planet Qt GUI.

### Changed
- Teach GPSTK discovery to search `GPSTK_ROOT`, the workspace
  `3rdparty/gpstk` tree, install prefixes, and common system prefixes.
- Prefer the in-tree `ossim-wms` target when resolving WMS support so
  `ossim-planet` links the actual library instead of a build directory.
- Print `BUILD_OSSIM_PLANET_GUI` in the CMake build summary.

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
