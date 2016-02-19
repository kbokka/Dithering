#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"

/* check whether a header is valid
 * assume that header has been read from fptr
 * the position of the indicator of fptr is not certain
 * could be at the beginning of the file, end of the file or 
 * anywhere in the file
 * note that the check is only for this exercise/assignment
 * in general, the format is more complicated
 */

int Is_BMP_Header_Valid(BMP_Header* header, FILE *fptr) {
  // Make sure this is a BMP file
  if (header->type != 0x4d42) {
     return FALSE;
  }
  // skip the two unused reserved fields

  // check the offset from beginning of file to image data
  // essentially the size of the BMP header
  // BMP_HEADER_SIZE for this exercise/assignment
  if (header->offset != BMP_HEADER_SIZE) {
     return FALSE;
  }
      
  // check the DIB header size == DIB_HEADER_SIZE
  // For this exercise/assignment
  if (header->DIB_header_size != DIB_HEADER_SIZE) {
     return FALSE;
  }

  // Make sure there is only one image plane
  if (header->planes != 1) {
    return FALSE;
  }
  // Make sure there is no compression
  if (header->compression != 0) {
    return FALSE;
  }

  // skip the test for xresolution, yresolution

  // ncolours and importantcolours should be 0
  if (header->ncolours != 0) {
    return FALSE;
  }
  if (header->importantcolours != 0) {
    return FALSE;
  }
  
  // Make sure we are getting 24 bits per pixel
  // or 16 bits per pixel
  // only for this assignment
  if (header->bits != 24 && header->bits != 16) {
    return FALSE;
  }

  // fill in extra to check for file size, image size
  // based on bits, width, and height
  int check;
  int bits;
  
  bits  = (header->width * header->bits) / 8;
  if(bits % 4 == 0)
  {
     check = 0;
  }
  else
  {
    //check = ((4 - (header->width % 4)) * (header->height));
    check = 4 - (bits % 4);
  }
  bits = bits + check;

  //if(header->imagesize != ((header->height * header->width * header->bits) / 8) + check)
  if(header->imagesize != bits * header->height)
  {
    return FALSE;
  }
  if((header->size) != (bits * header->height) + 54)
  {
    return FALSE;
  }

  return TRUE;
}

/* The input argument is the source file pointer. 
 * The function returns an address to a dynamically allocated BMP_Image only 
 * if the file * contains a valid image file 
 * Otherwise, return NULL
 * If the function cannot get the necessary memory to store the image, also 
 * return NULL
 * Any error messages should be printed to stderr
 */
BMP_Image *Read_BMP_Image(FILE* fptr) {

   BMP_Image *bmp_image = NULL;
   
  //Allocate memory for BMP_Image*;
   bmp_image = malloc(sizeof(BMP_Image));
   if(bmp_image == NULL)
   {
     fprintf(stderr,"Error allocating memory\n");
     return NULL;
   }
   int read;
   
   
  //Read the first 54 bytes of the source into the header
   read = fread(&(bmp_image->header), sizeof(BMP_Header), 1, fptr);
   if(read != 1)
   {
     fprintf(stderr,"Error reading inputfile\n");
     return NULL;
   }


 // if read successful, check validity of header
   read = Is_BMP_Header_Valid(&(bmp_image->header), fptr);
   if(read == FALSE)
   {
     free(bmp_image);
     fprintf(stderr,"Input file is not in expected format\n");
     return NULL;
   }

  // Allocate memory for image data
   bmp_image->data = malloc(bmp_image->header.imagesize);
   if((bmp_image->data) == NULL)
   {
     fprintf(stderr,"Error allocating memeory\n");
     return NULL;
   }
   read = fread(bmp_image->data, bmp_image->header.imagesize, 1, fptr); 
   if(read != 1)
   {
     fprintf(stderr, "Error reading inputfile\n");
     return NULL;
   }

 
  // read in the image data

  return bmp_image;
}

/* The input arguments are the destination file pointer, BMP_Image *image.
 * The function write the header and image data into the destination file.
 * return TRUE if write is successful
 * FALSE otherwise
 */
int Write_BMP_Image(FILE* fptr, BMP_Image* image) 
{
  int write;
  
   // write header
  write = fwrite(&(image->header),1,54,fptr);
  if(write != 54)
  {
    return 0;
  }
  
   // write image data
  write = fwrite(image->data,1,image->header.size - 54,fptr);
    if(write < 1)
    {
      return 0;
    }
   return 1;
}

/* The input argument is the BMP_Image pointer. The function frees memory of 
 * the BMP_Image.
 */
void Free_BMP_Image(BMP_Image* image) {
  free(image->data);
  free(image);

}

/* Given a BMP_Image, create a new image that retains top half of the 
 * given image
 */
BMP_Image *Top_Half_BMP_Image(BMP_Image *image)
{
  int nheight;
  BMP_Image *new;
  int pad = 0; 
  int nwidth;
  int byteswidth;
  int i;
  //int bytesperrow;

  nwidth  = image->header.width;

  if(((image->header).height / 2) % 2 == 0)
  {
    nheight = (image->header.height / 2);
  }
  else
  {
    nheight = ((image->header.height + 1) / 2);
  }
  byteswidth = (nwidth * image->header.bits) / 8;
  if((byteswidth % 4) != 0)
  {
    pad = 4 - (byteswidth  % 4);  
  }
 
  byteswidth = byteswidth + pad;

  new = malloc(nheight * byteswidth + 54);

  new->header = image->header;
  new->data = image->data;
  new->header.imagesize = nheight * byteswidth;
  new->header.height = nheight;
  new->header.size = nheight * byteswidth + 54;
  new->data = (unsigned char *)malloc((new->header).imagesize);
 
  for(i = 0; i < (nheight * byteswidth); i++)
  {
    new->data[i] = ((image->data)[i + ((image->header.height / 2) * byteswidth)]);
  }

  return new;
}

/* Given a BMP_Image, create a new image that retains left half of the given 
 * image
 */
BMP_Image *Left_Half_BMP_Image(BMP_Image *image)
{
   return NULL;
}
BMP_Image *Convert_24_to_16_BMP_Image(BMP_Image *image)
{
  BMP_Image *new;
  int row_bytes = (image->header.width * image->header.bits) / 8;
  int i, j, k;
  uint16_t pixel;
  int nimagesize; 
  uint16_t r,g,b;
  //unsigned char newpixel;
  int pad;
  int pad16;
  int bytesperrow16;

  //account for padding for 24 bit image
  if((row_bytes % 4) != 0)
  {
    pad = 4 - (row_bytes  % 4);  
  }
  else
  {
	pad = 0;
  }
  row_bytes = row_bytes + pad;
  
  //allocate space for new image
  new = malloc(sizeof(BMP_Image));

  //adjust header elements of new image
  new->header = image->header;
  //new->data = image->data;
  new->header.bits = 16;

  //account for padding in 16 bit image
  if((new->header.width % 2) != 0)
  {
     pad16 = 4 - (((new->header.width) * 16) / 8);
  }
  else
  {
    pad16 = 0;
  }
  bytesperrow16 = ((image->header.width * new->header.bits) / 8) + pad16; 
  nimagesize = bytesperrow16 * image->header.height;
  new->data = malloc(nimagesize);
  //new->header.size = (row_bytes * image->header.height) + 54;
  new->header.size = (bytesperrow16 * image->header.height) + 54;  
  
  for(j = 0; j < image->header.height; j++)
  {
    for(i = 0; i < image->header.width; i++)
    {
	    //extract bgr values from original data
	    b = (image->data[(i * 3) + (row_bytes * j)]) >> 3;
	    g = (image->data[(i * 3) + (row_bytes * j) + 1]) >> 3;
	    r = (image->data[(i * 3) + (row_bytes * j) + 2]) >> 3;
	    //extracting elements

	    //b = (image->data[i + (row_bytes * j) + (image->header.width * 3)]) >> 3;
	    //g = (image->data[i + 1 + (row_bytes * j) + (image->header.width * 3)]) >> 3;
	    //r = (image->data[i + 2 + (row_bytes * j) + (image->header.width * 3)]) >> 3;
	    
	    //put into 16 bit format
	    pixel = (r << 10 | g << 5 | b);
	    
	    //assign new image data with the pixel information 
	    new->data[(i * 2) + (bytesperrow16 * j)] = pixel;
 	    new->data[(i*2) + (bytesperrow16 * j) + 1] = pixel >> 8;	    

	    //put into 16 bit format
	    //nrgb = ((b >> 3) << 10)) | ((g >> 3)<<5)) | (r>>3);
	    //printf("\nrow : %d", i);
    }
	//account for padding for each row
	/*if(pad16 == 2)
	{
		new->data[(j + 1) * bytesperrow16 - 1] = 0;	
		new->data[(j + 2) * bytesperrow16 - 2] = 0;
	}
	*/
	for(k = 0; k < pad16; k++)
	{
	  //new->data[k + (bytesperrow16 * j) +(2 * new->header.width)] = 0;
	  new->data[k + (bytesperrow16 * j)] = 0;
	  new->data[k+1 + (bytesperrow16 * j)] = 0;
	  new->data[k+2 + (bytesperrow16 * j)] = 0;
	}
  	
	//printf("\nheight: %d",j);
		     
  } 
  
  
  return new;
}
BMP_Image *Convert_24_to_16_BMP_Image_with_Dithering(BMP_Image *image)
{
  BMP_Image *new = NULL;
  int row_bytes = (image->header.width * image->header.bits) / 8;
  int i,j,k;
  uint16_t pixel;
  int nimagesize;
  uint16_t r,g,b;
  int pad24;
  int pad16;
  int bytesperrow16;
  int qerror_r, qerror_g, qerror_b;
 
  //account for padding in 24 bit image
  if((row_bytes % 4) != 0)
  {
    pad24 = 4 - (row_bytes % 4);
  }
  else
  {
	pad24 = 0;
  }
  row_bytes = row_bytes + pad24;
  
  //allocate space for new image
  new = malloc(sizeof(BMP_Image));
  //adjust header elements of new image
  new->header = image->header;
  new->header.bits = 16;
  //account for padding in 16 bit image
  if((new->header.width % 2) != 0)
  {
    pad16 = 4 - ((new->header.width * 16) / 8);
  }
  else 
  {
    pad16 = 0;
  }
  bytesperrow16 = ((image->header.width * new->header.bits) / 8);
  nimagesize = bytesperrow16 * image->header.height;
  new->data = malloc(nimagesize);
  new->header.imagesize = nimagesize;
  new->header.size = (bytesperrow16 * image->header.height) + 54;
  
  for(j = 0; j < image->header.height; j++)
  {
	for(i = 0; i < image->header.width; i++)
	{
	  b = (image->data[(i * 3) + (row_bytes * j)]) >> 3;
	  g = (image->data[(i * 3) + (row_bytes * j) + 1]) >> 3;
	  r = (image->data[(i * 3) + (row_bytes * j) + 2]) >> 3;
	  
	  pixel = (r << 10 | g << 5 | b);

	  qerror_r = r - (((r >> 3) * 255) / 31);
	  qerror_g = g - (((r >> 3) * 255) / 31);
	  qerror_b = b - (((r >> 3) * 255) / 31);
	  
	  new->data[(i * 2) + (bytesperrow16 * j)] = pixel;
	  new->data[(i * 2) + (bytesperrow16 * j) + 1] = pixel >> 8; 
      
         
	  //account qerror for red green and blue
	  if((i * 2) < image->header.width)
	  {
	    image->data[(i * 2) + (bytesperrow16 * j)] += qerror_r * 7 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j)] += qerror_g * 7 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j)] += qerror_b * 7 / 16;
	  }
	  if((i * 2 + 1) < image->header.width)
	  {
	    image->data[(i * 2) + (bytesperrow16 * j) + 1] += qerror_r * 3 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j) + 1] += qerror_g * 3 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j) + 1] += qerror_b * 3 / 16;
	  }
	  if((i * 2 + 2) < image->header.width)
	  {
	    image->data[(i * 2) + (bytesperrow16 * j) + 2] += qerror_r * 5 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j) + 2] += qerror_g * 5 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j) + 2] += qerror_b * 5 / 16;
	  }
	  if(((i * 2) + 3) < image->header.width)
	  {
	    image->data[(i * 2) + (bytesperrow16 * j) + 3] += qerror_r * 1 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j) + 3] += qerror_g * 1 / 16;
	    image->data[(i * 2) + (bytesperrow16 * j) + 3] += qerror_b * 1 / 16;
	  }

	}
	for(k = 0; k < pad16; k++)
	{
	  new->data[k + (bytesperrow16 * j)] = 0;
	  new->data[k + 1 + (bytesperrow16 * j)] = 0;
	  new->data[k + 2 + (bytesperrow16 * j)] = 0;
	}
  }  
  return new;
}
BMP_Image *Convert_16_to_24_BMP_Image(BMP_Image *image)
{
  BMP_Image * new = NULL;
  int pad16;
  int rowbytes16;
  int rowbytes24;
  int pad24;
  int nimagesize;
  uint8_t r,g,b, byte1, byte2;
  //uint16_t pixel;
  int i,j, k;
  //uint16_t byte;
  uint16_t thing;
  //acount for padding in 16bit image
  rowbytes16 = (image->header.bits * image->header.width) / 8;	
  if(image->header.width % 2 != 0)
  {
	pad16 = 4 - (((image->header.width) * 16) / 8);
  }
  else
  {
	pad16 = 0;
  }
  rowbytes16 += pad16;
  
  //allocate space for new image 
  new = malloc(sizeof(BMP_Image));
  new->header = image->header;
  new->header.bits = 24;
  //account for padding in 24 bit image  
  rowbytes24 = (24 * new->header.width) / 8;

  if((rowbytes24 % 4) != 0)
  {
	pad24 = 4 - (rowbytes24 % 4);
  }
  else
  {
	pad24 = 0;
  }
  
  rowbytes24 += pad24;
  nimagesize = rowbytes24 * image->header.height;
  //new->data = malloc(nimagesize);
  new->header.imagesize = nimagesize;
  new->data = (unsigned char *)calloc(nimagesize , sizeof(unsigned char));
  new->header.size = (rowbytes24 * image->header.height) + 54;

  for(i = 0; i < image->header.height; i++)
  {
    for(j = 0; j < image->header.width; j++)
	{
	  byte1 = image->data[(j * 2 + (rowbytes16 * i))];
	  byte2 = image->data[(j * 2 + (rowbytes16 * i) + 1)];
	  thing = byte1 | byte2 << 8;
	  
	  r = (thing & RED_MASK)>> 10;
	  g = (thing & GREEN_MASK) >> 5;
	  b = thing & BLUE_MASK;
  
	  /*
	  r = ((byte1 & RED_MASK) << 1) >> 3;
	  g1 = ((byte1 & GREEN_MASK) << 6);
	  g2 = (byte2 & GREEN_MASK) >> 5;
	  g = g1 | g2;
	  b = (byte2 & BLUE_MASK)<< 3;
	  */
	  
	  r = (r * 255) / 31;
	  b = (b * 255) / 31;
	  g = (g * 255) / 31;

	  //extract bgr values
	  //r = (((image->data[(j * 2) + (rowbytes16 * i)]) >> 10) * 255) / 31;
	  //g = ((((image->data[(j * 2) + (rowbytes16 * i) + 1]) >> 5)) * 255) / 31;
	  //b = (((image->data[(j * 2) + (rowbytes16 * i) + 2])  << 10) * 255) / 31;
	  //r = (((image->data[(j * 2) + (rowbytes16 * i)]) << 1) >> 3 * 255) / 31;

	  //r = ((image->data[(j * 2) + (rowbytes16 * i)] << 1) >> 3) * 255 / 31;
	  //g1 = (image->data[(j * 2) + (rowbytes16 * i)] << 6) >> 1;  
          //g2 = (image->data[(j * 2) + (rowbytes16 * i) + 1]) >> 5;
          //g = g1 | g2;
	  //b = (((image->data[(j * 2) + (rowbytes16 * i) + 1])  << 3) * 255) / 31;
	  //g = (g * 255) / 31;
	  
	  //store values into new data
	  new->data[(j * 3) + (rowbytes24 * i)] = b;
	  new->data[(j * 3) + (rowbytes24 * i) + 1] = g;
	  new->data[(j * 3) + (rowbytes24 * i) + 2] = r;
	}
       
	for(k = 0; k < pad24; k++)
	{
	  new->data[(k)+ (rowbytes24 * i)] = 0;
	  new->data[(k) + (rowbytes24 * i) + 1] = 0;
	  new->data[(k) + (rowbytes24 * i) + 2] = 0;
	}
	
  } 
  return new;
}
