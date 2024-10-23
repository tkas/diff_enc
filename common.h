/* 
 * Tomas Kasparek, 2021, BUT, ESA Milani cubesat
 */

#ifndef _COMMON_H_
#define _COMMON_H_

static const int8_t RES_OK          = 0;
static const int8_t RES_WRONG_PARAM = 1;
static const int8_t RES_WIDTH_HIGH  = 2;
static const int8_t RES_WIDTH_ODD   = 3;
static const int8_t RES_HEIGHT_ODD  = 4;
static const int8_t RES_ENDIAN_UNK  = 5;

/* no sense in processing smaller then */
#define MIN_WIDTH 64
#define MIN_HEIGHT 64

/* max image row size in pixels for static buffers */
#define MAX_WIDTH 3096
#define MAX_HEIGHT 2048
#define MAX_GAPS 85
#define BYTES_PER_PIXEL 2    //expecting fixed 16bits per pixel
#define MAX_ROW_BYTES (MAX_WIDTH * BYTES_PER_PIXEL)

#define HISTO16_SIZE  (1 << (BYTES_PER_PIXEL*8))  //histogram for 2^16 gray levels

#define THRESHOLD_MAX ((1 << (BYTES_PER_PIXEL*8))/3)  //threshold should be in first third of histogram - othervise probably too close etc.
#define BLACK_SKY_MIN 75                              //if there is more then 75% of black sky,

//for raw 16bit per pixel image
#define WHITE_PIXEL 65535
#define BLACK_PIXEL 0

#define true 1
#define false 0

int8_t process_params_wh(uint16_t *w, uint16_t *h, int argc, char **argv);

int8_t process_params_end(uint8_t *s, int idx, int argc, char **argv);

int8_t process_params_uint16(uint16_t *s, int idx, int argc, char **argv);

void show_usage(void);

uint16_t swap16(uint16_t value, uint8_t swap);

double mean_ui16(uint16_t data[], uint16_t w, uint16_t h);

double mean_si32(int32_t data[], uint16_t w, uint16_t h);

double variance_ui16(uint16_t data[], uint16_t w, uint16_t h, double m);

double variance_si32(int32_t data[], uint16_t w, uint16_t h, double m);

void ma_filter_ui32(uint32_t in[], double out[], int32_t len, uint16_t points);

void ma_filter_dbl(double in[], double out[], int32_t len, uint16_t points);

void first_derivative(double in[], double out[], int32_t len);

uint8_t bits_for_number(int32_t n);

#endif
