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

#include "librfsynth.h"

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

	while ((opt = getopt(argc, argv, "hRf:p:mde")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
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

	if (usbOpenDevice(&handle, 0, NULL, 0, RFSYNTH_PRODUCT, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", RFSYNTH_PRODUCT);
		exit(1);
	}

	if (reset) {
		rfsynth_send_reset(handle);
		return 0;
	}

	if (freq) {
		printf("Setting frequency to %f MHz\n", freq / 1000000.0);

		st.clkin = RFSYNTH_CLKIN;
		st.chspc = 10000;
		rfsynth_set_freq(&st, freq);
		rfsynth_set_cfg(&cfg, &st);

		cfg.r4_output_power = power & 0x3;

		rfsynth_send_config(handle, &cfg);
	}

	if (rfsynth_get_ld(handle))
		printf("VCO locked\n");
	else
		printf("VCO NOT locked\n");

	usb_close(handle);

	return 0;
}
