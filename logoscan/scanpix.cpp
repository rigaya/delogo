/*********************************************************************
* 	class ScanPixel
* 	各ピクセルのロゴ色・不透明度解析用クラス
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
#include "../zlib/zlib.h"


// エラーメッセージ
const char* CANNOT_MALLOC = "メモリを確保できませんでした";
const char* CANNOT_GET_APPROXIMLINE = "サンプルが足りません\n背景色の異なるサンプルを加えてください";
const char* NO_SAMPLE = "サンプルが足りません\nロゴの背景が単一色のサンプルを加えてください";

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
* 	コンストラクタ
*===================================================================*/
ScanPixel::ScanPixel(void)
{
	lst_y    = nullptr;
	lst_cb   = nullptr;
	lst_cr   = nullptr;
	lst_bgy  = nullptr;
	lst_bgcb = nullptr;
	lst_bgcr = nullptr;
	
	buffer = (PIXEL_YC *)malloc(sizeof(PIXEL_YC) * SCAN_BUFFER_SIZE);
	buffer_idx = 0;
	n = 0;
}

/*====================================================================
* 	デストラクタ
*===================================================================*/
ScanPixel::~ScanPixel()
{
	ClearSample();

	std::vector<char *>().swap(compressed_datas);
}

/*====================================================================
* 	Alloc()
* 		あらかじめフレーム分だけメモリ確保
* 		すでにある領域はクリアされる
*===================================================================*/
int ScanPixel::Alloc(unsigned int f)
{
	lst_y    = (short*)malloc(f * sizeof(short)); //new short[f];
	lst_cb   = (short*)malloc(f * sizeof(short)); //new short[f];
	lst_cr   = (short*)malloc(f * sizeof(short)); //new short[f];
	lst_bgy  = (short*)malloc(f * sizeof(short)); //new short[f];
	lst_bgcb = (short*)malloc(f * sizeof(short)); //new short[f];
	lst_bgcr = (short*)malloc(f * sizeof(short)); //new short[f];

	// メモリ確保失敗
	if (lst_y==NULL || lst_cb==NULL || lst_cr==NULL ||
		lst_bgy==NULL || lst_bgcb==NULL || lst_bgcr==NULL)
			throw CANNOT_MALLOC;

	return f;
}

/*====================================================================
* 	AddSample()
* 		サンプルをバッファに加える
*===================================================================*/
// YCbCr用
int ScanPixel::AddSample(PIXEL_YC& ycp)
{
	if (buffer_idx >= SCAN_BUFFER_SIZE) {
		unsigned long dst_bytes = (buffer_idx + 10) * sizeof(buffer[0]);
		unsigned char *ptr_tmp = (unsigned char *)malloc(dst_bytes);
		unsigned long src_bytes = buffer_idx * sizeof(buffer[0]);
		compress2(ptr_tmp, &dst_bytes, (BYTE *)&buffer[0], src_bytes, 9);

		char *ptr_compressed = (char *)malloc(sizeof(unsigned short) + dst_bytes);
		if (ptr_compressed == nullptr)
			throw CANNOT_MALLOC;

		*(unsigned short *)ptr_compressed = (unsigned short)dst_bytes;
		memcpy(ptr_compressed + 2, ptr_tmp, dst_bytes);
		compressed_datas.push_back(ptr_compressed);

		buffer_idx = 0;
		free(ptr_tmp);
	}
	buffer[buffer_idx] = ycp;
	buffer_idx++;
	return (++n);
}

#if 0
//--------------------------------------------------------------------
// RGB用
int ScanPixel::AddSample(PIXEL& rgb, PIXEL& rgb_bg)
{
	PIXEL_YC ycp, ycp_bg;

	// RGB->YC2
	RGBtoYCbCr(ycp, rgb);
	RGBtoYCbCr(ycp_bg, rgb_bg);

	return AddSample(ycp, ycp_bg);
}
/*====================================================================
* 	EditSample()
* 		サンプルを書き換える
*===================================================================*/
// YCbCr用
int ScanPixel::EditSample(unsigned int num, PIXEL_YC& ycp, PIXEL_YC& ycp_bg)
{
	if (num >= n) // num番目の要素が存在しない時
		return AddSample(ycp, ycp_bg);

	lst_y[num]  = ycp.y;
	lst_cb[num] = ycp.cb;
	lst_cr[num] = ycp.cr;
	lst_bgy[num]  = ycp_bg.y;
	lst_bgcb[num] = ycp_bg.cb;
	lst_bgcr[num] = ycp_bg.cr;

	return num;
}


//--------------------------------------------------------------------
// RGB用
int ScanPixel::EditSample(unsigned int num, PIXEL& rgb, PIXEL rgb_bg)
{
	PIXEL_YC ycp, ycp_bg;

	// RGB->YC2
	RGBtoYCbCr(ycp, rgb);
	RGBtoYCbCr(ycp_bg, rgb_bg);

	return EditSample(num, ycp, ycp_bg);
}

/*====================================================================
* 	DeleteSample()
* 		サンプルを削除する
*===================================================================*/
int ScanPixel::DeleteSample(unsigned int num)
{
	n--;

	if (n <= 0) // サンプルが０以下になるとき
		return ClearSample();

	if (n == num) // 最後のサンプルを削除する時
		return n; // nを減らすだけ

	// 次のサンプルから一つ前にコピー
	memcpy(&lst_y[num],    &lst_y[num+1],    (n-num) * sizeof(short));
	memcpy(&lst_cb[num],   &lst_cb[num+1],   (n-num) * sizeof(short));
	memcpy(&lst_cr[num],   &lst_cr[num+1],   (n-num) * sizeof(short));
	memcpy(&lst_bgy[num],  &lst_bgy[num+1],  (n-num) * sizeof(short));
	memcpy(&lst_bgcb[num], &lst_bgcb[num+1], (n-num) * sizeof(short));
	memcpy(&lst_bgcr[num], &lst_bgcr[num+1], (n-num) * sizeof(short));

	return n;
}
#endif
/*====================================================================
* 	ClearSample()
* 		全サンプルを削除する
*===================================================================*/
int ScanPixel::ClearSample(void)
{
	if (lst_y)    free(lst_y);    //delete[] lst_y;
	if (lst_cb)   free(lst_cb);   //delete[] lst_cb;
	if (lst_cr)   free(lst_cr);   //delete[] lst_cr;
	if (lst_bgy)  free(lst_bgy);  //delete[] lst_bgy;
	if (lst_bgcb) free(lst_bgcb); //delete[] lst_bgcb;
	if (lst_bgcr) free(lst_bgcr); //delete[] lst_bgcr;

	for (auto ptr : compressed_datas)
		if (ptr) free(ptr);
	compressed_datas.clear();

	if (buffer) free(buffer);
	
	lst_y    = nullptr;
	lst_cb   = nullptr;
	lst_cr   = nullptr;
	lst_bgy  = nullptr;
	lst_bgcb = nullptr;
	lst_bgcr = nullptr;
	n = 0;

	buffer = nullptr;
	buffer_idx = 0;
	return 0;
}

/*====================================================================
* 	GetLGP()
* 		LOGO_PIXELを返す
*===================================================================*/
int ScanPixel::GetLGP(LOGO_PIXEL& lgp, const std::vector<PIXEL_YC>& bg)
{
	if (n<=1) throw NO_SAMPLE;

	Alloc(n);
	
	const unsigned long tmp_size = (SCAN_BUFFER_SIZE + 10) * sizeof(buffer[0]);
	unsigned char *ptr_tmp = (unsigned char *)malloc(tmp_size);
	if (ptr_tmp == nullptr)
		throw CANNOT_MALLOC;
	
	int i = 0;
	for (auto ptr_compressed_data : compressed_datas) {
		unsigned long src_size = (*(unsigned short *)ptr_compressed_data);
		char *ptr_src = ptr_compressed_data + 2;
		unsigned long dst_size = tmp_size;
		uncompress(ptr_tmp, &dst_size, (unsigned char *)ptr_src, src_size);

		PIXEL_YC *logo_pixel = (PIXEL_YC *)ptr_tmp;

		for (unsigned int j = 0; j < SCAN_BUFFER_SIZE; i++, j++) {
			lst_y[i]  = logo_pixel[j].y;
			lst_cb[i] = logo_pixel[j].cb;
			lst_cr[i] = logo_pixel[j].cr;
		}
	}
	free(ptr_tmp);
	
	for (int j = 0; j < buffer_idx; i++, j++) {
		lst_y[i]  = buffer[j].y;
		lst_cb[i] = buffer[j].cb;
		lst_cr[i] = buffer[j].cr;
	}

	for (int j = 0; j < n; j++) {
		lst_bgy[j]  = bg[j].y;
		lst_bgcb[j] = bg[j].cb;
		lst_bgcr[j] = bg[j].cr;
	}

	double A;
	double B;
	double temp;

	// 輝度
	GetAB_Y(A, B);
	if (A==1) {	// 0での除算回避
		lgp.y = lgp.dp_y = 0;
	} else {
		temp = B / (1-A) +0.5;
		if (Abs(temp) < 0x7FFF) {
			// shortの範囲内
			lgp.y = (short)temp;
			temp = ((double)1-A) * LOGO_MAX_DP +0.5;
			if (Abs(temp)>DP_RANGE || short(temp)==0) {
				lgp.y = lgp.dp_y = 0;
			} else {
				lgp.dp_y = (short)temp;
			}
		} else {
			lgp.y = lgp.dp_y = 0;
		}
	}

	// 色差(青)
	GetAB_Cb(A, B);
	if (A==1) {
		lgp.cb = lgp.dp_cb = 0;
	} else {
		temp = B / (1-A) +0.5;
		if (Abs(temp) < 0x7FFF) {
			// short範囲内
			lgp.cb = (short)temp;
			temp = ((double)1-A) * LOGO_MAX_DP +0.5;
			if (Abs(temp) > DP_RANGE || short(temp)==0) {
				lgp.cb = lgp.dp_cb = 0;
			} else {
				lgp.dp_cb = (short)temp;
			}
		} else {
			lgp.cb = lgp.dp_cb = 0;
		}
	}

	// 色差(赤)
	GetAB_Cr(A, B);
	if (A==1) {
		lgp.cr = lgp.dp_cr = 0;
	} else {
		temp = B / (1-A) +0.5;
		if (Abs(temp) < 0x7FFF) {
			// short範囲内
			lgp.cr = (short)temp;
			temp = ((double)1-A) * LOGO_MAX_DP + 0.5;
			if (Abs(temp) > DP_RANGE || short(temp)==0) {
				lgp.cr = lgp.dp_cr = 0;
			} else {
				lgp.dp_cr = (short)temp;
			}
		} else {
			lgp.cr = lgp.dp_cr = 0;
		}
	}
	return n;
}

/*====================================================================
* 	GetAB_?()
* 		回帰直線の傾きと切片を返す
*===================================================================*/
int ScanPixel::GetAB_Y(double& A, double& B)
{
	double A1, A2;
	double B1, B2;
	bool r;

	// XY入れ替えたもの両方で平均を取る
	// 背景がX軸
	r = approxim_line(lst_bgy, lst_y, n, A1, B1);
	if (r == false) throw CANNOT_GET_APPROXIMLINE;
	// 背景がY軸
	r = approxim_line(lst_y, lst_bgy, n, A2, B2);
	if (r == false) throw CANNOT_GET_APPROXIMLINE;

	A = (A1+(1/A2))/2;   // 傾きを平均
	B = (B1+(-B2/A2))/2; // 切片も平均

	return n;
}
int ScanPixel::GetAB_Cb(double& A, double& B)
{
	double A1, A2;
	double B1, B2;
	bool r;

	r = approxim_line(lst_bgcb, lst_cb, n, A1, B1);
	if (r == false) throw CANNOT_GET_APPROXIMLINE;

	r = approxim_line(lst_cb, lst_bgcb, n, A2, B2);
	if (r == false) throw CANNOT_GET_APPROXIMLINE;

	A = (A1+(1/A2))/2;   // 傾きを平均
	B = (B1+(-B2/A2))/2; // 切片も平均

	return n;
}
int ScanPixel::GetAB_Cr(double& A, double& B)
{
	double A1, A2;
	double B1, B2;
	bool r;

	r = approxim_line(lst_bgcr, lst_cr, n, A1, B1);
	if (r == false) throw CANNOT_GET_APPROXIMLINE;

	r = approxim_line(lst_cr, lst_bgcr, n, A2, B2);
	if (r == false) throw CANNOT_GET_APPROXIMLINE;

	A = (A1+(1/A2))/2;   // 傾きを平均
	B = (B1+(-B2/A2))/2; // 切片も平均

	return n;
}
