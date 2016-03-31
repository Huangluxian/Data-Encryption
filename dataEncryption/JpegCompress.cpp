//
//  JpegCompress.cpp
//  dataHiding
//
//  Created by 黄路衔 on 16/3/3.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include "JpegCompress.h"
#include "commonFun.h"

Jpeg_Compress::Jpeg_Compress()
{
}

Jpeg_Compress::~Jpeg_Compress()
{
}

bool Jpeg_Compress::read_Bmp(const char *bmpName)
{
    FILE *fp = fopen(bmpName, "rb");
    if(0 == fp) {
        return false;
    }
    
    /*跳过位图文件头结构BITMAPFILEHEADER*/
    fseek(fp, sizeof(BITMAPFILEHEADER), 0);
    
    /*定义位图信息头结构变量，读取位图信息头进内存*/
    BITMAPINFOHEADER header;
    fread(&header, sizeof(BITMAPINFOHEADER), 1, fp);
    width = header.biWidth;
    height = header.biHeight;
    biBitCount = header.biBitCount;
    
    /*灰度图像有颜色表，且颜色表表项为256*/
    if(8 == biBitCount) {
        pColorTable = new RGBQUAD[256];
        fread(pColorTable, sizeof(RGBQUAD), 256, fp);
    }
    
    /*计算图像每行像素所占的字节数（并转换为8的倍数）*/
    lineByte = width * biBitCount / 8;
    width = (width + 7) / 8 * 8;
    oldHeight = height;
    height = (height + 7) / 8 * 8;
    
    /*申请位图数据所需要的空间，读位图数据进内存*/
    pBmpBuf = new unsigned char[lineByte * oldHeight];
    fread(pBmpBuf, 1, lineByte * oldHeight, fp);
    
    fclose(fp);
    return true;
}

bool Jpeg_Compress::alloc_Mem()
{
    /*
     RGB 通道矩阵分配内存
     由于需将宽高都调整为8的倍数，所以多余空间需要补0
     */
    R = new unsigned char * [height];
    G = new unsigned char * [height];
    B = new unsigned char * [height];
    for(int i = 0; i < height; i++) {
        R[i] = new unsigned char[width];
        G[i] = new unsigned char[width];
        B[i] = new unsigned char[width];
    }
    
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            R[i][j] = 0;
            G[i][j] = 0;
            B[i][j] = 0;
        }
    }
    
    /*YCbCr 通道矩阵分配内存*/
    Y = new double * [height];
    Cb = new double * [height];
    Cr = new double * [height];
    for (int i = 0; i < height; i++) {
        Y [i] = new double[width];
        Cb [i] = new double[width];
        Cr [i] = new double[width];
    }
    
    /*压缩后的 YCbCr 一维数组分配内存*/
    Z_Y = new char [height * width];
    Z_Cb = new char [height * width];
    Z_Cr = new char [height * width];
    
    if (NULL == R || NULL == G || NULL == B || NULL == Y || NULL == Cb || NULL == Cr || NULL == Z_Y || NULL == Z_Cb || NULL == Z_Cr) {
        delete [] pBmpBuf;
        return false;
    }
    return true;
}

void Jpeg_Compress::release_RGB()
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

void Jpeg_Compress::release_YCbCr()
{
    for (int i = 0; i < height; i++) {
        delete[] Y[i];
        delete[] Cb[i];
        delete[] Cr[i];
    }
    delete []Y;
    delete []Cb;
    delete []Cr;
}

void Jpeg_Compress::release_HuffCode()
{
    delete []YDC_Huff_Code;
    delete []YAC_Huff_Code;
    delete []CDC_Huff_Code;
    delete []CAC_Huff_Code;
}

void Jpeg_Compress::release_Code()
{
    delete []Z_Y;
    delete []Z_Cb;
    delete []Z_Cr;
    delete []Ycode;
    delete []Cbcode;
    delete []Crcode;
}

void Jpeg_Compress::read_RGB()
{
    /*
     如果高为负值，则扫描顺序为自上而下，如果为正，则相反
     如果为8位图，则从颜色表读出三通道数据
     24位图没有调色盘，每个像素点直接存储三个通道的数据
     */
    int i, j;
    if(height < 0) {
        height = 0 - height;
        if(8 == biBitCount) {
            for(i = 0; i < oldHeight; i++) {
                for(j = 0; j < lineByte; j++) {
                    B[i][j] = pColorTable[pBmpBuf[i * lineByte + j]].rgbBlue;
                    G[i][j] = pColorTable[pBmpBuf[i * lineByte + j]].rgbGreen;
                    R[i][j] = pColorTable[pBmpBuf[i * lineByte + j]].rgbRed;
                }
            }
        } else if(24 == biBitCount) {
            for(i = 0; i < oldHeight; i++) {
                int k = 0;
                for(j = 0; j < lineByte; j += 3) {
                    B[i][k] = pBmpBuf[i * lineByte + j];
                    G[i][k] = pBmpBuf[i * lineByte + j + 1];
                    R[i][k] = pBmpBuf[i * lineByte + j + 2];
                    k++;
                }
            }
        }
    } else {
        if(8 == biBitCount) {
            for(i = 0; i < oldHeight; i++) {
                int k = 0;
                for(j = lineByte * oldHeight - lineByte * (i + 1); j < lineByte * oldHeight - lineByte * i; j++) {
                    B[i][k] = pColorTable[pBmpBuf[j]].rgbBlue;
                    G[i][k] = pColorTable[pBmpBuf[j]].rgbGreen;
                    R[i][k] = pColorTable[pBmpBuf[j]].rgbRed;
                    k++;
                }
            }
        } else if(24 == biBitCount) {
            for(i = 0; i < oldHeight; i++) {
                int k = 0;
                for(j = lineByte * oldHeight - lineByte * (i + 1); j < lineByte * oldHeight - lineByte * i; j += 3) {
                    B[i][k] = pBmpBuf[j];
                    G[i][k] = pBmpBuf[j + 1];
                    R[i][k] = pBmpBuf[j + 2];
                    k++;
                }
            }
        }
    }
    biBitCount = 24;
    delete [] pBmpBuf;
    delete [] pColorTable;
}

void Jpeg_Compress::tran_Colorspace()
{
    /*RGB 2 YCrCb*/
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            Y[i][j] = 0.299 * R[i][j] + 0.587 * G[i][j] + 0.114 * B[i][j] + 0;
            Cb[i][j] = -0.1687 * R[i][j] - 0.3313 * G[i][j] + 0.5 * B[i][j] + 128;
            Cr[i][j] = 0.5 * R[i][j] - 0.418 * G[i][j] - 0.0813 * B[i][j] + 128;
        }
    }
    
    release_RGB();
}

void Jpeg_Compress::start_Compress(double q)
{
    if (q < 0.5) {
        quality = 0.5;
    } else {
        quality = q;
    }
    /*分块，以进行 DCT、量化和 Zizag 扫描*/
    double **block_Y, **block_Cb, **block_Cr;
    block_Y = new double *[8];
    block_Cb = new double *[8];
    block_Cr = new double *[8];
    for (int i = 0; i < 8; i++) {
        block_Y[i] = new double[8];
        block_Cb[i] = new double[8];
        block_Cr[i] = new double[8];
    }
    int *block_Z_Y, *block_Z_Cb, *block_Z_Cr;
    block_Z_Y = new int [64];
    block_Z_Cb = new int [64];
    block_Z_Cr = new int [64];
    
    int k = 0;
    for (int i = 0; i < height / 8; i++) {
        for (int j = 0; j < width / 8; j++) {
            for (int m = 0; m < 8; m++) {
                for (int n = 0; n < 8; n++) {
                    block_Y[m][n] = Y[i * 8 + m][j * 8 + n];
                    block_Cb[m][n] = Cb[i * 8 + m][j * 8 + n];
                    block_Cr[m][n] = Cr[i * 8 + m][j * 8 + n];
                }
            }
            FDCT(block_Y);
            FQuan(block_Y, Y_Quan_Table, quality);
            FZigzag(block_Y, block_Z_Y);
            
            FDCT(block_Cb);
            FQuan(block_Cb, C_Quan_Table, quality);
            FZigzag(block_Cb, block_Z_Cb);
            
            FDCT(block_Cr);
            FQuan(block_Cr, C_Quan_Table, quality);
            FZigzag(block_Cr, block_Z_Cr);
            
            /*转换为一维数组形式储存*/
            for (int m = 0; m < 64; m++) {
                Z_Y[k * 64 + m] = block_Z_Y[m];
                Z_Cb[k * 64 + m] = block_Z_Cb[m];
                Z_Cr[k * 64 + m] = block_Z_Cr[m];
            }
            k++;
        }
    }
    
    for (int i = 0; i < 8; i++) {
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
    release_YCbCr();
}

void Jpeg_Compress::huff_Code_2_String()
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

void Jpeg_Compress::start_Encode()
{
    int codeSize = 0, codeLength = 0, index = 0, DC, zeroNum, table_size;
    huff_Code_2_String();
    
    /*********处理 Y 通道*********/
    string *Y_Code_Str; //用于储存编码
    Y_Code_Str = new string [height * width];
    int *Y_Code_Num; //用于储存将要进行编码的数据
    Y_Code_Num = new int [height * width];
    for (int i = 0; i < height * width / 64; i++) {
        int j = 64 * i;
        /*处理 DC 系数*/
        if (0 == j) {
            DC = Z_Y[j];
        } else {
            DC = Z_Y[j] - Z_Y[j - 64];
        }
        Y_Code_Num[index] = Get_VLC_Len(DC);
        table_size = 0;
        for (int l = 0; l < 16; l++) {
            table_size = table_size + YDC_Huff_Table[l];
        }
        int l;
        for (l = 0; l < table_size; l++) {
            if(Y_Code_Num[index] == YDC_Huff_Value[l])
                break;
        }
        Y_Code_Str[index] = YDC_Huff_Code[l];
        index++;
        Y_Code_Num[index] = DC;
        Y_Code_Str[index] = Encode_VLC(Y_Code_Num[index]);
        index++;
        j++;
        /*处理 AC 系数*/
        table_size = 0;
        for (int l = 0; l < 16; l++) {
            table_size = table_size + YAC_Huff_Table[l];
        }
        while (j < (64 * (i + 1))) {
            zeroNum = 0;
            /*监测是否为全0*/
            if ((64 * i + 1) == j) {
                while (0 == Z_Y[j] && j < (64 * (i + 1))) {
                    j++;
                }
                if (j == (64 * (i + 1))) {
                    Y_Code_Num[index] = 0;
                    int l;
                    for (l = 0; l < table_size; l++) {
                        if (Y_Code_Num[index] == YAC_Huff_Value[l])
                            break;
                    }
                    Y_Code_Str[index] = YAC_Huff_Code[l];
                    index++;
                    break;
                }
                j = 64 * i + 1;
            }
            
            while (0 == Z_Y[j] && zeroNum < 16 && j < (64 * (i + 1))) {
                zeroNum++;
                j++;
            }
            if (j == (64 * (i + 1))) {
                Y_Code_Num[index] = 0;
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Y_Code_Num[index] == YAC_Huff_Value[l])
                        break;
                }
                Y_Code_Str[index] = YAC_Huff_Code[l];
                index++;
                break;
            }
            if (16 == zeroNum) {
                Y_Code_Num[index] = 0xf0;
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Y_Code_Num[index] == YAC_Huff_Value[l])
                        break;
                }
                Y_Code_Str[index] = YAC_Huff_Code[l];
                index++;
            } else {
                int var = Z_Y[j];
                Y_Code_Num[index] = zeroNum * 16 + Get_VLC_Len(var);
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Y_Code_Num[index] == YAC_Huff_Value[l])
                        break;
                }
                Y_Code_Str[index] = YAC_Huff_Code[l];
                index++;
                Y_Code_Num[index] = var;
                Y_Code_Str[index] = Encode_VLC(Y_Code_Num[index]);
                index++;
                j++;
            }
        }
    }
    /*将string类型的编码，每8个用一个字节来储存*/
    codeSize = index;
    for (int i = 0; i < codeSize; i++) {
        codeLength = codeLength + (int)Y_Code_Str[i].length();
    }
    YcodeSize = (codeLength + 7) / 8;
    int yu = codeLength % 8;
    Ycode = new unsigned char [YcodeSize];
    index = 0;
    int k = 0;
    for (int i = 0; i < codeSize; i++) {
        for (int j = 0; j < Y_Code_Str[i].length(); j++) {
            if (8 == k) {
                index++;
                k = 0;
            }
            Ycode[index] <<= 1;

            if (Y_Code_Str[i][j] == '0') {
                Ycode[index] = Ycode[index] & 0xfe;
            } else {
                Ycode[index] = Ycode[index] | 1;
            }
            k++;
        }
    }
    if(0 != yu) {
        Ycode[index] <<= (8 - yu);
    }
    delete[] Y_Code_Str;
    delete[] Y_Code_Num;
    
    /*********处理 Cb 通道*********/
    string *Cb_Code_Str;
    Cb_Code_Str = new string [height * width];
    int *Cb_Code_Num;
    Cb_Code_Num = new int [height * width];
    index = 0;
    for (int i = 0; i < height * width / 64; i++) {
        int j = 64 * i;
        /*处理 DC 系数*/
        if(0 == j) {
            DC = Z_Cb[j];
        } else {
            DC = Z_Cb[j] - Z_Cb[j - 64];
        }
        Cb_Code_Num[index] = Get_VLC_Len(DC);
        table_size = 0;
        for (int l = 0; l < 16; l++) {
            table_size = table_size + CDC_Huff_Table[l];
        }
        int l;
        for (l = 0; l < table_size; l++) {
            if (Cb_Code_Num[index] == CDC_Huff_Value[l]) {
                break;
            }
        }
        Cb_Code_Str[index] = CDC_Huff_Code[l];
        index++;
        Cb_Code_Num[index] = DC;
        Cb_Code_Str[index] = Encode_VLC(Cb_Code_Num[index]);
        index++;
        j++;
        /*处理 AC 系数*/
        table_size = 0;
        for (int l = 0; l < 16; l++) {
            table_size += CAC_Huff_Table[l];
        }
        while (j < (64 * (i + 1))) {
            zeroNum = 0;
            /*监测是否为全0*/
            if ((64 * i + 1) == j) {
                while (0 == Z_Cb[j] && j < (64 * (i + 1))) {
                    j++;
                }
                if (j == (64 * (i + 1))) {
                    Cb_Code_Num[index] = 0;
                    int l;
                    for (l = 0; l < table_size; l++) {
                        if (Cb_Code_Num[index] == CAC_Huff_Value[l])
                            break;
                    }
                    Cb_Code_Str[index] = CAC_Huff_Code[l];
                    index++;
                    break;
                }
                j = 64 * i + 1;
            }
            while(0 == Z_Cb[j] && zeroNum < 16 && j < (64 * (i + 1))) {
                zeroNum++;
                j++;
            }
            if (j == (64 * (i + 1))) {
                Cb_Code_Num[index]=0;
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Cb_Code_Num[index] == CAC_Huff_Value[l]) {
                        break;
                    }
                }
                Cb_Code_Str[index] = CAC_Huff_Code[l];
                index++;
                break;
            }
            if (16 == zeroNum) {
                Cb_Code_Num[index] = 0xf0;
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Cb_Code_Num[index] == CAC_Huff_Value[l]) {
                        break;
                    }
                }
                Cb_Code_Str[index] = CAC_Huff_Code[l];
                index++;
            } else {
                int var = Z_Cb[j];
                Cb_Code_Num[index] = zeroNum * 16 + Get_VLC_Len(var);
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Cb_Code_Num[index] == CAC_Huff_Value[l]) {
                        break;
                    }
                }
                Cb_Code_Str[index] = CAC_Huff_Code[l];
                index++;
                Cb_Code_Num[index] = var;
                Cb_Code_Str[index] = Encode_VLC(Cb_Code_Num[index]);
                index++;
                j++;
            }
        }
    }
    /*将string类型的编码，每8个用一个字节来储存*/
    codeSize = index;
    codeLength = 0;
    for (int i = 0; i < codeSize; i++) {
        codeLength = codeLength + (int)Cb_Code_Str[i].length();
    }
    CbcodeSize = (codeLength + 7) / 8;
    yu = codeLength % 8;
    Cbcode = new unsigned char [CbcodeSize];
    index = 0;
    k = 0;
    for (int i = 0; i < codeSize; i++) {
        for (int j = 0; j < Cb_Code_Str[i].length(); j++) {
            if (8 == k) {
                index++;
                k = 0;
            }
            Cbcode[index] <<= 1;
            
            if (Cb_Code_Str[i][j] == '0') {
                Cbcode[index] = Cbcode[index] & 0xfe;
            } else {
                Cbcode[index] = Cbcode[index] | 1;
            }
            k++;
        }
    }
    if (0 != yu) {
        Cbcode[index] <<= (8 - yu);
    }
    delete[] Cb_Code_Str;
    delete[] Cb_Code_Num;
    
    /*********处理 Cr 通道*********/
    string *Cr_Code_Str;
    Cr_Code_Str = new string [height * width];
    int *Cr_Code_Num;
    Cr_Code_Num = new int [height * width];
    index = 0;
    for (int i = 0; i < height * width / 64; i++) {
        int j = 64 * i;
        /*处理 DC 系数*/
        if (0 == j) {
            DC = Z_Cr[j];
        } else {
            DC = Z_Cr[j] - Z_Cr[j - 64];
        }
        Cr_Code_Num[index] = Get_VLC_Len(DC);
        table_size = 0;
        for (int l = 0; l < 16; l++) {
            table_size = table_size + CDC_Huff_Table[l];
        }
        int l;
        for (l = 0; l < table_size; l++) {
            if (Cr_Code_Num[index] == CDC_Huff_Value[l]) {
                break;
            }
        }
        Cr_Code_Str[index] = CDC_Huff_Code[l];
        index++;
        Cr_Code_Num[index] = DC;
        Cr_Code_Str[index] = Encode_VLC(Cr_Code_Num[index]);
        index++;
        j++;
        /*处理 AC 系数*/
        table_size = 0;
        for (int l = 0; l < 16; l++) {
            table_size += CAC_Huff_Table[l];
        }
        while (j < (64 * (i + 1))) {
            zeroNum = 0;
            /*监测是否为全0*/
            if ((64 * i + 1) == j) {
                while (0 == Z_Cb[j] && j < (64 * (i + 1))) {
                    j++;
                }
                if (j == (64 * (i + 1))) {
                    Cb_Code_Num[index] = 0;
                    int l;
                    for (l = 0; l < table_size; l++) {
                        if (Cb_Code_Num[index] == CAC_Huff_Value[l])
                            break;
                    }
                    Cb_Code_Str[index] = CAC_Huff_Code[l];
                    index++;
                    break;
                }
                j = 64 * i + 1;
            }
            while(0 == Z_Cr[j] && zeroNum < 16 && j < (64 * (i + 1))) {
                zeroNum++;
                j++;
            }
            if (j == (64 * (i + 1))) {
                Cr_Code_Num[index] = 0;
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Cr_Code_Num[index] == CAC_Huff_Value[l]) {
                        break;
                    }
                }
                Cr_Code_Str[index] = CAC_Huff_Code[l];
                index++;
                break;
            }
            if (16 == zeroNum) {
                Cr_Code_Num[index] = 0xf0;
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Cr_Code_Num[index] == CAC_Huff_Value[l]) {
                        break;
                    }
                }
                Cr_Code_Str[index] = CAC_Huff_Code[l];
                index++;
            } else {
                int var = Z_Cr[j];
                Cr_Code_Num[index] = zeroNum * 16 + Get_VLC_Len(var);
                int l;
                for (l = 0; l < table_size; l++) {
                    if (Cr_Code_Num[index] == CAC_Huff_Value[l]) {
                        break;
                    }
                }
                Cr_Code_Str[index] = CAC_Huff_Code[l];
                index++;
                Cr_Code_Num[index] = var;
                Cr_Code_Str[index] = Encode_VLC(Cr_Code_Num[index]);
                index++;
                j++;
            }
        }
    }
    /*将string类型的编码，每8个用一个字节来储存*/
    codeSize = index;
    codeLength = 0;
    for (int i = 0; i < codeSize; i++) {
        codeLength = codeLength + (int)Cr_Code_Str[i].length();
    }
    CrcodeSize = (codeLength + 7) / 8;
    yu = codeLength % 8;
    Crcode = new unsigned char [CrcodeSize];
    index = 0;
    k = 0;
    for (int i = 0; i < codeSize; i++) {
        for (int j = 0; j < Cr_Code_Str[i].length(); j++) {
            if (8 == k) {
                index++;
                k = 0;
            }
            Crcode[index] <<= 1;

            if (Cr_Code_Str[i][j] == '0') {
                Crcode[index] = Crcode[index] & 0xfe;
            } else {
                Crcode[index] = Crcode[index] | 1;
            }
            k++;
        }
    }
    if (0 != yu) {
        Crcode[index] <<= (8 - yu);
    }
    delete[] Cr_Code_Str;
    delete[] Cr_Code_Num;
    release_HuffCode();
}


/*
 写jpeg图
 格式：
 图像的宽、高、位数（int）
 量化表（unsigned char）
 YDC哈夫曼表
 YAC哈夫曼表
 CDC哈夫曼表
 CAC哈夫曼表（unsigned int）
 图像数据，按照Y/Cb/Cr顺序
 */
void Jpeg_Compress::write_Jpeg(const char *jpegName)
{
    ofstream outFile;
    outFile.open(jpegName, ios::binary);
    
    /*图像的基本信息*/
    outFile << width << ' ';
    outFile << height << ' ';
    outFile << biBitCount << ' ';
    
    /*量化表*/
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            outFile << (unsigned char) (Y_Quan_Table[i][j] * quality);
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            outFile << (unsigned char) (C_Quan_Table[i][j] * quality);
        }
    }
    
    /*哈夫曼表*/
    int table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + YDC_Huff_Table[i];
        outFile << YDC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << YDC_Huff_Value[i] << ' ';
    }
    
    table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + YAC_Huff_Table[i];
        outFile << YAC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << YAC_Huff_Value[i] << ' ';
    }
    
    table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + CDC_Huff_Table[i];
        outFile << CDC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << CDC_Huff_Value[i] << ' ';
    }
    
    table_size = 0;
    for (int i = 0; i < 16; i++) {
        table_size = table_size + CAC_Huff_Table[i];
        outFile << CAC_Huff_Table[i] << ' ';
    }
    for (int i = 0; i < table_size; i++) {
        outFile << CAC_Huff_Value[i] << ' ';
    }
    
    /*图像数据*/
    unsigned char ff = 0xff;
    for (int i = 0; i < YcodeSize; i++) {
        outFile << Ycode[i];
    }
    outFile << ff << ff;
    for (int i = 0; i < CbcodeSize; i++) {
        outFile << Cbcode[i];
    }
    outFile << ff << ff;
    for (int i = 0; i < CrcodeSize; i++) {
        outFile << Crcode[i];
    }
    outFile << ff << ff;
    
    outFile.clear();
    outFile.close();
    release_Code();
}