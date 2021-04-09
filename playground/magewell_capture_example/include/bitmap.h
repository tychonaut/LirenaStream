#ifndef BITMAP_H
#define BITMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LibMWCapture/WinTypes.h"


#pragma pack(push)
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    DWORD biSize;
    DWORD biWidth;
    DWORD biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    DWORD biXPelsPerMeter;
    DWORD biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}BITMAPINFOHEADER;

static void create_bitmap(char *pFileName, unsigned long pbFrame, int cbFrame, int cx, int cy)
{
    //set the file deader for bitmap file
    BITMAPFILEHEADER bitMapFileHeader;
    memset(&bitMapFileHeader,0,sizeof(BITMAPFILEHEADER));

    bitMapFileHeader.bfType = 0x4D42;
    bitMapFileHeader.bfSize = cbFrame + 54;//  m_cx*m_cy*3+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)
    bitMapFileHeader.bfReserved1 = 0;
    bitMapFileHeader.bfReserved2 = 0;
    bitMapFileHeader.bfOffBits = 54;  //sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)

    //set the info header for Bitmap file
    BITMAPINFOHEADER bitMapInfoHeader;
    memset(&bitMapInfoHeader,0,sizeof(BITMAPINFOHEADER));

    bitMapInfoHeader.biSize = 40;//sizeof(BITMAPINFOHEADER)
    bitMapInfoHeader.biWidth = cx;//XI_CAPTURE_WIDTH
    bitMapInfoHeader.biHeight = cy;//XI_CAPTURE_HEIGHT
    bitMapInfoHeader.biPlanes = 1;
    bitMapInfoHeader.biBitCount = 24;
    bitMapInfoHeader.biCompression = 0;
    bitMapInfoHeader.biSizeImage = 0;
    bitMapInfoHeader.biXPelsPerMeter = 0;
    bitMapInfoHeader.biYPelsPerMeter = 0;
    bitMapInfoHeader.biClrUsed =0;
    bitMapInfoHeader.biClrImportant =0;

    //creae the BipMap file
    FILE *fd = fopen(pFileName,"wb");
    if(!fd)
        return;
    fwrite((const char *)&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, fd);
    fwrite((const char *)&bitMapInfoHeader, sizeof(BITMAPINFOHEADER),1 ,fd);

    fwrite((const char *)pbFrame,cbFrame,1,fd);

    fclose(fd);
}

#pragma pack(pop)
#endif // BITMAP_H
