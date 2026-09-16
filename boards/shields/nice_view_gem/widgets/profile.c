#include <zephyr/kernel.h>
#include "profile.h"

void draw_profile_status(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_rect_dsc_t rect_white_dsc;
    init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);

    // Five 8x8 profile boxes, active one filled solid
    for (int i = 0; i < 5; i++) {
        int x = 6 + i * 12;
        int y = 125 + BUFFER_OFFSET_BOTTOM;

        canvas_draw_rect(canvas, x, y, 8, 8, &rect_white_dsc);
        if (i != state->active_profile_index) {
            canvas_draw_rect(canvas, x + 1, y + 1, 6, 6, &rect_black_dsc);
        }
    }
}
