# nice_oled (vendored)

Vendored copy of the `nice_oled` shield from
[mctechnology17/zmk-nice-oled](https://github.com/mctechnology17/zmk-nice-oled),
taken from the Zephyr 4.1 / LVGL 9 port in
[PR #37](https://github.com/mctechnology17/zmk-nice-oled/pull/37)
(branch `4.0` of tokyo2006's fork).

It is vendored rather than pulled as a west module because the upstream port
did not work on nice!view hardware — the author only tested `nice_oled`, not
`nice_epaper`. Local fixes:

- **`nice_epaper.overlay`** — added `serial-vcom-inversion` and
  `serial-vcom-interval = <33>` to the `ls0xx` node, matching ZMK's stock
  `nice_view` overlay. Missing upstream.
- **`nice_epaper.conf`** — `CONFIG_ZMK_DISPLAY_DEDICATED_THREAD_STACK_SIZE=8192`
  plus `CONFIG_HW_STACK_PROTECTION=y`. **This is the important one.** LVGL 9's
  software rendering overflows the module's default 4K display-thread stack,
  which corrupted a return address and hard-faulted the board — bricking it at
  boot, and later crashing it roughly every 60s on the battery redraw.
- **`Kconfig.defconfig`** — `LV_Z_MEM_POOL_SIZE` raised to 16384 for the
  160x160 L8 canvases.
- **`custom_status_screen.c`** — 500ms deferred widget init.
- **`crash_breadcrumb.c`** (new) — overrides `k_sys_fatal_error_handler` to save
  the fault context to `__noinit` RAM, warm-reboot instead of halting, and log
  the breadcrumb on the next boot. Turns any future fatal error into a few
  seconds of self-recovery instead of a dead half.
- **`NICE_OLED_WIDGET_STATIC_IMAGE_PERIPHERAL_SPACEMAN`** (new option) — one
  static spaceman frame via the static-image path, no animation timer.

Upstream docs (including the RAW HID companion-app setup) live in the
repository linked above.
