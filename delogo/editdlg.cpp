/*====================================================================
* 	編集ダイアログ			editdlg.c
*===================================================================*/
#include <windows.h>
#include <commctrl.h>
#include <utility>
#include "resource.h"
#include "editdlg.h"
#include "logo.h"
#include "optdlg.h"
#include "logodef.h"
#include "dlg_util.h"

extern char filter_name[];	// フィルタ名[filter.c]
extern int  track_e[];      //トラックの最大値 [filter.c]

static HWND owner;	// 親ウインドウ
static int  list_n;

static ITEM_SIZE defaultWindow, border;
static int TargetIDs[] ={
	ID_EDIT_NAME, IDC_GROUP_FADE, IDC_GROUP_POS, ID_EDIT_X, ID_EDIT_Y, ID_EDIT_START, ID_EDIT_END, ID_EDIT_FIN, ID_EDIT_FOUT,
	IDOK, IDCANCEL, ID_EDIT_SPINST, ID_EDIT_SPINED, ID_EDIT_SPINFI, ID_EDIT_SPINFO, ID_EDIT_SPINX, ID_EDIT_SPINY,
	ID_STATIC_X, ID_STATIC_Y, ID_STATIC_START, ID_STATIC_END, ID_STATIC_FIN, ID_STATIC_FOUT, 
};
static ITEM_SIZE defaultControls[_countof(TargetIDs)];

//----------------------------
//	関数プロトタイプ
//----------------------------
void on_wm_initdialog(HWND hdlg);
static void on_wm_sizing(HWND hdlg, RECT *rect);
BOOL on_IDOK(HWND hdlg);

/*====================================================================
* 	EditDlgProc()		コールバックプロシージャ
*===================================================================*/
BOOL CALLBACK EditDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			owner = GetWindow(hdlg, GW_OWNER);
			list_n = (int)lParam;
			on_wm_initdialog(hdlg);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					if (on_IDOK(hdlg))
						EndDialog(hdlg, LOWORD(wParam));
					break;

				case IDCANCEL:
					EndDialog(hdlg, LOWORD(wParam));
					break;
			}
			break;
		case WM_SIZING:
			on_wm_sizing(hdlg, (RECT *)lParam);
			return TRUE;
	}

	return FALSE;
}

/*--------------------------------------------------------------------
* 	on_wm_initdialog()	初期化
*-------------------------------------------------------------------*/
void on_wm_initdialog(HWND hdlg)
{
	char title[LOGO_MAX_NAME+10];
	LOGO_HEADER* lp;

	// ロゴデータ取得
	lp = (LOGO_HEADER *)SendDlgItemMessage(owner, IDC_LIST, LB_GETITEMDATA, list_n, 0);

	// エディットボックス
	SendDlgItemMessage(hdlg, ID_EDIT_NAME,   EM_SETLIMITTEXT, LOGO_MAX_NAME-1, 0);
	SendDlgItemMessage(hdlg, ID_EDIT_START,  EM_SETLIMITTEXT, 4,               0);
	SendDlgItemMessage(hdlg, ID_EDIT_END,    EM_SETLIMITTEXT, 4,               0);
	SendDlgItemMessage(hdlg, ID_EDIT_FIN,    EM_SETLIMITTEXT, 3,               0);
	SendDlgItemMessage(hdlg, ID_EDIT_FOUT,   EM_SETLIMITTEXT, 3,               0);
	SendDlgItemMessage(hdlg, ID_EDIT_SPINST, UDM_SETRANGE,    0,      track_e[6]);
	SendDlgItemMessage(hdlg ,ID_EDIT_SPINED, UDM_SETRANGE,    0,      track_e[9]);
	SendDlgItemMessage(hdlg, ID_EDIT_SPINFI, UDM_SETRANGE,    0,      track_e[7]);
	SendDlgItemMessage(hdlg, ID_EDIT_SPINFO, UDM_SETRANGE,    0,      track_e[8]);
	SendDlgItemMessage(hdlg, ID_EDIT_X,      EM_SETLIMITTEXT, 5,               0);
	SendDlgItemMessage(hdlg, ID_EDIT_Y,      EM_SETLIMITTEXT, 5,               0);
	SendDlgItemMessage(hdlg, ID_EDIT_SPINX,  UDM_SETRANGE,    0,          0x7fff); // signed 16bitの上限
	SendDlgItemMessage(hdlg, ID_EDIT_SPINY,  UDM_SETRANGE,    0,          0x7fff);

	SetDlgItemText(hdlg, ID_EDIT_NAME,  lp->name);
	SetDlgItemInt(hdlg,  ID_EDIT_START, lp->st, FALSE);
	SetDlgItemInt(hdlg,  ID_EDIT_END,   lp->ed, FALSE);
	SetDlgItemInt(hdlg,  ID_EDIT_FIN,   lp->fi, FALSE);
	SetDlgItemInt(hdlg,  ID_EDIT_FOUT,  lp->fo, FALSE);
	SetDlgItemInt(hdlg,  ID_EDIT_X,     lp->x,  FALSE);
	SetDlgItemInt(hdlg,  ID_EDIT_Y,     lp->y,  FALSE);

	// キャプション
	wsprintf(title, "%s - 編集", lp->name);
	SetWindowText(hdlg, title);

	get_initial_dialog_size(hdlg, defaultWindow, border, defaultControls, TargetIDs);
}

/*--------------------------------------------------------------------
* 	on_wm_sizing()
*-------------------------------------------------------------------*/
static void on_wm_sizing(HWND hdlg, RECT *rect) {
	SendMessage(hdlg, WM_SETREDRAW, 0, 0);

	rect->right  = max(rect->right, rect->left + defaultWindow.w);
	rect->bottom = rect->top + defaultWindow.h;
	int new_width = rect->right - rect->left;

	SetWindowPos(GetDlgItem(hdlg, TargetIDs[0]), 0, 0, 0, defaultControls[0].w + (new_width - defaultWindow.w), defaultControls[0].h, SWP_NOMOVE | SWP_NOZORDER);

	int group_fade_move_x = new_width / 2 - defaultControls[1].w / 2 - border.rect.left - defaultControls[1].rect.left;
	for (int i = 1; i < _countof(TargetIDs); i++) {
		MoveControl(hdlg, TargetIDs[i], &defaultControls[i], group_fade_move_x);
	}
	SendMessage(hdlg, WM_SETREDRAW, 1, 0);
	InvalidateRect(hdlg,NULL,true);
}

/*--------------------------------------------------------------------
* 	on_IDOK()	OKボタン動作
* 		ロゴデータの変更
*-------------------------------------------------------------------*/
BOOL on_IDOK(HWND hdlg)
{
	char newname[LOGO_MAX_NAME];

	// 新ロゴ名前
	GetDlgItemText(hdlg, ID_EDIT_NAME, newname, LOGO_MAX_NAME);
	// リストボックスを検索
	int num = SendDlgItemMessage(owner, IDC_LIST, LB_FINDSTRING, (WPARAM)-1, (WPARAM)newname);
	if (num != CB_ERR && num != list_n) { // 編集中のもの以外に同名が見つかった
		MessageBox(hdlg, "同名のロゴがあります\n別の名称を設定してください", filter_name, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	LOGO_HEADER *olddata = (LOGO_HEADER *)SendDlgItemMessage(owner, IDC_LIST, LB_GETITEMDATA, list_n, 0);

	// メモリ確保
	LOGO_HEADER *newdata = (LOGO_HEADER *)malloc(logo_data_size(olddata));
	if (newdata == NULL) {
		MessageBox(hdlg, "メモリを確保できませんでした", filter_name, MB_OK|MB_ICONERROR);
		return TRUE;
	}
	// ロゴデータコピー
	memcpy(newdata, olddata, logo_data_size(olddata));

	// ロゴデータ設定
	lstrcpy(newdata->name, newname);
	newdata->st = (short)min(GetDlgItemInt(hdlg, ID_EDIT_START, NULL, FALSE), track_e[6]);
	newdata->ed = (short)min(GetDlgItemInt(hdlg, ID_EDIT_END,   NULL, FALSE), track_e[9]);
	newdata->fi = (short)min(GetDlgItemInt(hdlg, ID_EDIT_FIN,   NULL, FALSE), track_e[7]);
	newdata->fo = (short)min(GetDlgItemInt(hdlg, ID_EDIT_FOUT,  NULL, FALSE), track_e[8]);
	newdata->x  =     (short)GetDlgItemInt(hdlg, ID_EDIT_X,     NULL, FALSE);
	newdata->y  =     (short)GetDlgItemInt(hdlg, ID_EDIT_Y,     NULL, FALSE);

	// リストボックスを更新
	DeleteItem(owner, list_n);
	InsertItem(owner, list_n, newdata);

	return TRUE;
}

