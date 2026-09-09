param([string]$Packages = "$env:USERPROFILE/.platformio/packages")
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path "$PSScriptRoot/../..").Path
$outputDir = Join-Path $repo '.pio/srb-experiment'
node "$PSScriptRoot/replay.cjs"
if ($LASTEXITCODE -ne 0) { throw 'Generate witnesses with check.cjs first' }
$compiler = Join-Path $Packages 'toolchain-atmelavr/bin/avr-g++.exe'
$debugger = Join-Path $Packages 'toolchain-atmelavr/bin/avr-gdb.exe'
$core = Join-Path $Packages 'framework-arduino-avr-attiny/cores/tiny'
$elf = Join-Path $outputDir 'replay.elf'
& $compiler -mmcu=atmega2560 -DF_CPU=1000000L -std=gnu++11 -Os -g `
    -ffunction-sections -fdata-sections '-Wl,--gc-sections' `
    "-I$repo/test/test_srbpatrol" "-I$core" "-I$outputDir/build/src" `
    "$outputDir/replay.cpp" "$outputDir/build/src/apps/SrbApp.cpp" `
    "$outputDir/build/src/hardware/TinyConsoleGameApi.cpp" `
    "$outputDir/build/src/TinyRandom.cpp" -o $elf
if ($LASTEXITCODE -ne 0) { throw 'AVR replay build failed' }
& (Join-Path $Packages 'toolchain-atmelavr/bin/avr-objcopy.exe') -O binary -j .data $elf "$outputDir/data.bin"
if ($LASTEXITCODE -ne 0) { throw 'Data extraction failed' }
$symbols = & (Join-Path $Packages 'toolchain-atmelavr/bin/avr-nm.exe') $elf
$dataSymbol = $symbols | Where-Object { $_ -match '^([0-9a-fA-F]+)\s+\w\s+__data_load_start$' }
if (!$dataSymbol) { throw 'Missing initialized-data address' }
$dataAddress = ($dataSymbol -split '\s+')[0]
$restoreData = ''
if ((Get-Item -LiteralPath "$outputDir/data.bin").Length -gt 0) {
    $restoreData = 'restore data.bin binary (void(*)())0x' + $dataAddress
}
$commands = Join-Path $outputDir 'replay.gdb'
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
    printf "FAIL case %u line %u after %u checks\n", caseIndex, failureLine, checks
    quit 1
end
if completed == 0
    quit 1
end
printf "PASS: %u AVR generator/route checks\n", checks
quit 0
'@.Replace('RESTORE_DATA', $restoreData) | Set-Content -LiteralPath $commands -Encoding ascii
Push-Location $outputDir
try {
    $result = & $debugger -batch -x $commands $elf
    $code = $LASTEXITCODE
} finally { Pop-Location }
$result | Write-Output
if ($code -ne 0 -or !($result -match '^PASS: \d+ AVR generator/route checks$')) { throw 'AVR replay failed' }
