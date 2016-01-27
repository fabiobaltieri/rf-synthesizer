#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include <math.h>

#include <usb.h>

#include "opendevice.h"

#include "../firmware/requests.h"
#include "../firmware/adf4350.h"

#define PRODUCT "rf-synthesizer"
#define CLKIN 10000000

static int verbose = 0;

struct adf_state {
	unsigned long clkin;
	unsigned      ref_div_factor;
	unsigned      ref_doubler_en;
	unsigned      ref_div2_en;

	unsigned long chspc; /* Channel Spacing */
	unsigned long fpfd; /* Phase Frequency Detector */
	unsigned long min_out_freq;
	unsigned      r0_fract;
	unsigned      r0_int;
	unsigned      r1_mod;
	unsigned      r1_prescaler;
	unsigned      r2_r_cnt;
	unsigned      r4_rf_div_sel;
	unsigned      r4_band_sel_div;
};

static unsigned long do_div(unsigned long long *n, unsigned long base)
{
	lldiv_t ret;

	ret = lldiv(*n, base);

	*n = ret.quot;
	return ret.rem;
}

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

static unsigned long gcd(unsigned long a, unsigned long b)
{
        unsigned long r;

        if (a < b) {
		/* swap */
		r = a;
		a = b;
		b = r;
	}

        if (!b)
                return a;
        while ((r = a % b) != 0) {
                a = b;
                b = r;
        }
        return b;
}

static int adf_tune_r_cnt(struct adf_state *st, unsigned short r_cnt)
{
        do {
                r_cnt++;
                st->fpfd = (st->clkin * (st->ref_doubler_en ? 2 : 1)) /
                           (r_cnt * (st->ref_div2_en ? 2 : 1));
        } while (st->fpfd > ADF4350_MAX_FREQ_PFD);

        return r_cnt;
}

static int set_freq(struct adf_state *st, unsigned long long freq)
{
        unsigned long long tmp;
        unsigned long div_gcd, prescaler, chspc;
        unsigned short mdiv, r_cnt = 0;
        unsigned char band_sel_div;

        if (freq > ADF4350_MAX_OUT_FREQ || freq < st->min_out_freq)
                return -1;

        if (freq > ADF4350_MAX_FREQ_45_PRESC) {
                prescaler = ADF4350_REG1_PRESCALER;
                mdiv = 75;
        } else {
                prescaler = 0;
                mdiv = 23;
        }

        st->r4_rf_div_sel = 0;

        while (freq < ADF4350_MIN_VCO_FREQ) {
                freq <<= 1;
                st->r4_rf_div_sel++;
        }

        /*
         * Allow a predefined reference division factor
         * if not set, compute our own
         */
        if (st->ref_div_factor)
                r_cnt = st->ref_div_factor - 1;

        chspc = st->chspc;

        do  {
                do {
                        do {
                                r_cnt = adf_tune_r_cnt(st, r_cnt);
                                st->r1_mod = st->fpfd / chspc;
                                if (r_cnt > ADF4350_MAX_R_CNT) {
                                        /* try higher spacing values */
                                        chspc++;
                                        r_cnt = 0;
                                }
                        } while ((st->r1_mod > ADF4350_MAX_MODULUS) && r_cnt);
                } while (r_cnt == 0);

                tmp = freq * (unsigned long long)st->r1_mod + (st->fpfd >> 1);
                do_div(&tmp, st->fpfd); /* Div round closest (n + d/2)/d */
                st->r0_fract = do_div(&tmp, st->r1_mod);
                st->r0_int = tmp;
        } while (mdiv > st->r0_int);

        band_sel_div = DIV_ROUND_UP(st->fpfd, ADF4350_MAX_BANDSEL_CLK);

        if (st->r0_fract && st->r1_mod) {
                div_gcd = gcd(st->r1_mod, st->r0_fract);
                st->r1_mod /= div_gcd;
                st->r0_fract /= div_gcd;
        } else {
                st->r0_fract = 0;
                st->r1_mod = 1;
        }

	if (verbose)
		printf("VCO: %llu Hz, PFD %lu Hz\n"
		       "REF_DIV %d, R0_INT %d, R0_FRACT %d\n"
		       "R1_MOD %d, RF_DIV %d\nPRESCALER %s, BAND_SEL_DIV %d\n",
		       freq, st->fpfd, r_cnt, st->r0_int, st->r0_fract, st->r1_mod,
		       1 << st->r4_rf_div_sel, prescaler ? "8/9" : "4/5",
		       band_sel_div);

	st->r1_prescaler = prescaler;
	st->r2_r_cnt = r_cnt;
	st->r4_band_sel_div = band_sel_div;

	return 0;
}

static void send_config(usb_dev_handle *handle, struct adf_config *cfg)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_OUT,
			      CUSTOM_RQ_SEND_CONFIG,
			      0, 0, (char *)cfg, sizeof(*cfg), 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}
}

static int get_ld(usb_dev_handle *handle)
{
	int ret;
	char data;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_GET_LD,
			      0, 0, &data, sizeof(data), 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}

	return data;
}

static void send_reset(usb_dev_handle *handle)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_RESET,
			      0, 0, NULL, 0, 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}
}

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s -h\n", name);
	fprintf(stderr, "       %s -R\n", name);
	fprintf(stderr, "       %s [options]\n", name);
	fprintf(stderr, "options:\n"
			"  -h            this help\n"
			"  -R            reset device\n"
			"  -f frequency  the signal frequency\n"
			"  -p power      signal power, 0 to 3\n"
			"  -m            x2 multiplier\n"
			"  -d            /2 divider\n"
			"  -e            enable the RF output\n"
			);
	exit(1);
}

int main(int argc, char **argv)
{
	usb_dev_handle *handle = NULL;
	int opt;
	struct adf_state st;
	struct adf_config cfg;

	int reset = 0;
	unsigned long long freq = 0;
	unsigned int power = 3;

	memset(&st, 0, sizeof(st));
	memset(&cfg, 0, sizeof(cfg));

	usb_init();

	while ((opt = getopt(argc, argv, "hvRf:p:mde")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			break;
		case 'v':
			verbose++;
			break;
		case 'R':
			reset = 1;
			break;
		case 'f':
			freq = strtoll(optarg, NULL, 0);
			break;
		case 'p':
			power = strtol(optarg, NULL, 0);
			break;
		case 'm':
			cfg.flags |= ADF_RMULT2_EN;
			break;
		case 'd':
			cfg.flags |= ADF_RDIV2_EN;
			break;
		case 'e':
			cfg.flags |= ADF_RF_OUT_EN;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (usbOpenDevice(&handle, 0, NULL, 0, PRODUCT, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", PRODUCT);
		exit(1);
	}

	if (reset) {
		send_reset(handle);
		return 0;
	}

	if (freq) {
		printf("Setting frequency to %f MHz\n", freq / 1000000.0);

		st.clkin = CLKIN;
		st.chspc = 10000;
		st.min_out_freq = ADF4351_MIN_OUT_FREQ;
		set_freq(&st, freq);

		/* Populate cfg with the final state. */
		cfg.r0_fract = st.r0_fract;
		cfg.r0_int = st.r0_int;
		cfg.r1_mod = st.r1_mod;
		cfg.r2_r_cnt = st.r2_r_cnt;
		cfg.r2_charge_pump_current = 0x7;
		cfg.r2_muxout = 0x0;
		cfg.r2_noise_mode = 0x3;
		cfg.r4_rf_div_sel = st.r4_rf_div_sel;
		cfg.r4_band_sel_div = st.r4_band_sel_div;
		cfg.r4_output_power = power & 0x3;

		if (st.r1_prescaler)
			cfg.flags |= ADF_PRESCALER_8_9;

		cfg.flags |= ADF_MUTE_TILL_LOCK_EN;

		send_config(handle, &cfg);
	}

	if (get_ld(handle))
		printf("VCO locked\n");
	else
		printf("VCO NOT locked\n");

	usb_close(handle);

	return 0;
}
