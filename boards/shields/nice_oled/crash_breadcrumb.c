/*
 * Crash breadcrumb: on a fatal error, save the crash context into RAM that
 * survives a warm reboot, reboot instead of halting, and log the breadcrumb
 * on the next boot. Turns a hard fault from "dead until reset" into a ~5s
 * self-recovery, and captures where the fault happened.
 */
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/fatal.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define CRASH_MAGIC 0xDEADFA11u

struct crash_breadcrumb {
    uint32_t magic;
    uint32_t reason;
    uint32_t pc;
    uint32_t lr;
    char thread[16];
};

static struct crash_breadcrumb __noinit breadcrumb;

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf) {
    breadcrumb.magic = CRASH_MAGIC;
    breadcrumb.reason = reason;
#if defined(CONFIG_ARM)
    if (esf != NULL) {
        breadcrumb.pc = esf->basic.pc;
        breadcrumb.lr = esf->basic.lr;
    } else {
        breadcrumb.pc = 0;
        breadcrumb.lr = 0;
    }
#endif
    breadcrumb.thread[0] = '\0';
#if defined(CONFIG_THREAD_NAME)
    const char *name = k_thread_name_get(k_current_get());
    if (name != NULL) {
        strncpy(breadcrumb.thread, name, sizeof(breadcrumb.thread) - 1);
        breadcrumb.thread[sizeof(breadcrumb.thread) - 1] = '\0';
    }
#endif
    sys_reboot(SYS_REBOOT_WARM);
    CODE_UNREACHABLE;
}

static int crash_breadcrumb_report(void) {
    if (breadcrumb.magic == CRASH_MAGIC) {
        LOG_ERR("CRASH BREADCRUMB: reason=%u pc=0x%08x lr=0x%08x thread=%s",
                breadcrumb.reason, breadcrumb.pc, breadcrumb.lr,
                breadcrumb.thread[0] ? breadcrumb.thread : "?");
    }
    breadcrumb.magic = 0;
    return 0;
}
SYS_INIT(crash_breadcrumb_report, APPLICATION, 99);
