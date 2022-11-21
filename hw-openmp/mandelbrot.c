/*
  This program is an adaptation of the Mandelbrot program
  from the Programming Rosetta Stone, see
  http://rosettacode.org/wiki/Mandelbrot_set

  Compile the program with:

  gcc -o mandelbrot -O4 mandelbrot.c

  Usage:
 
  ./mandelbrot <xmin> <xmax> <ymin> <ymax> <maxiter> <xres> <out.ppm>

  Example:

  ./mandelbrot 0.27085 0.27100 0.004640 0.004810 1000 1024 pic.ppm

  The interior of Mandelbrot set is black, the levels are gray.
  If you have very many levels, the picture is likely going to be quite
  dark. You can postprocess it to fix the palette. For instance,
  with ImageMagick you can do (assuming the picture was saved to pic.ppm):

  convert -normalize pic.ppm pic.png

  The resulting pic.png is still gray, but the levels will be nicer. You
  can also add colors, for instance:

  convert -negate -normalize -fill blue -tint 100 pic.ppm pic.png

  See http://www.imagemagick.org/Usage/color_mods/ for what ImageMagick
  can do. It can do a lot.
*/
/*
Compute Times: (loop time, total elapsed time)
01 - 29.19, 34.73
02 - 15.32, 21.35
04 - 8.23,   14.84
08 - 4.74,  10.78
16 - 3.23,  9.28
32 - 2.62,  8.70

1. 
There are 20 cores on my machine

2.
The time halves as the threads doubles

3.
Sp = T1/Tp
t1 = 29.19
tp = 8.23
Sp = 3.55

4.
At 16 cores, the time elapsed stopped halving

5.
With only 20 cores being available, after increasing the number
of threads, each core can only run one thread at a time, so it
stops being efficient.

6.
Sp = T1/Tp
p = 4
T1 = 34.73
Tp = 14.84
Sp = 2.34

7.
Ep = Sp/p = T(p*Tp)
Sp = 2.34
p = 4
Ep = 0.59

8.
There is a part of the program that is not parrelizable,
and that lowers the efficiency.

9.
speedup of parallel region = 3.55 (a)
elapsed time for 4 threads = 14.84 (Ta)
elapsed time for 1 thread = 34.73 (T)

14.84 = p * 34.73 / 3.55 + (1 - p) * 34.73
14.84 = p * 9.78 + 34.73 - 34.73 * p
-19.89 = p * (9.78 - 34.73)
-19.89 = p * -24.95
p = 0.797

10.
Tα = pT/(inf) + (1-0.797)34.73
Tα = 7.05019

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <omp.h>

int main(int argc, char* argv[])
{
  /* Parse the command line arguments. */
  if (argc != 8) {
    printf("Usage:   %s <xmin> <xmax> <ymin> <ymax> <maxiter> <xres> <out.ppm>\n", argv[0]);
    printf("Example: %s 0.27085 0.27100 0.004640 0.004810 1000 1024 pic.ppm\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* The window in the plane. */
  const double xmin = atof(argv[1]);
  const double xmax = atof(argv[2]);
  const double ymin = atof(argv[3]);
  const double ymax = atof(argv[4]);

  /* Maximum number of iterations, at most 65535. */
  const uint16_t maxiter = (unsigned short)atoi(argv[5]);

  /* Image size, width is given, height is computed. */
  const int xres = atoi(argv[6]);
  const int yres = (xres*(ymax-ymin))/(xmax-xmin);

  /* The output file name */
  const char* filename = argv[7];

  /* Open the file and write the header. */
  FILE * fp = fopen(filename,"wb");
  char *comment="# Mandelbrot set";/* comment should start with # */

  /*write ASCII header to the file*/
  fprintf(fp,
          "P6\n# Mandelbrot, xmin=%lf, xmax=%lf, ymin=%lf, ymax=%lf, maxiter=%d\n%d\n%d\n%d\n",
          xmin, xmax, ymin, ymax, maxiter, xres, yres, (maxiter < 256 ? 256 : maxiter));

  /* Precompute pixel width and height. */
  double dx=(xmax-xmin)/xres;
  double dy=(ymax-ymin)/yres;

  double x, y; /* Coordinates of the current point in the complex plane. */
  double u, v; /* Coordinates of the iterated point. */
  int i,j; /* Pixel counters */
  int k; /* Iteration counter */
  int *saved = malloc(sizeof(int)*yres*xres);

  double start = omp_get_wtime();

  #pragma omp parallel for private(i,k,x,y,u,v) firstprivate(saved)
  for (j = 0; j < yres; j++) {
    y = ymax - j * dy;
    for(i = 0; i < xres; i++) {
      double u = 0.0;
      double v= 0.0;
      double u2 = u * u;
      double v2 = v*v;
      x = xmin + i * dx;
      /* iterate the point */
      for (k = 1; k < maxiter && (u2 + v2 < 4.0); k++) {
            v = 2 * u * v + y;
            u = u2 - v2 + x;
            u2 = u * u;
            v2 = v * v;
      };
      saved[xres * j + i] = k;
    }
  }

  double end = omp_get_wtime();


  for (j = 0; j < yres; j++) {
    for(i = 0; i < xres; i++) {
      /* compute  pixel color and write it to file */
      k = saved[xres * j + i];
      if (k >= maxiter) {
        /* interior */
        const unsigned char black[] = {0, 0, 0, 0, 0, 0};
        fwrite (black, 6, 1, fp);
      }
      else {
        /* exterior */
        unsigned char color[6];
        color[0] = k >> 8;
        color[1] = k & 255;
        color[2] = k >> 8;
        color[3] = k & 255;
        color[4] = k >> 8;
        color[5] = k & 255;
        fwrite(color, 6, 1, fp);
      };
    }
  }

  printf("Loop Time= %f\n",end-start);

  fclose(fp);
  free(saved);
  return 0;
}
