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
    uint32_t cfsr;
    uint32_t r0, r1, r2, r3, r12, xpsr;
    uint32_t uptime_ms;
    uint32_t in_isr;
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
        breadcrumb.r0 = esf->basic.r0;
        breadcrumb.r1 = esf->basic.r1;
        breadcrumb.r2 = esf->basic.r2;
        breadcrumb.r3 = esf->basic.r3;
        breadcrumb.r12 = esf->basic.ip;
        breadcrumb.xpsr = esf->basic.xpsr;
    }
    // Configurable Fault Status Register: which fault bits fired
    breadcrumb.cfsr = *(volatile uint32_t *)0xE000ED28;
#endif
    breadcrumb.uptime_ms = k_uptime_get_32();
    breadcrumb.in_isr = k_is_in_isr();
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
        LOG_ERR("CRASH BREADCRUMB: reason=%u pc=0x%08x lr=0x%08x cfsr=0x%08x thread=%s",
                breadcrumb.reason, breadcrumb.pc, breadcrumb.lr, breadcrumb.cfsr,
                breadcrumb.thread[0] ? breadcrumb.thread : "?");
        LOG_ERR("CRASH BREADCRUMB2: r0=0x%08x r1=0x%08x r2=0x%08x r3=0x%08x r12=0x%08x xpsr=0x%08x up=%ums isr=%u",
                breadcrumb.r0, breadcrumb.r1, breadcrumb.r2, breadcrumb.r3,
                breadcrumb.r12, breadcrumb.xpsr, breadcrumb.uptime_ms, breadcrumb.in_isr);
    }
    breadcrumb.magic = 0;
    return 0;
}
SYS_INIT(crash_breadcrumb_report, APPLICATION, 99);
