/*====================================================================
* 	経過表示・中断ダイアログ
*
* 2003
* 	06/22:	中断できるようにしよう！ついでに経過表示もこっちで。
* 	07/02:	ようやく完成
*
*===================================================================*/
#include <windows.h>
#include <commctrl.h>
#include "abort.h"
#include "resource.h"
#include "filter.h"
#include "logoscan.h"

#define tLOGOX   0
#define tLOGOY   1
#define tLOGOW   2
#define tLOGOH   3
#define tTHY     4


typedef struct {
    int x, y, w, h;
} XYWH;

bool Cal_BGcolor(PIXEL_YC& r, PIXEL_YC* pix, XYWH& xywh, int w, int thy, short *tmp[3]);
int comp_short(const void* x, const void* y);
short med_average(short* s, int n);
int CreateLogoData(AbortDlgParam* p, HWND hdlg);//FILTER* fp,ScanPixel*& sp,void*&);




/*====================================================================
* 	AbortDlgProc()		コールバックプロシージャ
*===================================================================*/
BOOL CALLBACK AbortDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    const  int IDD_TIMER = 4301;
    static int examine;	// スキャン済み
    static int useable;	// 有効サンプル
    static AbortDlgParam* p;
    static bool abort;

    switch (msg) {
        case WM_INITDIALOG:
            p = (AbortDlgParam*)lParam;
            SetTimer(hdlg, IDD_TIMER, 1, NULL);	// 初期化のために解析開始を遅らせる
            SetDlgItemInt(hdlg, IDC_ALLF, p->e-p->s + 1, false);
            examine = useable = 0;
            abort = false;
            break;

        case WM_CLOSE:
            EndDialog(hdlg, 0);
            break;


        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_ABORT:
                    abort = true;
                    break;
            }
            break;

        case WM_TIMER:	// 読み取り開始
            KillTimer(hdlg, IDD_TIMER);

            //----------------------------------------------- ロゴ解析
            SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, p->e - p->s + 1));

            // (幅+高さ+2)*2
            const int tmp_buf_count = ((p->w + p->h + 2) * 2);
            short *tmp[3];
            tmp[0] = (short *)malloc(sizeof(tmp[0][0]) * tmp_buf_count * 3);
            tmp[1] = tmp[0] + tmp_buf_count;
            tmp[2] = tmp[1] + tmp_buf_count;

            while (examine <= p->e - p->s && !abort) {
                // pump windows message
                MSG message;
                while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETPOS, examine, 0);
                SetDlgItemInt(hdlg, IDC_USEABLE, useable, FALSE);
                SetDlgItemInt(hdlg, IDC_EXAMF, examine, FALSE);

                // 画像取得
                PIXEL_YC* pix = p->fp->exfunc->get_ycp_filtering_cache_ex(p->fp, p->editp, p->s + examine, NULL, NULL);

                // 背景平均値計算
                PIXEL_YC bg;
                XYWH xywh = { p->x, p->y, p->w, p->h };
                if (Cal_BGcolor(bg, pix, xywh, p->max_w, p->t, tmp)) {
                    // 単一背景のときサンプルをセットする
                    useable++;
                    SendMessage(p->fp->hwnd, WM_SP_DRAWFRAME, 0, p->s+examine);

                    if (p->mark) {	// 有効フレームをマーク
                        FRAME_STATUS fs;
                        p->fp->exfunc->get_frame_status(p->editp, p->s+examine, &fs);
                        fs.edit_flag |= EDIT_FRAME_EDIT_FLAG_MARKFRAME;
                        p->fp->exfunc->set_frame_status(p->editp, p->s+examine, &fs);
                    }
                    if (p->list) {
                        fprintf(p->list, "%d\n", p->s + examine + 1);
                    }

                    p->bg.push_back(bg);
                    // ロゴサンプルセット
                    bool error = false;
                    for (int i = 0; i < xywh.h; i++) {
                        for (int j = 0; j < xywh.w; j++) {
                            if (!error) {
                                PIXEL_YC ptr = pix[(xywh.y + i) * p->max_w + xywh.x + j];
                                error |= (0 != AddSample(&p->sp[i * xywh.w + j], ptr));
                            }
                        }
                    }

                    abort |= error;
                }
                examine++;

            } // end of while

            if (tmp[0]) {
                free(tmp[0]);
            }
            // ロゴデータ作成
            p->ret = CreateLogoData(p,hdlg);

            if (!abort)
                MessageBeep((UINT)-1);
            EndDialog(hdlg, 0);

            break;
    }

    return 0;
}

/*--------------------------------------------------------------------
*	ロゴデータを作成
*-------------------------------------------------------------------*/
int CreateLogoData(AbortDlgParam* p, HWND hdlg) {
    SetDlgItemText(hdlg, IDC_STATUS, "ロゴデータ構築中...");

    // ロゴヘッダ作成（名称以外）
    LOGO_HEADER lgh;
    ZeroMemory(&lgh, sizeof(LOGO_HEADER));
    lgh.x = (short)p->x;//fp->track[tLOGOX];
    lgh.y = (short)p->y;//fp->track[tLOGOY];
    lgh.w = (short)p->w;//fp->track[tLOGOW];
    lgh.h = (short)p->h;//fp->track[tLOGOH];

    const int bg_length = p->bg.size();
    short* lst_bgy  = (short *)malloc(sizeof(short) * bg_length);
    short* lst_bgcb = (short *)malloc(sizeof(short) * bg_length);
    short* lst_bgcr = (short *)malloc(sizeof(short) * bg_length);

    // ロゴデータ領域確保
    *p->data = malloc(logo_data_size(&lgh));
    if (p->data == nullptr || lst_bgy == nullptr || lst_bgcb == nullptr || lst_bgcr == nullptr) {
        ShowErrorMessage(ERROR_MALLOC);
        return 1;
    }
    *((LOGO_HEADER*)*p->data) = lgh; // ヘッダコピー

    LOGO_PIXEL* lgp = (LOGO_PIXEL*) ((LOGO_HEADER*)*p->data+1);

    SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, lgh.w * lgh.h - 1));

    for (int i = 0; i < bg_length; i++) {
        lst_bgy[i]  = p->bg[i].y;
        lst_bgcb[i] = p->bg[i].cb;
        lst_bgcr[i] = p->bg[i].cr;
    }

    bool error = false;
    for (int i = 0; i < lgh.w * lgh.h; i++) {
        SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETPOS, i, 0);
        if (!error) {
            error |= 0 != GetLGP(lgp[i], &p->sp[i], lst_bgy, lst_bgcb, lst_bgcr);
        }
        ClearSample(&p->sp[i]);
    }

    free(lst_bgy);
    free(lst_bgcb);
    free(lst_bgcr);
    return error != false;
}


/*--------------------------------------------------------------------
*	背景色計算
*-------------------------------------------------------------------*/
bool Cal_BGcolor(PIXEL_YC& r, PIXEL_YC* pix, XYWH& xywh, int w, int thy, short *tmp[3]) {
    short *y  = tmp[0];
    short *cb = tmp[1];
    short *cr = tmp[2];
    int i, n = 0;

    pix += xywh.x-1 + (xywh.y - 1) * w; // X-1, Y-1に移動

    // 横線（上）合計
    for (i = 0; i <= xywh.w+1; i++) {
        y[n]  = pix->y;
        cb[n] = pix->cb;
        cr[n] = pix->cr;
        n++;
        pix++;
    }
    pix += w - i; // 次の行へ
    // 縦線
    for (i = 2; i <= xywh.h+1; i++) {
        // 左線
        y[n]  = pix->y;
        cb[n] = pix->cb;
        cr[n] = pix->cr;
        n++;
        // 右線
        y[n]  = pix[xywh.w+1].y;
        cb[n] = pix[xywh.w+1].cb;
        cr[n] = pix[xywh.w+1].cr;
        n++;

        pix += w; // 次の行へ
    }
    // 横線（下）合計
    for (i=0; i <= xywh.w+1; i++) {
        y[n]  = pix->y;
        cb[n] = pix->cb;
        cr[n] = pix->cr;
        n++;
        pix++;
    }

    bool ret = true; // 返却値

    // 最小と最大が閾値以上離れている場合、単一色でないと判断
    qsort(y, n, sizeof(short), comp_short);
    if (abs(y[0] - y[n-1]) > thy * 8) {
        ret = false;
    } else {
        qsort(cb, n, sizeof(short), comp_short);
        if (abs(cb[0] - cb[n-1]) > thy * 8) {
            ret = false;
        } else {
            qsort(cr, n, sizeof(short), comp_short);
            if (abs(cr[0] - cr[n-1]) > thy * 8)
                ret = false;
        }
    }

    if (ret) {	// 真中らへんを平均
        r.y  = med_average(y, n);
        r.cb = med_average(cb, n);
        r.cr = med_average(cr, n);
    }

    return ret;
}

/*--------------------------------------------------------------------
*	short型比較関数（qsort用）
*-------------------------------------------------------------------*/
int comp_short(const void* x, const void* y)
{
    return (*(const short*)x - *(const short*)y);
}

/*--------------------------------------------------------------------
*	真中らへんを平均
*-------------------------------------------------------------------*/
short med_average(short* s, int n)
{
    double t =0;
    int nn =0;

    // ソートする
//	qsort(s, n, sizeof(short), comp_short);

    // 真中らへんを平均
    for (int i= n / 4; i < n-(n/4); i++, nn++)
        t += s[i];

    t = (t + nn/2) / nn;

    return ((short)t);
}


//*/
