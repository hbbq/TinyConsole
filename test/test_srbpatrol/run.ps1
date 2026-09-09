param([string]$Packages = "$env:USERPROFILE/.platformio/packages")

$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path "$PSScriptRoot/../..").Path
$outputDir = Join-Path $repo '.pio/test_srbpatrol'
New-Item -ItemType Directory -Force $outputDir | Out-Null
$compiler = Join-Path $Packages 'toolchain-atmelavr/bin/avr-g++.exe'
$debugger = Join-Path $Packages 'toolchain-atmelavr/bin/avr-gdb.exe'
$core = Join-Path $Packages 'framework-arduino-avr-attiny/cores/tiny'
$elf = Join-Path $outputDir 'patrol.elf'

# avr-gdb's instruction simulator uses a 22-bit PC and does not emulate the
# ATtiny85's 8 KB relative-call wrap. Use its compatible AVR target for tests;
# PlatformIO separately verifies the production ATtiny85 build and size.
& $compiler -mmcu=atmega2560 -DF_CPU=1000000L -std=gnu++11 -Os -g `
    -ffunction-sections -fdata-sections '-Wl,--gc-sections' `
    "-I$PSScriptRoot" "-I$core" "-I$repo/src" `
    "$PSScriptRoot/main.cpp" "$PSScriptRoot/launcher.cpp" "$repo/src/main.cpp" `
    "$repo/src/apps/SrbApp.cpp" "$repo/src/apps/Game.cpp" `
    "$repo/src/apps/GameFactory.cpp" "$repo/src/apps/GameIcons.cpp" `
    "$repo/src/apps/RacerApp.cpp" "$repo/src/apps/BreakoutApp.cpp" `
    "$repo/src/apps/SkyHopApp.cpp" "$repo/src/apps/ShiftApp.cpp" `
    "$repo/src/hardware/TinyConsoleGameApi.cpp" "$repo/src/hardware/TinyConsole.cpp" `
    "$repo/src/TinyRandom.cpp" -o $elf
if ($LASTEXITCODE -ne 0) { throw 'AVR test build failed' }

# GDB loads .data at its SRAM address, but AVR startup copies it from flash.
$initialData = Join-Path $outputDir 'data.bin'
& (Join-Path $Packages 'toolchain-atmelavr/bin/avr-objcopy.exe') -O binary -j .data $elf $initialData
if ($LASTEXITCODE -ne 0) { throw 'Could not extract initialized data' }
$symbols = & (Join-Path $Packages 'toolchain-atmelavr/bin/avr-nm.exe') $elf
if ($LASTEXITCODE -ne 0) { throw 'Could not read AVR symbols' }
$dataSymbol = $symbols | Where-Object { $_ -match '^([0-9a-fA-F]+)\s+\w\s+__data_load_start$' }
if (!$dataSymbol) { throw 'Missing initialized-data flash address' }
$dataAddress = ($dataSymbol -split '\s+')[0]
$restoreData = 'restore data.bin binary (void(*)())0x' + $dataAddress

$commands = Join-Path $outputDir 'test.gdb'
@'
set confirm off
target sim
load
break *0
run
delete breakpoints
RESTORE_DATA
break testFinished
continue
if failureLine != 0
    printf "FAIL at main.cpp:%u after %u checks\n", failureLine, checks
    printf "Launcher failure line (0 if none): %u\n", launcherFailureLine
    quit 1
end
if completed == 0
    quit 1
end
printf "PASS: %u AVR simulation checks\n", checks
quit 0
'@.Replace('RESTORE_DATA', $restoreData) | Set-Content -LiteralPath $commands -Encoding ascii
Push-Location $outputDir
try {
    $simulationOutput = & $debugger -batch -x $commands $elf
    $simulationExit = $LASTEXITCODE
} finally {
    Pop-Location
}
$simulationOutput | Write-Output
if ($simulationExit -ne 0 -or !($simulationOutput -match '^PASS: \d+ AVR simulation checks$')) {
    throw 'AVR patrol simulation failed'
}
