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
#include <signal.h>

#include <usb.h>

#include "opendevice.h"

#include "../firmware/requests.h"
#include "../firmware/adf4350.h"
#include "../commandline/librfsynth.h"
#include "../../rf-power-detector/commandline/librfpower.h"

#define GNUPLOT "gnuplot"

#define to_mhz_f(a) (a / 1000000.0)

static int verbose = 0;
static usb_dev_handle *rfsynth_handle = NULL;
static usb_dev_handle *rfpower_handle = NULL;
static struct adf_state st;
static struct adf_config cfg;
static int loop = 1;

struct sample {
	float freq;
	float power;
};

static void set_frequency(unsigned long long freq)
{
	rfsynth_set_freq(&st, freq);
	rfsynth_set_cfg(&cfg, &st);
	rfsynth_send_config(rfsynth_handle, &cfg);
}

static void sweep(struct sample *samples, unsigned long long start, unsigned long stop, unsigned steps)
{
	unsigned long freq;
	unsigned int i;
	float dbm;

	for (i = 0; i < steps; i++) {
		freq = start + (stop - start) * i / steps;

		printf("sample %4d/%4d: %f", i, steps, to_mhz_f(freq));

		set_frequency(freq);
		printf(", set");

		do {
			usleep(15000);
			printf(", lock");
		} while (!rfsynth_get_ld(rfsynth_handle));

		dbm = rfpower_get_dbm(rfpower_handle);
		printf(", dbm: %f", dbm);

		printf("\r");
		fflush(stdout);

		samples[i].freq = freq;
		samples[i].power = dbm;
	}

	printf("\n");
}

static void dump(struct sample *samples, struct sample *reference, unsigned steps, char *fname)
{
	FILE *fd;
	unsigned int i;
	float freq;
	float power;

	fd = fopen(fname, "w");
	if (!fd) {
		perror("cannot open the output file");
		exit(1);
	}

	for (i = 0; i < steps; i++) {
		freq = samples[i].freq;
		power = samples[i].power;
		if (reference)
			power -= reference[i].power;
		fprintf(fd, "%f %f\n", freq, power);
	}

	fclose(fd);
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

static void plot_init(FILE *gp)
{
	if (!gp)
		return;

	fprintf(gp, "set title 'frequency vs power'\n");
	fprintf(gp, "set xlabel 'frequency [Hz]'\n");
	fprintf(gp, "set ylabel 'power [dB]'\n");
	fprintf(gp, "set format x '%%.0s%%c'\n");
}

static void plot_refresh(FILE *gp, char *fname)
{
	if (!gp)
		return;

	fprintf(gp, "plot '%s' with lines\n", fname);
	fflush(gp);
}

static void bailout(int signo)
{
	loop = 0;
	signal(SIGINT, SIG_DFL);
}

int main(int argc, char **argv)
{
	int opt;

	unsigned long long from = 350000000;
	unsigned long long to = 550000000;
	unsigned int steps = 100;
	char *outfile = "out.dat";
	int plot = 0;
	static FILE *gp = NULL;
	int normalize = 0;

	struct sample *samples;
	struct sample *reference = NULL;

	memset(&st, 0, sizeof(st));
	memset(&cfg, 0, sizeof(cfg));
	st.clkin = RFSYNTH_CLKIN;
	st.chspc = 10000;
	cfg.flags |= ADF_RF_OUT_EN;

	usb_init();

	while ((opt = getopt(argc, argv, "hvf:t:s:o:pn")) != -1) {
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
		case 'o':
			outfile = optarg;
			break;
		case 'p':
			plot = 1;
			break;
		case 'n':
			normalize = 1;
			break;
		default:
			usage(argv[0]);
		}
	}

	samples = calloc(steps, sizeof(struct sample));
	if (normalize)
		reference = calloc(steps, sizeof(struct sample));

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

	if (plot) {
		printf("Running %s...\n", GNUPLOT);
		gp = popen(GNUPLOT, "w");
		if (!gp) {
			perror("cannot run gnuplot");
			plot = 0;
		}
	}
	plot_init(gp);

	printf("Sweep: %f MHz to %f MHz in %d steps\n",
			to_mhz_f(from), to_mhz_f(to), steps);

	if (normalize) {
		printf("Normalizing...\n");
		sweep(reference, from, to, steps);
		dump(reference, NULL, steps, outfile);
		plot_refresh(gp, outfile);
	}

	signal(SIGINT, bailout);
	while (loop) {
		sweep(samples, from, to, steps);
		dump(samples, reference, steps, outfile);
		plot_refresh(gp, outfile);
	}

	printf("Disable the output...\n");
	cfg.flags &= ~ADF_RF_OUT_EN;
	set_frequency(to);

	free(samples);

	if (gp)
		fclose(gp);
	usb_close(rfpower_handle);
	usb_close(rfsynth_handle);

	return 0;
}
