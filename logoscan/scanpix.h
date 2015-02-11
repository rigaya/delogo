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

typedef struct {
	char** compressed_datas;
	int    compressed_data_n;
	int    compressed_data_idx;

	PIXEL_YC *buffer;
	int buffer_idx;
} SCAN_PIXEL;


int AddSample(SCAN_PIXEL *sp, const PIXEL_YC& ycp);
int ClearSample(SCAN_PIXEL *sp);
int GetLGP(LOGO_PIXEL& lgp, const SCAN_PIXEL *sp, const short *lst_bgy, const short *lst_bgcb, const short *lst_bgcr);
int GetAB(double& A, double& B, int data_count, const short *lst_pixel, const short *lst_bg);
#endif
