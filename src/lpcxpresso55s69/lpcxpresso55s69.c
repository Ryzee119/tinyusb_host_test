/* main.c - Synchronization demo */

/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <sys/printk.h>
#include "tusb.h"

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* thread_a is a dynamic thread that is spawned in main */
void thread_a_entry_point(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	while (1)
	{
		tuh_task();
		k_sleep(K_MSEC(0));
	}
}
K_THREAD_STACK_DEFINE(thread_a_stack_area, STACKSIZE);
static struct k_thread thread_a_data;

void printer(const char *outbuf)
{
	printf("%s", outbuf);
}

#include "fsl_device_registers.h"
#include "fsl_power.h"
#include "fsl_iocon.h"

ISR_DIRECT_DECLARE(urb_irq)
{
	tuh_int_handler(0);
	ISR_DIRECT_PM();
	return 1;
}

#define IOCON_PIO_DIGITAL_EN 0x0100u  /*!<@brief Enables digital function */
#define IOCON_PIO_SLEW_STANDARD 0x00u /*!<@brief Standard mode, output slew rate control is enabled */
void usbh_hw_init()
{
	const uint32_t port0_pin22_config = (/* Pin is configured as USB0_VBUS */
                                         IOCON_FUNC7 |
                                         /* No addition pin function */
                                         IOCON_MODE_INACT |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         0 |
                                         /* Enables digital function */
                                         IOCON_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         0);
    /* PORT0 PIN22 (coords: 78) is configured as USB0_VBUS */
    IOCON_PinMuxSet(IOCON, 0U, 22U, port0_pin22_config);

    const uint32_t port0_pin28_config = (/* Pin is configured as USB0_OVERCURRENTN */
                                         IOCON_FUNC7 |
                                         /* Selects pull-up function */
                                         IOCON_MODE_PULLUP |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         0 |
                                         /* Enables digital function */
                                         IOCON_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         0);
    /* PORT0 PIN28 (coords: 66) is configured as USB0_OVERCURRENTN */
    IOCON_PinMuxSet(IOCON, 0U, 28U, port0_pin28_config);

    const uint32_t port1_pin12_config = (/* Pin is configured as USB0_PORTPWRN */
                                         IOCON_FUNC4 |
                                         /* Selects pull-up function */
                                         IOCON_MODE_PULLUP |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         0 |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         0);
    /* PORT1 PIN12 (coords: 67) is configured as USB0_PORTPWRN */
    IOCON_PinMuxSet(IOCON, 1U, 12U, port1_pin12_config);

	NVIC_ClearPendingIRQ(USB0_IRQn);
	NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
	POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
	RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
	RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
	RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);

	CLOCK_EnableUsbfs0HostClock(kCLOCK_UsbfsSrcPll1, 48000000U);

	IRQ_DIRECT_CONNECT(USB0_IRQn, 2, urb_irq, 0);
   	irq_enable(USB0_IRQn);
}

void hcd_int_enable(uint8_t rhport)
{
	// KfLowerIrql(irql);
	irq_enable(USB0_IRQn);
}

void hcd_int_disable(uint8_t rhport)
{
	// irql = KeRaiseIrqlToDpcLevel();
	irq_disable(USB0_IRQn);
}

int main(void)
{
	usbh_hw_init();
	tusb_init();

	k_thread_create(&thread_a_data, thread_a_stack_area,
					K_THREAD_STACK_SIZEOF(thread_a_stack_area),
					thread_a_entry_point, NULL, NULL, NULL,
					PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&thread_a_data, "thread_a");

	k_thread_start(&thread_a_data);
	k_sleep(K_FOREVER);
	return 0;
}