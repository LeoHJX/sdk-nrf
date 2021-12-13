/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <nrfx_gpiote.h>
#include <helpers/nrfx_gppi.h>
#if defined(DPPI_PRESENT)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
#endif

#include <logging/log.h>

#include <modem/lte_lc.h>

LOG_MODULE_REGISTER(nrfx_sample, LOG_LEVEL_INF);

#define INPUT_PIN	DT_GPIO_PIN(DT_ALIAS(sw0), gpios)
//#define OUTPUT_PIN	DT_GPIO_PIN(DT_ALIAS(led0), gpios)

#define USE_PORT_EVENT 1

uint32_t g_count = 0;

static inline void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	++ g_count;
}
void display_pulse_cnt(void)
{
    uint32_t tmp;
    while(1)
    {
        k_sleep(K_MSEC(10000));
        tmp = g_count;
        g_count = 0;
        LOG_INF("counter val: %d\n", tmp);
    }
}
void main(void)
{
	LOG_INF("nrfx_gpiote sample on %s", CONFIG_BOARD);

	nrfx_err_t err;

#if 0   
	modem_init();
#endif
	lte_lc_func_mode_set(LTE_LC_FUNC_MODE_NORMAL);
	printk("Just switch Modem Off\n"); /* should output to RTT*/
	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_OFFLINE);
	if(err < 0 ){
		printk("lte_lc_func_mode_set error %d\n", err);
	}

	/* Connect GPIOTE_0 IRQ to nrfx_gpiote_irq_handler */
	IRQ_DIRECT_CONNECT(DT_IRQN(DT_NODELABEL(gpiote)),
		    0 /* DT_IRQ(DT_NODELABEL(gpiote), priority) */,
		    nrfx_gpiote_irq_handler, 0);

	/* Initialize GPIOTE (the interrupt priority passed as the parameter
	 * here is ignored, see nrfx_glue.h).
	 */
	err = nrfx_gpiote_init(0);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_init error: %08x", err);
		return;
	}

	nrfx_gpiote_in_config_t const in_config = {
		.sense = NRF_GPIOTE_POLARITY_HITOLO,
		.pull = NRF_GPIO_PIN_PULLUP,
		.is_watcher = false,
#if USE_PORT_EVENT
		.hi_accuracy = false,
#else
        .hi_accuracy = true,
#endif
		.skip_gpio_setup = false,
	};

	/* Initialize input pin to generate event on high to low transition
	 * (falling edge) and call button_handler()
	 */
	err = nrfx_gpiote_in_init(INPUT_PIN, &in_config, button_handler);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_in_init error: %08x", err);
		return;
	}
	nrfx_gpiote_in_event_enable(INPUT_PIN, true);
	
	LOG_INF("nrfx_gpiote initialized");
	display_pulse_cnt();
}
