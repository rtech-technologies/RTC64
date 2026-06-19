/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
/* Modified by Sovereign: Added OSAL configuration macros */
#ifndef USB_CONFIG_H
#define USB_CONFIG_H

#define USBD_DBGPRINTF(...)
#define USBH_DBGPRINTF(...)

#define CONFIG_USB_PRINTF(...)

/* Common configurations */
#define CONFIG_USB_ALIGN_SIZE 4
#define USB_NOCACHE_RAM_SECTION

/* Device configurations */
#define CONFIG_USBDEV_MAX_BUS 1
#define CONFIG_USBDEV_REQUEST_BUFFER_LEN 256

/* Host configurations */
#define CONFIG_USBHOST_MAX_BUS 1
#define CONFIG_USBHOST_MAX_ENDPOINTS 4
#define CONFIG_USBHOST_DEV_NAMELEN 16
#define CONFIG_USBHOST_MAX_INTF_ALTSETTINGS 1
#define CONFIG_USBHOST_MAX_INTERFACES 4
#define CONFIG_USBHOST_MAX_EHPORTS 4
#define CONFIG_USBHOST_REQUEST_BUFFER_LEN 512
#define CONFIG_USBHOST_MAX_EXTHUBS 1
#define CONFIG_USBHOST_CONTROL_TRANSFER_TIMEOUT 500

/* OSAL Configuration */
#define CONFIG_USB_OSAL_THREAD_SET_ARGV void *argument
#define CONFIG_USB_OSAL_THREAD_GET_ARGV ((uintptr_t)argument)
#define USB_OSAL_WAITING_FOREVER (0xFFFFFFFFU)

#endif
