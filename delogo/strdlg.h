/*====================================================================
* 	文字列表示ダイアログ			strdlg.h
*===================================================================*/
#ifndef ___STRDLG_H
#define ___STRDLG_H

#include <windows.h>

#define STRDLG_MAXSTR MAX_PATH+256

// ダイアログプロシージャ
BOOL CALLBACK StrDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
