/* 
 * Tomas Kasparek, 2021, BUT, ESA Milani cubesat
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void show_usage(void){// {{{
  fprintf(stderr, "XX_decode WIDTH HEIGHT ENDIANNESS <input.raw >output.gray\n");
} //}}}

int8_t process_params_wh(uint16_t *w, uint16_t *h, int argc, char **argv){// {{{
  int32_t i;

  if(argc < 3){
    fprintf(stderr, "Wrong number of parameters!\n");
    return RES_WRONG_PARAM;
  }

  i = atoi(argv[1]);
  //for buffers, check for max width
  if(i < MIN_WIDTH || i > MAX_WIDTH){
    fprintf(stderr, "Width too low/high, could not process\n");
    return RES_WIDTH_HIGH;
  }
#if 0
  //width have to be even
  if(i % 2 != 0){
    fprintf(stderr, "Width have to be even number\n");
    return RES_WIDTH_ODD;
  }
#endif
  *w = i;

  i = atoi(argv[2]);
  //for buffers, check for max width
  if(i < MIN_WIDTH || i > MAX_WIDTH){
    fprintf(stderr, "Height too low/high, could not process\n");
    return RES_WIDTH_HIGH;
  }

#if 0
  if(i % 2 != 0){
    fprintf(stderr, "Height have to be even number\n");
    return RES_HEIGHT_ODD;
  }
#endif

  *h = i;

  return RES_OK;
} //}}}

int8_t process_params_end(uint8_t *s, int idx, int argc, char **argv){// {{{
  if(argc <= idx){
    fprintf(stderr, "Wrong number of parameters!\n");
    return RES_WRONG_PARAM;
  }

  if(argv[idx][0] != 'L' && argv[idx][0] != 'B'){
    fprintf(stderr, "Endianness should be either L for little or B for big\n");
    return RES_ENDIAN_UNK;
  }

  *s = (argv[idx][0] == 'B') ? 1 : 0;

  return RES_OK;
} //}}}

int8_t process_params_uint16(uint16_t *s, int idx, int argc, char **argv){// {{{
  if(argc <= idx){
    fprintf(stderr, "Wrong number of parameters!\n");
    return RES_WRONG_PARAM;
  }

  *s = atoi(argv[idx]);

  return RES_OK;
} //}}}

//Swaps endianness of a 16-bit number
uint16_t swap16(uint16_t value, uint8_t swap)// {{{
{
  if (swap != 0) {
    return (((value >> 8) & 0xff) | ((value & 0xff) << 8));
  } else {
    return value;
  }
} //}}}

double mean_ui16(uint16_t data[], uint16_t w, uint16_t h){// {{{
  int16_t x, y = 0;
  double sum = 0;

  for(y=0; y < h; y++){
    for(x=0; x < w; x++){
      sum += data[y*w+x];
    }
  }
  return sum/(w*h);
} //}}}

double mean_si32(int32_t data[], uint16_t w, uint16_t h){// {{{
  int16_t x, y = 0;
  double sum = 0;

  for(y=0; y < h; y++){
    for(x=0; x < w; x++){
      sum += data[y*w+x];
    }
  }
  return sum/(w*h);
} //}}}

double variance_ui16(uint16_t data[], uint16_t w, uint16_t h, double m){// {{{
  uint16_t x=0, y=0;
  double sum = 0, a = 0;

  for(y=0; y < h; y++){
    for(x=0; x < w; x++){
      a = data[y*w+x] - m;
      sum += a*a;
    }
  }
  return sum/(w*h);
} //}}}

double variance_si32(int32_t data[], uint16_t w, uint16_t h, double m){// {{{
  uint16_t x=0, y=0;
  double sum = 0, a = 0;

  for(y=0; y < h; y++){
    for(x=0; x < w; x++){
      a = data[y*w+x] - m;
      sum += a*a;
    }
  }
  return sum/(w*h);
} //}}}

//filter input array with M-point moving average filter - input is uint32_t
void ma_filter_ui32(uint32_t in[], double out[], int32_t len, uint16_t points){ // {{{
  int32_t i=0;
  uint16_t maf_half=(points >> 1);
  uint16_t maf_cnt=0;
  int32_t maf=0;
  double avg=0;

  for(i=0; i<len ; i++){
    //go over MAF window for this position in histogram
    maf_cnt=0;
    avg=0;
    for(maf=(i - maf_half); maf <= (i + maf_half) && maf < len; maf++){
      //fprintf(stderr, "maf: %d\n", maf);

      //window on histogram start - replace nonexistent with zeros
      if (maf >= 0){ avg += in[maf]; }

      //actualy used MAF window size for averaging
      maf_cnt++;
    }
    out[i] = avg / maf_cnt; //average histogram value over used window
    //fprintf(stderr, "maf out[%d]: %lf (%lf/%d)\n", i, out[i], avg, maf_cnt);
  }
} //}}}

//filter input array with M-point moving average filter - input is double
void ma_filter_dbl(double in[], double out[], int32_t len, uint16_t points){ // {{{
  int32_t i=0;
  uint16_t maf_half=(points >> 1);
  uint16_t maf_cnt=0;
  int32_t maf=0;
  double avg=0;

  for(i=0; i<len ; i++){
    //go over MAF window for this position in histogram
    maf_cnt=0;
    avg=0;
    for(maf=(i - maf_half); maf <= (i + maf_half) && maf < len; maf++){
      //fprintf(stderr, "maf: %d\n", maf);

      //window on histogram start - replace nonexistent with zeros
      if (maf >= 0){ avg += in[maf]; }

      //actualy used MAF window size for averaging
      maf_cnt++;
    }
    out[i] = avg / maf_cnt; //average histogram value over used window
  }
} //}}}

void first_derivative(double in[], double out[], int32_t len){ //{{{
  int32_t i=0;

  //skip first and last - copy 2nd and pre-last respectively later
  for(i=1; i<(len-1) ; i++){
    out[i] = (in[i+1] - in[i-1])/2;
  }
  out[0] = out[1];
  out[len-1] = out[len-2];
} //}}}

//return numer of bits to store the number
uint8_t bits_for_number(int32_t n){
  uint8_t count = 0;
   
  if (n == 0){
    return n;
  }

  while( n != 0) {
    n >>= 1;
    count += 1;
  }
                  
  return count;
}
