# Local bare Raspberry Pi Pico 2 bring-up target; not an official Flipper board.
fw_base(f1)
# Replace the inherited broad application glob before resolving removals.
list(REMOVE_ITEM FW_SOURCE_ENTRIES "root:applications/*.c")
fw_sources(
    applications/services/usb/usb.c
    applications/services/cli/cli_vcp.c
    applications/services/cli/cli_main_shell.c
    applications/services/cli/cli_commands_common.c
    applications/services/gui/*.c
    applications/services/desktop/*.c
    applications/services/rpc/*.c
    applications/services/input/input.c
    applications/services/led/led_batches.c
    applications/debug/font_test/*.c
    applications/debug/keypad_test/*.c
)
fw_remove(lib/drivers/display/display_jd9853_qspi.c)
