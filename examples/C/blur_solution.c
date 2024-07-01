#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#pragma acc routine seq
unsigned char weight(unsigned char* pic, int x, int y, int l, size_t cols)
{
   /**
    * Compute the weight of the blurred pixel
    * @param pic(in): a pointer to the pixel to be blurred
    * @param x(in): the x coordinate of the pixel
    * @param y(in): the y coordinate of the pixel
    * @param l(in): the color coordinate of the pixel (0 for R, 1 for G, 2 for B)
    * @param cols(in) the number of columns in the picture
    * @return the weight of the blurred pixel
    */
   unsigned char coefs[5][5] = { {1,  4,  6,  4,  1},
                                 {4, 16, 24, 16,  4},
                                 {6, 24, 36, 24,  6},
                                 {4, 16, 24, 16,  4},
                                 {1,  4,  6,  4,  1}};
   int i, j;
   int pix = 0;
#pragma acc loop collapse(2) reduction(+:pix) 
   for (i=0; i<5; ++i)
      for (j=0; j<5; ++j)
         pix += pic[(x+i-2)*3*cols+y*3+l-2+j]*coefs[i][j];
   return (unsigned char) (pix/256);
}

void blur(unsigned char* pic,  unsigned char* blurred, size_t rows, size_t cols)
{
    /**
     * Perform the blurring of the picture
     * @ param pic(in): a pointer to the original picture
     * @ param blurred(out): a pointer to the blurred picture
     */
   size_t i, j, l;
#pragma acc parallel loop present(blurred[:rows*3*cols], pic[:rows*3*cols]) async(2)\
   num_gangs(200) 
   for (i=2; i<rows-2; ++i)
#pragma acc loop independent collapse(2)
      for (j=2; j<cols-2; ++j)
         for (l=0; l<3; ++l)
            blurred[i*3*cols+j*3+l] = weight(pic, i, j, l, cols);
}
void fill(unsigned char* pic, size_t rows, size_t cols)
{
    /**
     * Fill the picture with data
     * @param pic(in): a pointer to the pixel to be blurred
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     *
     */
   size_t i, j;
#pragma acc parallel loop collapse(2) present(pic[:rows*3*cols])
   for (i=0; i < rows; ++i)
      for (j=0; j < 3*cols; ++j)
         pic[i*3*cols+j] = (unsigned char) (i+(j%3)*j+i%256)%256;
}

void out_pic(unsigned char* pic, char* name, size_t rows, size_t cols)
{
    /**
     * Output of the picture into a sequence of pixel
     * Use show_rgb(filepath, rows, cols) to display
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     */
   FILE* f = fopen(name, "wb");
   fwrite(pic, sizeof(unsigned char), rows*3*cols, f);
   fclose(f);
}

unsigned char* allocate(size_t rows, size_t cols)
{
    /**
     * Allocate memory for the picture
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     */
   unsigned char* pic = (unsigned char*) malloc(rows*3*cols*sizeof(unsigned char));
#pragma acc enter data create(pic[0:rows*3*cols])
   return pic;
}

unsigned int checksum(unsigned char* pic, size_t rows, size_t cols)
{
    /**
     * Perform a simple checksum
     * @param pic(in): a pointer to the pixel to be blurred
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     *
     */
    unsigned int sum1=0;
    unsigned int sum2=0;
    unsigned int val=42424242;
#pragma acc parallel loop present(pic[:3*cols*rows]) reduction(+:sum1) firstprivate(sum2) wait(2)
    for (size_t i=2; i < rows-2; ++i)
    {
        sum2 = 0;

#pragma acc loop reduction(+:sum2) collapse(2)
        for (size_t j=2; j < cols-2; ++j)
            for (size_t l=0; l < 3; ++l)
                sum2 += pic[i*3*cols+j*3+l];

        sum1 += sum2 % 256 ;
    }
    return ((sum1 ^ val)<<8)+sum1;
}
void free_pic(unsigned char* pic, size_t rows, size_t cols)
{
    /**
     * Free the memory allocated for the picture
     * @param pic(in): a pointer to the picture to be freed
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     */
#pragma acc exit data delete(pic[:rows*3*cols])
   free(pic);
}

int main(int argc, char** argv)
{
   size_t rows,cols;
   unsigned int check;

   // Get the size of the picture
   // Default to 4000x 4000
   if (argc >= 3)
   {
       rows = (size_t) strtol(argv[1], NULL, 10);
       cols = (size_t) strtol(argv[2], NULL, 10);
   } else
   {
       rows = 4000;
       cols = 4000;
   }
   printf("Size of picture is %d x %d\n", rows, cols); 
   unsigned char* pic = allocate(rows, cols);
   unsigned char* blurred_pic = allocate(rows, cols);

   // Create the original picture
   fill(pic, rows, cols);
#pragma acc update self(pic[0:rows*3*cols]) async(1)

   // Apply the blurring filter
   blur(pic, blurred_pic, rows, cols);

   // Perform the checksum on the blurred picture
   check = checksum(blurred_pic, rows, cols);

#pragma acc update self(blurred_pic[:rows*3*cols]) wait(2) 
   // Output a picture if we have a reasonable size
   if (cols*rows <= 16e6)
   {
       out_pic(pic, "pic.rgb", rows, cols);
       out_pic(blurred_pic, "blurred.rgb", rows, cols);
   }

   printf("Checksum 0x%x\n", check);

   // Free the memory on the host and the device
   free_pic(pic, rows, cols);
   free_pic(blurred_pic, rows, cols);

   return 0;
}

