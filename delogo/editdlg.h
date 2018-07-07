/*====================================================================
* 	編集ダイアログ			editdlg.h
*===================================================================*/
#ifndef ___EDITDLG_H
#define ___EDITDLG_H

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// ダイアログプロシージャ
BOOL CALLBACK EditDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
