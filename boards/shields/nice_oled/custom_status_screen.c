#include "widgets/screen.h"
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <fonts.h>

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_STATUS)
static struct zmk_widget_screen screen_widget;
#endif

lv_obj_t *zmk_display_status_screen() {
    LOG_WRN("NICEOLED: deferring widget init 3s past boot races");
    k_sleep(K_SECONDS(3));
    LOG_WRN("NICEOLED: creating screen object");
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

#if IS_ENABLED(CONFIG_NICE_OLED_WIDGET_STATUS)
    LOG_WRN("NICEOLED: calling zmk_widget_screen_init");
    zmk_widget_screen_init(&screen_widget, screen);
    LOG_WRN("NICEOLED: zmk_widget_screen_init returned");
    lv_obj_align(zmk_widget_screen_obj(&screen_widget), LV_ALIGN_TOP_LEFT, 0, 0);
#endif

    return screen;
}
