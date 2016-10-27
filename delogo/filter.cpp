/*********************************************************************
* 	透過性ロゴ（BSマークとか）除去フィルタ
* 								ver 0.13
* 
* 2003
* 	02/01:	製作開始
* 	02/02:	拡張データ領域を使うとプロファイル切り替えた時AviUtlがくたばる。
* 	      	なぜ？バグ？どうしよう。
* 	      	と思ったら、領域サイズに制限があるのね… SDKには一言も書いてないけど。
* 	      	消えた－－!!（Ｂだけ）ちょっと感動。
* 	      	BSロゴって輝度だけの変化なのか？RGBでやると色変わるし。
* 	02/06:	プロファイルの途中切り替えに仮対応。
* 	02/08:	BS2ロゴ実装。（テスト用ver.1）
* 	02/11:	YCbCrを微調整できるようにした。
* 	02/17:	試験的に外部ファイル読み込み機能を搭載。（５つまで）
* 	03/02:	ロゴデータ保存領域関係（ポインタをコンボアイテムに関連付け）
* 	03/24:	ロゴデータ読み込み関係
* 	03/25:	フィルタ本体とロゴデータを切り離した。
* 	03/26:	ロゴデータの管理を完全にコンボアイテムと関連付け
* 	03/29:	ロゴデータ入出力ダイアログ（オプションダイアログ）作成
* 	03/30:	ロゴ付加モード追加
* 	04/03:	ロゴデータの構造を変更（dpをdp_y,dp_cb,dp_crに分離）
* 			オプションダイアログにプレビュー機能を追加
* 	04/06:	ロゴ解析アルゴリズムと、それを用いたフィルタのテスト版が
* 			完成したため計算式をそれに最適化。（不要な分岐を減らした）
* 	04/07:	ロゴプレビューの背景色を変更できるようにした。
* 	04/09:	解析プラグインからのデータを受信出来るようにした。
* 			深度調整の方法を変更(ofset->gain)
* 			プレビューで枠からはみ出さないようにした。（はみ出しすぎると落ちる）
* 	04/20:	フィルタ名変更。ロゴ付加モードを一時廃止。
* 	04/28:	1/4単位位置調整実装。
* 			ロゴ付加モード(あまり意味ないけど)復活
* 			オプションダイアログ表示中にAviUtlを終了できないように変更
* 			（エラーを出して落ちるバグ回避）
* 
* [正式版(0.01)公開]
* 
* 	05/04:	不透明度調整の方法を変更。
* 	05/08:	メモリ関連ルーチン大幅変更		(0.02)
* 			VFAPI動作に対応、プロファイルの途中切り替えに対応
* 			ロゴデータのサイズ制限を約４倍にした。
* 	05/10:	データが受信できなくなっていたバグを修正	(0.03)
* 	05/17:	ロゴ名を編集できるようにした。(0.04)
* 	06/12:	プレビューの背景色をRGBで指定できるように変更。
* 			位置調整が４の倍数のときcreate_adj_exdata()を呼ばないようにした。（0.05）
* 	06/30:	フェードイン・アウトに対応。 (0.06)
* 	07/02:	ロゴデータを受信できない場合があったのを修正。
* 	07/03:	YCbCrの範囲チェックをするようにした。(しないと落ちることがある)
* 			ロゴ名編集で同名にせっていできないようにした。(0.06a)
* 	08/01:	フェードの不透明度計算式を見直し
* 	08/02:	実数演算を止め、無駄な演算を削除して高速化。
* 			上に伴い深度のデフォルト値を変更。
* 			細かな修正
* 	09/05:	細かな修正
* 	09/27:	filter.hをAviUtl0.99SDKのものに差し替え。(0.07)
* 	10/20:	SSE2使用のrgb2ycがバグもちなので、自前でRGB->YCbCrするようにした。
* 			位置X/Yの最大･最小値を拡張した。(0.07a)
* 	10/25:	位置調整で-200以下にすると落ちるバグ修正。(0.07b)
* 2004
* 	02/18:	AviSynthスクリプトを吐くボタン追加。(0.08)
* 	04/17:	ロゴデータファイル読み込み時にデータが一つも無い時エラーを出さないようにした。
* 			開始･終了の最大値を4096まで増やした。(0.08a)
* 	09/19:	スタックを無駄遣いしていたのを修正。
* 			開始・フェードイン・アウト・終了の初期値をロゴデータに保存できるようにした。(0.09)
* 2005
* 	04/18:	フィルタ名、パラメタ名を変更できるようにした。(0.09a)
* 2007
* 	11/07:	プロファイルの境界をフェードの基点にできるようにした。(0.10)
* 2008
*	01/07:	ロゴのサイズ制限を撤廃
*			開始・終了パラメタの範囲変更(負の値も許可)
*			ロゴファイルのデータ数を拡張(1byte -> 4byte) (0.11)
*	06/21:	編集ダイアログで位置(X,Y)も編集できるようにした。(0.12)
*	07/03:	スピンコントロールで桁区切りのカンマが付かないようにした
*			ロゴ編集後、リストのロゴを選択状態にしなおすように
*			RGBtoYCの計算式を整数演算に (0.12a)
*	09/29:	スライダの最大最小値を変更できるようにした。(0.13)
* 
*********************************************************************/

/* ToDo:
* 	・ロゴデータの作成・編集機能
* 	・複数導入した時、ロゴリストを共有するように
* 	・フィルタの名称が変わっていてもロゴ解析から送信できるように
* 
*  MEMO:
* 	・ロゴの拡大縮小ルーチン自装しないとだめかなぁ。
* 		→必要なさげ。当面は自装しない。
* 	・ロゴ作成・編集は別アプリにしてしまおうか…
* 		仕様公開してるし、誰か作ってくれないかなぁ（他力本願）
* 	・ロゴ除去モードとロゴ付加モードを切り替えられるようにしようかな
* 		→付けてみた
* 	・解析プラグからデータを受け取るには…独自WndMsg登録してSendMessageで送ってもらう
* 		→ちゃんと動いた。…登録しなくてもUSER定義でよかったかも
* 	・ロゴに１ピクセル未満のズレがある。1/4ピクセルくらいでの位置調整が必要そう。
* 		→実装完了
* 	・ダイアログを表示したまま終了するとエラー吐く
* 		→親ウィンドウをAviUtl本体にすることで終了できなくした
* 	・ロゴデータ構造少し変えようかな… 色差要素のビットを半分にするとか。
* 
*  新メモリ管理について:(2003/05/08)
* 	fp->ex_data_ptrにはロゴの名称のみを保存。（7FFDバイトしかプロファイルに保存されず、不具合が生じるため）
* 	func_init()でロゴデータパックを読み込む。ldpファイル名は必ずフルパスであることが必要。
* 	読み込んだロゴデータのポインタはlogodata配列に保存。配列のデータ数はlogodata_nに。
* 	func_proc()ではex_data（ロゴ名称）と一致するデータをlogodata配列から検索。なかった場合は何もしない。
* 	位置パラメータを使って位置調整データを作成。その後で除去・付加関数を呼ぶ。
* 	WndProcでは、WM_FILTER_INITでコンボボックスアイテムをlogodata配列から作る。
* 	コンボアイテムのITEMDATAには従来どおりロゴデータのポインタを保存する。
* 	WM_FILTER_INITではコンボボックスアイテムからファイルに保存。（今までどおり）
* 	オプション設定ダイアログでのロゴデータの読み込み・削除は今までどおり。
* 	OKボタンが押されたときは、リストアイテムからlogodata配列を作り直す。コンボアイテムの更新は今までどおり。
*
*  複数導入でのロゴデータ共有の方法のアイディア (2009/01/24)
* 	初期化時にfpを走査、func_proc() に適当なメッセージを送る。(ロゴフィルタかどうか&バージョンチェック)
* 	データ共有は最初のフィルタがロゴデータを保持、他のフィルタは 最初のにfunc_proc()にメッセージをなげて取得。
* 	ロゴリスト編集ボタンどうしよう。最初のフィルタを呼び出すのがよいか。
* 	ロゴ解析も同じ方法でロゴフィルタを特定できる。
*/

/**************************************************************************************************
*  2014/10/05 (+r01)
*    delogo SIMD版 by rigaya
*    ロゴ消し部分のSIMD化
*
*  2015/01/08 (+r02)
*    トラックバーの上限を拡張しても、開始・終了などに256までの上限がかかる問題を修正。
*    AVX2/SSE2版を少し高速化
*
*  2015/01/10 (+r03)
*    トラックバーの上限を拡張しても、編集大アロログの入力欄に256までの上限がかかる問題を修正。
*
*  2015/02/03 (+r04)
*    ロゴ名の文字列を255文字までに拡張。
*    あわせて編集ダイアログのサイズを変更できるように。
*
*  2015/02/11 (+r05)
*    新しい形式のロゴファイルの拡張子を変更。
*    また書き出し時に古い形式での書き出しができるように。
*
*  2015/02/11 (+r07)
*    常に従来の設定ファイルで書き出されるようになっていたのを修正。
*    ファイルフィルタを選択型に変更。
*
*  2015/02/14 (+r08)
*    delogo.auf.iniの記述に従って、自動的にロゴを選択するモードを追加。
*
*  2015/02/28 (+r10)
*    自動選択の結果が"なし"のときに速度低下するのを抑制。
*    クリップボードへのコピーがうまく機能していなかったのを修正。
*
*  2015/05/24 (+r11)
*    編集ダイアログの開始・終了の入力欄で、負の値が入力できないのを修正。
*
*  2015/05/24 (+r12)
*    編集ダイアログの開始・終了の入力欄で負の値が保存時できていないのを修正。
*
*  2016/01/02 (+r13)
*    いくつかの箇所で128byte以上のロゴの名前だと正常に処理できないのを修正。
*
**************************************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <math.h>
#include "filter.h"
#include "logo.h"
#include "optdlg.h"
#include "resource.h"
#include "send_lgd.h"
#include "strdlg.h"
#include "logodef.h"
#include "delogo_proc.h"

#define LOGO_AUTO_SELECT 1

#define ID_BUTTON_OPTION          40001
#define ID_COMBO_LOGO             40002
#define ID_BUTTON_SYNTH           40003
#define ID_LABEL_LOGO_AUTO_SELECT 40004

#define Abs(x) ((x>0)? x:-x)
#define Clamp(n,l,h) ((n<l) ? l : (n>h) ? h : n)


#define LDP_KEY     "logofile"
#define LDP_DEFAULT "logodata.ldp2"
#define LDP_FILTER  "ロゴデータパック (*.ldp; *.ldp2)\0*.ldp;*.ldp2\0AllFiles (*.*)\0*.*\0"


// ダイアログアイテム
typedef struct {
	HFONT font;
	HWND  cb_logo;
	HWND  bt_opt;
	HWND  bt_synth;
#if LOGO_AUTO_SELECT
	HWND  lb_auto_select;
#endif
} FILTER_DIALOG;

static FILTER_DIALOG dialog = { 0 };

static char  logodata_file[MAX_PATH] = { 0 };	// ロゴデータファイル名(INIに保存)

#if LOGO_AUTO_SELECT
#define LOGO_AUTO_SELECT_STR "ファイル名から自動選択"
#define LOGO_AUTO_SELECT_NONE INT_MAX

typedef struct LOGO_SELECT_KEY {
	char *key;
	char logoname[LOGO_MAX_NAME];
} LOGO_SELECT_KEY;

typedef struct LOGO_AUTO_SELECT_DATA {
	int count;
	LOGO_SELECT_KEY *keys;
	char src_filename[MAX_PATH];
	int num_selected;
} LOGO_AUTO_SELECT_DATA;

static LOGO_HEADER LOGO_HEADER_AUTO_SELECT = { LOGO_AUTO_SELECT_STR };
static LOGO_AUTO_SELECT_DATA logo_select = { 0 };
#endif

#if LOGO_AUTO_SELECT
#define LOGO_AUTO_SELECT_USED (!!logo_select.count)
#else
#define LOGO_AUTO_SELECT_USED 0
#endif

static LOGO_HEADER** logodata   = NULL;
static unsigned int  logodata_n = 0;

static LOGO_HEADER *adjdata = NULL;	// 位置調節後ロゴデータ用バッファ
static unsigned int adjdata_size = 0;

static char ex_data[sizeof(LOGO_HEADER)] = { 0 };	// 拡張データ領域

static UINT  WM_SEND_LOGO_DATA = 0;	// ロゴ受信メッセージ

static FUNC_PROCESS_LOGO func_logo_erase = NULL; //ロゴ除去関数

//----------------------------
//	プロトタイプ宣言
//----------------------------
static void on_wm_filter_init(FILTER* fp);
static void on_wm_filter_exit(FILTER* fp);
static void init_dialog(HWND hwnd,HINSTANCE hinst);
static void update_cb_logo(char *name);
#if LOGO_AUTO_SELECT
static void on_wm_filter_file_close(FILTER* fp);
static void logo_auto_select_apply(FILTER *fp, int num);
static void logo_auto_select_remove(FILTER *fp);
static int logo_auto_select(FILTER* fp, FILTER_PROC_INFO *fpip);
#endif
static void load_logo_param(FILTER* fp, int num);
static void change_param(FILTER* fp);
static void set_cb_logo(FILTER* fp);
static int  set_combo_item(void* data);
static void del_combo_item(int num);
static void read_logo_pack(char *logodata_file,FILTER *fp);
static void set_sended_data(void* logodata,FILTER* fp);
static BOOL create_adj_exdata(FILTER *fp,LOGO_HEADER *adjdata,const LOGO_HEADER *data);
static int  find_logo(const char *logo_name);
static int calc_fade(FILTER *fp,FILTER_PROC_INFO *fpip);

static BOOL on_option_button(FILTER* fp);
static BOOL on_avisynth_button(FILTER* fp,void* editp);

BOOL func_proc_eraze_logo(FILTER *fp,FILTER_PROC_INFO *fpip,LOGO_HEADER *lgh,int);
BOOL func_proc_add_logo(FILTER *fp,FILTER_PROC_INFO *fpip,LOGO_HEADER *lgh,int);

//----------------------------
//	FILTER_DLL構造体
//----------------------------
char filter_name[] = LOGO_FILTER_NAME;
static char filter_info[] = LOGO_FILTER_NAME" ver 0.13+r12 by rigaya";
#define track_N 10
#if track_N
static TCHAR *track_name[track_N] = { 	"位置 X", "位置 Y", 
	                                      "深度", "Y", "Cb", "Cr", 
	                                      "開始", "FadeIn", "FadeOut", "終了" }; // トラックバーの名前
static int   track_default[track_N] = { 0, 0,
	                                     128, 0, 0, 0,
	                                     0, 0, 0, 0 }; // トラックバーの初期値
int          track_s[track_N] = { LOGO_XY_MIN, LOGO_XY_MIN,
	                               0, -100, -100, -100,
	                               LOGO_STED_MIN, 0, 0, LOGO_STED_MIN }; // トラックバーの下限値
int          track_e[track_N] = { LOGO_XY_MAX, LOGO_XY_MAX,
	                               256, 100, 100, 100,
	                               LOGO_STED_MAX, LOGO_FADE_MAX, LOGO_FADE_MAX, LOGO_STED_MAX }; // トラックバーの上限値
#endif
#define check_N 3
#if check_N
static TCHAR *check_name[]   = { "ロゴ付加モード","ロゴ除去モード","ﾌﾟﾛﾌｧｲﾙ境界をﾌｪｰﾄﾞ基点にする" }; // チェックボックス
static int    check_default[] = { 0, 1, 0 };	// デフォルト
#endif

#define LOGO_X      0
#define LOGO_Y      1
#define LOGO_YDP    2
#define LOGO_CBDP   2
#define LOGO_CRDP   2
#define LOGO_PY     3
#define LOGO_CB     4
#define LOGO_CR     5
#define LOGO_STRT   6
#define LOGO_FIN    7
#define LOGO_FOUT   8
#define LOGO_END    9

#define LOGO_ADDMODE 0
#define LOGO_DELMODE 1
#define LOGO_BASEPROFILE 2

// 設定ウィンドウの高さ
#define WND_Y (67+24*track_N+20*check_N)

static FILTER_DLL filter = {
	FILTER_FLAG_WINDOW_SIZE |	//	フィルタのフラグ
	FILTER_FLAG_EX_DATA |
	FILTER_FLAG_EX_INFORMATION,
	320,WND_Y,			// 設定ウインドウのサイズ
	filter_name,    	// フィルタの名前
#if track_N
	track_N,        	// トラックバーの数
	track_name,     	// トラックバーの名前郡
	track_default,  	// トラックバーの初期値郡
	track_s,track_e,	// トラックバーの数値の下限上限
#else
	0,NULL,NULL,NULL,NULL,
#endif
#if check_N
	check_N,      	// チェックボックスの数
	check_name,   	// チェックボックスの名前郡
	check_default,	// チェックボックスの初期値郡
#else
	0,NULL,NULL,
#endif
	func_proc,   	// フィルタ処理関数
	func_init,		// 開始時に呼ばれる
	func_exit,   	// 終了時に呼ばれる関数
	NULL,        	// 設定が変更されたときに呼ばれる関数
	func_WndProc,	// 設定ウィンドウプロシージャ
	NULL,NULL,   	// システムで使用
	ex_data,     	// 拡張データ領域
	sizeof(LOGO_HEADER),//57102, // 拡張データサイズ
	filter_info, 	// フィルタ情報
	NULL,			// セーブ開始直前に呼ばれる関数
	NULL,			// セーブ終了時に呼ばれる関数
	NULL,NULL,NULL,	// システムで使用
	ex_data,		// 拡張領域初期値
};




/*********************************************************************
*	DLL Export
*********************************************************************/
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable( void )
{
	return &filter;
}

/*====================================================================
*	開始時に呼ばれる関数
*===================================================================*/
BOOL func_init( FILTER *fp )
{
	//使用する関数を取得
	func_logo_erase = get_delogo_func();

	// INIからロゴデータファイル名を読み込む
	fp->exfunc->ini_load_str(fp, LDP_KEY, logodata_file, NULL);

	if (lstrlen(logodata_file) == 0) { // ロゴデータファイル名がなかったとき
		// 読み込みダイアログ
		if (!fp->exfunc->dlg_get_load_name(logodata_file, LDP_FILTER, LDP_DEFAULT)) {
			// キャンセルされた
			MessageBox(fp->hwnd, "ロゴデータファイルがありません", filter_name, MB_OK|MB_ICONWARNING);
			return FALSE;
		}
	} else { // ロゴデータファイル名があるとき
		// 存在を調べる
		HANDLE hFile = CreateFile(logodata_file, 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE) { // みつからなかったとき
			MessageBox(fp->hwnd, "ロゴデータファイルが見つかりません", filter_name, MB_OK|MB_ICONWARNING);
			if (!fp->exfunc->dlg_get_load_name(logodata_file, LDP_FILTER, LDP_DEFAULT)) {
				// キャンセルされた
				MessageBox(fp->hwnd, "ロゴデータファイルが指定されていません", filter_name, MB_OK|MB_ICONWARNING);
				return FALSE;
			}
		} else {
			CloseHandle(hFile);
		}
	}

	// ロゴファイル読み込み
	read_logo_pack(logodata_file, fp);

	if (logodata_n)
		// 拡張データ初期値設定
		fp->ex_data_def = logodata[0];

	return TRUE;
}

/*====================================================================
*	終了時に呼ばれる関数
*===================================================================*/
#pragma warning (push)
#pragma warning (disable: 4100) //'fp' : 引数は関数の本体部で 1 度も参照されません。
BOOL func_exit( FILTER *fp )
{
	// ロゴデータ開放
	if (logodata) {
		for (unsigned int i = LOGO_AUTO_SELECT_USED; i < logodata_n; i++) {
			if (logodata[i]) {
				free(logodata[i]);
				logodata[i] = NULL;
			}
		}
		free(logodata);
		logodata = NULL;
	}

	free(adjdata);
	adjdata = NULL;
	adjdata_size = 0;

	func_logo_erase = NULL;

	return TRUE;
}
#pragma warning (pop)
/*====================================================================
*	フィルタ処理関数
*===================================================================*/
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	// ロゴ検索
	int num = find_logo((const char *)fp->ex_data_ptr);
	if (num < 0) return FALSE;

#if LOGO_AUTO_SELECT
	//ロゴ自動選択
	if (logo_select.count) {
		if (num == 0) {
			if (0 <= (num = logo_auto_select(fp, fpip))) {
				//"0"あるいは正の値なら変更あり (負なら以前から変更なし)
				logo_auto_select_apply(fp, num);
			}
			num = Abs(num);
			if (num == LOGO_AUTO_SELECT_NONE) return TRUE; //選択されたロゴがなければ終了
		} else {
			logo_auto_select_remove(fp);
		}
	}
#endif

	unsigned int size = sizeof(LOGO_HEADER) + (logodata[num]->h + 1) * (logodata[num]->w + 1) * sizeof(LOGO_PIXEL);
	if (size > adjdata_size) {
		adjdata = (LOGO_HEADER *)realloc(adjdata, size);
		adjdata_size = size;
	}
	if (adjdata == NULL) { //確保失敗
		adjdata_size = 0;
		return FALSE;
	}

	int fade = calc_fade(fp, fpip);

	if (fp->track[LOGO_X]%4 || fp->track[LOGO_Y]%4) {
		// 位置調整が４の倍数でないとき、1/4ピクセル単位調整
		if (!create_adj_exdata(fp, adjdata, logodata[num]))
			return FALSE;
	} else {
		// 4の倍数のときはx,yのみ書き換え
		memcpy(adjdata, logodata[num], size);
		adjdata->x += (short)(fp->track[LOGO_X] / 4);
		adjdata->y += (short)(fp->track[LOGO_Y] / 4);
	}

	if (fp->check[LOGO_DELMODE]) // 除去モードチェック
		return func_logo_erase(fp, fpip, adjdata, fade); // ロゴ除去モード
	else
		return func_proc_add_logo(fp, fpip, adjdata, fade); // ロゴ付加モード
}

#if LOGO_AUTO_SELECT
/*--------------------------------------------------------------------
* 	logo_auto_select_apply()
*-------------------------------------------------------------------*/
static void logo_auto_select_apply(FILTER *fp, int num) {
	SetWindowPos(fp->hwnd, 0, 0, 0, 320, WND_Y + 20, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	SendMessage(dialog.lb_auto_select, WM_SETTEXT, 0, ((num == LOGO_AUTO_SELECT_NONE) ? (LPARAM)"なし" : (LPARAM)logodata[num]));
}

/*--------------------------------------------------------------------
* 	logo_auto_select_remove()
*-------------------------------------------------------------------*/
static void logo_auto_select_remove(FILTER *fp) {
	char buf[LOGO_MAX_NAME] = { 0 };
	GetWindowText(dialog.lb_auto_select, buf, _countof(buf));
	if (strlen(buf)) {
		SendMessage(dialog.lb_auto_select, WM_SETTEXT, 0, (LPARAM)"");
		SetWindowPos(fp->hwnd, 0, 0, 0, 320, WND_Y, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		fp->track[LOGO_STRT] = 0;
		fp->track[LOGO_FIN]  = 0;
		fp->track[LOGO_FOUT] = 0;
		fp->track[LOGO_END]  = 0;
		fp->exfunc->filter_window_update(fp);
	}
}

/*--------------------------------------------------------------------
* 	logo_auto_select() 自動選択のロゴ名を取得
*-------------------------------------------------------------------*/
int logo_auto_select(FILTER* fp, FILTER_PROC_INFO *fpip) {
	int source_id, source_video_number;
	FILE_INFO file_info;
	if (fp->exfunc->get_source_video_number(fpip->editp, fpip->frame, &source_id, &source_video_number)
		&& fp->exfunc->get_source_file_info(fpip->editp, &file_info, source_id)) {

		if (0 == _stricmp(file_info.name, logo_select.src_filename))
			return -logo_select.num_selected; //変更なし

		strcpy_s(logo_select.src_filename, file_info.name);
		for (int i = 0; i < logo_select.count; i++) {
			if (strstr(logo_select.src_filename, logo_select.keys[i].key)) {
				return (logo_select.num_selected = find_logo(logo_select.keys[i].logoname));
			}
		}
	}
	return (logo_select.num_selected = LOGO_AUTO_SELECT_NONE); //見つからなかった
}
#endif //LOGO_AUTO_SELECT

/*--------------------------------------------------------------------
* 	func_proc_eraze_logo()	ロゴ除去モード
*-------------------------------------------------------------------*/
BOOL func_proc_eraze_logo(FILTER* fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade)
{
	// LOGO_PIXELデータへのポインタ
	LOGO_PIXEL *lgp = (LOGO_PIXEL *)(lgh + 1);

	// 左上の位置へ移動
	PIXEL_YC *ptr = fpip->ycp_edit;
	ptr += lgh->x + lgh->y * fpip->max_w;

	for (int i = 0; i < lgh->h; ++i) {
		for (int j = 0; j < lgh->w; ++j) {

			if (ptr >= fpip->ycp_edit && // 画面内の時のみ処理
			   ptr < fpip->ycp_edit + fpip->max_w*fpip->h) {
				int dp;
				// 輝度
				dp = (lgp->dp_y * fp->track[LOGO_YDP] * fade +64)/128 /LOGO_FADE_MAX;
				if (dp) {
					if(dp==LOGO_MAX_DP) dp--; // 0での除算回避
					int yc = lgp->y + fp->track[LOGO_PY]*16;
					yc = (ptr->y*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);	// 逆算
					ptr->y = (short)Clamp(yc,-128,4096+128);
				}

				// 色差(青)
				dp = (lgp->dp_cb * fp->track[LOGO_CBDP] * fade +64)/128 /LOGO_FADE_MAX;
				if (dp) {
					if(dp==LOGO_MAX_DP) dp--; // 0での除算回避
					int yc = lgp->cb + fp->track[LOGO_CB]*16;
					yc = (ptr->cb*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);
					ptr->cb = (short)Clamp(yc,-2048-128,2048+128);
				}

				// 色差(赤)
				dp = (lgp->dp_cr * fp->track[LOGO_CRDP] * fade +64)/128 /LOGO_FADE_MAX;
				if (dp) {
					if(dp==LOGO_MAX_DP) dp--; // 0での除算回避
					int yc = lgp->cr + fp->track[LOGO_CR]*16;
					yc = (ptr->cr*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);
					ptr->cr = (short)Clamp(yc,-2048-128,2048+128);
				}

			} // if画面内

			++ptr; // 隣りへ
			++lgp;
		}
		// 次のラインへ
		ptr += fpip->max_w - lgh->w;
	}

	return TRUE;
}

/*--------------------------------------------------------------------
* 	func_proc_add_logo()	ロゴ付加モード
*-------------------------------------------------------------------*/
BOOL func_proc_add_logo(FILTER *fp, FILTER_PROC_INFO *fpip, LOGO_HEADER *lgh, int fade)
{
	// LOGO_PIXELデータへのポインタ
	LOGO_PIXEL *lgp = (LOGO_PIXEL *)(lgh + 1);

	// 左上の位置へ移動
	PIXEL_YC *ptr = fpip->ycp_edit;
	ptr += lgh->x + lgh->y * fpip->max_w;

	for (int i = 0; i < lgh->h; ++i) {
		for (int j = 0; j < lgh->w; ++j) {

			if(ptr >= fpip->ycp_edit && // 画面内の時のみ処理
			   ptr < fpip->ycp_edit + fpip->max_w*fpip->h) {
				int dp;
				// 輝度
				dp = (lgp->dp_y * fp->track[LOGO_YDP] *fade +64)/128 /LOGO_FADE_MAX;
				if (dp) {
					int yc = lgp->y    + fp->track[LOGO_PY]*16;
					yc = (ptr->y*(LOGO_MAX_DP-dp) + yc*dp +(LOGO_MAX_DP/2)) /LOGO_MAX_DP; // ロゴ付加
					ptr->y = (short)Clamp(yc,-128,4096+128);
				}


				// 色差(青)
				dp = (lgp->dp_cb * fp->track[LOGO_CBDP] *fade +64)/128 /LOGO_FADE_MAX;
				if (dp) {
					int yc = lgp->cb   + fp->track[LOGO_CB]*16;
					yc = (ptr->cb*(LOGO_MAX_DP-dp) + yc*dp +(LOGO_MAX_DP/2)) /LOGO_MAX_DP;
					ptr->cb = (short)Clamp(yc,-2048-128,2048+128);
				}

				// 色差(赤)
				dp = (lgp->dp_cr * fp->track[LOGO_CRDP] * fade +64)/128 /LOGO_FADE_MAX;
				if (dp) {
					int yc = lgp->cr   + fp->track[LOGO_CR]*16;
					yc = (ptr->cr*(LOGO_MAX_DP-dp) + yc*dp +(LOGO_MAX_DP/2)) /LOGO_MAX_DP;
					ptr->cr = (short)Clamp(yc,-2048-128,2048+128);
				}

			} // if画面内

			++ptr; // 隣りへ
			++lgp;
		}
		// 次のラインへ
		ptr += fpip->max_w - lgh->w;
	}

	return TRUE;
}

/*--------------------------------------------------------------------
* 	find_logo()		ロゴ名からロゴデータを検索
*-------------------------------------------------------------------*/
static int find_logo(const char *logo_name)
{
	for (unsigned int i = 0; i < logodata_n; ++i) {
		if (0 == lstrcmp((char *)logodata[i], logo_name))
			return i;
	}
	return -1;
}

/*--------------------------------------------------------------------
* 	calc_fade()		フェード不透明度計算
*-------------------------------------------------------------------*/
static int calc_fade(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	int s, e;

	if (fp->check[LOGO_BASEPROFILE]) {
		FRAME_STATUS fs;
		if (!fp->exfunc->get_frame_status(fpip->editp, fpip->frame, &fs))
			return LOGO_FADE_MAX;

		int profile = fs.config;

		int i;
		for (i = fpip->frame; i; --i) {
			if (!fp->exfunc->get_frame_status(fpip->editp, i-1, &fs))
				return LOGO_FADE_MAX;
			if (fs.config != profile)
				break;
		}
		s = i;

		for (i = fpip->frame; i < fpip->frame_n-1; ++i) {
			if (!fp->exfunc->get_frame_status(fpip->editp, i+1, &fs))
				return LOGO_FADE_MAX;
			if (fs.config != profile)
				break;
		}
		e = i;
	} else {
		// 選択範囲取得
		if (!fp->exfunc->get_select_frame(fpip->editp, &s, &e))
			return LOGO_FADE_MAX;
	}

	// フェード不透明度計算
	int fade = 0;
	if (fpip->frame < s+fp->track[LOGO_STRT]+fp->track[LOGO_FIN]) {
		if (fpip->frame < s+fp->track[LOGO_STRT])
			return 0; // フェードイン前
		else // フェードイン
			fade = ((fpip->frame-s-fp->track[LOGO_STRT])*2 +1)*LOGO_FADE_MAX / (fp->track[LOGO_FIN]*2);
	} else if (fpip->frame > e-fp->track[LOGO_FOUT]-fp->track[LOGO_END]) {
		if (fpip->frame > e-fp->track[LOGO_END])
			return 0; // フェードアウト後
		else // フェードアウト
			fade = ((e-fpip->frame-fp->track[LOGO_END])*2+1)*LOGO_FADE_MAX / (fp->track[LOGO_FOUT]*2);
	} else {
		fade = LOGO_FADE_MAX; // 通常
	}

	return fade;
}

/*====================================================================
*	設定ウィンドウプロシージャ
*===================================================================*/
BOOL func_WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp )
{
	static int mode = 1; // 0:addlogo; 1:delogo

	if (message == WM_SEND_LOGO_DATA) { // ロゴデータ受信
		set_sended_data((void *)wParam,fp);
		return TRUE;
	}

	switch (message) {
		case WM_FILTER_INIT: // 初期化
			on_wm_filter_init(fp);
			return TRUE;

		case WM_FILTER_EXIT: // 終了
			on_wm_filter_exit(fp);
			break;
#if LOGO_AUTO_SELECT
		case WM_FILTER_FILE_CLOSE:
			on_wm_filter_file_close(fp); //ファイルクローズ
			break;
#endif //LOGO_AUTO_SELECT
		case WM_FILTER_UPDATE: // フィルタ更新
		case WM_FILTER_SAVE_END: // セーブ終了
			// コンボボックス表示更新
			update_cb_logo(ex_data);
			break;

		case WM_FILTER_CHANGE_PARAM:
			if (fp->check[!mode]) { // モードが変更された
				fp->check[mode] = 0;
				fp->exfunc->filter_window_update(fp);
				mode = (!mode);
				return TRUE;
			} else if(!fp->check[mode]) {
				fp->check[mode] = 1;
				fp->exfunc->filter_window_update(fp);
				return TRUE;
			}
			break;

		//---------------------------------------------ボタン動作
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_BUTTON_OPTION: // オプションボタン
					return on_option_button(fp);

				case ID_COMBO_LOGO: // コンボボックス
					switch (HIWORD(wParam)) {
						case CBN_SELCHANGE: // 選択変更
							change_param(fp);
							return TRUE;
					}
					break;

				case ID_BUTTON_SYNTH:	// AviSynthボタン
					return on_avisynth_button(fp,editp);
			}
			break;

		case WM_KEYUP: // メインウィンドウへ送る
		case WM_KEYDOWN:
		case WM_MOUSEWHEEL:
			SendMessage(GetWindow(hwnd, GW_OWNER), message, wParam, lParam);
			break;
	}

	return FALSE;
}

/*--------------------------------------------------------------------
* 	on_wm_filter_init()		設定ウィンドウ初期化
*-------------------------------------------------------------------*/
static void on_wm_filter_init(FILTER* fp)
{
	init_dialog(fp->hwnd, fp->dll_hinst);
	// コンボアイテムセット
	for (unsigned int i = 0; i < logodata_n; i++)
		set_combo_item(logodata[i]);

	// ロゴデータ受信メッセージ登録
	WM_SEND_LOGO_DATA = RegisterWindowMessage(wm_send_logo_data);
}

/*--------------------------------------------------------------------
* 	on_wm_filter_exit()		終了処理
* 		読み込まれているロゴデータをldpに保存
*-------------------------------------------------------------------*/
static void on_wm_filter_exit(FILTER* fp)
{
	if (lstrlen(logodata_file) == 0) { // ロゴデータファイル名がないとき
		if (!fp->exfunc->dlg_get_load_name(logodata_file, LDP_FILTER, LDP_DEFAULT)) {
			// キャンセルされた
			MessageBox(fp->hwnd,"ロゴデータは保存されません", filter_name, MB_OK|MB_ICONWARNING);
			return;
		}
	} else { // ロゴデータファイル名があるとき
		// 存在を調べる
		HANDLE hFile = CreateFile(logodata_file, 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE) { // みつからなかったとき
			MessageBox(fp->hwnd,"ロゴデータファイルが見つかりません",filter_name, MB_OK|MB_ICONWARNING);
			if (!fp->exfunc->dlg_get_load_name(logodata_file, LDP_FILTER, LDP_DEFAULT)) {
				// キャンセルされた
				MessageBox(fp->hwnd,"ロゴデータは保存されません",filter_name,MB_OK|MB_ICONWARNING);
				return;
			}
		} else {
			CloseHandle(hFile);
		}
	}

	// 登録されているアイテムの数
	int num = SendMessage(dialog.cb_logo, CB_GETCOUNT, 0, 0);

	// ファイルオープン
	HANDLE hFile = CreateFile(logodata_file, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	SetFilePointer(hFile, 0, 0, FILE_BEGIN); // 先頭へ

	// ヘッダ書き込み
	LOGO_FILE_HEADER logo_file_header = { 0 };
	strcpy_s(logo_file_header.str, LOGO_FILE_HEADER_STR);
	DWORD data_written = 0;
	WriteFile(hFile, &logo_file_header, sizeof(LOGO_FILE_HEADER), &data_written, NULL);
	if (data_written != 32) {	// 書き込み失敗
		MessageBox(fp->hwnd, "ロゴデータ保存に失敗しました(1)", filter_name, MB_OK|MB_ICONERROR);
	} else { // 成功
		int total_count = 0;
		// 各データ書き込み
		for (int i = LOGO_AUTO_SELECT_USED; i < num; i++) {
			data_written = 0;
			LOGO_HEADER *data = (LOGO_HEADER *)SendMessage(dialog.cb_logo, CB_GETITEMDATA, i, 0); // データのポインタ取得
			WriteFile(hFile, data, logo_data_size(data), &data_written, NULL);
			if ((int)data_written != logo_data_size(data)) {
				MessageBox(fp->hwnd,"ロゴデータ保存に失敗しました(2)", filter_name, MB_OK|MB_ICONERROR);
				break;
			}
			total_count++;
		}

		logo_file_header.logonum.l = SWAP_ENDIAN(total_count);
		SetFilePointer(hFile,0, 0, FILE_BEGIN); // 先頭へ
		data_written = 0;
		WriteFile(hFile, &logo_file_header, sizeof(logo_file_header), &data_written, NULL);
		if (data_written != sizeof(logo_file_header))
			MessageBox(fp->hwnd, "ロゴデータ保存に失敗しました(3)", filter_name, MB_OK|MB_ICONERROR);
	}

	CloseHandle(hFile);

	// INIにロゴデータファイル名保存
	fp->exfunc->ini_save_str(fp, LDP_KEY, logodata_file);
}

#if LOGO_AUTO_SELECT
/*--------------------------------------------------------------------
* 	on_wm_filter_file_close() ファイルのクローズ
* 		ロゴの自動選択になっていたら、画面を元に戻す
*-------------------------------------------------------------------*/
static void on_wm_filter_file_close(FILTER* fp) {
	logo_auto_select_remove(fp);
	logo_select.src_filename[0] = '\0';
	logo_select.num_selected = 0;
}
#endif

/*--------------------------------------------------------------------
*	init_dialog()		ダイアログアイテムを作る
*		・コンボボックス
*		・オプションボタン
*-------------------------------------------------------------------*/
static void init_dialog(HWND hwnd, HINSTANCE hinst)
{
#define ITEM_Y (19+24*track_N+20*check_N)

	// フォント作成
	dialog.font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	// コンボボックス
	dialog.cb_logo = CreateWindow("COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL,
									8,ITEM_Y, 300,100, hwnd, (HMENU)ID_COMBO_LOGO, hinst, NULL);
	SendMessage(dialog.cb_logo, WM_SETFONT, (WPARAM)dialog.font, 0);

	// オプションボタン
	dialog.bt_opt = CreateWindow("BUTTON", "オプション", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_VCENTER,
									240,ITEM_Y-30, 66,24, hwnd, (HMENU)ID_BUTTON_OPTION, hinst, NULL);
	SendMessage(dialog.bt_opt, WM_SETFONT, (WPARAM)dialog.font, 0);

	// AviSynthボタン
	dialog.bt_synth = CreateWindow("BUTTON", "AviSynth", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_VCENTER,
									240,ITEM_Y-60, 66,24, hwnd, (HMENU)ID_BUTTON_SYNTH, hinst, NULL);
	SendMessage(dialog.bt_synth, WM_SETFONT, (WPARAM)dialog.font, 0);

#if LOGO_AUTO_SELECT
	//自動選択ロゴ表示
	dialog.lb_auto_select = CreateWindow("static", "", SS_SIMPLE | WS_CHILD | WS_VISIBLE,
									20,ITEM_Y+23, 288,24, hwnd, (HMENU)ID_LABEL_LOGO_AUTO_SELECT, hinst, NULL);
	SendMessage(dialog.lb_auto_select, WM_SETFONT, (WPARAM)dialog.font, 0);
#endif
}

/*--------------------------------------------------------------------
*	create_adj_exdata()		位置調整ロゴデータ作成
*-------------------------------------------------------------------*/
#pragma warning (push)
#pragma warning (disable: 4244) //C4244: '=' : 'int' から 'short' への変換です。データが失われる可能性があります。
static BOOL create_adj_exdata(FILTER *fp, LOGO_HEADER *adjdata, const LOGO_HEADER *data)
{
	int i, j;

	if (data == NULL)
		return FALSE;

	// ロゴ名コピー
	memcpy(adjdata->name, data->name, LOGO_MAX_NAME);

	// 左上座標設定（位置調整後）
	adjdata->x = data->x + (int)(fp->track[LOGO_X]-LOGO_XY_MIN)/4 + LOGO_XY_MIN/4;
	adjdata->y = data->y + (int)(fp->track[LOGO_Y]-LOGO_XY_MIN)/4 + LOGO_XY_MIN/4;

	const int w = data->w + 1; // 1/4単位調整するため
	const int h = data->h + 1; // 幅、高さを１増やす
	adjdata->w = w; 
	adjdata->h = h;

	// LOGO_PIXELの先頭
	LOGO_PIXEL *df = (LOGO_PIXEL *)(data +1);
	LOGO_PIXEL *ex = (LOGO_PIXEL *)(adjdata +1);

	const int adjx = (fp->track[LOGO_X]-LOGO_XY_MIN) % 4; // 位置端数
	const int adjy = (fp->track[LOGO_Y]-LOGO_XY_MIN) % 4;

	//----------------------------------------------------- 一番上の列
	ex[0].dp_y  = df[0].dp_y *(4-adjx)*(4-adjy)/16; // 左端
	ex[0].dp_cb = df[0].dp_cb*(4-adjx)*(4-adjy)/16;
	ex[0].dp_cr = df[0].dp_cr*(4-adjx)*(4-adjy)/16;
	ex[0].y  = df[0].y;
	ex[0].cb = df[0].cb;
	ex[0].cr = df[0].cr;
	for (i = 1; i < w-1; ++i) { //中
		// Y
		ex[i].dp_y = (df[i-1].dp_y*adjx*(4-adjy)
							+ df[i].dp_y*(4-adjx)*(4-adjy)) /16;
		if (ex[i].dp_y)
			ex[i].y  = (df[i-1].y*Abs(df[i-1].dp_y)*adjx*(4-adjy)
					+ df[i].y * Abs(df[i].dp_y)*(4-adjx)*(4-adjy))
				/(Abs(df[i-1].dp_y)*adjx*(4-adjy) + Abs(df[i].dp_y)*(4-adjx)*(4-adjy));
		// Cb
		ex[i].dp_cb = (df[i-1].dp_cb*adjx*(4-adjy)
							+ df[i].dp_cb*(4-adjx)*(4-adjy)) /16;
		if (ex[i].dp_cb)
			ex[i].cb = (df[i-1].cb*Abs(df[i-1].dp_cb)*adjx*(4-adjy)
					+ df[i].cb * Abs(df[i].dp_cb)*(4-adjx)*(4-adjy))
				/ (Abs(df[i-1].dp_cb)*adjx*(4-adjy)+Abs(df[i].dp_cb)*(4-adjx)*(4-adjy));
		// Cr
		ex[i].dp_cr = (df[i-1].dp_cr*adjx*(4-adjy)
							+ df[i].dp_cr*(4-adjx)*(4-adjy)) /16;
		if (ex[i].dp_cr)
			ex[i].cr = (df[i-1].cr*Abs(df[i-1].dp_cr)*adjx*(4-adjy)
					+ df[i].cr * Abs(df[i].dp_cr)*(4-adjx)*(4-adjy))
				/ (Abs(df[i-1].dp_cr)*adjx*(4-adjy)+Abs(df[i].dp_cr)*(4-adjx)*(4-adjy));
	}
	ex[i].dp_y  = df[i-1].dp_y * adjx *(4-adjy)/16; // 右端
	ex[i].dp_cb = df[i-1].dp_cb* adjx *(4-adjy)/16;
	ex[i].dp_cr = df[i-1].dp_cr* adjx *(4-adjy)/16;
	ex[i].y  = df[i-1].y;
	ex[i].cb = df[i-1].cb;
	ex[i].cr = df[i-1].cr;

	//----------------------------------------------------------- 中
	for (j = 1; j < h-1; ++j) {
		// 輝度Y  //---------------------- 左端
		ex[j*w].dp_y = (df[(j-1)*data->w].dp_y*(4-adjx)*adjy
						+ df[j*data->w].dp_y*(4-adjx)*(4-adjy)) /16;
		if (ex[j*w].dp_y)
			ex[j*w].y = (df[(j-1)*data->w].y*Abs(df[(j-1)*data->w].dp_y)*(4-adjx)*adjy
						+ df[j*data->w].y*Abs(df[j*data->w].dp_y)*(4-adjx)*(4-adjy))
				/ (Abs(df[(j-1)*data->w].dp_y)*(4-adjx)*adjy+Abs(df[j*data->w].dp_y)*(4-adjx)*(4-adjy));
		// 色差(青)Cb
		ex[j*w].dp_cb = (df[(j-1)*data->w].dp_cb*(4-adjx)*adjy
						+ df[j*data->w].dp_cb*(4-adjx)*(4-adjy)) / 16;
		if (ex[j*w].dp_cb)
			ex[j*w].cb = (df[(j-1)*data->w].cb*Abs(df[(j-1)*data->w].dp_cb)*(4-adjx)*adjy
						+ df[j*data->w].cb*Abs(df[j*data->w].dp_cb)*(4-adjx)*(4-adjy))
				/ (Abs(df[(j-1)*data->w].dp_cb)*(4-adjx)*adjy+Abs(df[j*data->w].dp_cb)*(4-adjx)*(4-adjy));
		// 色差(赤)Cr
		ex[j*w].dp_cr = (df[(j-1)*data->w].dp_cr*(4-adjx)*adjy
						+ df[j*data->w].dp_cr*(4-adjx)*(4-adjy)) / 16;
		if (ex[j*w].dp_cr)
			ex[j*w].cr = (df[(j-1)*data->w].cr*Abs(df[(j-1)*data->w].dp_cr)*(4-adjx)*adjy
						+ df[j*data->w].cr*Abs(df[j*data->w].dp_cr)*(4-adjx)*(4-adjy))
				/ (Abs(df[(j-1)*data->w].dp_cr)*(4-adjx)*adjy+Abs(df[j*data->w].dp_cr)*(4-adjx)*(4-adjy));
		for (i = 1; i < w-1; ++i) { //------------------ 中
			// Y
			ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y*adjx*adjy
							+ df[(j-1)*data->w+i].dp_y*(4-adjx)*adjy
							+ df[j*data->w+i-1].dp_y*adjx*(4-adjy)
							+ df[j*data->w+i].dp_y*(4-adjx)*(4-adjy) ) /16;
			if (ex[j*w+i].dp_y)
				ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
							+ df[(j-1)*data->w+i].y*Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy
							+ df[j*data->w+i-1].y*Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy)
							+ df[j*data->w+i].y*Abs(df[j*data->w+i].dp_y)*(4-adjx)*(4-adjy) )
					/ (Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy + Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy
						+ Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy)+Abs(df[j*data->w+i].dp_y)*(4-adjx)*(4-adjy));
			// Cb
			ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb*adjx*adjy
							+ df[(j-1)*data->w+i].dp_cb*(4-adjx)*adjy
							+ df[j*data->w+i-1].dp_cb*adjx*(4-adjy)
							+ df[j*data->w+i].dp_cb*(4-adjx)*(4-adjy) ) /16;
			if (ex[j*w+i].dp_cb)
				ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
							+ df[(j-1)*data->w+i].cb*Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy
							+ df[j*data->w+i-1].cb*Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy)
							+ df[j*data->w+i].cb*Abs(df[j*data->w+i].dp_cb)*(4-adjx)*(4-adjy) )
					/ (Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy + Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy
						+ Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy) + Abs(df[j*data->w+i].dp_cb)*(4-adjx)*(4-adjy));
			// Cr
			ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr*adjx*adjy
							+ df[(j-1)*data->w+i].dp_cr*(4-adjx)*adjy
							+ df[j*data->w+i-1].dp_cr*adjx*(4-adjy)
							+ df[j*data->w+i].dp_cr*(4-adjx)*(4-adjy) ) /16;
			if (ex[j*w+i].dp_cr)
				ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
							+ df[(j-1)*data->w+i].cr*Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy
							+ df[j*data->w+i-1].cr*Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy)
							+ df[j*data->w+i].cr*Abs(df[j*data->w+i].dp_cr)*(4-adjx)*(4-adjy) )
					/ (Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy
						+ Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy)+Abs(df[j*data->w+i].dp_cr)*(4-adjx)*(4-adjy));
		}
		// Y //----------------------- 右端
		ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y*adjx*adjy
						+ df[j*data->w+i-1].dp_y*adjx*(4-adjy)) / 16;
		if (ex[j*w+i].dp_y)
			ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
						+ df[j*data->w+i-1].y*Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy))
				/ (Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy+Abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy));
		// Cb
		ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb*adjx*adjy
						+ df[j*data->w+i-1].dp_cb*adjx*(4-adjy)) / 16;
		if (ex[j*w+i].dp_cb)
			ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
						+ df[j*data->w+i-1].cb*Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy))
				/ (Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy+Abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy));
		// Cr
		ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr*adjx*adjy
						+ df[j*data->w+i-1].dp_cr*adjx*(4-adjy)) / 16;
		if (ex[j*w+i].dp_cr)
			ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
						+ df[j*data->w+i-1].cr*Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy))
				/ (Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy+Abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy));
	}
	//--------------------------------------------------------- 一番下
	ex[j*w].dp_y  = df[(j-1)*data->w].dp_y *(4-adjx)*adjy /16; // 左端
	ex[j*w].dp_cb = df[(j-1)*data->w].dp_cb*(4-adjx)*adjy /16;
	ex[j*w].dp_cr = df[(j-1)*data->w].dp_cr*(4-adjx)*adjy /16;
	ex[j*w].y  = df[(j-1)*data->w].y;
	ex[j*w].cb = df[(j-1)*data->w].cb;
	ex[j*w].cr = df[(j-1)*data->w].cr;
	for (i = 1; i < w-1; ++i) { // 中
		// Y
		ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y * adjx * adjy
								+ df[(j-1)*data->w+i].dp_y * (4-adjx) *adjy) /16;
		if (ex[j*w+i].dp_y)
			ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
						+ df[(j-1)*data->w+i].y*Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy)
				/ (Abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy);
		// Cb
		ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb * adjx * adjy
								+ df[(j-1)*data->w+i].dp_cb * (4-adjx) *adjy) /16;
		if (ex[j*w+i].dp_cb)
			ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
						+ df[(j-1)*data->w+i].cb*Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy )
				/ (Abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy);
		// Cr
		ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr * adjx * adjy
								+ df[(j-1)*data->w+i].dp_cr * (4-adjx) *adjy) /16;
		if (ex[j*w+i].dp_cr)
			ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
						+ df[(j-1)*data->w+i].cr*Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy)
				/ (Abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy +Abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy);
	}
	ex[j*w+i].dp_y  = df[(j-1)*data->w+i-1].dp_y *adjx*adjy /16; // 右端
	ex[j*w+i].dp_cb = df[(j-1)*data->w+i-1].dp_cb*adjx*adjy /16;
	ex[j*w+i].dp_cr = df[(j-1)*data->w+i-1].dp_cr*adjx*adjy /16;
	ex[j*w+i].y  = df[(j-1)*data->w+i-1].y;
	ex[j*w+i].cb = df[(j-1)*data->w+i-1].cb;
	ex[j*w+i].cr = df[(j-1)*data->w+i-1].cr;

	return TRUE;
}
#pragma warning (pop)

/*--------------------------------------------------------------------
*	update_combobox()		コンボボックスの選択を更新
*		拡張データ領域のロゴ名にコンボボックスのカーソルを合わせる
*-------------------------------------------------------------------*/
static void update_cb_logo(char *name)
{
	// コンボボックス検索
	int num = SendMessage(dialog.cb_logo, CB_FINDSTRING, (WPARAM)-1, (WPARAM)name);

	if (num == CB_ERR) // みつからなかった
		num = -1;

	SendMessage(dialog.cb_logo, CB_SETCURSEL, num, 0); // カーソルセット
}

/*--------------------------------------------------------------------
*	load_logo_param()
*		指定したロゴデータを拡張データ領域にコピー
*-------------------------------------------------------------------*/
static void load_logo_param(FILTER* fp, int num) {
	if (logodata[num]->fi || logodata[num]->fo || logodata[num]->st || logodata[num]->ed) {
		fp->track[LOGO_STRT] = logodata[num]->st;
		fp->track[LOGO_FIN]  = logodata[num]->fi;
		fp->track[LOGO_FOUT] = logodata[num]->fo;
		fp->track[LOGO_END]  = logodata[num]->ed;
		fp->exfunc->filter_window_update(fp);
	}
}

/*--------------------------------------------------------------------
*	change_param()		パラメータの変更
*		選択されたロゴデータを拡張データ領域にコピー
*-------------------------------------------------------------------*/
static void change_param(FILTER* fp)
{
	LRESULT ret;

	// 選択番号取得
	ret = SendMessage(dialog.cb_logo, CB_GETCURSEL, 0, 0);
	ret = SendMessage(dialog.cb_logo, CB_GETITEMDATA, ret, 0);

	if (ret != CB_ERR)
		memcpy(ex_data, (void *)ret, LOGO_MAX_NAME); // ロゴ名をコピー

	// 開始･フェードイン･アウト･終了の初期値があるときはパラメタに反映
	ret = find_logo((char *)ret);
	if (ret < 0) return;

	load_logo_param(fp, ret);
}

/*--------------------------------------------------------------------
* 	set_combo_item()		コンボボックスにアイテムをセット
* 			dataはmallocで確保したポインタ
* 			個別にlogodata配列に書き込む必要がある
*-------------------------------------------------------------------*/
static int set_combo_item(void* data)
{
	// コンボボックスアイテム数
	int num = SendMessage(dialog.cb_logo, CB_GETCOUNT, 0, 0);

	// 最後尾に追加
	SendMessage(dialog.cb_logo, CB_INSERTSTRING, num, (LPARAM)data);
	SendMessage(dialog.cb_logo, CB_SETITEMDATA, num, (LPARAM)data);

	return num;
}

/*--------------------------------------------------------------------
*	del_combo_item()		コンボボックスからアイテムを削除
*-------------------------------------------------------------------*/
static void del_combo_item(int num)
{
	void *ptr = (void *)SendMessage(dialog.cb_logo, CB_GETITEMDATA, num, 0);
	if (ptr) free(ptr);

	// ロゴデータ配列再構成
	logodata_n = SendMessage(dialog.cb_logo, CB_GETCOUNT, 0, 0);
	logodata = (LOGO_HEADER **)realloc(logodata, logodata_n * sizeof(logodata));

	for (unsigned int i = 0; i < logodata_n; i++)
		logodata[i] = (LOGO_HEADER *)SendMessage(dialog.cb_logo, CB_GETITEMDATA, i, 0);

	SendMessage(dialog.cb_logo, CB_DELETESTRING, num, 0);
}

/*--------------------------------------------------------------------
*	get_logo_file_header_ver() LOGO_HEADERのバージョンを取得
*-------------------------------------------------------------------*/
int get_logo_file_header_ver(const LOGO_FILE_HEADER *logo_file_header) {
	int logo_header_ver = 0;
	if (0 == strcmp(logo_file_header->str, LOGO_FILE_HEADER_STR)) {
		logo_header_ver = 2;
	} else if (0 == strcmp(logo_file_header->str, LOGO_FILE_HEADER_STR_OLD)) {
		logo_header_ver = 1;
	}
	return logo_header_ver;
}

/*--------------------------------------------------------------------
*	convert_logo_header_v1_to_v2() LOGO_HEADERをv1からv2に変換
*-------------------------------------------------------------------*/
void convert_logo_header_v1_to_v2(LOGO_HEADER *logo_header) {
	LOGO_HEADER_OLD old_header;
	memcpy(&old_header,       logo_header,      sizeof(old_header));
	memset(logo_header,       0,                sizeof(logo_header[0]));
	memcpy(logo_header->name, &old_header.name, sizeof(old_header.name));
	memcpy(&logo_header->x,   &old_header.x,    sizeof(short) * 8);
}

/*--------------------------------------------------------------------
* 	read_logo_pack()		ロゴデータ読み込み
* 		ファイルからロゴデータを読み込み
* 		コンボボックスにセット
* 		拡張領域にコピー
*-------------------------------------------------------------------*/
static void read_logo_pack(char *fname, FILTER *fp)
{
	// ファイルオープン
	HANDLE hFile = CreateFile(fname,GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(fp->hwnd, "ロゴデータファイルが見つかりません", filter_name, MB_OK|MB_ICONERROR);
		return;
	}
	if (GetFileSize(hFile, NULL) < sizeof(LOGO_FILE_HEADER)) { // サイズ確認
		CloseHandle(hFile);
		MessageBox(fp->hwnd, "ロゴデータファイルが不正です", filter_name, MB_OK|MB_ICONERROR);
		return;
	}

//	SetFilePointer(hFile,31, 0, FILE_BEGIN);	// 先頭から31byteへ
	DWORD readed = 0;
	LOGO_FILE_HEADER logo_file_header = { 0 };
	ReadFile(hFile, &logo_file_header, sizeof(logo_file_header), &readed, NULL); // ファイルヘッダ取得

	int logo_header_ver = get_logo_file_header_ver(&logo_file_header);
	if (logo_header_ver == 0) {
		CloseHandle(hFile);
		MessageBox(fp->hwnd, "ロゴデータファイルが不正です", filter_name, MB_OK|MB_ICONERROR);
		return;
	}

	const size_t logo_header_size = (logo_header_ver == 2) ? sizeof(LOGO_HEADER) : sizeof(LOGO_HEADER_OLD);
	logodata_n = LOGO_AUTO_SELECT_USED; // 書き込みデータカウンタ
	logodata = nullptr;
#if LOGO_AUTO_SELECT
	if (logo_select.count) {
		logodata  = (LOGO_HEADER**)malloc(sizeof(LOGO_HEADER *) * 1);
		logodata[0] = &LOGO_HEADER_AUTO_SELECT;
	}
#endif
	int logonum = SWAP_ENDIAN(logo_file_header.logonum.l);

	for (int i = 0; i < logonum; i++) {

		// LOGO_HEADER 読み込み
		readed = 0;
		LOGO_HEADER logo_header = { 0 };
		ReadFile(hFile, &logo_header, logo_header_size, &readed, NULL);
		if (readed != logo_header_size) {
			MessageBox(fp->hwnd, "ロゴデータの読み込みに失敗しました", filter_name, MB_OK|MB_ICONERROR);
			break;
		}
		if (logo_header_ver == 1) {
			convert_logo_header_v1_to_v2(&logo_header);
		}

//  ldpには基本的に同名のロゴは存在しない
//
//		// 同名ロゴがあるか
//		int same = find_logodata(lgh.name);
//		if (same>0) {
//			wsprintf(message,"同名のロゴがあります\n置き換えますか？\n\n%s",lgh.name);
//			if (MessageBox(fp->hwnd, message, filter_name, MB_YESNO|MB_ICONQUESTION) == IDYES) {
//				// 削除
//				del_combo_item(same);
//			} else {	// 上書きしない
//				// ファイルポインタを進める
//				SetFilePointer(hFile, LOGO_PIXELSIZE(&lgh), 0, FILE_CURRENT);
//				continue;
//			}
//		}

		// メモリ確保
		BYTE *data = (BYTE *)calloc(logo_data_size(&logo_header), 1);
		if (data == NULL) {
			MessageBox(fp->hwnd, "メモリが足りません", filter_name, MB_OK|MB_ICONERROR);
			break;
		}

		// LOGO_HEADERコピー
		memcpy(data, &logo_header, sizeof(logo_header));

		LOGO_HEADER* ptr = (LOGO_HEADER *)(data + sizeof(LOGO_HEADER));

		// LOGO_PIXEL読み込み
		readed = 0;
		ReadFile(hFile, ptr, logo_pixel_size(&logo_header), &readed, NULL);

		if (logo_pixel_size(&logo_header) > (int)readed) { // 尻切れ対策
			readed -= readed % 2;
			ptr    += readed;
			memset(ptr, 0, logo_pixel_size(&logo_header) - readed);
		}

		// logodataポインタ配列に追加
		logodata_n++;
		logodata = (LOGO_HEADER **)realloc(logodata, logodata_n * sizeof(logodata));
		logodata[logodata_n-1] = (LOGO_HEADER *)data;
	}

	CloseHandle(hFile);

	if (logodata_n) {
		// 拡張データ設定
		memcpy(ex_data, logodata[0], LOGO_MAX_NAME);
	}

	if (logo_header_ver == 1) {
		//古いロゴデータなら自動的にバックアップ
		char backup_filename[1024];
		strcpy_s(backup_filename, fname);
		strcat_s(backup_filename, ".old_v1");
		CopyFile(fname, backup_filename, FALSE);
	}

	if (0 == _stricmp(".ldp", PathFindExtension(fname))) {
		//新しい形式だが、拡張子がldp2になっていなければ、変更する
		char new_filename[1024];
		strcpy_s(new_filename, fname);
		strcat_s(new_filename, "2");
		if (MoveFile(fname, new_filename) && 0 == _stricmp(logodata_file, fname)) {
			strcpy_s(logodata_file, new_filename);
		}
	}
}

/*--------------------------------------------------------------------
*	on_option_button()		オプションボタン動作
*-------------------------------------------------------------------*/
static BOOL on_option_button(FILTER* fp)
{
	// オプションダイアログが参照する
	optfp = fp;
	hcb_logo = dialog.cb_logo;

	EnableWindow(dialog.bt_opt, FALSE); // オプションボタン無効化

	// オプションダイアログ表示（モーダルフレーム）
	LRESULT res = DialogBox(fp->dll_hinst, "OPT_DLG", GetWindow(fp->hwnd, GW_OWNER), OptDlgProc);

	EnableWindow(dialog.bt_opt, TRUE); // 有効に戻す

	if (res == IDOK) { // OKボタン
		logodata_n = SendMessage(dialog.cb_logo, CB_GETCOUNT, 0, 0);

		// logodata配列再構成
		logodata = (LOGO_HEADER **)realloc(logodata, logodata_n * sizeof(logodata));
		for (unsigned int i = LOGO_AUTO_SELECT_USED; i < logodata_n; i++)
			logodata[i] = (LOGO_HEADER *)SendMessage(dialog.cb_logo, CB_GETITEMDATA, i, 0);

		if (logodata_n)	// 拡張データ初期値設定
			fp->ex_data_def = logodata[0];
		else
			fp->ex_data_def = NULL;
	}

	return TRUE;
}

/*--------------------------------------------------------------------
*	set_sended_data()		受信したロゴデータをセット
*-------------------------------------------------------------------*/
static void set_sended_data(void* data, FILTER* fp)
{
	LOGO_HEADER *logo_header = (LOGO_HEADER *)data;

	// 同名のロゴがあるかどうか
	int same = SendMessage(dialog.cb_logo, CB_FINDSTRING, (WPARAM)-1, (WPARAM)logo_header->name);
	if (same != CB_ERR) {
		char message[256] = { 0 };
		wsprintf(message,"同名のロゴがあります\n置き換えますか？\n\n%s", data);
		if (MessageBox(fp->hwnd, message, filter_name, MB_YESNO|MB_ICONQUESTION) != IDYES)
			return; // 上書きしない

		del_combo_item(same);
	}

	LOGO_HEADER *ptr = (LOGO_HEADER *)malloc(logo_data_size(logo_header));
	if (ptr == NULL) {
		MessageBox(fp->hwnd,"メモリが確保できませんでした", filter_name, MB_OK|MB_ICONERROR);
		return;
	}

	memcpy(ptr, data, logo_data_size(logo_header));

	logodata_n++;
	logodata = (LOGO_HEADER **)realloc(logodata, logodata_n * sizeof(logodata));
	logodata[logodata_n-1] = ptr;
	set_combo_item(ptr);

	memcpy(fp->ex_data_ptr, ptr, LOGO_MAX_NAME); // 拡張領域にロゴ名をコピー
}


/*--------------------------------------------------------------------
*	on_avisynth_button()	AviSynthボタン動作
*-------------------------------------------------------------------*/
static BOOL on_avisynth_button(FILTER* fp, void *editp)
{
	char str[STRDLG_MAXSTR] = { 0 };
	const char *logo_ptr = (char *)fp->ex_data_ptr;
#if LOGO_AUTO_SELECT
	if (0 == strcmp(logo_ptr, LOGO_AUTO_SELECT_STR)) {
		logo_ptr = (logo_select.num_selected == LOGO_AUTO_SELECT_NONE) ? "なし" : logodata[logo_select.num_selected]->name;
	}
#endif
	// スクリプト生成
	wsprintf(str,"%sLOGO(logofile=\"%s\",\r\n"
	             "\\           logoname=\"%s\"",
				(fp->check[0]? "Add":"Erase"), logodata_file, logo_ptr);

	if (fp->track[LOGO_X] || fp->track[LOGO_Y])
		wsprintf(str, "%s,\r\n\\           pos_x=%d, pos_y=%d",
					str, fp->track[LOGO_X], fp->track[LOGO_Y]);

	if (fp->track[LOGO_YDP]!=128 || fp->track[LOGO_PY] || fp->track[LOGO_CB] || fp->track[LOGO_CR])
		wsprintf(str, "%s,\r\n\\           depth=%d, yc_y=%d, yc_u=%d, yc_v=%d",
					str, fp->track[LOGO_YDP], fp->track[LOGO_PY], fp->track[LOGO_CB], fp->track[LOGO_CR]);


	if (fp->exfunc->get_frame_n(editp)) { // 画像が読み込まれているとき
		int s, e;
		fp->exfunc->get_select_frame(editp, &s, &e); // 選択範囲取得
		wsprintf(str, "%s,\r\n\\           start=%d", str, s + fp->track[LOGO_STRT]);

		if (fp->track[LOGO_FIN] || fp->track[LOGO_FOUT])
			wsprintf(str, "%s, fadein=%d, fadeout=%d", str, fp->track[LOGO_FIN], fp->track[LOGO_FOUT]);

		wsprintf(str, "%s, end=%d", str, e - fp->track[LOGO_END]);
	} else {
		if (fp->track[LOGO_FIN] || fp->track[LOGO_FOUT])
			wsprintf(str, "%s,\r\n\\           fadein=%d, fadeout=%d", str, fp->track[LOGO_FIN], fp->track[LOGO_FOUT]);
	}

	wsprintf(str,"%s)\r\n", str);

	
	EnableWindow(dialog.bt_synth, FALSE); // synthボタン無効化

	// ダイアログ呼び出し
	DialogBoxParam(fp->dll_hinst, "STR_DLG", GetWindow(fp->hwnd, GW_OWNER), StrDlgProc, (LPARAM)str);

	EnableWindow(dialog.bt_synth, TRUE); // synthボタン無効化解除

	return TRUE;
}



/*********************************************************************
*	DLLMain
*********************************************************************/
#pragma warning (push)
#pragma warning (disable: 4100)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
#define TRACK_N track_N
#define CHECK_N check_N
#define FILTER_NAME_MAX  32
#define FILTER_TRACK_MAX 16
#define FILTER_CHECK_MAX 32

	//FILTER filter = ::filter;
	static char *strings[1+TRACK_N+CHECK_N] = { 0 };
	char key[16];
	char ini_name[MAX_PATH];

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH: // 開始時
		// iniファイル名を取得
		GetModuleFileName(hinstDLL, ini_name, MAX_PATH-4);
		strcat_s(ini_name, ".ini");

		// フィルタ名
		strings[0] = (char *)calloc(FILTER_NAME_MAX, 1);
		if (strings[0] == NULL) break;
		GetPrivateProfileString("string", "name", filter.name, strings[0], FILTER_NAME_MAX, ini_name);
		filter.name = strings[0];

		// トラック名
		for (int i = 0; i < TRACK_N; i++) {
			strings[i+1] = (char *)calloc(FILTER_TRACK_MAX, 1);
			if (strings[i+1] == NULL) break;
			wsprintf(key, "track%d", i);
			GetPrivateProfileString("string", key, filter.track_name[i], strings[i+1], FILTER_TRACK_MAX, ini_name);
			filter.track_name[i] = strings[i+1];
		}
		// トラック デフォルト値
		for (int i = 0; i < TRACK_N; i++) {
			wsprintf(key, "track%d_def", i);
			filter.track_default[i] = GetPrivateProfileInt("int", key, filter.track_default[i], ini_name);
		}
		// トラック 最小値
		for (int i = 0; i < TRACK_N; i++) {
			wsprintf(key, "track%d_min", i);
			filter.track_s[i] = GetPrivateProfileInt("int", key, filter.track_s[i], ini_name);
		}
		// トラック 最大値
		for (int i = 0; i < TRACK_N; i++) {
			wsprintf(key, "track%d_max", i);
			filter.track_e[i] = GetPrivateProfileInt("int", key, filter.track_e[i], ini_name);
		}

		// チェック名
		for (int i = 0; i < CHECK_N; i++) {
			strings[i+TRACK_N+1] = (char *)calloc(FILTER_CHECK_MAX, 1);
			if (strings[i+TRACK_N+1] == NULL) break;
			wsprintf(key, "check%d", i);
			GetPrivateProfileString("string", key, filter.check_name[i], strings[i+TRACK_N+1], FILTER_CHECK_MAX, ini_name);
			filter.check_name[i] = strings[i+TRACK_N+1];
		}

#if LOGO_AUTO_SELECT
		//自動選択キー
		logo_select.count = 0;
		for (; ; logo_select.count++) {
			char buf[512] = { 0 };
			sprintf_s(key, "logo%d", logo_select.count+1);
			GetPrivateProfileString("LOGO_AUTO_SELECT", key, "", buf, sizeof(buf), ini_name);
			if (strlen(buf) == 0)
				break;
		}
		if (logo_select.count) {
			logo_select.keys = (LOGO_SELECT_KEY *)calloc(logo_select.count, sizeof(logo_select.keys[0]));
			for (int i = 0; i < logo_select.count; i++) {
				char buf[512] ={ 0 };
				sprintf_s(key, "logo%d", i+1);
				GetPrivateProfileString("LOGO_AUTO_SELECT", key, "", buf, sizeof(buf), ini_name);
				char *ptr = strchr(buf, ',');
				logo_select.keys[i].key = (char *)calloc(ptr - buf + 1, sizeof(logo_select.keys[i].key[0]));
				memcpy(logo_select.keys[i].key, buf, ptr - buf);
				strcpy_s(logo_select.keys[i].logoname, ptr+1);
			}
		}
#endif
		break;

	case DLL_PROCESS_DETACH: // 終了時
		// stringsを破棄
		for (int i = 0; i < 1+TRACK_N+CHECK_N && strings[i]; i++) {
			free(strings[i]);
			strings[i] = NULL;
		}
#if LOGO_AUTO_SELECT
		if (logo_select.keys) {
			for (int i = 0; i < logo_select.count; i++) {
				if (logo_select.keys[i].key)
					free(logo_select.keys[i].key);
			}
			free(logo_select.keys);
			memset(&logo_select, 0, sizeof(logo_select));
		}
#endif
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
#pragma warning (pop)
//*/
