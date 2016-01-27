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
#include <string.h>

#include "usbdrv.h"

#include "board.h"
#include "requests.h"
#include "spi.h"
#include "adf4350.h"

static struct adf_config cfg;
static int wptr;

static void reset_cpu(void)
{
	wdt_disable();

	wdt_enable(WDTO_15MS);

	for (;;);
}

static void write_config(void)
{
	uint32_t regs[6];

	regs[ADF4350_REG0] = ADF4350_REG0_INT(cfg.r0_int) |
		             ADF4350_REG0_FRACT(cfg.r0_fract);

	regs[ADF4350_REG1] = ADF4350_REG1_PHASE(1) |
		             ADF4350_REG1_MOD(cfg.r1_mod) |
			     (cfg.flags & ADF_PRESCALER_8_9 ?
			      ADF4350_REG1_PRESCALER : 0);

	regs[ADF4350_REG2] =
		ADF4350_REG2_10BIT_R_CNT(cfg.r2_r_cnt) |
		ADF4350_REG2_DOUBLE_BUFF_EN |
		(cfg.flags & ADF_RMULT2_EN ? ADF4350_REG2_RMULT2_EN : 0) |
		(cfg.flags & ADF_RDIV2_EN ? ADF4350_REG2_RDIV2_EN : 0) |
		ADF4350_REG2_PD_POLARITY_POS |
		ADF4350_REG2_CHARGE_PUMP_CURR_uA(cfg.r2_charge_pump_current) |
		ADF4350_REG2_MUXOUT(cfg.r2_muxout) |
		ADF4350_REG2_NOISE_MODE(cfg.r2_noise_mode);

	regs[ADF4350_REG3] =
		ADF4350_REG3_12BIT_CLKDIV_MODE(0);

	regs[ADF4350_REG4] =
		ADF4350_REG4_FEEDBACK_FUND |
		ADF4350_REG4_RF_DIV_SEL(cfg.r4_rf_div_sel) |
		ADF4350_REG4_8BIT_BAND_SEL_CLKDIV(cfg.r4_band_sel_div) |
		(cfg.flags & ADF_RF_OUT_EN ?
		 ADF4350_REG4_RF_OUT_EN : 0) |
		(cfg.flags & ADF_MUTE_TILL_LOCK_EN ?
		 ADF4350_REG4_MUTE_TILL_LOCK_EN : 0) |
		ADF4350_REG4_OUTPUT_PWR(cfg.r4_output_power);

	regs[ADF4350_REG5] = ADF4350_REG5_LD_PIN_MODE_DIGITAL;

	int8_t i;
	for (i = 5; i >= 0; i--) {
		adf_cs_l();
		spi_io((regs[i] >> 24) & 0xff);
		spi_io((regs[i] >> 16) & 0xff);
		spi_io((regs[i] >> 8) & 0xff);
		spi_io((regs[i] & 0xff) | i);
		adf_cs_h();
	}
	adf_pdbrf_h();
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
	uint8_t *x = (uint8_t *)&cfg;

	memcpy(x + wptr, data, len);
	wptr += len;

	if (wptr == sizeof(cfg)) {
		write_config();
		return 1;
	}

	return 0;
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	struct usbRequest *rq = (void *)data;
	static uint8_t ld;

	switch (rq->bRequest) {
	case CUSTOM_RQ_SEND_CONFIG:
		wptr = 0;
		/* handle the write in usbFunctionWrite */
		return USB_NO_MSG;
	case CUSTOM_RQ_GET_LD:
		ld = adf_ld();
		usbMsgPtr = &ld;
		return sizeof(ld);
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
