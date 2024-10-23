/* 
 * Tomas Kasparek, 2022, BUT
 */

/*
 * call diff_bpp_decode WIDTH HEIGHT GAPS OFF0,OFF1,OFF2 <input.diff.raw >output.raw
 *
 * Decode the differentialy encoded data cube (inverse to diff_bpp)
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "common.h"

//program parameters
uint16_t  width = 0;
uint16_t  height = 0;
uint16_t  gaps = 0;

//global static buffer
uint16_t input_img[MAX_GAPS*MAX_HEIGHT*MAX_WIDTH];  // buffer to read the whole raw image, for 16bit 2048x2048 this is 8MB
int32_t  gaps_off[MAX_GAPS];            //list of gaps offsets, the first have to be 0 (no offset of the first image to itself)

int main(int argc, char **argv) {
  size_t len=0;
  uint16_t x=0, y=0, g=0;
  char *off_str=NULL;
  uint16_t decoded_val=0;
 
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
  if(argc <= 4){
    fprintf(stderr, "Offset string not given!\n");
    return 1;
  } 
  off_str = argv[4];
  for(g=0; g<gaps; g++){
    //current wvl offset
    gaps_off[g] = atoi(off_str);
    fprintf(stderr, "g_off[%d]: %d\n", g, gaps_off[g]);
    //find start of next wvl offset delimiter
    off_str=strchr(off_str, ',');
    //wvl offset start one character after delimiter (',')
    if(off_str!=NULL) off_str++;
  }

  fprintf(stderr, "w: %d, h: %d, g: %d, o: %s\n", width, height, gaps, argv[4]);

  //read whole image from input data
  bzero(&input_img, MAX_GAPS*MAX_HEIGHT*MAX_ROW_BYTES);
  if((len = read(0, &input_img, gaps*width*height*BYTES_PER_PIXEL)) != gaps*width*height*BYTES_PER_PIXEL){
    fprintf(stderr, "Unable to read image got %ld B\n", len);
    return 1;
  }

  //output first gap as is - no difference
  write(1, input_img, width*height*BYTES_PER_PIXEL);

  for(g=1; g < gaps; g++){

    //generate diff image for this gap
    for(y=0; y < height; y++){
      for (x=0; x < width; x++){
        decoded_val =  input_img[(g-1)*(width*height)+y*width+x] - input_img[g*(width*height)+y*width+x] - gaps_off[g];

        //store this decoded pixel for next wvl to be used
        input_img[g*(width*height)+y*width+x] = decoded_val;

        //output this decoded pixel
        write(1, &decoded_val, sizeof(uint16_t));
      }
    }
  }

  return 0;
}
