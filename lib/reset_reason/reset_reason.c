/*
 * Boot-time reset reason readout for the XIAO nRF54L15 samples.
 *
 * Reads RESETREAS straight from the RESET peripheral (rather than through
 * hwinfo, which folds every CTRL-AP variant into RESET_DEBUG and tamper into
 * RESET_SECURITY) so the printout maps one-to-one onto the datasheet bits.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <helpers/nrfx_reset_reason.h>

#include <xiao_reset_reason.h>

struct reason_bit {
	uint32_t mask;
	const char *name;
	const char *desc;
};

#define REASON(bit, text) {RESET_RESETREAS_##bit##_Msk, #bit, text}

static const struct reason_bit reasons[] = {
#ifdef RESET_RESETREAS_RESETPIN_Msk
	REASON(RESETPIN, "reset pin"),
#endif
#ifdef RESET_RESETREAS_DOG0_Msk
	REASON(DOG0, "watchdog 0 timeout"),
#endif
#ifdef RESET_RESETREAS_DOG1_Msk
	REASON(DOG1, "watchdog 1 timeout"),
#endif
#ifdef RESET_RESETREAS_CTRLAP_Msk
	REASON(CTRLAP, "debugger (CTRL-AP) reset"),
#endif
#ifdef RESET_RESETREAS_CTRLAPSOFT_Msk
	REASON(CTRLAPSOFT, "debugger (CTRL-AP) soft reset"),
#endif
#ifdef RESET_RESETREAS_CTRLAPHARD_Msk
	REASON(CTRLAPHARD, "debugger (CTRL-AP) hard reset"),
#endif
#ifdef RESET_RESETREAS_CTRLAPPIN_Msk
	REASON(CTRLAPPIN, "debugger (CTRL-AP) pin reset"),
#endif
#ifdef RESET_RESETREAS_SREQ_Msk
	REASON(SREQ, "software reset (SYSRESETREQ)"),
#endif
#ifdef RESET_RESETREAS_LOCKUP_Msk
	REASON(LOCKUP, "CPU lockup"),
#endif
#ifdef RESET_RESETREAS_OFF_Msk
	REASON(OFF, "wake from System OFF (GPIO)"),
#endif
#ifdef RESET_RESETREAS_LPCOMP_Msk
	REASON(LPCOMP, "wake from System OFF (LPCOMP)"),
#endif
#ifdef RESET_RESETREAS_DIF_Msk
	REASON(DIF, "wake from System OFF (debug interface)"),
#endif
#ifdef RESET_RESETREAS_GRTC_Msk
	REASON(GRTC, "wake from System OFF (GRTC)"),
#endif
#ifdef RESET_RESETREAS_NFC_Msk
	REASON(NFC, "wake from System OFF (NFC field)"),
#endif
#ifdef RESET_RESETREAS_VBUS_Msk
	REASON(VBUS, "wake from System OFF (VBUS)"),
#endif
#ifdef RESET_RESETREAS_LSREQ_Msk
	REASON(LSREQ, "software reset (local core)"),
#endif
#ifdef RESET_RESETREAS_LLOCKUP_Msk
	REASON(LLOCKUP, "CPU lockup (local core)"),
#endif
#ifdef RESET_RESETREAS_LDOG_Msk
	REASON(LDOG, "watchdog timeout (local core)"),
#endif
#ifdef RESET_RESETREAS_LCTRLAP_Msk
	REASON(LCTRLAP, "debugger reset (local core)"),
#endif
#ifdef RESET_RESETREAS_MFORCEOFF_Msk
	REASON(MFORCEOFF, "forced off by main core"),
#endif
#ifdef RESET_RESETREAS_SECTAMPER_Msk
	REASON(SECTAMPER, "tamper controller (glitch detector / active shield)"),
#endif
};

static uint32_t boot_resetreas;

uint32_t xiao_reset_reason_get(void)
{
	return boot_resetreas;
}

static int xiao_reset_reason_init(void)
{
	uint32_t unknown;

	boot_resetreas = nrfx_reset_reason_get();
	unknown = boot_resetreas;

	printk("*** Reset reason: 0x%08x ***\n", boot_resetreas);

	if (boot_resetreas == 0) {
		printk("***   (none set) power-on reset or brownout\n");
	}

	for (size_t i = 0; i < ARRAY_SIZE(reasons); i++) {
		if (boot_resetreas & reasons[i].mask) {
			printk("***   %s: %s\n", reasons[i].name, reasons[i].desc);
			unknown &= ~reasons[i].mask;
		}
	}

	if (unknown) {
		printk("***   unknown bits: 0x%08x\n", unknown);
	}

#ifdef RESET_RESETREAS_SECTAMPER_Msk
	if (boot_resetreas & RESET_RESETREAS_SECTAMPER_Msk) {
		printk("!!! Tamper reset: check the supply rail, USB cable/hub and "
		       "probe wiring before blaming the firmware\n");
	}
#endif

	if (IS_ENABLED(CONFIG_XIAO_RESET_REASON_CLEAR)) {
		nrfx_reset_reason_clear(UINT32_MAX);
	}

	return 0;
}

/* Late in APPLICATION so anything else that reads RESETREAS during init
 * (e.g. hwinfo users) sees it before it is cleared.
 */
SYS_INIT(xiao_reset_reason_init, APPLICATION, 99);
