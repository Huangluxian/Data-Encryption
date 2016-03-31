//
//  bmpStruct.h
//  dataHiding
//
//  Created by 黄路衔 on 16/3/3.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#ifndef bmpStruct_h
#define bmpStruct_h

#define DWORD unsigned int
#define WORD unsigned short
#define BYTE unsigned char

/*
 定义 bmp 文件结构
 */
// bmp 文件头结构
#include "pshpack2.h"  //字节对齐
typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;    // 位图文件类型
    DWORD   bfSize;    // 文件大小，以字节为单位
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits; // 位图数据的起始位置，以相对于位图文件头的偏移量表示，以字节为单位
} BITMAPFILEHEADER;
#include "poppack.h"   //恢复默认字节对齐

// bmp 文件信息头结构
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    int biWidth;
    int biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

// bmp 颜色表
typedef struct tagRGBQUAD {
    BYTE rgbBlue;// 蓝色的亮度(值范围为0-255)
    BYTE rgbGreen; // 绿色的亮度(值范围为0-255)
    BYTE rgbRed; // 红色的亮度(值范围为0-255)
    BYTE rgbReserved;// 保留，必须为0
} RGBQUAD;


#endif /* bmpStruct_h */
