/*
 * lps25hb.h
 *
 *  Created on: 4. 11. 2026
 *      Author: dell
 */

#ifndef LPS22HB_H_
#define LPS22HB_H_

#include <stdint.h>

#define LPS22HB_ADDR          0xBA
#define LPS22HB_WHO_AM_I_REG  0x0F
#define LPS22HB_WHO_AM_I_VAL  0xB1
#define LPS22HB_CTRL_REG1     0x10
#define LPS22HB_CTRL1_CONFIG  0x12
#define LPS22HB_PRESS_OUT_XL  0x28

typedef void (*lps22hb_read_fn)(uint8_t *buf, uint8_t len, uint8_t reg, uint8_t addr, uint8_t multi);
typedef void (*lps22hb_write_fn)(uint8_t *data, uint8_t len, uint8_t reg, uint8_t addr, uint8_t multi);

void    lps22hb_register_callbacks(lps22hb_read_fn r, lps22hb_write_fn w);
uint8_t lps22hb_init(void);
float   lps22hb_get_pressure(void);
float   lps22hb_calc_altitude(float p, float p0);

#endif /* LPS22HB_H_ */
