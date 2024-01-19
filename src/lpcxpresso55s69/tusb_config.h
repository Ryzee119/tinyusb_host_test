#define CFG_TUH_ENABLED 1
#define CFG_TUH_MAX_SPEED OPT_MODE_FULL_SPEED
#define TUP_USBIP_OHCI
#define TUH_OPT_RHPORT 0

#define CFG_TUH_API_EDPT_XFER 1
#define CFG_TUSB_OS OPT_OS_NONE

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG 0
#endif

#ifndef CFG_TUH_ENUMERATION_BUFSIZE
#define CFG_TUH_ENUMERATION_BUFSIZE 512
#endif

#ifndef CFG_TUH_HUB
#define CFG_TUH_HUB 2
#endif

#define CFG_TUH_HID 16

//Not including USB Hubs
#ifndef CFG_TUH_DEVICE_MAX 
#define CFG_TUH_DEVICE_MAX 6
#endif

//Max number of endpoints pair per device
#ifndef CFG_TUH_ENDPOINT_MAX
#define CFG_TUH_ENDPOINT_MAX   2
#endif

#ifndef TUP_OHCI_RHPORTS
#define TUP_OHCI_RHPORTS 1
#endif

#ifndef CFG_TUH_MEM_SECTION
//#define CFG_TUH_MEM_SECTION __attribute__((section("m_usb_global")))
#endif
