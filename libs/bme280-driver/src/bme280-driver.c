#include "bme280-driver.h"
#include "bme280-regs.h"
#include <stdio.h>   // для printf

typedef struct {
    bme280_i2c_read i2c_read;
    bme280_i2c_write i2c_write;
} bme280_ctx_t;

static bme280_ctx_t bme280_ctx = {0};

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write) {
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;

    // 1. Проверка идентификатора чипа
    uint8_t id = 0;
    bme280_read_regs(BME280_REG_id, &id, 1);
    if (id != 0x60) {
        printf("BME280 not found! ID=0x%02X (expected 0x60)\n", id);
    } else {
        printf("BME280 detected, ID=0x60\n");
    }

    // 2. Настройка ctrl_hum: osrs_h = 001 (oversampling x1)
    uint8_t ctrl_hum_reg_value = 0;
    ctrl_hum_reg_value |= (0b001 << 0); // osrs_h[2:0] = 1
    bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);

    // 3. Настройка config: spi3w_en=0, filter=000 (off), t_sb=001 (62.5 ms)
    uint8_t config_reg_value = 0;
    config_reg_value |= (0b0 << 0);   // spi3w_en[0]
    config_reg_value |= (0b000 << 2); // filter[4:2]
    config_reg_value |= (0b001 << 5); // t_sb[7:5]
    bme280_write_reg(BME280_REG_config, config_reg_value);

    // 4. Настройка ctrl_meas: mode=11 (normal), osrs_t=001 (x1), osrs_p=001 (x1)
    uint8_t ctrl_meas_reg_value = 0;
    ctrl_meas_reg_value |= (0b001 << 5); // osrs_t[7:5]
    ctrl_meas_reg_value |= (0b001 << 2); // osrs_p[4:2]
    ctrl_meas_reg_value |= (0b11 << 0);  // mode[1:0]
    bme280_write_reg(BME280_REG_ctrl_meas, ctrl_meas_reg_value);
}

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length) {
    uint8_t reg_addr[1] = {start_reg_address};
    bme280_ctx.i2c_write(reg_addr, sizeof(reg_addr));
    bme280_ctx.i2c_read(buffer, length);
}

void bme280_write_reg(uint8_t reg_address, uint8_t value) {
    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

uint16_t bme280_read_temp_raw(void) {
    uint8_t read[2] = {0};
    // Читаем два байта температуры: lsb (0xFB), затем msb (0xFA) — за один вызов, начиная с lsb
    bme280_read_regs(BME280_REG_temp_msb, read, 2);
    uint16_t value = ((uint16_t)read[0] << 8) | read[1];
    return value;
}

uint16_t bme280_read_pres_raw(void) {
    uint8_t read[2] = {0};
    bme280_read_regs(BME280_REG_press_msb, read, 2);
    uint16_t value = ((uint16_t)read[0] << 8) | read[1];
    return value;
}

uint16_t bme280_read_hum_raw(void) {
    uint8_t read[2] = {0};
    // Для влажности регистры: MSB (0xFD), LSB (0xFE) — читаем оба за раз
    bme280_read_regs(BME280_REG_hum_msb, read, 2);
    uint16_t value = ((uint16_t)read[0] << 8) | read[1];
    return value;
}