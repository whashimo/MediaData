/***********************************/
/* BMPファイルを生成するプログラム */
/***********************************/
/* 2019.11.11 long型をint型に変更  */
/***********************************/

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

typedef struct tagBITMAPFILEHEADER { /* ファイルヘッダ構造体 */
  unsigned char  bfType[2];     /* ファイルタイプ */
  unsigned int   bfSize;        /* ファイルサイズ */
  unsigned short bfReserved1;   /* 予約 */
  unsigned short bfReserved2;   /* 予約 */
  unsigned int   bfOffBits;     /* 画像データ開始アドレス(オフセット) */
}__attribute__((gcc_struct, packed)) BITMAPFILEHEADER ; /* 計14Byte */

typedef struct tagBITMAPINFOHEADER{ /* 情報ヘッダ構造体 */
  unsigned int   biSize;      /* この構造体のサイズ */
  int            biWidth;     /* 画像サイズ(横) */
  int            biHeight;    /* 画像サイズ(縦) */
  unsigned short biPlanes;    /* プレーン数 */
  unsigned short biBitCount;  /* 画素あたりのデータサイズ */
  unsigned int   biCompression; /* 圧縮方式 */
  unsigned int   biSizeImage;   /* 画像データ部のサイズ */
  int            biXPixPerMeter; /* 横方向解像度(pixel/m) */
  int            biYPixPerMeter; /* 縦方向解像度(pixel/m) */
  unsigned int   biClrUsed;     /* 格納されているパレット色数 */
  unsigned int   biClrImporant; /* 重要パレットのインデックス */
}__attribute__((gcc_struct, packed)) BITMAPINFOHEADER; /* 計40Byte */

int main(void)
{
  FILE *fp;
  int i,j;
  unsigned char c;
  
  BITMAPFILEHEADER *bitmapFileHeader;
  BITMAPINFOHEADER *bitmapInfoHeader;
  
  if((fp=fopen("sample.bmp", "wb"))==NULL){ /* 出力ファイル */
    fprintf(stderr, "file cannot open\n");
    exit(1);
  }
  
  /* 構造体メモリ確保 */
  bitmapFileHeader=(BITMAPFILEHEADER *)malloc(sizeof(BITMAPFILEHEADER));
  bitmapInfoHeader=(BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
  
  /* ヘッダ */
  bitmapFileHeader->bfType[0]='B';
  bitmapFileHeader->bfType[1]='M';
  bitmapFileHeader->bfReserved1=0x0000; /* 予約 */
  bitmapFileHeader->bfReserved2=0x0000; /* 予約 */

  /* データ開始アドレスはヘッダ構造体の合計値 */
  bitmapFileHeader->bfOffBits=
    sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
  
  /* 情報ヘッダ構造体のサイズ */
  bitmapInfoHeader->biSize=sizeof(BITMAPINFOHEADER); 
  bitmapInfoHeader->biWidth=(long)128; /* 画像横サイズ=128 */
  bitmapInfoHeader->biHeight=(long)128; /* 画像縦サイズ=128 */
  bitmapInfoHeader->biPlanes=0x0001;
  bitmapInfoHeader->biBitCount=0x0018; /* 24bit 1677万色 */
  bitmapInfoHeader->biCompression=0x0000; /* 無圧縮 */
  /* データ部の大きさは画像の縦サイズ×横サイズ×3byte */
  bitmapInfoHeader->biSizeImage= 
    bitmapInfoHeader->biWidth* bitmapInfoHeader->biHeight*3; 
  bitmapInfoHeader->biXPixPerMeter=3780; /* 96dpi */
  bitmapInfoHeader->biYPixPerMeter=3780; /* 96dpi */
  bitmapInfoHeader->biClrUsed=0; /* パレットなし */
  bitmapInfoHeader->biClrImporant=0; /* パレットなし */
  
  /* ファイルサイズはヘッダ構造体とデータ部のサイズの合計 */
  bitmapFileHeader->bfSize=
    bitmapFileHeader->bfOffBits+bitmapInfoHeader->biSizeImage;
  
  /* BITMAPFILEHEADER型の構造体を，その大きさ分だけ，unsigned charとして
     １回fpに書き込む*/
  fwrite((unsigned char *)bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
  /* BITMAPINFOHEADER型の構造体を，その大きさ分だけ，unsigned charとして
     １回fpに書き込む*/
  fwrite((unsigned char *)bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
  
  /* 画像データ本体 */
  for(j=0;j<bitmapInfoHeader->biHeight;j++){
    for(i=0;i<bitmapInfoHeader->biWidth;i++){
      c=( (((i&0x08)==0)^((j&0x08)==0)))*255; 
      fputc(c, fp);             /* B */
      fputc(c, fp);             /* G */
      fputc(c, fp);             /* R */
    }
  }
  free(bitmapFileHeader);
  free(bitmapInfoHeader);
  fclose(fp);
  return(0);
}
