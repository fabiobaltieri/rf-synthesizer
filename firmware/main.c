/*
 * Copyright 2013 Fabio Baltieri (fabio.baltieri@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv.h"

#include "board.h"
#include "requests.h"
#include "spi.h"

static void reset_cpu(void)
{
	wdt_disable();

	wdt_enable(WDTO_15MS);

	for (;;);
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	struct usbRequest *rq = (void *)data;

	switch (rq->bRequest) {
	case CUSTOM_RQ_RESET:
		reset_cpu();
		break;
	default:
		/* do nothing */
		;
	}

	return 0;
}

static void io_init(void)
{
	ADF_PDBRF_DDR |= _BV(ADF_PDBRF);
	adf_pdbrf_l();

	ADF_CE_DDR |= _BV(ADF_CE);
	adf_ce_h();
}

static void ld_poll(void)
{
	if (adf_ld())
		led_b_on();
	else
		led_b_off();
}

static void hello(void)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		led_a_toggle();
		led_b_toggle();
		_delay_ms(50);
	}
}

int __attribute__((noreturn)) main(void)
{
	uint8_t i;

	led_init();
	io_init();

	spi_init();

	wdt_enable(WDTO_1S);

	hello();
	led_a_on();
	led_b_off();

	usbInit();
	usbDeviceDisconnect();

	i = 0;
	while (--i) {
		wdt_reset();
		_delay_ms(1);
	}

	usbDeviceConnect();

	sei();
	for (;;) {
		wdt_reset();
		usbPoll();
		ld_poll();
	}
}
