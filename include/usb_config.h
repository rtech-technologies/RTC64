#ifndef USB_CONFIG_H
#define USB_CONFIG_H

#include <stdio.h>

#define USBD_DBGPRINTF(...)
#define USBH_DBGPRINTF(...)

#define CONFIG_USB_PRINTF printf

/* Device configurations */
#define CONFIG_USBDEV_MAX_BUS 1

/* Host configurations */
#define CONFIG_USBHOST_MAX_BUS 1
#define CONFIG_USBHOST_MAX_ENDPOINTS 4
#define CONFIG_USBHOST_DEV_NAMELEN 16
#define CONFIG_USBHOST_MAX_INTF_ALTSETTINGS 1
#define CONFIG_USBHOST_MAX_INTERFACES 4
#define CONFIG_USBHOST_MAX_EHPORTS 4

#endif
