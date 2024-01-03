#include <Arduino.h>
#include<stdarg.h>
#include "tusb.h"

#define DEBUG_MAX_LINES 16
#define DEBUG_MAX_LEN 128
static char outbuf[32][DEBUG_MAX_LEN];

void usbh_hw_init();

void setup()
{
    Serial1.begin(115200);
    usbh_hw_init();
    tusb_init();
}

void loop()
{
    tuh_task();
    for (int i = 0; i < DEBUG_MAX_LINES; i++)
    {
        if (outbuf[i][0] != 0)
        {
            Serial1.print(outbuf[i]);
            Serial1.flush();
            outbuf[i][0] = 0;
        }
    }
}


#ifdef CFG_TUSB_DEBUG_PRINTF
extern "C" int CFG_TUSB_DEBUG_PRINTF(const char *format, ...)
{
    //This can be in IRQ context. Store message and output to user later
    va_list args;
    va_start(args, format);
    for (int i = 0; i < DEBUG_MAX_LINES; i++)
    {
        if (outbuf[i][0] == 0)
        {
            vsnprintf(outbuf[i], DEBUG_MAX_LEN, format, args);
            break;
        }
    }
    return 1;
}
#endif

void USB_OTG2_IRQHandler(void)
{
    tuh_int_handler(1);
}

void usbh_hw_init()
{
    // Teensy 4.0 PLL & USB PHY powerup
    while (1)
    {
        uint32_t n = CCM_ANALOG_PLL_USB2;
        if (n & CCM_ANALOG_PLL_USB2_DIV_SELECT)
        {
            CCM_ANALOG_PLL_USB2_CLR = 0xC000; // get out of 528 MHz mode
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_BYPASS;
            CCM_ANALOG_PLL_USB2_CLR = CCM_ANALOG_PLL_USB2_POWER |
                                      CCM_ANALOG_PLL_USB2_DIV_SELECT |
                                      CCM_ANALOG_PLL_USB2_ENABLE |
                                      CCM_ANALOG_PLL_USB2_EN_USB_CLKS;
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_ENABLE))
        {
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_ENABLE; // enable
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_POWER))
        {
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_POWER; // power up
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_LOCK))
        {
            continue; // wait for lock
        }
        if (n & CCM_ANALOG_PLL_USB2_BYPASS)
        {
            CCM_ANALOG_PLL_USB2_CLR = CCM_ANALOG_PLL_USB2_BYPASS; // turn off bypass
            continue;
        }
        if (!(n & CCM_ANALOG_PLL_USB2_EN_USB_CLKS))
        {
            CCM_ANALOG_PLL_USB2_SET = CCM_ANALOG_PLL_USB2_EN_USB_CLKS; // enable
            continue;
        }
        break; // USB2 PLL up and running
    }
    // turn on USB clocks (should already be on)
    CCM_CCGR6 |= CCM_CCGR6_USBOH3(CCM_CCGR_ON);
    // turn on USB2 PHY
    USBPHY2_CTRL_CLR = USBPHY_CTRL_SFTRST | USBPHY_CTRL_CLKGATE;
    USBPHY2_CTRL_SET = USBPHY_CTRL_ENUTMILEVEL2 | USBPHY_CTRL_ENUTMILEVEL3;
    USBPHY2_PWD = 0;

#ifdef ARDUINO_TEENSY41
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 = 5;
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_40 = 0x0008; // slow speed, weak 150 ohm drive
    GPIO8_GDIR |= 1 << 26;
    GPIO8_DR_SET = 1 << 26;
#endif
    attachInterruptVector(IRQ_USB2, USB_OTG2_IRQHandler);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    TU_LOG1("Got data: ");
    TU_LOG1_MEM(report, len, 4);
    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    TU_LOG1("HID UNMOUNTED %02x %d\n", dev_addr, instance);
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
{
    TU_LOG1("HID MOUNTED %02x %d\n", dev_addr, instance);
}
