#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

int main(int argc, char * argv[])
{
  BMP_Image * image;
  FILE * fptr;
  FILE * ofptr;
  BMP_Image * new_image;
  int check;

  if(argc < 3)
  {
    fprintf(stderr, "Insufficient arguments");
    return EXIT_FAILURE;
  }
  if(argv[1] == NULL)
  {
    return EXIT_FAILURE;
  }

  fptr = fopen(argv[1], "rb");
  if(fptr == NULL)
  {
    fprintf(stderr, "Error reading input file\n");
    return EXIT_FAILURE;
  }
 
  image = Read_BMP_Image(fptr);
  if(image != NULL)
  {
    if(image->header.bits != 24)
    {
      fprintf(stderr, "Not a 24-bit BMP format, can't convert\n");
      fclose(fptr);
      Free_BMP_Image(image);
      return EXIT_FAILURE;
    }
    
    new_image = Convert_24_to_16_BMP_Image_with_Dithering(image);
   
    if(new_image != NULL)
    {
      
      ofptr = fopen(argv[2], "wb");
      check = Write_BMP_Image(ofptr, new_image);
      if(check == 0)
      {
	Free_BMP_Image(image);
	Free_BMP_Image(new_image);
	fclose(ofptr);
	fclose(fptr);
	fprintf(stderr, "Error writing to output file");
	return EXIT_FAILURE;
      }
      
      Free_BMP_Image(image);
      Free_BMP_Image(new_image);
      fclose(fptr);
      fclose(ofptr);
   }
   else if(new_image == NULL)
   {
     fclose(fptr);
     Free_BMP_Image(image);
     fprintf(stderr,"Error converting");
     return EXIT_FAILURE;
   }
  }
  else
  {
    fprintf(stderr, "Can't read image from file\n");
    fclose(fptr);
    return EXIT_FAILURE;
  }

  return 0;
}
