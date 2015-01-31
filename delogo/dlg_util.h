/*====================================================================
* 	ダイアログの位置関係の関数			dlg_util.h
*===================================================================*/
#ifndef ___DLG_UTIL_H
#define ___DLG_UTIL_H

#include <windows.h>

typedef struct {
	RECT rect;
	int w, h;
} ITEM_SIZE;

static ITEM_SIZE GetSize(HWND hwnd, ITEM_SIZE *parent, ITEM_SIZE *border) {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	ITEM_SIZE size;
	size.rect = rect;
	size.w = rect.right - rect.left;
	size.h = rect.bottom - rect.top;
	if (parent && border) {
		size.rect.top -= parent->rect.top + border->rect.top;
		size.rect.bottom -= parent->rect.top + border->rect.top;
		size.rect.right -= parent->rect.left + border->rect.left;
		size.rect.left -= parent->rect.left + border->rect.left;
	}
	return size;
}

static void MoveControl(HWND hdlg, int ControlId, const ITEM_SIZE *defaultPos, int move) {
	MoveWindow(GetDlgItem(hdlg, ControlId), defaultPos->rect.left + move, defaultPos->rect.top, defaultPos->w, defaultPos->h, TRUE);
}

static ITEM_SIZE GetBorderSize(HWND hdlg, const ITEM_SIZE& defualtWindow) {
	POINT point = { 0 };
	ClientToScreen(hdlg, &point);
	RECT rc;
	GetClientRect(hdlg, &rc);
	
	ITEM_SIZE border = { 0 };
	border.rect.top = point.y - defualtWindow.rect.top;
	border.rect.left = point.x - defualtWindow.rect.left;

	border.rect.bottom = defualtWindow.rect.bottom - defualtWindow.rect.top - border.rect.top;
	border.rect.right = defualtWindow.rect.right - defualtWindow.rect.left - border.rect.left;
	return border;
}

#endif
