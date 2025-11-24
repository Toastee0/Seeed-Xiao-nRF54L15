/*
 * Simple COMP Touch Test
 * Tests capacitive touch sensing on D0 (P1.04 / AIN0) using COMP peripheral
 * Detects if oscillator is running (presence of capacitance changes)
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <hal/nrf_comp.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(comp_touch, LOG_LEVEL_INF);

// Touch detection baseline
static uint32_t baseline_reading = 0;

/**
 * @brief Initialize COMP for capacitive touch on D0 (AIN0)
 */
static int init_comp_touch(void)
{
	NRF_COMP_Type *comp = NRF_COMP;
	
	// Must configure BEFORE enabling
	// Select P1.04 (D0) - PORT=1, PIN=4 for AIN0
	comp->PSEL = (1 << COMP_PSEL_PORT_Pos) | (4 << COMP_PSEL_PIN_Pos);
	
	// Configure: VDD reference, Single-ended mode, 10μA current source
	comp->REFSEL = COMP_REFSEL_REFSEL_VDD << COMP_REFSEL_REFSEL_Pos;
	comp->MODE = (COMP_MODE_MAIN_SE << COMP_MODE_MAIN_Pos) |
	             (COMP_MODE_SP_Normal << COMP_MODE_SP_Pos);
	
	// EXTREFSEL not used in VDD mode (set to 0)
	comp->EXTREFSEL = 0;
	
	// Set thresholds (VUP=32/63*VDD, VDOWN=31/63*VDD) 
	comp->TH = (32 << COMP_TH_THUP_Pos) | (31 << COMP_TH_THDOWN_Pos);
	
	// Enable 10μA current source
	comp->ISOURCE = COMP_ISOURCE_ISOURCE_Ien10uA << COMP_ISOURCE_ISOURCE_Pos;
	
	// NOW enable and start
	comp->ENABLE = COMP_ENABLE_ENABLE_Enabled << COMP_ENABLE_ENABLE_Pos;
	comp->TASKS_START = 1;
	
	k_msleep(10);  // Stabilization delay
	
	// Dump register values
	LOG_INF("COMP Registers:");
	LOG_INF("  PSEL=0x%08x (PORT=%d, PIN=%d)", 
		comp->PSEL, 
		(comp->PSEL >> COMP_PSEL_PORT_Pos) & 0x7,
		(comp->PSEL >> COMP_PSEL_PIN_Pos) & 0x1F);
	LOG_INF("  MODE=0x%08x", comp->MODE);
	LOG_INF("  REFSEL=0x%08x", comp->REFSEL);
	LOG_INF("  EXTREFSEL=0x%08x", comp->EXTREFSEL);
	LOG_INF("  TH=0x%08x (UP=%d, DOWN=%d)", 
		comp->TH,
		(comp->TH >> COMP_TH_THUP_Pos) & 0x3F,
		(comp->TH >> COMP_TH_THDOWN_Pos) & 0x3F);
	LOG_INF("  ISOURCE=0x%08x", comp->ISOURCE);
	LOG_INF("  ENABLE=0x%08x", comp->ENABLE);
	LOG_INF("  RESULT=0x%08x", comp->RESULT);
	
	LOG_INF("COMP initialized on D0 (P1.04/AIN0) with 10uA current source");
	return 0;
}

/**
 * @brief Sample COMP state changes to detect capacitance
 * Returns number of transitions in a short sampling period
 */
static uint32_t sample_comp_activity(void)
{
	NRF_COMP_Type *comp = NRF_COMP;
	uint32_t changes = 0;
	bool last_state = (comp->RESULT & COMP_RESULT_RESULT_Msk) ? true : false;
	
	// Sample for 100us - if oscillating at ~500kHz, should see ~50 transitions
	for (int i = 0; i < 1000; i++) {
		bool state = (comp->RESULT & COMP_RESULT_RESULT_Msk) ? true : false;
		if (state != last_state) {
			changes++;
			last_state = state;
		}
		k_busy_wait(1);  // 1us delay
	}
	
	return changes;
}

int main(void)
{
	LOG_INF("COMP Touch Test - D0 (P1.04/AIN0)");

	if (init_comp_touch() < 0) {
		LOG_ERR("COMP init failed");
		return -1;
	}

	// Calibrate baseline (untouched)
	k_msleep(100);
	baseline_reading = 0;
	for (int i = 0; i < 10; i++) {
		baseline_reading += sample_comp_activity();
		k_msleep(10);
	}
	baseline_reading /= 10;
	
	LOG_INF("Baseline: %u transitions/100us", baseline_reading);
	LOG_INF("Touch D0 to test (activity will change)");

	bool touch_state = false;

	while (1) {
		uint32_t activity = sample_comp_activity();
		
		// Touch adds capacitance, changes oscillation
		// Could increase OR decrease activity depending on initial conditions
		int32_t diff = (int32_t)activity - (int32_t)baseline_reading;
		bool touched = (abs(diff) > (baseline_reading / 4));  // 25% change
		
		if (touched != touch_state) {
			touch_state = touched;
			if (touched) {
				LOG_INF("*** TOUCH! *** activity=%u (baseline=%u, diff=%d)", 
					activity, baseline_reading, diff);
			} else {
				LOG_INF("Released. activity=%u", activity);
			}
		}
		
		// Periodic logging
		static int counter = 0;
		if (++counter >= 10) {
			LOG_INF("activity=%u, baseline=%u, RESULT=0x%08x, %s", 
				activity, baseline_reading, NRF_COMP->RESULT, touched ? "TOUCHED" : "idle");
			counter = 0;
		}
		
		k_msleep(50);
	}

	return 0;
}
