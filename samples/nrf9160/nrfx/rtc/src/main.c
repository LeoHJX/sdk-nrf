/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>
#include <modem/lte_lc.h>

#include <nrf9160.h>
#include <zephyr.h>
#include <string.h>
#include <stdlib.h>
#include <nrfx_rtc.h>
#include <nrfx_timer.h>
#include <nrfx_dppi.h>
#include <nrfx_gpiote.h>
#include <hal/nrf_gpiote.h>
#include <hal/nrf_timer.h>

#include <hal/nrf_gpio.h>

#define LED_0 2
#define LED_1 3

#define COMPARE_COUNTERTIME 3

const nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(0);

static void rtc_handler(nrfx_rtc_int_type_t int_type)
{
	if (int_type == NRFX_RTC_INT_COMPARE0) {
		nrf_gpio_pin_toggle(LED_0);
	} else if (int_type == NRFX_RTC_INT_TICK) {
		nrf_gpio_pin_toggle(LED_1);
	}
}

static void rtc_config(void)
{
	uint32_t err_code;

	nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;
	config.prescaler = 4095;
	err_code = nrfx_rtc_init(&rtc, &config, rtc_handler);
	if (err_code != NRFX_SUCCESS) {
		printk("Failure in setup\n");
		return;
	}

	nrfx_rtc_tick_enable(&rtc, true);
	err_code = nrfx_rtc_cc_set(&rtc, 0, COMPARE_COUNTERTIME * 8, true);
	if (err_code != NRFX_SUCCESS) {
		printk("Failure in setup\n");
		return;
	}

	nrfx_rtc_enable(&rtc);
}

static void manual_isr_setup()
{
	IRQ_DIRECT_CONNECT(RTC0_IRQn, 0, nrfx_rtc_0_irq_handler, 0);
	irq_enable(RTC0_IRQn);
}

void main(void)
{
	int err;
#if 0
	modem_init();
#endif
	lte_lc_func_mode_set(LTE_LC_FUNC_MODE_NORMAL);
	printk("Just switch Modem Off\n"); /* should output to RTT*/
	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_OFFLINE);
	if(err < 0 ){
		printk("lte_lc_func_mode_set error %d\n", err);
	}
	printk("Starting nrfx rtc sample!\n");
	printk("To run in non-secure mode, ");
	printk("you need to add RTC0 to spm.c\n");
	nrf_gpio_cfg_output(LED_0);
	nrf_gpio_cfg_output(LED_1);
	rtc_config();
	manual_isr_setup();
	return;
}
