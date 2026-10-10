#include <furi.h>
#include <applications.h>
#include <led/led.h>
#include <input/input.h>
#include <input_touch/input_touch.h>
#include <drivers/display/display_jd9853_qspi.h>

// GUI owns the framebuffer; RPC subscribes to it after every redraw.
struct DisplayJd9853QSPI { bool ready; };
static DisplayJd9853QSPI virtual_display = {.ready = true};
DisplayJd9853QSPI* display_jd9853_qspi_init(void) { return &virtual_display; }
void display_jd9853_qspi_deinit(DisplayJd9853QSPI* d) { UNUSED(d); }
void display_jd9853_load_config(DisplayJd9853QSPI* d, const uint8_t* c) { UNUSED(d); UNUSED(c); }
void display_jd9853_qspi_on_sleep_enter(void) {}
void display_jd9853_qspi_on_sleep_exit(void) {}
void display_jd9853_qspi_write_buffer(DisplayJd9853QSPI* d, const uint8_t* b, size_t n) { UNUSED(d); UNUSED(b); UNUSED(n); }
void display_jd9853_qspi_fill(DisplayJd9853QSPI* d, uint8_t c) { UNUSED(d); UNUSED(c); }
void display_jd9853_qspi_eco_mode(DisplayJd9853QSPI* d, bool e) { UNUSED(d); UNUSED(e); }
bool display_jd9853_qspi_is_init(void) { return true; }

// Volatile preview settings, deliberately no GPIO or flash writes.
struct Led { FuriState* brightness[LedGroupMax]; FuriState* timeout; };
static Led virtual_led;
void led_set_color_single(Led* i, FuriBspLedType t, LedColor c) { UNUSED(i); UNUSED(t); UNUSED(c); }
void led_set_color_batch(Led* i, const LedBatch* b) { UNUSED(i); UNUSED(b); }
void led_set_brightness(Led* i, LedGroup g, uint8_t b) { furi_check(g < LedGroupMax); furi_state_set(i->brightness[g], &b); }
bool led_backlight_set_time(Led* i, uint32_t t) { furi_state_set(i->timeout, &t); return true; }
void led_backlight_timeout_control(Led* i, bool p, bool a) { UNUSED(i); UNUSED(p); UNUSED(a); }
FuriState* led_get_brightness_state(Led* i, LedGroup g) { furi_check(g < LedGroupMax); return i->brightness[g]; }
FuriState* led_get_backlight_time_state(Led* i) { return i->timeout; }

void virtual_hardware_init(void) {
    furi_record_create(RECORD_INPUT_EVENTS, furi_pubsub_alloc());
    furi_record_create(RECORD_INPUT_TOUCH_EVENTS, furi_pubsub_alloc());
    for(size_t g = 0; g < LedGroupMax; g++) {
        uint8_t brightness = 255;
        virtual_led.brightness[g] = furi_state_alloc(sizeof(brightness));
        furi_state_set(virtual_led.brightness[g], &brightness);
    }
    uint32_t timeout = 30000;
    virtual_led.timeout = furi_state_alloc(sizeof(timeout));
    furi_state_set(virtual_led.timeout, &timeout);
    furi_record_create(RECORD_LEDS, &virtual_led);
}

extern int32_t font_test_app(void*);
extern int32_t keypad_test_app(void*);
const FlipperInternalApplication FLIPPER_APPS[] = {
    {.app=font_test_app, .name="Font Test", .appid="font_test", .stack_size=4096},
    {.app=keypad_test_app, .name="Keypad Test", .appid="keypad_test", .stack_size=4096},
};
const size_t FLIPPER_APPS_COUNT = COUNT_OF(FLIPPER_APPS);
const FlipperInternalApplication FLIPPER_AUTORUN_APPS[] = {{0}};
const size_t FLIPPER_AUTORUN_APPS_COUNT = 0;
