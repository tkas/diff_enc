/* 
 * Tomas Kasparek, 2022, BUT
 */

/*
 * call diff_bpp WIDTH HEIGHT GAPS <input.raw >output.diff.raw
 *
 * Apply a differential encoding filter to RAW input data
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

//program parameters
uint16_t  width = 0;
uint16_t  height = 0;
uint16_t  gaps = 0;

//global static buffer
uint16_t input_img[MAX_GAPS*MAX_HEIGHT*MAX_WIDTH];  // buffer to read the whole raw image, for 16bit 2048x2048 this is 8MB
int32_t diff_img[MAX_HEIGHT*MAX_WIDTH];  // buffer to store diff image for one gap

int main(int argc, char **argv) {
  size_t len=0;
  uint16_t x=0, y=0, g=0;
  int32_t diff_val, diff_min, diff_max;
  uint16_t diff_out=0;
  
  if (process_params_wh(&width, &height, argc, argv) != RES_OK){
    show_usage();
    return 1;
  };

  if (process_params_uint16(&gaps, 3, argc, argv) != RES_OK){
    show_usage();
    return 1;
  };
  if(gaps <2){
    fprintf(stderr, "Need at least 2 gaps!\n");
    return 1;
  }

  fprintf(stderr, "w: %d, h: %d, g: %d\n", width, height, gaps);

  //read whole image from input data
  bzero(&input_img, MAX_GAPS*MAX_HEIGHT*MAX_ROW_BYTES);
  if((len = read(0, &input_img, gaps*width*height*BYTES_PER_PIXEL)) != gaps*width*height*BYTES_PER_PIXEL){
    fprintf(stderr, "Unable to read image got %ld B\n", len);
    return 1;
  }

  //output first gap as is - no difference
  write(1, input_img, width*height*BYTES_PER_PIXEL);

  for(g=1; g < gaps; g++){
    //min and max teoreticaly possible values
    diff_min = (1<<16)-1; diff_max = -1*((1<<16)-1);
    //fprintf(stderr, "initial: min: %d max: %d, int: %d\n", diff_min, diff_max, diff_max - diff_min);

    //generate diff image for this gap
    for(y=0; y < height; y++){
      for (x=0; x < width; x++){
        diff_val =  input_img[(g-1)*(width*height)+y*width+x] - input_img[g*(width*height)+y*width+x];

        //store raw diff value for this gap
        diff_img[y*width+x] = diff_val;

        if(diff_val > diff_max) diff_max = diff_val;
        if(diff_val < diff_min) diff_min = diff_val;
      }
    }
    fprintf(stderr, "g[%d]: min: %d max: %d, int: %d, bits: %d\n",
      g, diff_min, diff_max, diff_max - diff_min,
      bits_for_number(diff_max - diff_min));

    //output shifted diff values for j2k compression test 
    for(y=0; y < height; y++){
      for (x=0; x < width; x++){
        //shift potentialy signed diff to unsigned value using minimal diff value per gap
        diff_out = diff_img[y*width+x] - diff_min;
        write(1, &diff_out, sizeof(uint16_t));
      }
    }
  }

  return 0;
}
