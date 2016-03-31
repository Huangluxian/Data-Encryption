//
//  commonFun.h
//  dataHiding
//
//  Created by 黄路衔 on 16/3/4.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#ifndef commonFun_h
#define commonFun_h

#include <stdio.h>
#include <math.h>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <iostream>
#include <random>

using namespace std;

const double pi = 3.141592653;
const unsigned char zigzag_order[] = {0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};
#define LENGTH_MAX 1024 * 1024

/*DCT 变换*/
void FDCT(double **p);

/*DCT 逆变换*/
void IDCT(double **p);

/*量化，其中 Y 通道和 Cb、Cr 通道采用不同的量化表*/
void FQuan(double **p, const unsigned char table[][8], double quality);

/*反量化*/
void IQuan(double **p, const unsigned char table[][8]);

/*对8*8矩阵进行Zigzag扫描*/
void FZigzag(double **p, int* temp);
void IZigzag(int *p, double** temp);

/*给定一个数，返回其VLC编码长度*/
int Get_VLC_Len(int n);

/*
 给定一个数，返回其VLC编码
编码规则：输入正数，返回其二进制码；输入复数，返回其绝对值二进制码的反码
 */
string Encode_VLC(int n);

/*给定一个 VLC 编码，解码出对应的数值*/
int Decode_VLC(string code);

/*生成指定长度的伪随机 01 序列*/
string Generate_Key(int length, unsigned int rand_seed);

/*异或操作*/
string String_Xor(string word, string key);
unsigned char Char_Xor(unsigned char word, string key);

#endif /* commonFun_h */
