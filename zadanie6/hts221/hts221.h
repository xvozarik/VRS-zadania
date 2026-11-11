/*
 * hts221.h
 *
 *  Created on: 4. 11. 2026
 *      Author: dell
 */

#ifndef HTS221_H_
#define HTS221_H_

#include <stdint.h>

#define HTS221_ADDR        0xBE
#define HTS221_WHO_AM_I_REG 0x0F
#define HTS221_WHO_AM_I_VAL 0xBC
#define HTS221_AV_CONF     0x10
#define HTS221_CTRL_REG1   0x20
#define HTS221_STATUS_REG  0x27
#define HTS221_HUM_OUT_L   0x28
#define HTS221_TEMP_OUT_L  0x2A
#define HTS221_CALIB_START 0x30
#define HTS221_CTRL1_CONFIG 0x85

typedef void (*hts221_read_fn)(uint8_t *buf, uint8_t len, uint8_t reg, uint8_t addr, uint8_t multi);
typedef void (*hts221_write_fn)(uint8_t *data, uint8_t len, uint8_t reg, uint8_t addr, uint8_t multi);

void    hts221_register_callbacks(hts221_read_fn r, hts221_write_fn w);
uint8_t hts221_init(void);
float   hts221_get_temperature(void);
float   hts221_get_humidity(void);

#endif /* HTS221_H_ */
