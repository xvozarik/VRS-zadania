/*
 * hts221.c
 *
 *  Created on: 4. 11. 2026
 *      Author: dell
 */


#include "hts221.h"

static hts221_read_fn  sensor_read  = 0;
static hts221_write_fn sensor_write = 0;

static float H0_rh, H1_rh, T0_degC, T1_degC;
static int16_t H0_T0_out, H1_T0_out, T0_out, T1_out;

void hts221_register_callbacks(hts221_read_fn r, hts221_write_fn w)
{
	sensor_read  = r;
	sensor_write = w;
}

static uint8_t hts221_read_reg(uint8_t reg)
{
	uint8_t v = 0;
	sensor_read(&v, 1, reg, HTS221_ADDR, 0);
	return v;
}

static void hts221_write_reg(uint8_t reg, uint8_t val)
{
	sensor_write(&val, 1, reg, HTS221_ADDR, 0);
}

uint8_t hts221_init(void)
{
	if (!sensor_read || !sensor_write) return 0;

	if (hts221_read_reg(HTS221_WHO_AM_I_REG) != HTS221_WHO_AM_I_VAL) return 0;

	hts221_write_reg(HTS221_CTRL_REG1, HTS221_CTRL1_CONFIG);

	uint8_t c[16];
	sensor_read(c, 16, HTS221_CALIB_START, HTS221_ADDR, 1);

	H0_rh = c[0] / 2.0f;
	H1_rh = c[1] / 2.0f;
	T0_degC = (float)(((c[5] & 0x03) << 8) | c[2]) / 8.0f;
	T1_degC = (float)(((c[5] & 0x0C) << 6) | c[3]) / 8.0f;
	H0_T0_out = (int16_t)((c[7]  << 8) | c[6]);
	H1_T0_out = (int16_t)((c[11] << 8) | c[10]);
	T0_out    = (int16_t)((c[13] << 8) | c[12]);
	T1_out    = (int16_t)((c[15] << 8) | c[14]);

	return 1;
}

float hts221_get_temperature(void)
{
	uint8_t b[2];
	sensor_read(b, 2, HTS221_TEMP_OUT_L, HTS221_ADDR, 1);
	int16_t raw = (int16_t)((b[1] << 8) | b[0]);

	return T0_degC + (float)(raw - T0_out) * (T1_degC - T0_degC) / (float)(T1_out - T0_out);
}

float hts221_get_humidity(void)
{
	uint8_t b[2];
	sensor_read(b, 2, HTS221_HUM_OUT_L, HTS221_ADDR, 1);
	int16_t raw = (int16_t)((b[1] << 8) | b[0]);

	float h = H0_rh + (float)(raw - H0_T0_out) * (H1_rh - H0_rh) / (float)(H1_T0_out - H0_T0_out);
	if (h < 0.0f)   h = 0.0f;
	if (h > 100.0f) h = 100.0f;
	return h;
}
