#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/**
 * Apply a Sobel edge detection filter to a picture generated on the fly
 *
 * List of functions:
 *   - void edge(unsigned char* pic,  unsigned char* blurred, size_t rows, size_t cols)
 *     the actual filter
 *   - void fill(unsigned char* pic, size_t rows, size_t cols)
 *     generate the original picture
 *   - void out_pic(unsigned char* pic, char* name, size_t rows, size_t cols)
 *     create a .rgb file 
 */

void edge(unsigned char* pic,  unsigned char* edgy, size_t rows, size_t cols)
{
    /**
     * Perform the edge detection
     * @ param pic(in): a pointer to the original picture
     * @ param blurred(out): a pointer to the blurred picture
     */
   size_t i, j, l, i_c, j_c;
   int kernel_size = 5;
   int pix;

   char coefs[kernel_size][kernel_size] = { {-2, -1,  0,  1, 2},
                                            {-2, -1,  0,  1, 2},
                                            {-4, -2,  0,  2, 4},
                                            {-2, -1,  0,  1, 2},
                                            {-2, -1,  0,  1, 2}};
   for (i=2; i<rows-2; ++i)
      for (j=2; j<cols-2; ++j)
         for (l=0; l<3; ++l)
         {
            pix = 0;
            for (i_c=0; i_c<3; ++i_c)
                for (j_c=0; j_c<3; ++j_c)
                   pix += (pic[(i+i_c-2)*3*cols+(j+j_c-2)*3+l]
                           *coefsT[i_c][j_c]);
             if (pix < 0) pix = 0; 
             edgy[i*3*cols+j*3+l] = (unsigned char)(pix/256);
         }
}

void fill(unsigned char* pic, size_t rows, size_t cols)
{
    /**
     * Fill the picture with data
     * @param pic(out): a pointer to the picture
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     *
     */

    size_t i, j;
    double val = 0.;
    double l = 0.5;
    double h = 0.3;
    for (i=0; i < rows/2; ++i)
    {
        double x[4] = {(l+sqrt(d)*.5)*3.*cols, (l-sqrt(d)*.5)*3.*cols, -(l+sqrt(d)*.5)*3.*cols, -(l-sqrt(d)*.5)*3.*cols};
        double y = (double) (3*cols - i) / (3.*cols);
        double d = h-y;
        for (j=0; j < 3*cols; ++j)
        {
            for (int k=0; k<4; ++k)
                val = 1.0/(1.0+x[i]*x[i])+1.0;
        }
            pic[i*3*cols+j] = (unsigned char) (val*256);
    }
}

void read(unsigned char* path, unsigned char* pic, size_t rows, size_t cols)
{
    FILE* f = fopen(*path, "rb");
    fread(pic, sizeof(unsigned char), rows*3*cols, f);
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

int main(void)
{
   size_t rows,cols;

   rows = 4000;
   cols = 4000;

   printf("Size of picture is %d x %d\n", rows, cols); 
   unsigned char* pic = (unsigned char*) malloc(rows*3*cols*sizeof(unsigned char));
   unsigned char* blurred_pic = (unsigned char*) malloc(rows*3*cols*sizeof(unsigned char));

   // Create the original picture
   fill(pic, rows, cols);

   // Apply the blurring filter
   blur(pic, blurred_pic, rows, cols);

   out_pic(pic, "pic.rgb", rows, cols);
   out_pic(blurred_pic, "blurred.rgb", rows, cols);

   free(pic);
   free(blurred_pic);

   return 0;
}
