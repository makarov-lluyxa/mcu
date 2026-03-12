#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "bme280-driver.h"

#define DEVICE_NAME "Pico BME280"
#define DEVICE_VRSN "1.0"

#define BME280_I2C_ADDR 0x76

void rp2040_i2c_read(uint8_t* buffer, uint16_t length) {
    i2c_read_timeout_us(i2c1, BME280_I2C_ADDR, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size) {
    i2c_write_timeout_us(i2c1, BME280_I2C_ADDR, data, size, false, 100000);
}

void version_callback(const char* args) {
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args) {
    led_task_state_set(LED_STATE_ON);
    printf("LED ON\n");
}

void led_off_callback(const char* args) {
    led_task_state_set(LED_STATE_OFF);
    printf("LED OFF\n");
}

void led_blink_callback(const char* args) {
    led_task_state_set(LED_STATE_BLINK);
    printf("LED BLINK\n");
}

void read_regs_callback(const char* args) {
    uint8_t addr, count;
    if (sscanf(args, "%hhx %hhu", &addr, &count) != 2) {
        printf("Usage: read_regs <addr_hex> <count_dec>\n");
        return;
    }

    if (addr > 0xFF || count > 0xFF) {
        printf("Address and count must be <= 0xFF\n");
        return;
    }
    if (addr + count > 0x100) {
        printf("Address + count must not exceed 0x100\n");
        return;
    }

    uint8_t buffer[256];
    bme280_read_regs(addr, buffer, count);

    for (int i = 0; i < count; i++) {
        printf("bme280 register [0x%02X] = 0x%02X\n", addr + i, buffer[i]);
    }
}

void write_reg_callback(const char* args) {
    uint8_t addr, value;
    if (sscanf(args, "%hhx %hhx", &addr, &value) != 2) {
        printf("Usage: write_reg <addr_hex> <value_hex>\n");
        return;
    }

    if (addr > 0xFF || value > 0xFF) {
        printf("Address and value must be <= 0xFF\n");
        return;
    }

    bme280_write_reg(addr, value);
    printf("Written 0x%02X to register 0x%02X\n", value, addr);
}

// Колбэки для чтения сырых значений
void temp_raw_callback(const char* args) {
    uint16_t raw = bme280_read_temp_raw();
    printf("%u\n", raw);
}

void pres_raw_callback(const char* args) {
    uint16_t raw = bme280_read_pres_raw();
    printf("%u\n", raw);
}

void hum_raw_callback(const char* args) {
    uint16_t raw = bme280_read_hum_raw();
    printf("%u\n", raw);
}

api_t device_api[] = {
    {"version",   version_callback,   "get device name and firmware version"},
    {"on",        led_on_callback,    "turn on LED"},
    {"off",       led_off_callback,   "turn off LED"},
    {"blink",     led_blink_callback, "make LED blink"},
    {"read_regs", read_regs_callback, "read BME280 registers: <addr_hex> <count_dec>"},
    {"write_reg", write_reg_callback, "write BME280 register: <addr_hex> <value_hex>"},
    {"temp_raw",  temp_raw_callback,  "read raw temperature value (16 bits)"},
    {"pres_raw",  pres_raw_callback,  "read raw pressure value (16 bits)"},
    {"hum_raw",   hum_raw_callback,   "read raw humidity value (16 bits)"},
    {NULL, NULL, NULL}
};

int main() {
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);  // SDA
    gpio_set_function(15, GPIO_FUNC_I2C);  // SCL

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);

    sleep_ms(1000);

    printf("\n--- Pico BME280 Terminal ---\n");
    printf("Available commands:\n");
    printf("  version   - get device info\n");
    printf("  on        - turn LED on\n");
    printf("  off       - turn LED off\n");
    printf("  blink     - make LED blink\n");
    printf("  read_regs - read BME280 registers (e.g. read_regs d0 1)\n");
    printf("  write_reg - write BME280 register (e.g. write_reg f4 25)\n");
    printf("  temp_raw  - read raw temperature\n");
    printf("  pres_raw  - read raw pressure\n");
    printf("  hum_raw   - read raw humidity\n");
    printf("Type a command and press Enter:\n");

    while (true) {
        char* cmd = stdio_task_handle();

        if (cmd != NULL) {
            protocol_task_handle(cmd);
        }

        led_task_handle();
    }

    return 0;
}