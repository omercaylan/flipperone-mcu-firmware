$ErrorActionPreference = 'Stop'
$toolRoot = Join-Path $PSScriptRoot '../tools'
$sdkRoot = (Resolve-Path (Join-Path $PSScriptRoot '../pico-sdk')).Path
$compilerRoot = Join-Path $env:USERPROFILE '.pico-sdk/toolchain/14_2_Rel1'
$ninjaRoot = Join-Path $env:USERPROFILE '.pico-sdk/ninja/v1.12.1'
$pythonRoot = Join-Path $env:USERPROFILE '.cache/codex-runtimes/codex-primary-runtime/dependencies/python'
$env:PATH = "$compilerRoot/bin;$ninjaRoot;$pythonRoot;$env:PATH"
& cmake -S $PSScriptRoot -B "$PSScriptRoot/build-pico2" -G Ninja `
    -DFW_TARGET=f3 -DFIRMWARE_ORIGIN=Local-Pico2 -DCMAKE_BUILD_TYPE=Release `
    "-DPICO_SDK_PATH=$sdkRoot" "-DPICO_TOOLCHAIN_PATH=$compilerRoot" `
    "-Dpioasm_DIR=$toolRoot/pioasm" "-Dpicotool_DIR=$toolRoot/picotool"
if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
& cmake --build "$PSScriptRoot/build-pico2" --parallel 8
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
Write-Output "Firmware: $PSScriptRoot/build-pico2/flipperone-mcu-firmware.uf2"
