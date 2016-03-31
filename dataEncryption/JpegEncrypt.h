//
//  JpegEncrypt.h
//  dataHiding
//
//  Created by 黄路衔 on 16/3/8.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#ifndef JpegEncrypt_h
#define JpegEncrypt_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <string>
using namespace std;

class Jpeg_Encrypt {
public:
    Jpeg_Encrypt();
    ~Jpeg_Encrypt();
    
    void read_Jpeg(const char *jpegName);
    void part_Decode();
    void encrypt_Qtable();
    void encrypt_Appended_Bit();
    void write_Jpeg(const char *jpegName);
    
private:
    /*jpeg 文件相关信息*/
    int width;                  //图像的宽
    int height;                 //图像的高
    int biBitCount;             //图像类型，每像素位数
    unsigned char *Ycode, *Cbcode, *Crcode;   //三通道的编码
    int YcodeSize, CbcodeSize, CrcodeSize;
    string Y_Code_Str, Cb_Code_Str, Cr_Code_Str;
    
    /*加密密钥*/
    unsigned int key = 142857;   //作为产生伪随机数的种子
    
    /*加密字节*/
    string *Y_Huff_Bit, *Cb_Huff_Bit, *Cr_Huff_Bit;
    string *Y_Appended_Bit, *Cb_Appended_Bit, *Cr_Appended_Bit;
    int Y_Segment_Size, Cb_Segment_Size, Cr_Segment_Size;
    
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
    
    /*
     将便于储存的哈弗曼编码转化为便于查找的哈弗曼编码
     按顺序依次读出各个哈弗曼编码，以便与解码后的值相对应
     */
    void huff_Code_2_String();
    void replace_Appended_Bit();
    void release_Code();
};

#endif /* JpegEncrypt_h */
