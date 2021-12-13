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

#include <nrfx_timer.h>
#include <hal/nrf_timer.h>

#include <logging/log.h>

#include <modem/lte_lc.h>

#include <nrf9160.h>

LOG_MODULE_REGISTER(nrfx_sample, LOG_LEVEL_INF);

#define INPUT_PIN	DT_GPIO_PIN(DT_ALIAS(sw0), gpios)
//#define OUTPUT_PIN	DT_GPIO_PIN(DT_ALIAS(led0), gpios)
#define COUNTER DT_TIMER

#define USE_PORT_EVENT 0


static const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(0);

uint32_t timer_handler_cnt = 0;
static void timer_handler(nrf_timer_event_t event_type, void *context)
{
   /* do nothing, this won't be called anyway */
   ++timer_handler_cnt;
}
static void timer_init(void)
{
	nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;
	timer_config.bit_width = TIMER_BITMODE_BITMODE_32Bit;
	timer_config.mode = NRF_TIMER_MODE_LOW_POWER_COUNTER;
	//timer_config.frequency = NRF_TIMER_FREQ_16MHz; // don't care in counter mode. 

	uint32_t err = nrfx_timer_init(&timer, &timer_config, timer_handler);
	if (err != NRFX_SUCCESS) {
		LOG_INF("Err :%x\n", err);
	}
	nrfx_timer_enable(&timer);
}
#if 0 /*  comment out to disable interrupt handler */
uint32_t g_count = 0;

static void button_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	LOG_INF("GPIO input event callback %d\n", ++g_count);
}
#endif


void display_pulse_cnt(void)
{
    while(1)
    {
         k_sleep(K_MSEC(10000));
        /* Read out timer CC register and clear it */
        nrf_timer_task_trigger(timer.p_reg, NRF_TIMER_TASK_CAPTURE0);
        uint32_t timer_cc_val = nrf_timer_cc_get(timer.p_reg, 0);   /* change read to get */
		nrfx_timer_clear(&timer);   /**/ /* call this one to clear the counter */
        if(USE_PORT_EVENT){
            timer_cc_val = timer_cc_val/2;
        }
        printk("Raw counter val: %d\n", timer_cc_val); 
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
	LOG_INF("Just switch Modem Off\n"); /* should output to RTT*/
	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_OFFLINE);
	if(err < 0 ){
		LOG_INF("lte_lc_func_mode_set error %d\n", err);
	}

  
	/* Connect GPIOTE_0 IRQ to nrfx_gpiote_irq_handler */
	IRQ_CONNECT(DT_IRQN(DT_NODELABEL(gpiote)),
		    DT_IRQ(DT_NODELABEL(gpiote), priority),
		    nrfx_isr, nrfx_gpiote_irq_handler, 0);

	/* Initialize GPIOTE (the interrupt priority passed as the parameter
	 * here is ignored, see nrfx_glue.h).
	 */
    timer_init(); /* timer as counter */
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
	err = nrfx_gpiote_in_init(INPUT_PIN, &in_config, NULL); /* replace NULL to button_handler, if require interrupt handler as well. */
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_in_init error: %08x", err);
		return;
	}
#if 0
	nrfx_gpiote_out_config_t const out_config = {
		.action = NRF_GPIOTE_POLARITY_TOGGLE,
		.init_state = 1,
		.task_pin = true,
	};

	/* Initialize output pin. SET task will turn the LED on,
	 * CLR will turn it off and OUT will toggle it.
	 */
	err = nrfx_gpiote_out_init(OUTPUT_PIN, &out_config);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
		return;
	}
#endif
	nrfx_gpiote_in_event_enable(INPUT_PIN, true);
	//nrfx_gpiote_out_task_enable(OUTPUT_PIN);
    

	LOG_INF("nrfx_gpiote initialized");

	/* Allocate a (D)PPI channel. */
#if defined(DPPI_PRESENT)
	uint8_t channel;
	err = nrfx_dppi_channel_alloc(&channel);
#else
	nrf_ppi_channel_t channel;
	err = nrfx_ppi_channel_alloc(&channel);
#endif
	if (err != NRFX_SUCCESS) {
		LOG_ERR("(D)PPI channel allocation error: %08x", err);
		return;
	}
    nrf_timer_subscribe_set(timer.p_reg, NRF_TIMER_TASK_COUNT, channel);

	/* Configure endpoints of the channel so that the input pin event is
	 * connected with the output pin OUT task. This means that each time
	 * the button is pressed, the LED pin will be toggled.
	 */
	nrfx_gppi_channel_endpoints_setup(channel,
#if USE_PORT_EVENT
		nrf_gpiote_event_address_get(NRF_GPIOTE, NRF_GPIOTE_EVENT_PORT),
#else
        nrf_gpiote_event_address_get(NRF_GPIOTE, nrfx_gpiote_in_event_get(INPUT_PIN)),
#endif
        nrf_timer_task_address_get(NRF_TIMER0, NRF_TIMER_TASK_COUNT));
		//nrf_gpiote_task_address_get(NRF_GPIOTE, nrfx_gpiote_in_event_get(OUTPUT_PIN)));

	/* Enable (D)PPI channel. */
#if defined(DPPI_PRESENT)
	err = nrfx_dppi_channel_enable(channel);
#else
	err = nrfx_ppi_channel_enable(channel);
#endif
	if (err != NRFX_SUCCESS) {
		LOG_ERR("Failed to enable (D)PPI channel, error: %08x", err);
		return;
	}

	LOG_INF("(D)PPI configured, leaving main()");

    display_pulse_cnt();

}
