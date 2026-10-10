#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_cortex.h>
#include <furi_hal_nvm.h>
#include <furi_hal_log.h>
#include <furi_hal_usb_cdc.h>
#include <furi_bsp_stdio.h>
#include <hardware/gpio.h>
#include <hardware/watchdog.h>
#include <pico/multicore.h>
#include <pico/bootrom.h>
#include <boot/picoboot.h>
#include <cli/cli_registry.h>
#include <cli/cli_commands.h>
#include <cli/cli_commands_common.h>
#include <cli/cli_vcp.h>

extern int32_t usb_srv(void*);
extern int32_t cli_vcp_srv(void*);
extern int32_t gui_srv(void*);
extern int32_t desktop_srv(void*);
extern void rpc_on_system_start(void*);
extern void virtual_hardware_init(void);
#define pico2_boot_stage watchdog_hw->scratch[0]

static void board_info(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe); UNUSED(args); UNUSED(context);
    printf("Local Flipper One MCU bring-up: RP2350A / Pico 2 build\r\n"
           "Furi/FreeRTOS + GUI + Desktop + USB RPC; physical peripherals disabled\r\n"
           "No external GPIO outputs configured\r\n");
}

static void bootsel(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe); UNUSED(args); UNUSED(context);
    reset_usb_boot(0, 0);
}

static int32_t heartbeat(void* context) {
    UNUSED(context);
    while(true) {
        watchdog_update();
        furi_delay_ms(1000);
    }
    return 0;
}

static int32_t init_task(void* context) {
    UNUSED(context);
    pico2_boot_stage = 10;
    furi_bsp_stdio_init();
    pico2_boot_stage = 11;
    pico2_boot_stage = 15;
    CliRegistry* registry = cli_registry_alloc();
    cli_registry_add_command(registry, "board_info", CliCommandFlagParallelSafe, board_info, NULL);
    cli_registry_add_command(registry, "uptime", CliCommandFlagParallelSafe, cli_command_uptime, NULL);
    cli_registry_add_command(registry, "free", CliCommandFlagParallelSafe, cli_command_free, NULL);
    cli_registry_add_command(registry, "top", CliCommandFlagParallelSafe, cli_command_top, NULL);
    cli_registry_add_command(registry, "log", CliCommandFlagParallelSafe, cli_command_log, NULL);
    cli_registry_add_command(registry, "bootsel", CliCommandFlagParallelSafe, bootsel, NULL);
    furi_record_create(RECORD_CLI, registry);
    pico2_boot_stage = 12;
    furi_thread_start(furi_thread_alloc_service("UsbSrv", 2048, usb_srv, NULL));
    furi_thread_start(furi_thread_alloc_service("CliVcpSrv", 4096, cli_vcp_srv, NULL));
    CliVcp* vcp = furi_record_open(RECORD_CLI_VCP);
    pico2_boot_stage = 13;
    cli_vcp_enable(vcp);
    pico2_boot_stage = 14;
    furi_record_close(RECORD_CLI_VCP);
    furi_thread_start(furi_thread_alloc_service("Watchdog", 1024, heartbeat, NULL));
    virtual_hardware_init();
    furi_thread_start(furi_thread_alloc_service("GuiSrv", 16384, gui_srv, NULL));
    furi_thread_start(furi_thread_alloc_service("DesktopSrv", 16384, desktop_srv, NULL));
    rpc_on_system_start(NULL);
    FURI_LOG_I("Pico2", "USB-only bring-up ready");
    furi_background();
    return 0;
}

int main(void) {
    if(watchdog_enable_caused_reboot()) {
        uint32_t failed_stage = pico2_boot_stage;
        rom_reboot(REBOOT2_FLAG_REBOOT_TYPE_BOOTSEL | REBOOT2_FLAG_NO_RETURN_ON_SUCCESS, 10, 0, failed_stage);
    }
    watchdog_enable(6000, false);
    pico2_boot_stage = 1;
    furi_init();
    pico2_boot_stage = 2;
    furi_hal_log_init(FuriLogLevelInfo, FuriHalLogOutputRtt);
    furi_hal_cortex_init_early();
    pico2_boot_stage = 3;
    pico2_boot_stage = 4;
    FuriThread* thread = furi_thread_alloc_ex("Init", 4096, init_task, NULL);
    furi_thread_set_priority(thread, FuriThreadPriorityInit);
    furi_thread_start(thread);
    sleep_ms(5);
    multicore_reset_core1();
    pico2_boot_stage = 5;
    // SDK 2.3.1 already consumes the reset handshake inside reset_core1().
    furi_run();
    furi_crash("Kernel is Dead");
}

void abort(void) { furi_crash("AbortHandler"); }

bool furi_crash_handler(bool debug) {
    UNUSED(debug);
    rom_reboot(REBOOT2_FLAG_REBOOT_TYPE_BOOTSEL | REBOOT2_FLAG_NO_RETURN_ON_SUCCESS,
               10, 0, 0x100 + pico2_boot_stage);
    while(true) tight_loop_contents();
}
