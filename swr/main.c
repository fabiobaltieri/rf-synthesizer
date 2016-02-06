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
#include "../commandline/librfsynth.h"
#include "../../rf-power-detector/commandline/librfpower.h"

#define to_mhz_f(a) (a / 1000000.0)

static int verbose = 0;
static usb_dev_handle *rfsynth_handle = NULL;
static usb_dev_handle *rfpower_handle = NULL;
static struct adf_state st;
static struct adf_config cfg;

static void set_frequency(unsigned long long freq)
{
	rfsynth_set_freq(&st, freq);
	rfsynth_set_cfg(&cfg, &st);
	rfsynth_send_config(rfsynth_handle, &cfg);
}

static void sweep(unsigned long long start, unsigned long stop, unsigned steps)
{
	unsigned long freq;
	unsigned int i;
	float dbm;

	printf("Sweep: %f MHz to %f MHz in %d steps\n",
			to_mhz_f(start), to_mhz_f(stop), steps);

	for (i = 0; i < steps; i++) {
		freq = start + (stop - start) * i / steps;

		printf("sample: %f", to_mhz_f(freq));

		set_frequency(freq);
		printf(", set");

		do {
			usleep(10000);
		} while (!rfsynth_get_ld(rfsynth_handle));
		printf(", lock");

		dbm = rfpower_get_dbm(rfpower_handle);
		printf(", dbm: %f", dbm);

		printf("\n");
	}

}

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s -h\n", name);
	fprintf(stderr, "       %s -R\n", name);
	fprintf(stderr, "       %s [options]\n", name);
	fprintf(stderr, "options:\n"
			"  -h            this help\n"
			"TODO\n"
			);
	exit(1);
}

int main(int argc, char **argv)
{
	int opt;

	unsigned long long from = 350000000;
	unsigned long long to = 550000000;
	unsigned int steps = 100;

	memset(&st, 0, sizeof(st));
	memset(&cfg, 0, sizeof(cfg));
	st.clkin = RFSYNTH_CLKIN;
	st.chspc = 10000;
	cfg.flags |= ADF_RF_OUT_EN;

	usb_init();

	while ((opt = getopt(argc, argv, "hvf:t:s:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			break;
		case 'v':
			verbose++;
			break;
		case 'f':
			from = strtoll(optarg, NULL, 0);
			break;
		case 't':
			to = strtoll(optarg, NULL, 0);
			break;
		case 's':
			steps = strtoll(optarg, NULL, 0);
			break;
		default:
			usage(argv[0]);
		}
	}

	printf("Connecting to %s\n", RFSYNTH_PRODUCT);
	if (usbOpenDevice(&rfsynth_handle, 0, NULL, 0, RFSYNTH_PRODUCT, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", RFSYNTH_PRODUCT);
		exit(1);
	}

	printf("Connecting to %s\n", RFPOWER_PRODUCT);
	if (usbOpenDevice(&rfpower_handle, 0, NULL, 0, RFPOWER_PRODUCT, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", RFPOWER_PRODUCT);
		exit(1);
	}

	printf("Normalizing...\n");
	sweep(from, to, steps);
	printf("Sweeping...\n");

	printf("Disable the output...\n");
	cfg.flags &= ~ADF_RF_OUT_EN;
	set_frequency(to);

	usb_close(rfpower_handle);
	usb_close(rfsynth_handle);

	return 0;
}
