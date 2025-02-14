#include <stdio.h>
#include <stdlib.h>  // Para função abs()
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "bsp/board.h"
#include "tusb.h"
#include "class/hid/hid.h"
#include "class/hid/hid_device.h"

// Definições dos pinos dos botões
const uint BUTTON_A_PIN = 5;  // Botão A no GPIO 5
const uint BUTTON_B_PIN = 6;  // Botão B no GPIO 6

void setup_buttons() {
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_init(22); // Inicializa o botão SW do joystick
    gpio_set_dir(22, GPIO_IN);
    gpio_pull_up(22);
}

// Variáveis do estado do joystick
int16_t x_axis = 0;
int16_t y_axis = 0;
uint8_t buttons = 0;

// Estrutura do relatório HID
typedef struct {
    int16_t x;
    int16_t y;
    uint8_t buttons;
} __attribute__((packed)) custom_gamepad_report_t;

// Descritor HID para gamepad compatível com Xbox
const uint8_t desc_hid_report[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Gamepad)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    
    // Botões
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x08,        //   Usage Maximum (Button 8)
    0x95, 0x08,        //   Report Count (8)
    0x75, 0x01,        //   Report Size (1)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    // Eixos X e Y
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x16, 0x00, 0x80,  //   Logical Minimum (-32768)
    0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    0xC0               // End Collection
};

// Descritor de dispositivo
uint8_t const desc_device[] = {
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0xC0, 0x16, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02,
    0x01, 0x02, 0x03, 0x01
};

// Função para enviar o estado do controle via USB HID
void send_hid_report() {
    if (!tud_hid_ready()) return;

    custom_gamepad_report_t report = {
        .x = x_axis,
        .y = y_axis,
        .buttons = buttons
    };

    tud_hid_report(0, &report, sizeof(report));
}

// Função para leitura dos botões
void read_buttons() {
    buttons = 0;
    if (gpio_get(BUTTON_A_PIN) == 0) buttons |= (1 << 0); // Mapeado para "Botão A"
    if (gpio_get(BUTTON_B_PIN) == 0) buttons |= (1 << 1); // Mapeado para "Botão B"
    if (gpio_get(22) == 0) buttons |= (1 << 2); // Mapeado para "Botão SW do Joystick"
}

// Função para leitura do joystick analógico com zona morta e calibração
void read_joystick() {
    const int DEADZONE = 3000; // Ajuste conforme necessário
    
    adc_select_input(0);
    uint16_t raw_x = adc_read();
    adc_select_input(1);
    uint16_t raw_y = adc_read();
    
    // Converte para intervalo -32768 a 32767
    x_axis = ((int32_t)raw_x - 2048) * 16;
    y_axis = ((int32_t)raw_y - 2048) * 16;
    
    // Aplica zona morta
    if (abs(x_axis) < DEADZONE) x_axis = 0;
    if (abs(y_axis) < DEADZONE) y_axis = 0;

    // Evita que o joystick seja tratado como botão
    if (x_axis != 0 || y_axis != 0) {
        // Ações específicas para movimento do joystick
    }
}

int main() {
    // Inicialização
    stdio_init_all();
    board_init();
    tusb_init();
    
    adc_init();
    adc_gpio_init(26); // X do joystick
    adc_gpio_init(27); // Y do joystick

    setup_buttons();

    while (1) {
        tud_task();
        read_joystick();
        read_buttons();
        send_hid_report();
        sleep_ms(10);
    }

    return 0;
}

// Função para manipular solicitações de descritor HID
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    if (report_type == HID_REPORT_TYPE_INPUT) {
        custom_gamepad_report_t report = {
            .x = x_axis,
            .y = y_axis,
            .buttons = buttons
        };
        memcpy(buffer, &report, sizeof(report));
        return sizeof(report);
    }
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    // Pode ser usado para feedback de force feedback/vibração no futuro
}

// Função necessária para configuração do USB HID
uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return desc_hid_report;
}

// Função de callback para descritor de dispositivo
uint8_t const* tud_descriptor_device_cb(void) {
    return desc_device;
}

// Descritor de configuração para um dispositivo HID
uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    0x09, 0x02, 0x22, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32,

    // Interface Descriptor
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,

    // HID Descriptor
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, sizeof(desc_hid_report), 0x00,

    // Endpoint Descriptor
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A
};

// Função de callback para descritor de configuração
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // Para evitar aviso de variável não utilizada
    return desc_configuration;
}

// Função de callback para descritor de string
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid; // Para evitar aviso de variável não utilizada
    static uint16_t desc_str[32];
    uint8_t chr_count;

    // String de exemplo
    char const* string_desc[] = {
        (const char[]){0x09, 0x04}, // Idioma: Inglês (EUA)
        "Raspberry Foundation",
        "PI PICO W",
        "191008", // Serial
    };

    if (index == 0) {
        memcpy(&desc_str[1], string_desc[0], 2);
        chr_count = 1;
    } else {
        if (!(index < sizeof(string_desc) / sizeof(string_desc[0]))) return NULL;
        const char* str = string_desc[index];
        chr_count = strlen(str);
        for (uint8_t i = 0; i < chr_count; i++) {
            desc_str[1 + i] = str[i];
        }
    }

    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return desc_str;
}