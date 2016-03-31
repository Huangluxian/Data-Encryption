//
//  commonFun.cpp
//  dataHiding
//
//  Created by 黄路衔 on 16/3/4.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include "commonFun.h"

/*离散余弦变换*/
void FDCT(double **p)
{
    for (int i = 0; i < 8; i++) {
        /*将原数据减去128，使得输入变量在DCT公式的定义域里*/
        for (int j = 0; j < 8; j++) {
            p[i][j] = p[i][j] - 128;
        }
    }
    
    double sum;
    double Cu, Cv;
    double **np;
    np = new double * [8];
    for (int i = 0; i < 8; i++) {
        np[i] = new double [8];
    }
    
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            sum = 0;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    sum = sum + p[i][j] * cos((2.0 * i + 1.0) * u * pi / 16.0) * cos((2.0 * j + 1.0) * v * pi / 16.0);
                }
            }
            if (0 == u) {
                Cu = 1.0 / sqrt(2.0);
            } else {
                Cu = 1;
            }
            if (0 == v) {
                Cv = 1.0 / sqrt(2.0);
            } else {
                Cv = 1;
            }
            np[u][v] = 1.0 / 4.0 * Cu * Cv * sum;
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            p[i][j] = np[i][j];
        }
    }
    
    for (int i = 0; i < 8; i++) {
        delete[] np[i];
    }
    delete[] np;
}

void IDCT(double **p)
{
    double sum;
    double Cu,Cv;
    double **np;
    np = new double *[8];
    for (int i = 0; i < 8; i++) {
        np[i] = new double[8];
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum = 0;
            for (int u = 0; u < 8; u++) {
                for (int v = 0; v < 8; v++) {
                    if (0 == u) {
                        Cu = 1.0 / sqrt(2.0);
                    } else {
                        Cu = 1;
                    }
                    if (0 == v) {
                        Cv = 1.0 / sqrt(2.0);
                    } else {
                        Cv = 1;
                    }
                    sum = sum + 1.0 / 4.0 * Cu * Cv * p[u][v] * cos((2.0 * i + 1.0) * u * pi / 16.0) * cos((2.0 * j + 1.0) * v * pi / 16.0);
                }
            }
            np[i][j] = sum;
        }
    }
    /*由于进行DCT变换之前数据先减去128，故逆变换后要加上128才能恢复到原数据*/
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            p[i][j] = np[i][j] + 128;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        delete[] np[i];
    }
    delete[] np;
}

/*量化*/
void FQuan(double **p, const unsigned char table[][8], double quality)
{
    double **temp;
    temp = new double *[8];
    for (int i = 0; i < 8; i++) {
        temp[i] = new double [8];
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int new_table = table[i][j] * quality;
            temp[i][j] = int(p[i][j] / new_table + 0.5);
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            p[i][j] = temp[i][j];
        }
    }
    for (int i = 0; i < 8; i++) {
        delete[] temp[i];
    }
    delete[] temp;
}

void IQuan(double **p, const unsigned char table[][8])
{
    double **temp;
    temp = new double *[8];
    for (int i = 0; i < 8; i++) {
        temp[i] = new double[8];
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            temp[i][j] = p[i][j] * table[i][j];
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            p[i][j] = temp[i][j];
        }
    }
    
    for (int i = 0; i < 8; i++) {
        delete[] temp[i];
    }
    delete[] temp;
}

/*Zigzag 扫描*/
void FZigzag(double **p, int* temp)
{
    for (int i = 0; i < 64; i++) {
        temp[zigzag_order[i]] = p[i / 8][i % 8];
    }
}

void IZigzag(int *p, double** temp)
{
    for (int i = 0; i < 64; i++) {
        temp[i / 8][i % 8] = p[zigzag_order[i]];
    }
}

/*变长编码*/
int Get_VLC_Len(int n)
{
    int length = 0;
    int m = abs(n);
    while (0 != m) {
        m = m / 2;
        length++;
    }
    return length;
}

string Encode_VLC(int n)
{
    string code = "0000000000000000";
    int m, length;
    if (n > 0) {
        length = Get_VLC_Len(n);
        code = code.substr(0, length);
        m = n;
        while (0 != m) {
            if (0 == m % 2) {
                code[--length] = '0';
            } else {
                code[--length] = '1';
            }
            m = m / 2;
        }
    } else {
        length = Get_VLC_Len(n);
        code = code.substr(0, length);
        m = -n;
        while (0 != m) {
            if (0 == m % 2) {
                code[--length] = '1';
            } else {
                code[--length] = '0';
            }
            m = m / 2;
        }
    }
    return code;
}

int Decode_VLC(string code)
{
    int n;
    if ('0' == code[0]) {
        n = 0;
        for (int i = 0; i < code.length(); i++) {
            if ('0' == code[i]) {
                code[i] = '1';
            } else {
                code[i] = '0';
            }
            if ('1' == code[i]) {
                n = n + pow(2.0, int(code.length() - 1 - i));
            }
        }
        return -1 * n;
    } else {
        n = 0;
        for (int i = 0; i < code.length(); i++) {
            if ('1' == code[i]) {
                n = n + pow(2.0, int(code.length() - 1 - i));
            }
        }
        return n;
    }
}

/*生成指定长度的密钥*/
string Generate_Key(int length, unsigned int rand_seed)
{
    string key;
    mt19937_64 mt(rand_seed);
    for (int i = 0; i < length; i++) {
        key += mt() % 2 + 0x30;
    }
    return key;
}

/*字符串异或，先将 ASCII 转换为数值再转回去*/
string String_Xor(string word, string key)
{
    string code;
    for (int i = 0; i < word.length(); i++) {
        char temp = (word[i] - 0x30) ^ (key[i] - 0x30) + 0x30;
        code += temp;
    }
    return code;
}

/*字节异或，将字符串转为字节*/
unsigned char Char_Xor(unsigned char word, string key)
{
    char code;
    for (int i = 0; i < 8; i++) {
        code <<= 1;
        if ('0' == key[i]) {
            code &= 0xfe;
        } else {
            code |= 1;
        }
    }
    return code ^ word;
}