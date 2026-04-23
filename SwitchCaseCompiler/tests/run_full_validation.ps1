param(
    [string]$Configuration = "Release",
    [string]$QtDeployPath = "",
    [switch]$SkipPackage
)

$ErrorActionPreference = "Stop"

function Get-QtDeployPath {
    param([string]$CachePath)

    if (!(Test-Path $CachePath)) {
        return $null
    }

    $qtPrefixLine = Select-String -Path $CachePath -Pattern '^CMAKE_PREFIX_PATH:.*=' -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $qtPrefixLine) {
        return $null
    }

    $prefix = ($qtPrefixLine.Line -split '=', 2)[1]
    if ([string]::IsNullOrWhiteSpace($prefix)) {
        return $null
    }

    $candidate = Join-Path $prefix 'bin\windeployqt.exe'
    if (Test-Path $candidate) {
        return $candidate
    }

    return $null
}

Write-Host "[1/5] Building project ($Configuration)..." -ForegroundColor Cyan
cmake --build build --config $Configuration
if ($LASTEXITCODE -ne 0) {
    throw "Build failed."
}

Write-Host "[2/5] Running regression suite..." -ForegroundColor Cyan
powershell -ExecutionPolicy Bypass -File tests/run_regression_tests.ps1
if ($LASTEXITCODE -ne 0) {
    throw "Regression suite failed."
}

$releaseDir = Join-Path "build" $Configuration
$guiExe = Join-Path $releaseDir "SwitchCaseCompiler.exe"
$cliExe = Join-Path $releaseDir "SwitchCaseCompilerCLI.exe"

if (!(Test-Path $guiExe)) {
    throw "GUI executable missing: $guiExe"
}
if (!(Test-Path $cliExe)) {
    throw "CLI executable missing: $cliExe"
}

if ([string]::IsNullOrWhiteSpace($QtDeployPath)) {
    $QtDeployPath = Get-QtDeployPath -CachePath "build/CMakeCache.txt"
}

Write-Host "[3/5] Deploying Qt runtime..." -ForegroundColor Cyan
if (-not [string]::IsNullOrWhiteSpace($QtDeployPath) -and (Test-Path $QtDeployPath)) {
    & $QtDeployPath --release --compiler-runtime $guiExe
    if ($LASTEXITCODE -ne 0) {
        throw "windeployqt failed with exit code $LASTEXITCODE"
    }
} else {
    throw "Could not find windeployqt. Pass -QtDeployPath explicitly."
}

$requiredFiles = @(
    (Join-Path $releaseDir "Qt6Core.dll"),
    (Join-Path $releaseDir "Qt6Gui.dll"),
    (Join-Path $releaseDir "Qt6Widgets.dll"),
    (Join-Path $releaseDir "platforms\qwindows.dll")
)

foreach ($file in $requiredFiles) {
    if (!(Test-Path $file)) {
        throw "Missing deployed runtime file: $file"
    }
}

Write-Host "[4/5] Runtime file checks passed." -ForegroundColor Green

if (-not $SkipPackage) {
    Write-Host "[5/5] Creating distributable package..." -ForegroundColor Cyan
    $distDir = "dist"
    $packageRoot = Join-Path $distDir "SwitchCaseCompiler-$Configuration"

    if (Test-Path $packageRoot) {
        Remove-Item -Path $packageRoot -Recurse -Force
    }
    New-Item -Path $packageRoot -ItemType Directory | Out-Null

    Copy-Item -Path (Join-Path $releaseDir "*") -Destination $packageRoot -Recurse -Force

    $zipPath = Join-Path $distDir "SwitchCaseCompiler-$Configuration.zip"
    if (Test-Path $zipPath) {
        Remove-Item -Path $zipPath -Force
    }
    Compress-Archive -Path (Join-Path $packageRoot "*") -DestinationPath $zipPath -Force

    Write-Host "Package folder: $packageRoot" -ForegroundColor Green
    Write-Host "Package zip: $zipPath" -ForegroundColor Green
} else {
    Write-Host "[5/5] Package step skipped." -ForegroundColor Yellow
}

Write-Host "Validation completed successfully." -ForegroundColor Green
