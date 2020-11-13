/*----------------------------------------------------------------------
  File:    texture.h
  Purpose: Stuff concerning textures
           reads PPM files (only in P6 format, i.e., RGB binary)
----------------------------------------------------------------------*/

#ifndef TEXTURE_H
#define TEXTURE_H

struct Texture
{
   int mWidth;
   int mHeight;
   unsigned char *mRGB; /* the size is 3*mWidth*mHeight */
};

struct Texture *viReadPPM(char *filename);
int viWritePPM(char *filename, int width, int height, unsigned char *pixels);

#endif
