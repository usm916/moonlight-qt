[CmdletBinding()]
param(
    [string]$QtRoot = 'D:\data\tools\Qt',
    [ValidateSet('release','debug')][string]$Configuration = 'release',
    [int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual Studio C++ build tools are required.' }
$qmake = Join-Path $QtRoot 'bin\qmake.exe'
if (!(Test-Path -LiteralPath $qmake)) { throw "Missing Qt SDK: $qmake" }
if (!(Test-Path -LiteralPath "$root\libs\windows\lib\x64\SDL2.dll")) { throw 'Run setup-deps.ps1 first to install upstream native dependencies.' }
$build = Join-Path $root "build\build-custom-x64-$Configuration"
$deploy = Join-Path $root "build\deploy-custom-x64-$Configuration"
New-Item -ItemType Directory -Force $build,$deploy | Out-Null
$jom = Join-Path $QtRoot 'jom\jom.exe'
$make = if (Test-Path -LiteralPath $jom) { "`"$jom`" -j $Jobs $Configuration" } else { "nmake /nologo $Configuration" }
$commands = @"
@echo off
call "$vs\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b %errorlevel%
cd /d "$build"
set "PATH=$QtRoot\bin;%PATH%"
"$qmake" "$root\moonlight-qt.pro"
if errorlevel 1 exit /b %errorlevel%
$make
if errorlevel 1 exit /b %errorlevel%
cl /nologo /W4 /I"$root\moonlight-common-c\moonlight-common-c\src" "$root\moonlight-common-c\moonlight-common-c\tests\clipboard.c" /Fe:clipboard-test.exe
if errorlevel 1 exit /b %errorlevel%
clipboard-test.exe
if errorlevel 1 exit /b %errorlevel%
if not exist clipboard-state mkdir clipboard-state
cd clipboard-state
"$qmake" "$root\tests\custom\clipboard.pro" CONFIG+=release
if errorlevel 1 exit /b %errorlevel%
nmake /nologo
if errorlevel 1 exit /b %errorlevel%
clipboard-state-test.exe
exit /b %errorlevel%
"@
$launcher = Join-Path $build 'build-custom.cmd'
Set-Content -LiteralPath $launcher -Value $commands -Encoding ascii
& $env:ComSpec /d /c "`"$launcher`""
if ($LASTEXITCODE) { throw "Moonlight build or protocol test failed: $LASTEXITCODE" }
Copy-Item -LiteralPath "$build\app\$Configuration\Moonlight.exe" -Destination $deploy
Copy-Item "$root\libs\windows\lib\x64\*.dll" -Destination $deploy
Copy-Item -LiteralPath "$build\AntiHooking\$Configuration\AntiHooking.dll" -Destination $deploy
Copy-Item -LiteralPath "$root\app\SDL_GameControllerDB\gamecontrollerdb.txt" -Destination $deploy
& "$QtRoot\bin\windeployqt.exe" --dir $deploy --$Configuration --qmldir "$root\app\gui" --no-opengl-sw --no-ffmpeg --no-system-d3d-compiler --no-system-dxc-compiler "$deploy\Moonlight.exe"
if ($LASTEXITCODE) { throw 'Qt deployment failed.' }
$runtime = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'VC\Redist\MSVC\*\x64\Microsoft.VC*.CRT'
if (!$runtime) { throw 'MSVC redistributable DLLs were not found.' }
Copy-Item "$($runtime | Select-Object -First 1)\*.dll" -Destination $deploy
Set-Content -LiteralPath "$deploy\portable.dat" -Value ''
Write-Host "Portable Moonlight: $deploy\Moonlight.exe"
