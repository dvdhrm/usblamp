/*
 * usblamp - driver api
 * Written 2011 by David Herrmann <dh.herrmann@googlemail.com>
 * Dedicated to the Public Domain
 */

/*
 * USB Lamp Driver API
 * This is a device driver for the DreamCheeky USB Mail Notifier Lamp.
 * This driver uses libusb to implement the device driver in user space. To use
 * this driver, you first need to create a new usblamp context:
 *
 * Functions returning "int" return 0 on success and a negative number on failure.
 *
 *   usblamp_open()
 * Returns NULL on error (malloc fail or libusb initialization failed).
 * This context can be destroyed at any time with:
 *   usblamp_close(context)
 * After this call, the context is invalid and should not be used anymore.
 *
 * A new context contains a list of all devices that where available at time
 * of function call of usblamp_open(). The function usblamp_devnum() returns
 * the number of available usb devices in that list (this contains ALL usb devices,
 * not only the usblamp devices!) or 0 when the list was destroyed.
 * You can test whether a specific device at index \index is a usblamp device with:
 *   usblamp_test(context, index)
 * It returns true if the device is a usblamp device.
 *
 * You can open a specific device with:
 *   usblamp_choose(context, index, false)
 * The last parameter specifies whether the internal device list should be destroyed.
 * That means, you cannot open another device on this context after that call.
 * If the last parameter is "false", then the device list remains available and you
 * may call usblamp_choose() again on that context, which will close the current device
 * and open the new one. usblamp_close always frees all allocated memory, so it is
 * safe to always pass "false" as third parameter to usblamp_choose.
 * This function may fail if your process has not enough privileges to claim this device.
 *
 * After choosing a device, you need to initialize the device with:
 *   usblamp_init
 * This function initializes the currently choosen device on that context. This function
 * may fail if the device has gone away or closed the pipeline.
 *
 * After initializing the device, you may send raw requests via usblamp_send, which is
 * deprecated. Use the higher level interface usblamp_set() to set the usblamp to the
 * given RGB value.
 *
 * If one of the functions usblamp_send, usblamp_init or usblamp_set return non-0, you
 * should consider this device closed and either close your context with usblamp_close
 * or open another device with usblamp_choose().
 *
 * It is totally safe to use multiple contexts simultaneously, however, a single context
 * should not be worked on with multiple threads simultaneously.
 * Two different threads may work on two different contexts simultaneously, though.
 */

#ifndef USBLAMP_DRIVER_H
#define USBLAMP_DRIVER_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define USBLAMP_VENDOR 0x1d34
#define USBLAMP_PRODUCT 0x0004

struct usblamp_dev;

extern struct usblamp_dev *usblamp_open();
extern void usblamp_close(struct usblamp_dev *dev);

extern size_t usblamp_devnum(struct usblamp_dev *dev);
extern int usblamp_choose(struct usblamp_dev *dev, size_t index, bool free);
extern int usblamp_send(struct usblamp_dev *dev, const char *data, size_t size);
extern int usblamp_init(struct usblamp_dev *dev);
extern int usblamp_set(struct usblamp_dev *dev, uint8_t red, uint8_t green, uint8_t blue);

extern bool usblamp_test(struct usblamp_dev *dev, size_t index);

#endif /* USBLAMP_DRIVER_H */
