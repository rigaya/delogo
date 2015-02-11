/*********************************************************************
* 	class ScanPixel
* 	各ピクセルのロゴ色・不透明度解析用クラス
* 
* 								(最終更新：2003/05/10)
*********************************************************************/
#ifndef ___SCANPIX_H
#define ___SCANPIX_H


#include <windows.h>
#include <vector>
#include "filter.h"
#include "logo.h"

#define SCAN_BUFFER_SIZE 256

class ScanPixel {
protected:
	short*     lst_y;
	short*     lst_cb;
	short*     lst_cr;
	int n; // サンプル枚数

	char** compressed_datas;
	int    compressed_data_n;
	int    compressed_data_idx;

	PIXEL_YC *buffer;
	int buffer_idx;
public:
	ScanPixel(void);
	~ScanPixel();

	int  Alloc(unsigned int f);
	int  AddSample(PIXEL_YC& ycp);
	//int  AddSample(PIXEL& rgb,PIXEL& rgb_bg);
	//int  EditSample(unsigned int num,PIXEL_YC& ycp,PIXEL_YC& ycp_bg);
	//int  EditSample(unsigned int num,PIXEL& rgb,PIXEL rgb_bg);
	//int  DeleteSample(unsigned int num);
	int  ClearSample(void);
	int  GetLGP(LOGO_PIXEL& lgp, const short *lst_bgy, const short *lst_bgcb, const short *lst_bgcr);
	int GetAB(double& A, double& B, int data_count, const short *lst_pixel, const short *lst_bg);
};


#endif
