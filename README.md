# FVD++

## Downloads

[![Download for Windows](https://img.shields.io/badge/Download-Windows%20x64-0078D4?logo=windows)](https://github.com/leojsjdhducucjjc/openFVD/releases/latest/download/FVD-Windows-x64.zip)
[![Download for Apple Silicon](https://img.shields.io/badge/Download-macOS%20Apple%20Silicon-111111?logo=apple)](https://github.com/leojsjdhducucjjc/openFVD/releases/latest/download/FVD-Apple-Silicon.zip)

Both downloads are self-contained and include the Qt and graphics runtime
libraries needed to launch FVD++. The Windows executable is not Authenticode
signed and the macOS app is not Apple-notarized, so SmartScreen or Gatekeeper
may ask you to confirm that you trust the app.
Downloads and release notes are also available on the
[latest GitHub release](https://github.com/leojsjdhducucjjc/openFVD/releases/latest).

GitHub Actions builds both packages on every push and pull request. Development
artifacts are available from the successful
[Build Downloads workflow](https://github.com/leojsjdhducucjjc/openFVD/actions/workflows/downloads.yml)
for 30 days.

Pushing a version tag such as `v0.80` publishes both ZIP files, generated release
notes, and SHA-256 checksums to GitHub Releases:

```bash
git tag v0.80
git push origin v0.80
```

## Build for Apple Silicon macOS

The current source builds as a native `arm64` application for Apple Silicon
Macs. This avoids Rosetta translating the old Intel-only app and its Qt/lib3ds
libraries.

Requirements:

```bash
brew install qt@5 glm
```

Clone with dependencies and build:

```bash
git clone --recurse-submodules https://github.com/leojsjdhducucjjc/openFVD.git
cd openFVD
./scripts/build-macos-arm64.sh
```

The packaged app is written to `dist/FVD-Apple-Silicon.zip`. The build targets
macOS 12 or newer and is ad-hoc signed for local use.

## Build for Windows

The automated Windows build targets 64-bit Windows with Qt 5.15.2 and MSVC.
It installs GLM and GLEW through vcpkg, builds vendored lib3ds directly into the
application, and runs `windeployqt` before creating
`dist/FVD-Windows-x64.zip`.

From a Visual Studio x64 developer PowerShell with Qt's `bin` directory on
`PATH`, install the dependencies and run:

```powershell
vcpkg install glm:x64-windows glew:x64-windows
$env:OPENFVD_DEPS_ROOT = "$env:VCPKG_INSTALLATION_ROOT\installed\x64-windows"
.\scripts\build-windows-x64.ps1
```

Saving now writes a temporary file beside the project and atomically replaces
the old file only after serialization succeeds. This keeps the previous save
intact if a write fails or the app stops during saving.

Preferences are stored in the user's macOS config directory instead of inside
the signed app bundle, so they remain writable after moving FVD to Applications.

## Regression tests

The regression target exercises the production project loader and saver against
the committed `v0.30` and `v0.77` fixtures, then saves and reloads a populated
track through the same code path used by the application.

```bash
test_build="${TMPDIR:-/tmp}/openfvd-regression-tests"
mkdir -p "$test_build"
cd "$test_build"
/opt/homebrew/bin/qmake /path/to/openFVD/tests/tests.pro CONFIG+=release
make -j"$(sysctl -n hw.logicalcpu)"
./fvd_regression_tests
```

Before merging coaster editor or track-builder changes, also run the checklist in
`tests/MANUAL_COASTER_REGRESSION.md`.

Development of the direct, Planet Coaster-style track editing workflow is
tracked in `docs/TRACK_BUILDER_ROADMAP.md`. It is designed to coexist with the
existing advanced force and geometry section editors.

####################
# open FVD++ v0.79 #
####################

Stephan (Lenny) Alt, 2012-2016, alt.stephan@web.de

#########
# Intro #
#########

Welcome to FVD++, a next generation FVD tool inspired by newton2 (by Keith (Entropy) Cohn).
It is mainly aimed for coaster enthusiasts creating sophisticated force and geometry based coasters.
That said the tool is still development and I take no warranty of its usefulness in its current state (see License information below).

#############
# Changelog #
#############

v0.79:
- added nolimits 2 csv import
- fixed renderer bugs when importing nl1 track or nl2 csv files on osx
- fixed compilation errors on clang
- reverted performance improvements due to fatal bugs

v0.78:
- fixed export of straight sections with non zero roll transitions
- fixed camera stuttering
- fixed distance based forced sections
- first 64bit Release

v0.77 (open Source release):
- fixed export of individual sections
- fixed force based sections not saving constant speed
- changed to Qt 5.2.1, qcustomplot 1.2.1 and glm 0.9.5.1-1

v0.76 Beta:
- fixed tozero transitions to only work on the first section properly
- fixed velocity problems on some section types
- fixed NL2 exporter exporting non rotated sections (when exporting a subset of FVD sections)

v0.75 Beta:
- double spine track style
- new exporter type: NL2 exporter
- NL2 style friction parameters
- EXPERIMENTAL: export coaster to .3ds file format
- EXPERIMENTAL: "roll back to zero" transition type for force and geometric time based sections

v0.7b Beta:
- fixed having no control over the camera in OpenGL 2 mode
- FVD now loads a double-clicked project on startup if the .fvd file format is asociated with the FVD++ executable
- fixed camera having gimbal lock related issues

v0.7a Beta:
- fixed unsufficient mesh update in combination with smoothing
- fixed spine and rails disappearing at the end of the track on rare occasions

v0.7 Beta:
- added fully rendered tracks as well as several track styles
- improved drag and speed calculations
- new keybindings during fly mode (change POV position by pressing the arrow keys, B toggles the NoLimits 1 building border, F goes into fullscreen)
- several graphics options
- straight segments are now exported as one NoLimits 1 segment (please tell me if you really need the old system back)
- performance increase in track generation
- imported NoLimits 1 tracks have now simulated physics
- transition editor with mutliple y-axes

v0.6 Beta:
- added a rollspeed smoothing tool
- improved both tangent and spline exporter by a significant margin
- improved coaster physics (you can load 0.5x .fvd files but your track may be misshapen due to the new physics engine)

v0.51 Beta:
- fixed two bugs leading to crashes on most Mac and some Windows systems
- minor bug fix in the options menu and section list

v0.5 Beta:
- all new GUI
    - project tab lets you manage multiple tracks in one project
    - each track has its own management tab
    - improved graph window with tons more options
- import tracks from other FVD++ projects
- import from *.nltrack format (for now FVD++ assumes a closed track and calculates no physics)
- customizable background and track colors
- customizable ground texture (only viewable in OpenGL 3 mode)
- additional anchor point options
- improved track flexion visualization

v0.31 Beta:
- better Mac compatibility (enabled earlier OpenGL versions and removed some external dependencies)
- fixed issue with symmetric functions

v0.3 Beta:
- improved visuals for non legacy OpenGL (improved shadows and selected subsection gets a green highlight)
- added undo / redo support
- added options window
- added conversion panel window
- added roll orientation option
- added function argument option
- fixed spinning camera bug which occured on some systems
- reworked tangent exporter (new standard exporter for now)
- new Mac version (now also with OpenGL 3 support)

v0.14a Alpha:
- fixed floor in PoV mode (legacy OpenGL only)

v0.14 Alpha:
- trackwork now invariant under position and rotation changes of the anchor point
- improved visuals for non legacy OpenGL (underground track distinguishable)
- Anchor Node represents now the anchor of the track rather than the anchor of the Heartline

v0.12 Alpha:
- Mac version (64 bit only)
- new exporter option (export as spline or export with preserving tangents)
- fixed a slight bug in straight sections using non zero roll functions (section didn't preserve length of heartline)

v0.11 Alpha:
- new exporter option (threshold angle at which the exporter decides to switch to RelRoll option)
- Compatibility with OpenGL version 3.0 and lower. Visualizations will automatically be disabled! Check *.log file which OpenGL context could be created on your machine
- Fixed a bug on functions the train will never be able to reach

v0.1 Alpha:
- introduction of 4 different section types (straight, curved, force based, geometry based)
- introduction of 6 different transition types (with 12 different flavors)
- timewarping of all transition types
- Exporter to the *.nlelem format with per section exporting and variable segment lengths
- save to and load from a lightweight data format (*.fvd files) with autosaves to a (*.bak) file each time an export is performed.
- OpenGL shaders visualising important data (like forces, roll speed and track flexion).
- PoV mode enabling the ability to check the track inside FVD++


###################
# Having Trouble? #
###################

#### I can't use visualizations ####
- The visualizations use OpenGL 3.1 shader technology, make sure your graphics card / drivers support OpenGL 3.1


#### I don't know how to do XY ####
- read through the official documentation, this document should explain nearly every feature of FVD++
- Visit the FVD++ Thread on NoLimitsExchange (http://forum.nolimits-exchange.com/)


#######################
# Found a Bug / Crash #
#######################

Please let me know if you found a bug / crash either by posting in the NLE forum or by mailing me (alt.stephan@web.de).
Please include as much information as possible, try to save the track that bugged out and send me the saved .fvd file.
If the program crashed try to recreate the crash scenario and describe how you got the program to crash in a detailed way.


#############
# Thanks to #
#############

- Lucas van den Bosch for creating the official documentation and youtube tutorial channel
- Ercan "geforcefan" Akyürek for deploying and testing the tool on Mac systems
 

#######################
# License information #
#######################

FVD++, an advanced coaster design tool for NoLimits and NoLimits2
Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
