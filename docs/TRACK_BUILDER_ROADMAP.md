# Planet Coaster-style track builder roadmap

The track builder is a second editing workflow alongside FVD++'s existing
force, geometric, straight, and curved section editors. It must feed the same
`mnode` pipeline so track rendering, force visualization, POV, smoothing, and
NoLimits export continue to work.

## Builder model

One builder section represents one directly manipulated track piece. Its end
position and direction are stored relative to the section's entry pose. This
keeps it connected when an earlier advanced or builder section changes.

Each piece has:

- A stable ID for selection and undo commands.
- An entry-relative end offset.
- An entry-relative end direction.
- Independent incoming and outgoing tangent lengths.
- An end bank angle with smooth interpolation from the entry bank.

The existing imported `secbezier` representation is not used as the editable
domain model. It assumes an imported, absolute control-point collection and
does not reliably follow changes to preceding mixed section types.

## Incremental milestones

1. **Curve domain (complete):** deterministic builder-segment data and sampling
   tests.
2. **Builder section (complete):** a new `section` implementation that samples builder
   curves into normal `mnode` data and can coexist with advanced sections.
3. **Persistence (complete):** a versioned builder payload with legacy `v0.30`/`v0.77`
   loading retained and migration tests for known vector serialization issues.
4. **Basic builder UI (complete):** add-piece, ghost next piece, numeric
   length/elevation/direction/bank controls, cancel/commit behavior, and exact
   append undo/redo restoration.
5. **Viewport editing (complete):** labeled length, elevation, direction, and
   bank handles with live values, plus hold-to-drag right-click camera
   navigation and cached preview rendering.
6. **Editing commands:** move, reshape, split, duplicate, and delete with
   mergeable undo/redo transactions.
7. **Snapping and presets:** angle, height, length, and connection snapping;
   straight, turn, hill, drop, inversion, brake, launch, and station presets.
8. **Polish and scale:** multi-piece selection, profiling, validation warnings,
   renderer overlays, documentation, and large-track regression coverage.

Every milestone must pass the automated legacy save/load suite and the manual
coaster editor checklist before the next milestone begins.
