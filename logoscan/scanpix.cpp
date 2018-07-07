/*********************************************************************
* 	構造体 SCAN_PIXEL
* 	各ピクセルのロゴ色・不透明度解析用
*
* 2003
* 	05/10:	operator new[](size_t, T* p) を使いすぎると落ちる。
* 			ScanPixel::Allocを最初に使うことで回避できる。
* 	06/16:	やっぱりnewやめて素直にrealloc使うことにした。
* 	06/17:	昨日の修正で入れてしまったバグを修正
* 			エラーメッセージを追加（サンプルが無い状態で
*
*********************************************************************/
#include <windows.h>
#include "filter.h"
#include "logo.h"
#include "approxim.h"
#include "scanpix.h"
#include "logoscan.h"
#include "../zlib/zlib.h"


// エラーメッセージ
static const char* CANNOT_GET_APPROXIMLINE = "サンプルが足りません\n背景色の異なるサンプルを加えてください";
static const char* NO_SAMPLE = "サンプルが足りません\nロゴの背景が単一色のサンプルを加えてください";

#define DP_RANGE 0x3FFF

/*--------------------------------------------------------------------
* 	RGBtoYCbCr()
*-------------------------------------------------------------------*/
inline void RGBtoYCbCr(PIXEL_YC& ycp, PIXEL& rgb)
{
//	RGB -> YCbCr
//	Y  =  0.2989*Red + 0.5866*Green + 0.1145*Blue
//	Cb = -0.1687*Red - 0.3312*Green + 0.5000*Blue
//	Cr =  0.5000*Red - 0.4183*Green - 0.0816*Blue
//	(rgbがそれぞれ0～1の範囲の時、
//	Y:0～1、Cb:-0.5～0.5、Cr:-0.5～0.5)
//
//	AviUtlプラグインでは、
//		Y  :     0 ～ 4096
//		Cb : -2048 ～ 2048
//		Cr : -2048 ～ 2048
	ycp.y  = (short)( 0.2989*4096*rgb.r + 0.5866*4096*rgb.g + 0.1145*4096*rgb.b +0.5);
	ycp.cb = (short)(-0.1687*4096*rgb.r - 0.3312*4096*rgb.g + 0.5000*4096*rgb.b +0.5);
	ycp.cr = (short)( 0.5000*4096*rgb.r - 0.4183*4096*rgb.g - 0.0816*4096*rgb.b +0.5);
}

/*--------------------------------------------------------------------
* 	Abs()	絶対値
*-------------------------------------------------------------------*/
template <class T>
inline T Abs(T x) {
	return ((x>0) ? x : -x);
}

/*====================================================================
* 	AddSample()
* 		サンプルをバッファに加える
*===================================================================*/
// YCbCr用
int AddSample(SCAN_PIXEL *sp, const PIXEL_YC& ycp) {
	if (sp->buffer == nullptr) {
		sp->buffer = (PIXEL_YC *)malloc(sizeof(PIXEL_YC) * SCAN_BUFFER_SIZE);
		if (sp->buffer == nullptr) {
			ShowErrorMessage(ERROR_MALLOC);
			return 1;
		}
		sp->buffer_idx = 0;
	}
	if (sp->buffer_idx >= SCAN_BUFFER_SIZE) {
		unsigned long dst_bytes = (sp->buffer_idx + 10) * sizeof(sp->buffer[0]);
		unsigned char *ptr_tmp = (unsigned char *)malloc(dst_bytes);
		if (ptr_tmp == nullptr) {
			ShowErrorMessage(ERROR_MALLOC);
			return 1;
		}

		unsigned long src_bytes = sp->buffer_idx * sizeof(sp->buffer[0]);
		compress2(ptr_tmp, &dst_bytes, (BYTE *)&sp->buffer[0], src_bytes, 9);

		char *ptr_compressed = (char *)malloc(sizeof(unsigned short) + dst_bytes);
		if (ptr_tmp == nullptr || ptr_compressed == nullptr) {
			ShowErrorMessage(ERROR_MALLOC);
			return 1;
		}

		*(unsigned short *)ptr_compressed = (unsigned short)dst_bytes;
		memcpy(ptr_compressed + 2, ptr_tmp, dst_bytes);

		if (sp->compressed_data_idx >= sp->compressed_data_n) {
			sp->compressed_data_n += 4;
			sp->compressed_datas = (char **)realloc(sp->compressed_datas, sizeof(sp->compressed_datas[0]) * sp->compressed_data_n);
		}
		sp->compressed_datas[sp->compressed_data_idx] = ptr_compressed;
		sp->compressed_data_idx++;

		sp->buffer_idx = 0;
		free(ptr_tmp);
	}
	sp->buffer[sp->buffer_idx] = ycp;
	sp->buffer_idx++;
	return 0;
}
/*====================================================================
* 	ClearSample()
* 		全サンプルを削除する
*===================================================================*/
int ClearSample(SCAN_PIXEL *sp) {
	if (sp->compressed_datas) {
		for (int i = 0; i < sp->compressed_data_idx; i++) {
			if (sp->compressed_datas[i]) {
				free(sp->compressed_datas[i]);
			}
		}
		free(sp->compressed_datas);
	}

	if (sp->buffer) free(sp->buffer);

	memset(sp, 0, sizeof(sp[0]));

	return 0;
}

/*====================================================================
* 	GetPixelAndBG()
* 		LOGO_PIXELを返す
*===================================================================*/
void GetPixelAndBG(short *pixel, short *bg, double A, double B) {
	if (A==1) {	// 0での除算回避
		*pixel = *bg = 0;
	} else {
		double temp = B / (1-A) +0.5;
		if (Abs(temp) < 0x7FFF) {
			// shortの範囲内
			*pixel = (short)temp;
			temp = ((double)1-A) * LOGO_MAX_DP +0.5;
			if (Abs(temp)>DP_RANGE || short(temp)==0) {
				*pixel = *bg = 0;
			} else {
				*bg = (short)temp;
			}
		} else {
			*pixel = *bg = 0;
		}
	}
}

/*====================================================================
* 	GetLGP()
* 		LOGO_PIXELを返す
*===================================================================*/
int GetLGP(LOGO_PIXEL& lgp, const SCAN_PIXEL *sp, const short *lst_bgy, const short *lst_bgcb, const short *lst_bgcr) {
	int ret = 1;
	const int n = sp->compressed_data_idx * SCAN_BUFFER_SIZE + sp->buffer_idx;
	if (n <= 1)  {
		ShowErrorMessage(NO_SAMPLE);
		return ret;
	}

	short* lst_y  = (short*)malloc(n * sizeof(short));
	short* lst_cb = (short*)malloc(n * sizeof(short));
	short* lst_cr = (short*)malloc(n * sizeof(short));

	const unsigned long tmp_size = (SCAN_BUFFER_SIZE + 10) * sizeof(sp->buffer[0]);
	unsigned char *ptr_tmp = (unsigned char *)malloc(tmp_size);
	if (ptr_tmp == nullptr || lst_y == nullptr || lst_cb == nullptr || lst_cr == nullptr) {
		ShowErrorMessage(ERROR_MALLOC);
		return ret;
	}

	int i = 0;
	for (int k = 0; k < sp->compressed_data_idx; k++) {
		char *ptr_compressed_data = sp->compressed_datas[k];
		unsigned long src_size = (*(unsigned short *)ptr_compressed_data);
		char *ptr_src = ptr_compressed_data + 2;
		unsigned long dst_size = tmp_size;
		uncompress(ptr_tmp, &dst_size, (unsigned char *)ptr_src, src_size);

		PIXEL_YC *logo_pixel = (PIXEL_YC *)ptr_tmp;

		for (int j = 0; j < SCAN_BUFFER_SIZE; i++, j++) {
			lst_y[i]  = logo_pixel[j].y;
			lst_cb[i] = logo_pixel[j].cb;
			lst_cr[i] = logo_pixel[j].cr;
		}
	}
	free(ptr_tmp);

	for (int j = 0; j < sp->buffer_idx; i++, j++) {
		lst_y[i]  = sp->buffer[j].y;
		lst_cb[i] = sp->buffer[j].cb;
		lst_cr[i] = sp->buffer[j].cr;
	}

	double A_Y, B_Y, A_Cb, B_Cb, A_Cr, B_Cr;
	if (   0 == GetAB(A_Y,  B_Y,  n, lst_y,  lst_bgy)
		&& 0 == GetAB(A_Cb, B_Cb, n, lst_cb, lst_bgcb)
		&& 0 == GetAB(A_Cr, B_Cr, n, lst_cr, lst_bgcr)) {
		GetPixelAndBG(&lgp.y,  &lgp.dp_y,  A_Y,  B_Y);
		GetPixelAndBG(&lgp.cb, &lgp.dp_cb, A_Cb, B_Cb);
		GetPixelAndBG(&lgp.cr, &lgp.dp_cr, A_Cr, B_Cr);
		ret = 0;
	}
	free(lst_y);
	free(lst_cb);
	free(lst_cr);
	return ret;
}

/*====================================================================
* 	GetAB_?()
* 		回帰直線の傾きと切片を返す
*===================================================================*/
int GetAB(double& A, double& B, int data_count, const short *lst_pixel, const short *lst_bg) {
	double A1, A2;
	double B1, B2;
	// XY入れ替えたもの両方で平均を取る
	// 背景がX軸
	if (   false == approxim_line(lst_bg, lst_pixel, data_count, A1, B1)
		|| false == approxim_line(lst_pixel, lst_bg, data_count, A2, B2)) {
		ShowErrorMessage(CANNOT_GET_APPROXIMLINE);
		return 1;
	}

	A = (A1+(1/A2))/2;   // 傾きを平均
	B = (B1+(-B2/A2))/2; // 切片も平均

	return 0;
}
