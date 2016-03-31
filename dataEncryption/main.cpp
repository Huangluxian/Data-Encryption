//
//  本科毕业设计
//  main.cpp
//  dataHiding
//
//  Created by 黄路衔 on 16/3/3.
//  Copyright © 2016年 黄路衔. All rights reserved.
//

#include <iostream>
#include "commonFun.h"
#include "JpegCompress.h"
#include "JpegDecompress.h"
#include "JpegEncrypt.h"

int main(int argc, const char * argv[]) {
    string image_original;
    string image_compress;
    string image_encrypt;
    string image_decompress;
    string image_recover;
    
    /*bmp2jpeg*/
    image_original = "/Users/huangluxian/Desktop/Lena.bmp";
    image_compress = "/Users/huangluxian/Desktop/lena.jpeg";
    double quality = 1;
    cout << "Begin to compress image..." << endl;
    Jpeg_Compress cjpeg;
    if (false == cjpeg.read_Bmp(image_original.c_str())) {
        cout << "read bmp fail!" << endl;
        return -1;
    }
    if (false == cjpeg.alloc_Mem()) {
        cout << "alloc memory fail!" << endl;
        return -1;
    }
    cjpeg.read_RGB();
    cjpeg.tran_Colorspace();
    cjpeg.start_Compress(quality);
    cjpeg.start_Encode();
    cjpeg.write_Jpeg(image_compress.c_str());
    cout << "Done!" << endl << endl;
    
    /*encrypt Jpeg bitstream*/
    image_encrypt = "/Users/huangluxian/Desktop/lena_encrypted.jpeg";
    cout << "Begin to encrypt image..." << endl;
    Jpeg_Encrypt ejpeg;
    ejpeg.read_Jpeg(image_compress.c_str());
    ejpeg.encrypt_Qtable();
    ejpeg.part_Decode();
    ejpeg.encrypt_Appended_Bit();
    ejpeg.write_Jpeg(image_encrypt.c_str());
    cout << "Done!" << endl << endl;
    
    /*decompress jpeg*/
    image_decompress = "/Users/huangluxian/Desktop/lena_encrypted.bmp";
    cout << "Begin to decompress image..." << endl;
    Jpeg_Decompress djpeg;
    djpeg.read_Jpeg(image_encrypt.c_str());
    if (false == djpeg.alloc_Mem()) {
        cout << "alloc memory fail!" << endl;
        return -1;
    }
    djpeg.start_Decode();
    djpeg.start_Decompress();
    djpeg.tran_Colorspace();
    if (false == djpeg.write_Bmp(image_decompress.c_str())) {
        cout << "Write bmp fail!" << endl;
        return -1;
    }
    cout << "Done!" << endl;
    return 0;
}
