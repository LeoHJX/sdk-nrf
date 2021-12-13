/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>
#include <modem/lte_lc.h>

#if 0
static void modem_init(void)
{
	int err;

	if (IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT)) {
		/* Do nothing, modem is already configured and LTE connected. */
	} else {
		err = lte_lc_init();
		if (err) {
			printk("Modem initialization failed, error: %d\n", err);
			return;
		}
	}
}
#endif
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


#if 0
	while(1)
	{
		k_cpu_idle();
		k_sleep(K_MSEC(5000));
	}
#endif
	return;
}
