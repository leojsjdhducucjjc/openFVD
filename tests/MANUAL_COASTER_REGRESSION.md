# Manual coaster editor regression checklist

Run this checklist before merging each track-builder milestone. Use a disposable
project and keep one known-good `v0.77` project available for comparison.

## Project lifecycle

- Create a new project and confirm one empty track appears.
- Save the project, close it, reopen it, and confirm the track is present.
- Use Save As and confirm the new file opens independently.
- Open a `v0.30` project and confirm the version-conversion warning appears.
- Open a `v0.77` project and confirm it loads without a version warning.
- Wait for or invoke autosave and confirm the `.bak` file can be loaded.
- Cancel Open and Save dialogs and confirm editing/rendering resumes.

## Track editing

- Add, rename, hide/show, edit, and delete a track.
- Change anchor position, pitch, yaw, roll, speed, heartline, and friction.
- Add each supported section type: straight, curved, forced, geometric,
  Bezier, and NoLimits CSV where a source fixture is available.
- Edit section length and transition functions in the graph panel.
- Apply and remove roll smoothing.
- Undo and redo representative anchor, section, and transition changes.
- Close and reopen a track editor tab without losing its state.

## Viewport

- Hold the right mouse button and drag to look around; releasing it must stop camera rotation.
- Move with W/A/S/D, change speed with Shift/Control, and use the mouse wheel.
- Enter and leave coaster POV mode and adjust its lateral/vertical position.
- Toggle visualization shaders, grid, building border, and fullscreen.
- Confirm rails, cross-ties, supports, heartline, floor, sky, and shadows render.

## Import and export

- Import a track from another FVD project.
- Import a known-good NoLimits track or CSV when fixtures are available.
- Export a normal track file and confirm the exporter completes.
- Export a 3DS model and confirm a non-empty file is produced.

## Failure handling

- Attempt to load a non-FVD file and confirm a useful error appears.
- Attempt to load a truncated copy of a project and confirm the app stays open.
- Attempt to save into a non-writable or missing directory and confirm the
  previous project file remains intact.

Record the application commit, macOS version, renderer mode, and any failed
step with exact reproduction details.
