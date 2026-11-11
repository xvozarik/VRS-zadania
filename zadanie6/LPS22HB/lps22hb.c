#include "lps22hb.h"

#include <math.h>

static lps22hb_read_fn  sensor_read  = 0;
static lps22hb_write_fn sensor_write = 0;

void lps22hb_register_callbacks(lps22hb_read_fn r, lps22hb_write_fn w)
{
	sensor_read  = r;
	sensor_write = w;
}

uint8_t lps22hb_init(void)
{
	if (!sensor_read || !sensor_write) return 0;

	uint8_t id = 0;
	sensor_read(&id, 1, LPS22HB_WHO_AM_I_REG, LPS22HB_ADDR, 0);
	if (id != LPS22HB_WHO_AM_I_VAL) return 0;

	uint8_t cfg = LPS22HB_CTRL1_CONFIG;
	sensor_write(&cfg, 1, LPS22HB_CTRL_REG1, LPS22HB_ADDR, 0);

	return 1;
}

float lps22hb_get_pressure(void)
{
	uint8_t b[3];
	sensor_read(b, 3, LPS22HB_PRESS_OUT_XL, LPS22HB_ADDR, 0);

	int32_t raw = ((int32_t)b[2] << 16) | ((int32_t)b[1] << 8) | b[0];
	if (raw & 0x800000) raw |= 0xFF000000;

	return (float)raw / 4096.0f;
}

float lps22hb_calc_altitude(float p, float p0)
{
	return 44330.0f * (1.0f - powf(p / p0, 1.0f / 5.255f));
}
