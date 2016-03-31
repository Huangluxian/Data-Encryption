//
//  JpegDecompress.cpp
//  dataHiding
//
//  Created by 黄路衔 on 16/3/7.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include "bmpStruct.h"
#include "JpegDecompress.h"
#include "commonFun.h"

Jpeg_Decompress::Jpeg_Decompress()
{
    width = 0;
    height = 0;
    biBitCount = 0;
}

Jpeg_Decompress::~Jpeg_Decompress()
{
}

void Jpeg_Decompress::read_Jpeg(const char *jpegName)
{
    ifstream inFile;
    inFile.open(jpegName, ios::binary);
    char ch;
    
    /*读取图像的基本信息*/
    inFile >> width >> height >> biBitCount;
    
    /*读取量化表*/
    ch = inFile.get();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            Y_Quan_Table[i][j] = inFile.get();
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            C_Quan_Table[i][j] = inFile.get();
        }
    }
    
    /*读取哈夫曼表*/
    YDC_table_size = 0;
    for (int i = 0; i < 16; i++) {
        inFile >> YDC_Huff_Table[i];
        YDC_table_size = YDC_table_size + YDC_Huff_Table[i];
    }
    for (int i = 0; i < YDC_table_size; i++) {
        inFile >> YDC_Huff_Value[i];
    }
    
    YAC_table_size = 0;
    for (int i = 0; i < 16; i++) {
        inFile >> YAC_Huff_Table[i];
        YAC_table_size = YAC_table_size + YAC_Huff_Table[i];
    }
    for (int i = 0; i < YAC_table_size; i++) {
        inFile >> YAC_Huff_Value[i];
    }
    
    CDC_table_size = 0;
    for (int i = 0; i < 16; i++) {
        inFile >> CDC_Huff_Table[i];
        CDC_table_size = CDC_table_size + CDC_Huff_Table[i];
    }
    for (int i = 0; i < CDC_table_size; i++) {
        inFile >> CDC_Huff_Value[i];
    }
    
    CAC_table_size = 0;
    for (int i = 0; i < 16; i++) {
        inFile >> CAC_Huff_Table[i];
        CAC_table_size = CAC_table_size + CAC_Huff_Table[i];

    }
    for (int i = 0; i < CAC_table_size; i++) {
        inFile >> CAC_Huff_Value[i];
    }
    
    /*
     读取图像数据
     遇到连续两个 0xff 即表示该通道数据终止
     */
    ch = inFile.get();
    int i = 0, count = 0;
    unsigned char *Ych;
    Ych = new unsigned char[100000000];
    Ych[i] = inFile.get();
    while (count < 2) {
        i++;
        Ych[i] = inFile.get();
        if (0xff == Ych[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    YcodeSize = i - 1;
    Ycode = new unsigned char [YcodeSize];
    for (int z = 0; z < YcodeSize; z++) {
        Ycode[z] = Ych[z];
    }
    
    i = 0;
    count = 0;
    unsigned char *Cbch;
    Cbch = new unsigned char[100000000];
    Cbch[i] = inFile.get();
    while (count < 2) {
        i++;
        Cbch[i] = inFile.get();
        if (0xff == Cbch[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    CbcodeSize = i - 1;
    Cbcode = new unsigned char [CbcodeSize];
    for (int z = 0; z < CbcodeSize; z++) {
        Cbcode[z] = Cbch[z];
    }
    
    i = 0;
    count = 0;
    unsigned char *Crch;
    Crch = new unsigned char[100000000];
    Crch[i] = inFile.get();
    while (count < 2) {
        i++;
        Crch[i] = inFile.get();
        if (0xff == Crch[i]) {
            count++;
        } else {
            count = 0;
        }
    }
    CrcodeSize = i - 1;
    Crcode = new unsigned char [CrcodeSize];
    for (int z = 0; z < CrcodeSize; z++) {
        Crcode[z] = Crch[z];
    }
    
    delete [] Ych;
    delete [] Cbch;
    delete [] Crch;
    inFile.clear();
    inFile.close();
}

bool Jpeg_Decompress::alloc_Mem()
{
    Z_Y = new char[width * height];
    Z_Cb = new char[width * height];
    Z_Cr = new char[width * height];
    
    Y = new double *[height];
    Cb = new double *[height];
    Cr = new double *[height];
    R = new unsigned char *[height];
    G = new unsigned char *[height];
    B = new unsigned char *[height];
    for (int i = 0; i < height; i++) {
        Y[i] = new double[width];
        Cb[i] = new double[width];
        Cr[i] = new double[width];
        R[i] = new unsigned char[width];
        G[i] = new unsigned char[width];
        B[i] = new unsigned char[width];
    }
    
    if (NULL == Z_Y || NULL == Z_Cb || NULL == Z_Cr || NULL == Y || NULL == Cb || NULL == Cr || NULL == R || NULL == G || NULL == B) {
        delete []Ycode;
        delete []Cbcode;
        delete []Crcode;
        return false;
    }
    return true;
}

void Jpeg_Decompress::release_Code()
{
    delete []Ycode;
    delete []Cbcode;
    delete []Crcode;
    delete []YAC_Huff_Code;
    delete []YDC_Huff_Code;
    delete []CAC_Huff_Code;
    delete []CDC_Huff_Code;
}

void Jpeg_Decompress::release_YCbCr()
{
    for (int i = 0; i < height; i++) {
        delete[] Y[i];
        delete[] Cb[i];
        delete[] Cr[i];
    }
    delete []Y;
    delete []Cb;
    delete []Cr;
    delete []Z_Y;
    delete []Z_Cb;
    delete []Z_Cr;
}

void Jpeg_Decompress::release_RGB()
{
    for (int i = 0; i < height; i++) {
        delete[] R[i];
        delete[] G[i];
        delete[] B[i];
    }
    delete []R;
    delete []G;
    delete []B;
}

void Jpeg_Decompress::huff_Code_2_String()
{
    int table_Size, index;
    string code;
    
    /*转换亮度 DC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + YDC_Huff_Table[i];
    }
    YDC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != YDC_Huff_Table[i]) {
            YDC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < YDC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            YDC_Huff_Code[index++] = code;
        }
        int k = 1;
        while('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换色度 DC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + CDC_Huff_Table[i];
    }
    CDC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != CDC_Huff_Table[i]) {
            CDC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < CDC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            CDC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换亮度 AC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + YAC_Huff_Table[i];
    }
    YAC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != YAC_Huff_Table[i]) {
            YAC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < YAC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            YAC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
    
    /*转换色度 AC 哈弗曼编码*/
    table_Size = 0;
    for (int i = 0; i < 16; i++) {
        table_Size = table_Size + CAC_Huff_Table[i];
    }
    CAC_Huff_Code = new string [table_Size];
    
    code = "00";
    index = 0;
    for (int i = 1; i < 16; i++) {
        if (0 != CAC_Huff_Table[i]) {
            CAC_Huff_Code[index++] = code;
        }
        for (int j = 1; j < CAC_Huff_Table[i]; j++) {
            int k = 1;
            while ('0' != code[code.length() - k]) {
                code[code.length() - k] = '0';
                k++;
            }
            code[code.length() - k] = '1';
            CAC_Huff_Code[index++] = code;
        }
        int k = 1;
        while ('0' != code[code.length() - k]) {
            code[code.length() - k] = '0';
            k++;
        }
        code[code.length() - k] = '1';
        code = code + "0";
    }
}

void Jpeg_Decompress::start_Decode()
{
    huff_Code_2_String();
    int i, j, index, num;
    int zeronum;
    int DClength, AClength;
    string allcode, subcode;
    unsigned char temp;
    
    /**********处理 Y 通道**********/
    for (i = 0; i < YcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Ycode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    /*解码为 DC、AC 值*/
    index = 0;
    num = 0;
    DClength = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < YDC_table_size; i++) {
                if (subcode == YDC_Huff_Code[i]) {
                    DClength = YDC_Huff_Value[i];
                    if (0 == DClength) {
                        if(0 == num) {
                            Z_Y[num] = 0;
                        } else {
                            Z_Y[num] = Z_Y[num - 64];
                        }
                        index++;
                        num++;
                        break;
                    }
                    index++;
                    string DCcode = allcode.substr(index, DClength);
                    if(0 == num) {
                        Z_Y[num] = Decode_VLC(DCcode);
                        num++;
                    } else {
                        Z_Y[num] = Decode_VLC(DCcode) + Z_Y[num - 64];
                        num++;
                    }
                    index = index + DClength;
                    break;
                }
            }
            if (i == YDC_table_size) {
                /*表明查表无结果，即需要再读一位继续查表*/
                index++;
            } else {
                break;
            }
        }
        /*读取 AC 值*/
        i = 0;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < YAC_table_size; j++) {
                if (subcode == YAC_Huff_Code[j]) {
                    if (0 == YAC_Huff_Value[j]) {
                        for (int k = i; k < 63; k++) {
                            Z_Y[num++] = 0;
                        }
                        i = 63;
                        index++;
                        break;
                    } else if(0xf0 == YAC_Huff_Value[j]) {
                        for (int k = 0; k < 16; k++) {
                            Z_Y[num++] = 0;
                        }
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = YAC_Huff_Value[j] / 16;
                        AClength = YAC_Huff_Value[j] % 16;
                        for (int k = 0; k < zeronum; k++) {
                            Z_Y[num++] = 0;
                        }
                        index++;
                        string ACcode = allcode.substr(index, AClength);
                        Z_Y[num++] = Decode_VLC(ACcode);
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == YAC_table_size) {
                index++;
            } else {
                subcode=subcode.substr(0, 0);
            }
        }
        if(num == width * height) {
            break;
        }
    }
    
    /**********处理 Cb 通道**********/
    allcode = allcode.substr(0, 0);
    subcode = subcode.substr(0, 0);
    for (i = 0; i < CbcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Cbcode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    /*解码为 DC、AC 值*/
    index = 0;
    num = 0;
    DClength = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < CDC_table_size; i++) {
                if (subcode == CDC_Huff_Code[i]) {
                    DClength = CDC_Huff_Value[i];
                    if (0 == DClength) {
                        if (0 == num) {
                            Z_Cb[num] = 0;
                        } else {
                            Z_Cb[num] = Z_Cb[num - 64];
                        }
                        index++;
                        num++;
                        break;
                    }
                    index++;
                    string DCcode = allcode.substr(index, DClength);
                    if (0 == num) {
                        Z_Cb[num++] = Decode_VLC(DCcode);
                    } else {
                        Z_Cb[num] = Decode_VLC(DCcode) + Z_Cb[num - 64];
                        num++;
                    }
                    index = index + DClength;
                    break;
                }
            }
            if (i == CDC_table_size) {
                index++;
            } else {
                break;
            }
        }
        /*读取 AC 值*/
        i = 0;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < CAC_table_size; j++) {
                if (subcode == CAC_Huff_Code[j]) {
                    if (0 == CAC_Huff_Value[j]) {
                        for (int k = i; k < 63; k++) {
                            Z_Cb[num++] = 0;
                        }
                        i = 63;
                        index++;
                        break;
                    } else if (0xf0 == CAC_Huff_Value[j]) {
                        for (int k = 0; k < 16; k++) {
                            Z_Cb[num++] = 0;
                        }
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = CAC_Huff_Value[j] / 16;
                        AClength=CAC_Huff_Value[j] % 16;
                        for (int k = 0; k < zeronum; k++) {
                            Z_Cb[num++] = 0;
                        }
                        index++;
                        string ACcode = allcode.substr(index, AClength);
                        Z_Cb[num++] = Decode_VLC(ACcode);
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == CAC_table_size) {
                index++;
            } else {
                subcode = subcode.substr(0, 0);
            }
        }
        if (num == width * height) {
            break;
        }
    }
    
    /**********处理 Cr 通道**********/
    allcode = allcode.substr(0, 0);
    subcode = subcode.substr(0, 0);
    for (i = 0; i < CrcodeSize; i++) {
        for (j = 0; j < 8; j++) {
            temp = Crcode[i] >> (8 - j - 1);
            if (0 == (temp & 1)) {
                allcode += "0";
            } else {
                allcode += "1";
            }
        }
    }
    /*解码为 DC、AC 值*/
    index = 0;
    num = 0;
    DClength = 0;
    while (index < allcode.length()) {
        /*读取 DC 值*/
        while (index < allcode.length()) {
            subcode += allcode[index];
            for (i = 0; i < CDC_table_size; i++) {
                if (subcode == CDC_Huff_Code[i]) {
                    DClength = CDC_Huff_Value[i];
                    if (0 == DClength) {
                        if (0 == num) {
                            Z_Cr[num] = 0;
                        } else {
                            Z_Cr[num] = Z_Cr[num - 64];
                        }
                        index++;
                        num++;
                        break;
                    }
                    index++;
                    string DCcode = allcode.substr(index, DClength);
                    if (0 == num) {
                        Z_Cr[num++] = Decode_VLC(DCcode);
                    } else {
                        Z_Cr[num] = Decode_VLC(DCcode) + Z_Cr[num - 64];
                        num++;
                    }
                    index = index + DClength;
                    break;
                }
            }
            if (i == CDC_table_size) {
                index++;
            } else {
                break;
            }
        }
        /*读取 AC 值*/
        i = 0;
        subcode = subcode.substr(0, 0);
        while (i < 63 && (index < allcode.length())) {
            subcode += allcode[index];
            for (j = 0; j < CAC_table_size; j++) {
                if (subcode == CAC_Huff_Code[j]) {
                    if (0 == CAC_Huff_Value[j]) {
                        for (int k = i; k < 63; k++) {
                            Z_Cr[num++] = 0;
                        }
                        i = 63;
                        index++;
                        break;
                    } else if (0xf0 == CAC_Huff_Value[j]) {
                        for (int k = 0; k < 16; k++) {
                            Z_Cr[num++] = 0;
                        }
                        i += 16;
                        index++;
                        break;
                    } else {
                        zeronum = CAC_Huff_Value[j] / 16;
                        AClength = CAC_Huff_Value[j] % 16;
                        for (int k = 0; k < zeronum; k++) {
                            Z_Cr[num++] = 0;
                        }
                        index++;
                        string ACcode = allcode.substr(index, AClength);
                        Z_Cr[num++] = Decode_VLC(ACcode);
                        index = index + AClength;
                        i = i + 1 + zeronum;
                        break;
                    }
                }
            }
            if (j == CAC_table_size) {
                index++;
            } else
                subcode = subcode.substr(0, 0);
        }
        if (num == width * height) {
            break;
        }
    }
    
    release_Code();
}

void Jpeg_Decompress::start_Decompress()
{
    int i, j, k = 0, l = 0;
    
    /*分块*/
    double **block_Y, **block_Cb, **block_Cr;
    block_Y = new double *[8];
    block_Cb = new double *[8];
    block_Cr = new double *[8];
    for (i = 0; i < 8; i++) {
        block_Y[i] = new double[8];
        block_Cb[i] = new double[8];
        block_Cr[i] = new double[8];
    }
    int *block_Z_Y, *block_Z_Cb, *block_Z_Cr;
    block_Z_Y = new int [64];
    block_Z_Cb = new int [64];
    block_Z_Cr = new int [64];
    
    /*依次取出64个数，转化为8*8矩阵进行逆变换*/
    for (i = 0; i < width * height / 64; i++) {
        for (j = 0; j < 64; j++) {
            block_Z_Y[j] = Z_Y[i * 64 + j];
            block_Z_Cb[j] = Z_Cb[i * 64 + j];
            block_Z_Cr[j] = Z_Cr[i * 64 + j];
        }
        
        IZigzag(block_Z_Y, block_Y);
        IZigzag(block_Z_Cb, block_Cb);
        IZigzag(block_Z_Cr, block_Cr);
        
        IQuan(block_Y, Y_Quan_Table);
        IQuan(block_Cb, C_Quan_Table);
        IQuan(block_Cr, C_Quan_Table);
        
        IDCT(block_Y);
        IDCT(block_Cb);
        IDCT(block_Cr);
        
        /*将数据写进图像矩阵*/
        for (int m = 0; m < 8; m++) {
            for (int n = 0; n < 8; n++) {
                Y[k * 8 + m][l * 8 + n] = block_Y[m][n];
                Cb[k * 8 + m][l * 8 + n] = block_Cb[m][n];
                Cr[k * 8 + m][l * 8 + n] = block_Cr[m][n];
            }
        }
        l++;
        if (l * 8 == width) {
            l = 0;
            k++;
        }
    }
    
    for (i = 0; i < 8; i++) {
        delete[] block_Y[i];
        delete[] block_Cb[i];
        delete[] block_Cr[i];
    }
    delete []block_Y;
    delete []block_Cb;
    delete []block_Cr;
    delete []block_Z_Y;
    delete []block_Z_Cb;
    delete []block_Z_Cr;
}

void Jpeg_Decompress::tran_Colorspace()
{
    double r, g, b;
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            r = Y[i][j] + 0 * (Cb[i][j] - 128) + 1.402 * (Cr[i][j] - 128);
            g = Y[i][j] - 0.34414 * (Cb[i][j] - 128) - 0.71414 * (Cr[i][j] - 128);
            b = Y[i][j] + 1.772 * (Cb[i][j] - 128) + 0 * (Cr[i][j] - 128);
            
            /*将 double 类型的数据转化为 unsigned char 类型的数据，便于写进 bmp 文件*/
            if (r < 0) {
                R[i][j] = 0;
            } else if (r > 255) {
                R[i][j] = 255;
            } else {
                R[i][j] = (unsigned char)r;
            }
            if (g < 0) {
                G[i][j] = 0;
            } else if (g > 255) {
                G[i][j] = 255;
            } else {
                G[i][j] = (unsigned char)g;
            }
            if (b < 0) {
                B[i][j] = 0;
            } else if (b > 255) {
                B[i][j] = 255;
            } else {
                B[i][j] = (unsigned char)b;
            }
        }
    }
    release_YCbCr();
}

bool Jpeg_Decompress::write_Bmp(const char *bmpName)
{
    int i, j, k;
    /*每行字节数*/
    int lineByte = (width * biBitCount / 8 + 3) / 4 * 4;
    /*图像数据*/
    unsigned char* imgBuf;
    imgBuf = new unsigned char[lineByte * height];
    j = 0;
    for (i = (height - 1); i >= 0; i--) {
        for (k = 0; k < lineByte / 3; k++) {
            imgBuf[j++] = B[i][k];
            imgBuf[j++] = G[i][k];
            imgBuf[j++] = R[i][k];
        }
    }
    /*如果位图数据指针为0，则没有数据传入，函数返回 false*/
    if (!imgBuf) {
        cout << "写入数据为空" << endl;
        release_RGB();
        return false;
    }
    /*颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0*/
    int colorTablesize = 0;
    if (8 == biBitCount) {
        colorTablesize = 1024;
    }
    
    /*开始写 bmp 文件*/
    FILE *fp = fopen(bmpName, "wb");
    if (0 == fp) {
        cout << "创建 bmp 文件失败" << endl;
        delete []imgBuf;
        release_RGB();
        return false;
    }
    
    /*申请位图文件头结构变量，填写文件头信息*/
    BITMAPFILEHEADER fileHead;
    fileHead.bfType = 0x4D42; //bmp 类型
    /*bfSize 是图像文件4个组成部分之和*/
    fileHead.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte * height;
    fileHead.bfReserved1 = 0;
    fileHead.bfReserved2 = 0;
    /*bfOffBits是图像文件前3个部分所需空间之和*/
    fileHead.bfOffBits = 54 + colorTablesize;
    /*写文件头进文件*/
    fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);
    
    /*申请位图信息头结构变量，填写信息头信息*/
    BITMAPINFOHEADER head;
    head.biBitCount = biBitCount;
    head.biClrImportant = 0;
    head.biClrUsed = 0;
    head.biCompression = 0;
    head.biHeight = height;
    head.biPlanes = 1;
    head.biSize = 40;
    head.biSizeImage = lineByte * height;
    head.biWidth = width;
    head.biXPelsPerMeter = 0;
    head.biYPelsPerMeter = 0;
    /*写位图信息头进内存*/
    fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp);
    
    /*写位图数据进文件*/
    fwrite(imgBuf, height * lineByte, 1, fp);
    
    fclose(fp);
    delete []imgBuf;
    release_RGB();
    return true;
}