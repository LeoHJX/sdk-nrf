/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>
#include <modem/lte_lc.h>

#include <nrf9160.h>
#include <string.h>
#include <stdlib.h>

#include <nrfx_timer.h>
#include <nrfx_dppi.h>
#include <nrfx_gpiote.h>
#include <hal/nrf_gpiote.h>
#include <hal/nrf_timer.h>

#include <hal/nrf_gpio.h>

const uint32_t counting_pin = 6;

static const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(0);

static void timer_handler(nrf_timer_event_t event_type, void *context)
{
   /* do nothing */
}

static void timer_init(void)
{
	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;
	timer_config.bit_width = TIMER_BITMODE_BITMODE_32Bit;
	timer_config.mode = NRF_TIMER_MODE_LOW_POWER_COUNTER;
	//timer_config.frequency = NRF_TIMER_FREQ_16MHz; // don't care in counter mode. 

	uint32_t err = nrfx_timer_init(&timer, &timer_config, timer_handler);
	if (err != NRFX_SUCCESS) {
		printk("Err :%x\n", err);
	}
	nrfx_timer_enable(&timer);
}

static void dppi_init(void)
{
	uint8_t dppi_ch_1;
	uint32_t err = nrfx_dppi_channel_alloc(&dppi_ch_1);
	if (err != NRFX_SUCCESS) {
		printk("Err %d\n", err);
		return;
	}
	err = nrfx_dppi_channel_enable(dppi_ch_1);
	if (err != NRFX_SUCCESS) {
		printk("Err %d\n", err);
		return;
	}

	/* Tie it all together */
	nrf_gpiote_publish_set(NRF_GPIOTE1, NRF_GPIOTE_EVENT_PORT, dppi_ch_1); /* new name change */

	nrf_timer_subscribe_set(timer.p_reg, NRF_TIMER_TASK_COUNT, dppi_ch_1);

}


static void gpiote_init(void)
{

	nrfx_gpiote_in_config_t pulse_input = { 0 };


	pulse_input.hi_accuracy = false;
	pulse_input.pull = NRF_GPIO_PIN_PULLUP;
    pulse_input.is_watcher = false;
	pulse_input.sense = NRF_GPIOTE_POLARITY_HITOLO;
    pulse_input.skip_gpio_setup = false;

	uint32_t err = nrfx_gpiote_init(0);  /* pass interupt prioritity */
	if (err != NRFX_SUCCESS) {
		printk("gpiote1 err: %x", err);
		return;
	}

	err = nrfx_gpiote_in_init(counting_pin, &pulse_input, NULL);
	if (err != NRFX_SUCCESS) {
		printk("gpiote1 err: %x", err);
		return;
	}
	nrfx_gpiote_in_event_enable(counting_pin, true);
}


void display_pulse_cnt(void)
{
    while(1)
    {
         k_sleep(K_MSEC(10000));
        /* Read out timer CC register and clear it */
        nrf_timer_task_trigger(timer.p_reg, NRF_TIMER_TASK_CAPTURE0);
        uint32_t timer_cc_val = nrf_timer_cc_get(timer.p_reg, 0);   /* change read to get */
		nrfx_timer_clear(&timer);   /**/ /* call this one to clear the counter */
        printk("Raw counter val: %d\n", timer_cc_val); 
    }
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
   
	printk("This sample requires DPPIC to be set as nonsecure\n");

	printk("This sample takes in a signal,"
	       "and uses GPIO in events to count pulse via  DPPIC\n");
	timer_init();
	dppi_init();
	gpiote_init();

    display_pulse_cnt();

}
