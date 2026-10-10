#include <furi_hal_os.h>
#include <furi.h>
#include <furi_hal_power.h>
#include <furi_hal_clock.h>
#include <FreeRTOS.h>
#include <stdbool.h>
#include <task.h>
#include <furi_hal_debug.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <pico/time.h>
#include <applications.h>

#define TAG "FuriHalOs"

extern void SysTick_Handler(void);

void furi_hal_os_init(void) {}

void furi_hal_os_tick(void) {
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        SysTick_Handler();
    }
}

void vPortSuppressTicksAndSleep(TickType_t expected_idle_ticks) {
    (void)expected_idle_ticks;
    __WFI();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    /* Log the task name and stack boundaries for easier debugging */
    TaskStatus_t status;
    vTaskGetInfo(xTask, &status, pdTRUE, eRunning);
    char buf[256];
    snprintf(
        buf,
        sizeof(buf),
        "\r\n[E][" TAG "] stack overflow in task '%s' (handle=0x%08x) "
        "stack_base=0x%08x stack_min_free=%u\r\n",
        pcTaskName,
        (unsigned)xTask,
        (unsigned)status.pxStackBase,
        (unsigned)(status.usStackHighWaterMark * sizeof(StackType_t)));
    furi_log_puts(buf);
    furi_crash("Stack overflow");
}

__attribute__((__noreturn__)) void vFreeRTOSAssertFailed(const char* expr, const char* file, int line) {
    /* Called from within vTaskSwitchContext (PendSV context).
     * uxTaskGetSystemState() is unsafe here – just log the expression and crash.
     * The standard HardFault handler will print registers/stack. */
    char buf[256];
    snprintf(buf, sizeof(buf), "\r\n[E][" TAG "] FreeRTOS assert FAILED: (%s) at %s:%d\r\n", expr, file, line);
    furi_log_puts(buf);
    furi_crash("FreeRTOS Assert");
}
