# RP2350 / Pico 2 USB development target

The `f3` target runs the existing Flipper One GUI, Desktop and USB RPC services
on a bare Pico 2 without the Flipper display, keypad, battery or LEDs. Display
frames come from the firmware GUI; button events arrive through the existing RPC
protocol. Font Test and Keypad Test are included.

## Build

Run `./build-pico2.ps1` in PowerShell. The helper currently expects Pico SDK
2.3.1 at `../pico-sdk`, picotool and pioasm packages under `../tools`, the
14_2_Rel1 ARM toolchain and Ninja v1.12.1 under `$env:USERPROFILE/.pico-sdk`,
and Python from the local Codex runtime. Adjust those paths for another machine.
The output is `build-pico2/flipperone-mcu-firmware.uf2`.

The target assumes 4 MiB flash, reserving the last 256 KiB for storage. Persistent
storage initialization is currently disabled; settings do not survive reboot.
Crash and watchdog recovery request the ROM USB bootloader.

## Desktop viewer

Use a Python installation with working Tkinter support:

```
python -m pip install -r applications/debug/usb_viewer/requirements.txt
python applications/debug/usb_viewer/viewer.py
```

The viewer reuses `applications/debug/rpc_test/rpc_test.py` and the repository's
protobuf definitions. It detects USB VID/PID `37c1:f101`; close other serial-port
applications first. Arrow keys navigate, Enter selects, Escape goes back,
4 opens Settings, and Tab opens Tests.

On the test board, firmware flashing and verification passed, a 258 x 144 L8
frame was received, and KEY_4 press/release opened Settings. The desktop wrapper
is experimental: its startup/reconnection behavior has not been fully verified.

Physical peripheral drivers are stubbed. Brightness/LED settings only update
RAM. Visible power, self-check and other hardware menu entries do not imply
working Flipper hardware functionality. This target is not a full Flipper One
device or qFlipper implementation.
