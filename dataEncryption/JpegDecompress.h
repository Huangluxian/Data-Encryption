//
//  JpegDecompress.h
//  dataHiding
//
//  Created by 黄路衔 on 16/3/7.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#ifndef JpegDecompress_h
#define JpegDecompress_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <string>
using namespace std;

class Jpeg_Decompress {
public:
    Jpeg_Decompress();
    ~Jpeg_Decompress();
    
    void read_Jpeg(const char *jpegName);
    bool alloc_Mem();
    void start_Decode();
    void start_Decompress();
    
    /*保存为 bmp 格式，便于查看*/
    void tran_Colorspace();
    bool write_Bmp(const char *bmpName);
    
private:
    /*jpeg 文件相关信息*/
    int width;                  //图像的宽
    int height;                 //图像的高
    int biBitCount;             //图像类型，每像素位数
    unsigned char *Ycode, *Cbcode, *Crcode;   //三通道的原始编码
    char *Z_Y, *Z_Cb, *Z_Cr;    //以 Zigzag 扫描并量化后的 YCbCr 通道一维数组
    double **Y, **Cb, **Cr;     //YCbCr 矩阵
    unsigned char **R, **G, **B;//RGB 矩阵
    
    /*亮度和色度的量化表*/
    unsigned char Y_Quan_Table[8][8];
    unsigned char C_Quan_Table[8][8];
    
    /* jpeg 储存标准的哈弗曼编码表*/
    unsigned int YDC_Huff_Table[16];
    unsigned int YDC_Huff_Value[12];
    unsigned int CDC_Huff_Table[16];
    unsigned int CDC_Huff_Value[12];
    unsigned int YAC_Huff_Table[16];
    unsigned int YAC_Huff_Value[130];
    unsigned int CAC_Huff_Table[16];
    unsigned int CAC_Huff_Value[130];
    
    string *YDC_Huff_Code, *CDC_Huff_Code, *YAC_Huff_Code, *CAC_Huff_Code;//便于查表
    int YDC_table_size, YAC_table_size, CDC_table_size, CAC_table_size;
    int YcodeSize, CbcodeSize, CrcodeSize;
    
    /*
     将便于储存的哈弗曼编码转化为便于查找的哈弗曼编码
     按顺序依次读出各个哈弗曼编码，以便与解码后的值相对应
     */
    void huff_Code_2_String();
    
    /*释放中间变量*/
    void release_Code();
    void release_YCbCr();
    void release_RGB();
};

#endif /* JpegDecompress_h */
