/*
 * ADF4350/ADF4351 SPI PLL driver
 *
 * Originally from kernel.org:
 *   include/linux/iio/frequency/adf4350.h
 *
 * Copyright 2012-2013 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */

/* Registers */
#define ADF4350_REG0	0
#define ADF4350_REG1	1
#define ADF4350_REG2	2
#define ADF4350_REG3	3
#define ADF4350_REG4	4
#define ADF4350_REG5	5

/* REG0 Bit Definitions */
#define ADF4350_REG0_FRACT(x)			((uint32_t)((x) & 0xFFF) << 3)
#define ADF4350_REG0_INT(x)			((uint32_t)((x) & 0xFFFF) << 15)

/* REG1 Bit Definitions */
#define ADF4350_REG1_MOD(x)			((uint32_t)((x) & 0xFFF) << 3)
#define ADF4350_REG1_PHASE(x)			((uint32_t)((x) & 0xFFF) << 15)
#define ADF4350_REG1_PRESCALER			(1UL << 27)

/* REG2 Bit Definitions */
#define ADF4350_REG2_COUNTER_RESET_EN		(1UL << 3)
#define ADF4350_REG2_CP_THREESTATE_EN		(1UL << 4)
#define ADF4350_REG2_POWER_DOWN_EN		(1UL << 5)
#define ADF4350_REG2_PD_POLARITY_POS		(1UL << 6)
#define ADF4350_REG2_LDP_6ns			(1UL << 7)
#define ADF4350_REG2_LDP_10ns			(0UL << 7)
#define ADF4350_REG2_LDF_FRACT_N		(0UL << 8)
#define ADF4350_REG2_LDF_INT_N			(1UL << 8)
#define ADF4350_REG2_CHARGE_PUMP_CURR_uA(x)	((uint32_t)((((x)-312) / 312) & 0xF) << 9)
#define ADF4350_REG2_DOUBLE_BUFF_EN		(1UL << 13)
#define ADF4350_REG2_10BIT_R_CNT(x)		((uint32_t)(x) << 14)
#define ADF4350_REG2_RDIV2_EN			(1UL << 24)
#define ADF4350_REG2_RMULT2_EN			(1UL << 25)
#define ADF4350_REG2_MUXOUT(x)			((uint32_t)(x) << 26)
#define ADF4350_REG2_NOISE_MODE(x)		(((uint32_t)(x)) << 29)
#define ADF4350_MUXOUT_THREESTATE		0
#define ADF4350_MUXOUT_DVDD			1
#define ADF4350_MUXOUT_GND			2
#define ADF4350_MUXOUT_R_DIV_OUT		3
#define ADF4350_MUXOUT_N_DIV_OUT		4
#define ADF4350_MUXOUT_ANALOG_LOCK_DETECT	5
#define ADF4350_MUXOUT_DIGITAL_LOCK_DETECT	6

/* REG3 Bit Definitions */
#define ADF4350_REG3_12BIT_CLKDIV(x)		((uint32_t)(x) << 3)
#define ADF4350_REG3_12BIT_CLKDIV_MODE(x)	((uint32_t)(x) << 16)
#define ADF4350_REG3_12BIT_CSR_EN		(1UL << 18)
#define ADF4351_REG3_CHARGE_CANCELLATION_EN	(1UL << 21)
#define ADF4351_REG3_ANTI_BACKLASH_3ns_EN	(1UL << 22)
#define ADF4351_REG3_BAND_SEL_CLOCK_MODE_HIGH	(1UL << 23)

/* REG4 Bit Definitions */
#define ADF4350_REG4_OUTPUT_PWR(x)		((uint32_t)(x) << 3)
#define ADF4350_REG4_RF_OUT_EN			(1UL << 5)
#define ADF4350_REG4_AUX_OUTPUT_PWR(x)		((uint32_t)(x) << 6)
#define ADF4350_REG4_AUX_OUTPUT_EN		(1UL << 8)
#define ADF4350_REG4_AUX_OUTPUT_FUND		(1UL << 9)
#define ADF4350_REG4_AUX_OUTPUT_DIV		(0UL << 9)
#define ADF4350_REG4_MUTE_TILL_LOCK_EN		(1UL << 10)
#define ADF4350_REG4_VCO_PWRDOWN_EN		(1UL << 11)
#define ADF4350_REG4_8BIT_BAND_SEL_CLKDIV(x)	((uint32_t)(x) << 12)
#define ADF4350_REG4_RF_DIV_SEL(x)		((uint32_t)(x) << 20)
#define ADF4350_REG4_FEEDBACK_DIVIDED		(0UL << 23)
#define ADF4350_REG4_FEEDBACK_FUND		(1UL << 23)

/* REG5 Bit Definitions */
#define ADF4350_REG5_LD_PIN_MODE_LOW		(0UL << 22)
#define ADF4350_REG5_LD_PIN_MODE_DIGITAL	(1UL << 22)
#define ADF4350_REG5_LD_PIN_MODE_HIGH		(3UL << 22)

/* Specifications */
#define ADF4350_MAX_OUT_FREQ		4400000000ULL /* Hz */
#define ADF4350_MIN_OUT_FREQ		137500000 /* Hz */
#define ADF4351_MIN_OUT_FREQ		34375000 /* Hz */
#define ADF4350_MIN_VCO_FREQ		2200000000ULL /* Hz */
#define ADF4350_MAX_FREQ_45_PRESC	3000000000ULL /* Hz */
#define ADF4350_MAX_FREQ_PFD		32000000 /* Hz */
#define ADF4350_MAX_BANDSEL_CLK		125000 /* Hz */
#define ADF4350_MAX_FREQ_REFIN		250000000 /* Hz */
#define ADF4350_MAX_MODULUS		4095
#define ADF4350_MAX_R_CNT		1023
