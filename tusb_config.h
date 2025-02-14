#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

// Configurações gerais do TinyUSB
#define CFG_TUSB_MCU OPT_MCU_RP2040
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE
#define CFG_TUSB_OS OPT_OS_PICO

// Configurações de memória
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))

// Configurações de dispositivo
#define CFG_TUD_HID 1

// Configurações de HID
#define CFG_TUD_HID_EP_BUFSIZE 64

#endif // _TUSB_CONFIG_H_ 