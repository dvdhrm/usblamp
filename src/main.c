/*
 * usblamp - user interface
 * Written 2011 by David Herrmann <dh.herrmann@googlemail.com>
 * Dedicated to the Public Domain
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver.h"

#define ERGB_MAKE(error, red, green, blue) (((error) << 24) | ((red) << 16) | ((green) << 8) | (blue))
#define ERGB_ERROR(ergb) (((ergb) >> 24) & 0xff)
#define ERGB_RED(ergb) (((ergb) >> 16) & 0xff)
#define ERGB_GREEN(ergb) (((ergb) >> 8) & 0xff)
#define ERGB_BLUE(ergb) ((ergb) & 0xff)

static uint32_t parse_args(int argc, char **argv, size_t *id)
{
	uint32_t ret;
	uint8_t red, green, blue;
	int dev = 0;

	ret = ERGB_MAKE(0xff, 0x0, 0x0, 0x0);

	if (argc > 3) {
		if (argc > 4)
			dev = 4;
		red = strtol(argv[1], NULL, 16);
		green = strtol(argv[2], NULL, 16);
		blue = strtol(argv[3], NULL, 16);
		ret = ERGB_MAKE(0x0, red, green, blue);
	}
	else if (argc > 1) {
		if (argc > 2)
			dev = 2;
		if (0 == strcmp(argv[1], "off")) {
			ret = ERGB_MAKE(0x0, 0x0, 0x0, 0x0);
		}
		else if (0 == strcmp(argv[1], "white")) {
			ret = ERGB_MAKE(0x0, 0xff, 0xff, 0xff);
		}
		else if (0 == strcmp(argv[1], "red")) {
			ret = ERGB_MAKE(0x0, 0xff, 0x00, 0x00);
		}
		else if (0 == strcmp(argv[1], "green")) {
			ret = ERGB_MAKE(0x0, 0x00, 0xff, 0x00);
		}
		else if (0 == strcmp(argv[1], "blue")) {
			ret = ERGB_MAKE(0x0, 0x00, 0x00, 0xff);
		}
		else if (0 == strcmp(argv[1], "yellow")) {
			ret = ERGB_MAKE(0x0, 0xff, 0xff, 0x00);
		}
		else if (0 == strcmp(argv[1], "magenta")) {
			ret = ERGB_MAKE(0x0, 0xff, 0x00, 0xff);
		}
		else if (0 == strcmp(argv[1], "cyan")) {
			ret = ERGB_MAKE(0x0, 0x00, 0xff, 0xff);
		}
		else {
			fprintf(stderr, "Invalid arguments\n");
			ret = ERGB_MAKE(0xff, 0x0, 0x0, 0x0);
		}
	}

	if (dev) {
		if (0 == strcmp(argv[dev], "all"))
			*id = 0;
		else
			*id = atoi(argv[dev]);
	}
	else {
		*id = 0;
	}

	if (ERGB_ERROR(ret)) {
		fprintf(stderr, "Usage: usblamp [color] [dev]\n");
		fprintf(stderr, "       usblamp [#red] [#green] [#blue] [dev]\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Written by David Herrmann, 2011\n");
		fprintf(stderr, "Dedicated to the Public Domain\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "[dev] is an integer which specifies the device to operate on.\n");
		fprintf(stderr, "If it is 0 or 'all', then the operation is performed on all devices.\n");
		fprintf(stderr, "If the device with this id is no longer available, nothing is done.\n");
		fprintf(stderr, "Device numbers start at 1, not 0!\n");
		fprintf(stderr, "[dev] defaults to 0 if not given\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "[#red], [#green] and [#blue] are interpreted as\n");
		fprintf(stderr, "hexadecimal values and converted into a 24bit RGB value.\n");
		fprintf(stderr, "Additional arguments are ignored.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "[color] is converted into RGB with:\n");
		fprintf(stderr, "  'off'     => 00 00 00 (default)\n");
		fprintf(stderr, "  'white'   => ff ff ff\n");
		fprintf(stderr, "  'red'     => ff 00 00\n");
		fprintf(stderr, "  'green'   => 00 ff 00\n");
		fprintf(stderr, "  'blue'    => 00 00 ff\n");
		fprintf(stderr, "  'yellow'  => ff ff 00\n");
		fprintf(stderr, "  'magenta' => ff 00 ff\n");
		fprintf(stderr, "  'cyan'    => 00 ff ff\n");
	}
	return ret;
}

static int handle_dev(struct usblamp_dev *dev, size_t index, uint32_t ergb)
{
	if (0 != usblamp_choose(dev, index, false)) {
		fprintf(stderr, "Cannot open device\n");
		return EXIT_FAILURE;
	}
	if (0 != usblamp_init(dev)) {
		fprintf(stderr, "Cannot initialize device\n");
		return EXIT_FAILURE;
	}
	if (0 != usblamp_set(dev, ERGB_RED(ergb), ERGB_GREEN(ergb), ERGB_BLUE(ergb))) {
		fprintf(stderr, "Cannot configure device\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	struct usblamp_dev *dev;
	size_t i, num, id, matches;
	uint32_t ergb;
	int ret = EXIT_SUCCESS;

	ergb = parse_args(argc, argv, &id);
	if (ERGB_ERROR(ergb))
		return EXIT_FAILURE;

	dev = usblamp_open();
	if (!dev) {
		fprintf(stderr, "Cannot open usb driver\n");
		return EXIT_FAILURE;
	}

	matches = 0;
	num = usblamp_devnum(dev);
	for (i = 0; i < num; ++i) {
		if (usblamp_test(dev, i)) {
			++matches;
			if (!id || matches == id) {
				if (handle_dev(dev, i, ergb) == EXIT_FAILURE)
					ret = EXIT_FAILURE;
				if (id)
					break;
			}
		}
	}

	usblamp_close(dev);

	return ret;
}
