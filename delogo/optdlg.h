/*====================================================================
* 	オプションダイアログ			optdlg.h
*===================================================================*/
#ifndef ___OPTDLG_H
#define ___OPTDLG_H

#include <windows.h>
#include "filter.h"

extern FILTER* optfp;
extern HWND    hcb_logo;	// コンボボックスのハンドル
extern HWND    hoptdlg;		// オプションダイアログのハンドル

// ダイアログプロシージャ
BOOL CALLBACK OptDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);


//	リストアイテム編集用関数
void InsertItem(HWND hdlg, int n, void *data);
void DeleteItem(HWND list, int num);

#endif
