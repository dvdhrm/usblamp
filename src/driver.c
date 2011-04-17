/*
 * usblamp - driver api
 * Written 2011 by David Herrmann <dh.herrmann@googlemail.com>
 * Dedicated to the Public Domain
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>
#include "driver.h"

struct usblamp_dev {
	struct libusb_context *usb;
	struct libusb_device **devices;
	size_t devnum;
	struct libusb_device_handle *handle;
};

/* three initialization commands (they also switch off the lamp so you should resend the current color immediately) */
static const char initbuf1[] = { 0x1f, 0x02, 0x00, 0x2e, 0x00, 0x00, 0x2b, 0x03 };
static const char initbuf2[] = { 0x00, 0x02, 0x00, 0x2e, 0x00, 0x00, 0x2b, 0x04 };
static const char initbuf3[] = { 0x00, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x2b, 0x05 };
/* command to set RGB color; first three bytes are R, G and B values */
static const char setbuf[]   = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05 };

struct usblamp_dev *usblamp_open()
{
	struct usblamp_dev *dev;

	dev = malloc(sizeof(*dev));
	if (!dev)
		return NULL;

	if (libusb_init(&dev->usb) < 0) {
		free(dev);
		return NULL;
	}

	dev->devnum = libusb_get_device_list(dev->usb, &dev->devices);
	if (dev->devnum < 0) {
		libusb_exit(dev->usb);
		free(dev);
		return NULL;
	}

	dev->handle = NULL;
	return dev;
}

void usblamp_close(struct usblamp_dev *dev)
{
	if (dev->handle) {
		libusb_release_interface(dev->handle, 0);
		libusb_close(dev->handle);
	}
	if (dev->devices)
		libusb_free_device_list(dev->devices, 1);
	libusb_exit(dev->usb);
	free(dev);
}

size_t usblamp_devnum(struct usblamp_dev *dev)
{
	return dev->devnum;
}

int usblamp_choose(struct usblamp_dev *dev, size_t index, bool free)
{
	int ret;

	assert(dev->devices);
	assert(dev->devnum >= index);

	if (dev->handle) {
		libusb_release_interface(dev->handle, 0);
		libusb_close(dev->handle);
		dev->handle = NULL;
	}

	ret = libusb_open(dev->devices[index], &dev->handle);
	if (ret < 0)
		return -1;
	ret = libusb_claim_interface(dev->handle, 0);
	if (ret < 0) {
		libusb_close(dev->handle);
		dev->handle = NULL;
		return -2;
	}

	if (free) {
		libusb_free_device_list(dev->devices, 1);
		dev->devices = NULL;
		dev->devnum = 0;
	}
	return 0;
}

int usblamp_send(struct usblamp_dev *dev, const char *data, size_t size)
{
	uint8_t reqtype = LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE;
	uint8_t request = 0x09;
	uint16_t wvalue = 0x0200;
	uint16_t windex = 0x00;
	unsigned int timeout = 10;
	int ret;
	unsigned char buf[100];

	assert(dev->handle);

	/* copy into static buffer because libusb buffer must not be constant (very strange...) */
	memcpy(buf, data, sizeof(buf));
	if (size > sizeof(buf))
		size = sizeof(buf);

	ret = libusb_control_transfer(dev->handle, reqtype, request, wvalue, windex, buf, size, timeout);
	if (ret != size)
		return -1;
	return 0;
}

int usblamp_init(struct usblamp_dev *dev)
{
	int ret;

	ret = usblamp_send(dev, initbuf1, sizeof(initbuf1));
	if (ret < 0)
		return -1;
	ret = usblamp_send(dev, initbuf2, sizeof(initbuf2));
	if (ret < 0)
		return -2;
	ret = usblamp_send(dev, initbuf3, sizeof(initbuf3));
	if (ret < 0)
		return -3;
	return 0;
}

int usblamp_set(struct usblamp_dev *dev, uint8_t red, uint8_t green, uint8_t blue)
{
	char data[sizeof(setbuf)];
	size_t size;

	size = sizeof(data);
	memcpy(data, setbuf, size);
	data[0] = red;
	data[1] = green;
	data[2] = blue;

	return usblamp_send(dev, data, size);
}

bool usblamp_test(struct usblamp_dev *dev, size_t index)
{
	struct libusb_device_descriptor desc;
	int ret;

	assert(dev->devices);
	assert(index <= dev->devnum);

	ret = libusb_get_device_descriptor(dev->devices[index], &desc);
	if (ret < 0)
		return false;

	if (desc.idVendor == USBLAMP_VENDOR && desc.idProduct == USBLAMP_PRODUCT)
		return true;
	return false;
}
