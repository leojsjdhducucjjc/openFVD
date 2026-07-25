$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildDir = if ($env:OPENFVD_BUILD_DIR) {
    $env:OPENFVD_BUILD_DIR
} else {
    Join-Path $env:TEMP "openfvd-build-windows-x64"
}
$distDir = if ($env:OPENFVD_DIST_DIR) {
    $env:OPENFVD_DIST_DIR
} else {
    Join-Path $projectRoot "dist"
}

if (-not $env:OPENFVD_DEPS_ROOT) {
    throw "OPENFVD_DEPS_ROOT must point to a dependency prefix containing include/, lib/, and bin/."
}

$dependencyRoot = (Resolve-Path $env:OPENFVD_DEPS_ROOT).Path
$requiredCommands = @("qmake", "nmake", "windeployqt")
foreach ($command in $requiredCommands) {
    if (-not (Get-Command $command -ErrorAction SilentlyContinue)) {
        throw "$command is required and must be available on PATH."
    }
}

$glewDll = Join-Path $dependencyRoot "bin\glew32.dll"
if (-not (Test-Path $glewDll)) {
    throw "Missing GLEW runtime: $glewDll"
}

New-Item -ItemType Directory -Force -Path $buildDir, $distDir | Out-Null

Push-Location $buildDir
try {
    & qmake (Join-Path $projectRoot "fvd.pro") "CONFIG+=release"
    if ($LASTEXITCODE -ne 0) { throw "qmake failed with exit code $LASTEXITCODE." }

    & nmake /NOLOGO
    if ($LASTEXITCODE -ne 0) { throw "nmake failed with exit code $LASTEXITCODE." }

    $releaseDir = Join-Path $buildDir "release"
    $executable = Join-Path $releaseDir "FVD.exe"
    if (-not (Test-Path $executable)) {
        throw "Expected application executable was not produced: $executable"
    }

    & windeployqt --release --no-translations --compiler-runtime $executable
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed with exit code $LASTEXITCODE." }

    Copy-Item $glewDll $releaseDir -Force

    $packageDir = Join-Path $distDir "FVD-Windows-x64"
    if (Test-Path $packageDir) {
        Remove-Item $packageDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $packageDir | Out-Null
    Copy-Item (Join-Path $releaseDir "*") $packageDir -Recurse -Force
    Copy-Item (Join-Path $projectRoot "LICENSE") $packageDir -Force
    Copy-Item (Join-Path $projectRoot "README.md") $packageDir -Force

    $requiredRuntimeFiles = @(
        "FVD.exe",
        "glew32.dll",
        "Qt5Core.dll",
        "Qt5Gui.dll",
        "Qt5Widgets.dll",
        "platforms\qwindows.dll"
    )
    foreach ($relativePath in $requiredRuntimeFiles) {
        $runtimePath = Join-Path $packageDir $relativePath
        if (-not (Test-Path $runtimePath)) {
            throw "Packaged application is missing required runtime file: $relativePath"
        }
    }

    $artifact = Join-Path $distDir "FVD-Windows-x64.zip"
    if (Test-Path $artifact) {
        Remove-Item $artifact -Force
    }
    Compress-Archive -Path (Join-Path $packageDir "*") -DestinationPath $artifact -CompressionLevel Optimal
    Write-Host "Built $artifact"
}
finally {
    Pop-Location
}
