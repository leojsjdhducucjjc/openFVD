# Legacy project fixtures

The `*.fvd.b64` files are base64-encoded binary FVD++ projects. Keeping the
binary payload encoded makes changes reviewable and prevents line-ending or
text-encoding tools from modifying the fixtures.

Both fixtures contain the embedded default ground texture and one populated
track. The payloads are intentionally equivalent apart from their version so
both complete track-reader paths are exercised:

- `v0.30-one-track.fvd.b64` exercises the legacy project and track readers.
- `v0.77-one-track.fvd.b64` exercises the current pre-scenery readers.

The integration tests decode each fixture into a temporary `.fvd` file before
passing it to the production `saver` and `projectWidget` load path.
