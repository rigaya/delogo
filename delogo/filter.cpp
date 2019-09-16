/*********************************************************************
*   透過性ロゴ（BSマークとか）除去フィルタ
*                               ver 0.13
*
* 2003
*   02/01:  製作開始
*   02/02:  拡張データ領域を使うとプロファイル切り替えた時AviUtlがくたばる。
*           なぜ？バグ？どうしよう。
*           と思ったら、領域サイズに制限があるのね… SDKには一言も書いてないけど。
*           消えた－－!!（Ｂだけ）ちょっと感動。
*           BSロゴって輝度だけの変化なのか？RGBでやると色変わるし。
*   02/06:  プロファイルの途中切り替えに仮対応。
*   02/08:  BS2ロゴ実装。（テスト用ver.1）
*   02/11:  YCbCrを微調整できるようにした。
*   02/17:  試験的に外部ファイル読み込み機能を搭載。（５つまで）
*   03/02:  ロゴデータ保存領域関係（ポインタをコンボアイテムに関連付け）
*   03/24:  ロゴデータ読み込み関係
*   03/25:  フィルタ本体とロゴデータを切り離した。
*   03/26:  ロゴデータの管理を完全にコンボアイテムと関連付け
*   03/29:  ロゴデータ入出力ダイアログ（オプションダイアログ）作成
*   03/30:  ロゴ付加モード追加
*   04/03:  ロゴデータの構造を変更（dpをdp_y,dp_cb,dp_crに分離）
*           オプションダイアログにプレビュー機能を追加
*   04/06:  ロゴ解析アルゴリズムと、それを用いたフィルタのテスト版が
*           完成したため計算式をそれに最適化。（不要な分岐を減らした）
*   04/07:  ロゴプレビューの背景色を変更できるようにした。
*   04/09:  解析プラグインからのデータを受信出来るようにした。
*           深度調整の方法を変更(ofset->gain)
*           プレビューで枠からはみ出さないようにした。（はみ出しすぎると落ちる）
*   04/20:  フィルタ名変更。ロゴ付加モードを一時廃止。
*   04/28:  1/4単位位置調整実装。
*           ロゴ付加モード(あまり意味ないけど)復活
*           オプションダイアログ表示中にAviUtlを終了できないように変更
*           （エラーを出して落ちるバグ回避）
*
* [正式版(0.01)公開]
*
*   05/04:  不透明度調整の方法を変更。
*   05/08:  メモリ関連ルーチン大幅変更       (0.02)
*           VFAPI動作に対応、プロファイルの途中切り替えに対応
*           ロゴデータのサイズ制限を約４倍にした。
*   05/10:  データが受信できなくなっていたバグを修正    (0.03)
*   05/17:  ロゴ名を編集できるようにした。(0.04)
*   06/12:  プレビューの背景色をRGBで指定できるように変更。
*           位置調整が４の倍数のときcreate_adj_exdata()を呼ばないようにした。（0.05）
*   06/30:  フェードイン・アウトに対応。 (0.06)
*   07/02:  ロゴデータを受信できない場合があったのを修正。
*   07/03:  YCbCrの範囲チェックをするようにした。(しないと落ちることがある)
*           ロゴ名編集で同名にせっていできないようにした。(0.06a)
*   08/01:  フェードの不透明度計算式を見直し
*   08/02:  実数演算を止め、無駄な演算を削除して高速化。
*           上に伴い深度のデフォルト値を変更。
*           細かな修正
*   09/05:  細かな修正
*   09/27:  filter.hをAviUtl0.99SDKのものに差し替え。(0.07)
*   10/20:  SSE2使用のrgb2ycがバグもちなので、自前でRGB->YCbCrするようにした。
*           位置X/Yの最大･最小値を拡張した。(0.07a)
*   10/25:  位置調整で-200以下にすると落ちるバグ修正。(0.07b)
* 2004
*   02/18:  AviSynthスクリプトを吐くボタン追加。(0.08)
*   04/17:  ロゴデータファイル読み込み時にデータが一つも無い時エラーを出さないようにした。
*           開始･終了の最大値を4096まで増やした。(0.08a)
*   09/19:  スタックを無駄遣いしていたのを修正。
*           開始・フェードイン・アウト・終了の初期値をロゴデータに保存できるようにした。(0.09)
* 2005
*   04/18:  フィルタ名、パラメタ名を変更できるようにした。(0.09a)
* 2007
*   11/07:  プロファイルの境界をフェードの基点にできるようにした。(0.10)
* 2008
*   01/07:  ロゴのサイズ制限を撤廃
*           開始・終了パラメタの範囲変更(負の値も許可)
*           ロゴファイルのデータ数を拡張(1byte -> 4byte) (0.11)
*   06/21:  編集ダイアログで位置(X,Y)も編集できるようにした。(0.12)
*   07/03:  スピンコントロールで桁区切りのカンマが付かないようにした
*           ロゴ編集後、リストのロゴを選択状態にしなおすように
*           RGBtoYCの計算式を整数演算に (0.12a)
*   09/29:  スライダの最大最小値を変更できるようにした。(0.13)
*
*********************************************************************/

/* ToDo:
*   ・ロゴデータの作成・編集機能
*   ・複数導入した時、ロゴリストを共有するように
*   ・フィルタの名称が変わっていてもロゴ解析から送信できるように
*
*  MEMO:
*   ・ロゴの拡大縮小ルーチン自装しないとだめかなぁ。
*       →必要なさげ。当面は自装しない。
*   ・ロゴ作成・編集は別アプリにしてしまおうか…
*       仕様公開してるし、誰か作ってくれないかなぁ（他力本願）
*   ・ロゴ除去モードとロゴ付加モードを切り替えられるようにしようかな
*       →付けてみた
*   ・解析プラグからデータを受け取るには…独自WndMsg登録してSendMessageで送ってもらう
*       →ちゃんと動いた。…登録しなくてもUSER定義でよかったかも
*   ・ロゴに１ピクセル未満のズレがある。1/4ピクセルくらいでの位置調整が必要そう。
*       →実装完了
*   ・ダイアログを表示したまま終了するとエラー吐く
*       →親ウィンドウをAviUtl本体にすることで終了できなくした
*   ・ロゴデータ構造少し変えようかな… 色差要素のビットを半分にするとか。
*
*  新メモリ管理について:(2003/05/08)
*   fp->ex_data_ptrにはロゴの名称のみを保存。（7FFDバイトしかプロファイルに保存されず、不具合が生じるため）
*   func_init()でロゴデータパックを読み込む。ldpファイル名は必ずフルパスであることが必要。
*   読み込んだロゴデータのポインタはlogodata配列に保存。配列のデータ数はlogodata_nに。
*   func_proc()ではex_data（ロゴ名称）と一致するデータをlogodata配列から検索。なかった場合は何もしない。
*   位置パラメータを使って位置調整データを作成。その後で除去・付加関数を呼ぶ。
*   WndProcでは、WM_FILTER_INITでコンボボックスアイテムをlogodata配列から作る。
*   コンボアイテムのITEMDATAには従来どおりロゴデータのポインタを保存する。
*   WM_FILTER_INITではコンボボックスアイテムからファイルに保存。（今までどおり）
*   オプション設定ダイアログでのロゴデータの読み込み・削除は今までどおり。
*   OKボタンが押されたときは、リストアイテムからlogodata配列を作り直す。コンボアイテムの更新は今までどおり。
*
*  複数導入でのロゴデータ共有の方法のアイディア (2009/01/24)
*   初期化時にfpを走査、func_proc() に適当なメッセージを送る。(ロゴフィルタかどうか&バージョンチェック)
*   データ共有は最初のフィルタがロゴデータを保持、他のフィルタは 最初のにfunc_proc()にメッセージをなげて取得。
*   ロゴリスト編集ボタンどうしよう。最初のフィルタを呼び出すのがよいか。
*   ロゴ解析も同じ方法でロゴフィルタを特定できる。
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

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <cmath>
#include <algorithm>
#include <memory>
#include "filter.h"
#include "logo.h"
#include "optdlg.h"
#include "resource.h"
#include "send_lgd.h"
#include "strdlg.h"
#include "logodef.h"
#include "delogo_proc.h"
#include "delogo_util.h"

using std::unique_ptr;

#define LOGO_AUTO_SELECT 1

#define ID_BUTTON_OPTION            (40001)
#define ID_COMBO_LOGO               (40002)
#define ID_BUTTON_SYNTH             (40003)
#define ID_LABEL_LOGO_AUTO_SELECT   (40004)

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

static char  logodata_file[MAX_PATH] = { 0 };   // ロゴデータファイル名(INIに保存)

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

#define FADE_DIV_COUNT      (8)
#define PRE_DIV_COUNT       (4)

static LOGO_HEADER** logodata   = nullptr;
static int  logodata_n = 0;

static unique_ptr<LOGO_HEADER, aligned_malloc_deleter> adjdata;
static int adjdata_size = 0;

static char ex_data[sizeof(LOGO_HEADER)] = { 0 };   // 拡張データ領域

static UINT  WM_SEND_LOGO_DATA = 0; // ロゴ受信メッセージ

static const DELOGO_FUNC_LIST *func_logo = nullptr; //ロゴ除去関数

typedef struct {
    int fade;
    int nNR;
} FADE_ARRAY_ELM;

typedef struct {
    unique_ptr<short, aligned_malloc_deleter> mask;             //評価用Mask(Original)
    unique_ptr<short, aligned_malloc_deleter> mask_adjusted;    //評価用Mask(Frame毎の調整後)
    unique_ptr<short, aligned_malloc_deleter> mask_nr;          //NR用Mask
    unique_ptr<short, aligned_malloc_deleter> mask_nr_adjusted; //NR用Mask(Frame毎の調整後)
    int mask_size = 0;        // maskの要素数
    int mask_logo_index = -1; // maskの対象とするロゴのインデックス

    unique_ptr<short, aligned_malloc_deleter> logo_y;           // ロゴの輝度成分
    unique_ptr<short, aligned_malloc_deleter> logo_y_delogo;    // logo_yについてロゴ除去したもの
    unique_ptr<short, aligned_malloc_deleter> logo_y_delogo_nr; // logo_y_delogoについてNR処理をしたもの
    unique_ptr<short, aligned_malloc_deleter> temp; // 自動Fade値計算用Buffer
    unique_ptr<short, aligned_malloc_deleter> buf_eval[PRE_DIV_COUNT+1]; // 自動Fade値計算用Buffer
    int NR_area_coef = -1; // NR領域係数.
    int mask_valid_pixels = 0; // LogoMaskの有効要素数.
    bool bSaving = false; // Save処理中かどうかのFlag

    FADE_ARRAY_ELM fade_array[7];           // 近傍のFrameのFade値
    int     fade_array_index = -1;          // g_logo.fade_arrayのCenterのFrameIndex.
    int     enable_auto_nr = -1;            // AutoNRが有効かどうか.
    int     y_depth = -1;               // Y深度の保存
} LOGO_CONTEXT;

static LOGO_CONTEXT g_logo;


//----------------------------
//  プロトタイプ宣言
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

BOOL func_proc_eraze_logo(FILTER* const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *const lgh, int fade);
BOOL func_proc_add_logo(FILTER *const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *const lgh, int fade);

BOOL func_logo_NR1(short *mask_nr_adjusted, const short *mask_adjusted, int nr_value, FILTER *const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *lgh);
unsigned int calc_auto_fade_coef2(
    short *fade_eval, short *logo_y_delogo_nr, short *logo_y_delogo,
    const short *logo_y, FILTER *const fp, const LOGO_HEADER *const lgh, const short *mask_nr, const short *mask,
    int fade, int nr_value, int nr_area, unsigned int auto_result);
int calc_auto_fade(int *auto_nr,
    FILTER *const fp, const LOGO_HEADER *const lgh, bool bEnableDebug);
void create_adjusted_logo_mask(short *mask_adjusted, const short *mask_nr, const short *mask, FILTER *const fp, const LOGO_HEADER *const lgh);
BOOL func_debug_adjusted_mask(FILTER *const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *lgh, const short *mask_adjusted, const short *mask);
BOOL func_debug_evaluate_value(FILTER *const fp, FILTER_PROC_INFO *const fpip, const int max_nr, const int fade);

//----------------------------
//  FILTER_DLL構造体
//----------------------------
enum {
    LOGO_TRACK_X = 0,
    LOGO_TRACK_Y,
    LOGO_TRACK_YDP,
    LOGO_TRACK_CBDP = LOGO_TRACK_YDP,
    LOGO_TRACK_CRDP = LOGO_TRACK_YDP,
    LOGO_TRACK_PY,
    LOGO_TRACK_CB,
    LOGO_TRACK_CR,
    LOGO_TRACK_START,
    LOGO_TRACK_FADE_IN,
    LOGO_TRACK_FADE_OUT,
    LOGO_TRACK_END,
    LOGO_TRACK_NR_AREA,
    LOGO_TRACK_NR_VALUE,

    LOGO_TRACK_COUNT,
};

enum {
    LOGO_CHECK_ADDMODE = 0,
    LOGO_CHECK_DELMODE,
    LOGO_CHECK_BASEPROFILE,
    LOGO_CHECK_AUTO_FADE,
    LOGO_CHECK_AUTO_NR,
    LOGO_CHECK_DEBUG,

    LOGO_CHECK_COUNT
};

#define track_N (LOGO_TRACK_COUNT)
#define check_N (LOGO_CHECK_COUNT)

char filter_name[] = LOGO_FILTER_NAME;
static char filter_info[] = LOGO_FILTER_NAME" ver 0.13+r13 by rigaya";

static TCHAR *track_name[track_N] = {   "位置 X", "位置 Y",
                                          "深度", "Y", "Cb", "Cr",
                                          "開始", "FadeIn", "FadeOut", "終了",
                                          "NR範囲", "NR強度" }; // トラックバーの名前
static int   track_default[track_N] = { 0, 0,
                                         128, 0, 0, 0,
                                         0, 0, 0, 0 }; // トラックバーの初期値
int          track_s[track_N] = { LOGO_XY_MIN, LOGO_XY_MIN,
                                   0, -100, -100, -100,
                                   LOGO_STED_MIN, 0, 0, LOGO_STED_MIN }; // トラックバーの下限値
int          track_e[track_N] = { LOGO_XY_MAX, LOGO_XY_MAX,
                                   256, 100, 100, 100,
                                   LOGO_STED_MAX, LOGO_FADE_MAX, LOGO_FADE_MAX, LOGO_STED_MAX }; // トラックバーの上限値

static TCHAR *check_name[check_N]   = { "ロゴ付加モード","ロゴ除去モード","ﾌﾟﾛﾌｧｲﾙ境界をﾌｪｰﾄﾞ基点にする",
                                    "自動Fade" , "自動NR", "デバッグ" }; // チェックボックス
static int    check_default[check_N] = { 0, 1, 0, 0, 0, 0 };  // デフォルト

static unsigned int result_list[LOGO_NR_MAX+1][512];
static unsigned int debug_ave_result;   // MaskのPixelあたりの平均Prewitt値.

// 設定ウィンドウの高さ
#define WND_Y (67+24*track_N+20*check_N)

static FILTER_DLL filter = {
    FILTER_FLAG_WINDOW_SIZE |   //  フィルタのフラグ
    FILTER_FLAG_EX_DATA |
    FILTER_FLAG_EX_INFORMATION,
    320,WND_Y,          // 設定ウインドウのサイズ
    filter_name,        // フィルタの名前
#ifdef track_N
    track_N,            // トラックバーの数
    track_name,         // トラックバーの名前郡
    track_default,      // トラックバーの初期値郡
    track_s,track_e,    // トラックバーの数値の下限上限
#else
    0,nullptr,nullptr,nullptr,nullptr,
#endif
#ifdef check_N
    check_N,        // チェックボックスの数
    check_name,     // チェックボックスの名前郡
    check_default,  // チェックボックスの初期値郡
#else
    0,nullptr,nullptr,
#endif
    func_proc,      // フィルタ処理関数
    func_init,      // 開始時に呼ばれる
    func_exit,      // 終了時に呼ばれる関数
    nullptr,           // 設定が変更されたときに呼ばれる関数
    func_WndProc,   // 設定ウィンドウプロシージャ
    nullptr,nullptr,      // システムで使用
    ex_data,        // 拡張データ領域
    sizeof(LOGO_HEADER),//57102, // 拡張データサイズ
    filter_info,    // フィルタ情報
    nullptr,           // セーブ開始直前に呼ばれる関数
    nullptr,           // セーブ終了時に呼ばれる関数
    nullptr,nullptr,nullptr, // システムで使用
    ex_data,        // 拡張領域初期値
};

#if LOGO_FADE_FAST_ANALYZE
//--------------------------------------------------------------------
// LogoMaskの作成
//--------------------------------------------------------------------
BOOL CreateLogoMask() {
    // Memoryの準備
    const LOGO_HEADER *logo4work = (LOGO_HEADER *)adjdata.get();
    const int logo_pitch = PITCH(logo4work->w);
    const int new_mask_size = logo4work->h * logo_pitch;    // Maskに格納する要素数
    if (new_mask_size > g_logo.mask_size) {
        // SIMDを使用するため、_aligned_mallocを使用する
        /*
            #1 評価用Mask(Original)
            #2 評価用Mask(Frame毎の調整後)
            #3 NR用Mask
            #4 NR用Mask(Frame毎の調整後)
        */
        g_logo.mask.reset(            (short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        g_logo.mask_adjusted.reset(   (short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        g_logo.mask_nr.reset(         (short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        g_logo.mask_nr_adjusted.reset((short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        /*
            #1 処理対象FrameのLogo位置のY成分を格納する(Y成分のみで評価するため)
            #2 評価用の処理結果格納Buffer
            #3 作業用Buffer
            #4 調整後のMask
            #5～9 評価結果格納用
        */
        g_logo.logo_y.reset(          (short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        g_logo.logo_y_delogo.reset(   (short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        g_logo.logo_y_delogo_nr.reset((short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        g_logo.temp.reset(            (short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
        for (int i = 0; i < _countof(g_logo.buf_eval); i++) {
            g_logo.buf_eval[i].reset((short *)_aligned_malloc(new_mask_size * sizeof(short), 32));
            if (!g_logo.buf_eval[i]) {
                g_logo.mask_size = 0;
                g_logo.mask_logo_index = -1;
                return FALSE;
            }
        }
        if (!g_logo.mask
            || !g_logo.mask_adjusted
            || !g_logo.mask_nr
            || !g_logo.mask_nr_adjusted
            || !g_logo.logo_y
            || !g_logo.logo_y_delogo
            || !g_logo.logo_y_delogo_nr
            || !g_logo.temp) {
            g_logo.mask_size = 0;
            g_logo.mask_logo_index = -1;
            return FALSE;
        }
        g_logo.mask_size = new_mask_size;

        memset(g_logo.mask.get(), 0, new_mask_size * sizeof(short));

        //---------------------------------------------------------------------------------
#ifdef __LOGO_AUTO_ERASE_SIMD__
        // Prewittフィルタの処理結果をlogo_maskに格納する
        memset(g_logo.mask.get(), 0, sizeof(short) * logo_pitch);
        func_logo->prewitt_filter(g_logo.mask.get(), logodata[g_logo.mask_logo_index]);
        memset(g_logo.mask.get() + logo_pitch * (logo4work->h - 1), 0, sizeof(short) * logo_pitch);

        // LogoMaskの有効要素数を数える.
        g_logo.mask_valid_pixels = func_logo->count_valid_pixels(g_logo.mask.get(), logo4work->w, logo_pitch, logo4work->h);
#else   /* __LOGO_AUTO_ERASE_SIMD__ */
        // Prewittフィルタの処理結果をlogo_maskに格納する
        for (int y = 1; y <  logo4work->h - 1; y++) {
            for (int x = 1; x < logo4work->w - 1; x++) {
                int y_sum_h, y_sum_v;
                y_sum_h = -GetLogoY(x - 1, y - 1);
                y_sum_h -= GetLogoY(x - 1, y);
                y_sum_h -= GetLogoY(x - 1, y + 1);
                y_sum_h += GetLogoY(x + 1, y - 1);
                y_sum_h += GetLogoY(x + 1, y);
                y_sum_h += GetLogoY(x + 1, y + 1);
                y_sum_v = -GetLogoY(x - 1, y - 1);
                y_sum_v -= GetLogoY(x    , y - 1);
                y_sum_v -= GetLogoY(x + 1, y - 1);
                y_sum_v += GetLogoY(x - 1, y + 1);
                y_sum_v += GetLogoY(x    , y + 1);
                y_sum_v += GetLogoY(x + 1, y + 1);
                g_logo.mask.get()[x + y * logo_pitch] = (short)(std::sqrt( (double)(y_sum_h * y_sum_h + y_sum_v * y_sum_v)));
            }
        }

        // LogoMaskの有効要素数を数える.
        g_logo.mask_valid_pixels = 0;
        for (int y = 1; y <  logo4work->h - 1; y++) {
            for (int x = 1; x < logo4work->w - 1; x++) {
                g_logo.mask_valid_pixels += (g_logo.mask.get()[x + y * logo_pitch] > 1024);
            }
        }
 #endif
    }
    return TRUE;
}

//--------------------------------------------------------------------
// NR処理maskの作成
//--------------------------------------------------------------------
void CreateNRMask(short *target, const short *src_logo_mask, const int mask_coef, const bool force) {
    if (force
        || (mask_coef > 0 && g_logo.NR_area_coef != mask_coef) ) {
        const LOGO_HEADER *logo4work = (LOGO_HEADER *)adjdata.get();
        const int logo_w = logo4work->w;
        const int logo_h = logo4work->h;
        const int logo_pitch = PITCH(logo_w);

        if (src_logo_mask == g_logo.mask.get()) // 処理対象がOriginalのLogoMaskの場合に限定する
            g_logo.NR_area_coef = mask_coef;        // NR処理maskを作成したNR領域係数を保存

        int pos = 0;
        for (int i = 0; i < logo_h; ++i) {
            for (int j = 0; j < logo_w; ++j) {
                const short * refer = src_logo_mask + pos - (mask_coef * logo_pitch) - mask_coef;   // 参照点から(-mask_coef, -mask_coef)の位置
                for (int y = -mask_coef; y <= mask_coef; y++) {
                    int referpos = 0;
                    target[pos] = 0;
                    for (int x = -mask_coef; x <= mask_coef; x++) {
                        if (x + j >= 0 && x + j < logo_w && y + i >= 0 && y + i < logo_h) {
                            if (refer[referpos] > 1024) {
                                target[pos] = 1025;
                                break;
                            }
                        }
                        if (target[pos] > 1024) break;
                        referpos ++;
                    }
                    refer += logo_pitch;
                }
                ++pos;
            }
        }
    }
}

//--------------------------------------------------------------------
// ChangeScene検出処理の準備
//--------------------------------------------------------------------
#if 0
void PrepareSearchChangeScene(FILTER *const fp) {
    CreateLogoMask();

    DELOGO_EXT_DATA * pExtData = (DELOGO_EXT_DATA *)fp->ex_data_ptr;
    DELOGO_EXTRA &extra = pExtData->extra;
    const int mask_coef = std::min(extra.nNRArea, LOGO_NR_AREA_MAX);
    CreateNRMask(g_logo.mask_nr.get(), g_logo.mask.get(), mask_coef, false); // NR処理maskの生成
}
#endif

//--------------------------------------------------------------------
// これまで取得したFade値の再整列
//--------------------------------------------------------------------
void realignFadeArray(FILTER *const fp, FILTER_PROC_INFO *const fpip, const int target_frame) {
    //-------------------------------------------------------
    /*
    (2015/03/15:+h31) 編集点付近でのFade値の処理
    前のFrameとのVideoIndexの差が1でない場合は編集点 or SourceChangeを含むので
    参照しないようにする.(Tableのfade値=nValをセットする)
    */
    auto func_SearchChangeScene = [&](int nCenterFrame, int nVal) {
        int nCurrentVideoIndex = -1;
        bool bFoundSC = false;
        FRAME_STATUS fs;
        if (fp->exfunc->get_frame_status(fpip->editp, nCenterFrame, &fs)) {
            nCurrentVideoIndex = fs.video;
        }
        int nPrevVideoIndex = nCurrentVideoIndex;
        for (int i = 4; i < 7; i++) {
            if (bFoundSC) {
                g_logo.fade_array[i].fade = nVal;
            } else if (fp->exfunc->get_frame_status(fpip->editp, nCenterFrame + i - 3, &fs)) {
                if (fs.video - nPrevVideoIndex != 1) {
                    g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                    bFoundSC = true;
                }
            } else {
                g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                bFoundSC = true;
            }
            nPrevVideoIndex = fs.video;
        }

        nPrevVideoIndex = nCurrentVideoIndex;
        bFoundSC = false;
        for (int i = 2; i >= 0; i--) {
            if (bFoundSC) {
                g_logo.fade_array[i].fade = nVal;
            } else if (fp->exfunc->get_frame_status(fpip->editp, nCenterFrame + i - 3, &fs)) {
                if (fs.video - nPrevVideoIndex != -1) {
                    g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                    bFoundSC = true;
                }
            } else {
                g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                bFoundSC = true;
            }
            nPrevVideoIndex = fs.video;
        }
    };

    if (g_logo.fade_array_index == -1) {
        memset(g_logo.fade_array, 0xff, sizeof(g_logo.fade_array)); // Arrayを初期化
    } else {
        // これまでに取得したFade値を再配置して再利用する.
        const int delta = target_frame - g_logo.fade_array_index;
        if (delta != 0) {
            if (std::abs(delta) <= 6) {
                func_SearchChangeScene(g_logo.fade_array_index, -1);    // (2015/03/15:+h31) Tableを再利用する前に参照できないTableのFade値=-1にする

                if (delta > 0) {
                    for (int i = 0; i < 7 - delta; i++)
                        g_logo.fade_array[i] = g_logo.fade_array[i + delta];
                    for (int i = 7 - delta; i < 7; i++)
                        g_logo.fade_array[i].fade = -1;
                } else {
                    for (int i = 6; i >= -delta; i--)
                        g_logo.fade_array[i] = g_logo.fade_array[i + delta];
                    for (int i = -delta - 1; i >= 0; i--)
                        g_logo.fade_array[i].fade = -1;
                }
            } else {
                memset(g_logo.fade_array, 0xff, sizeof(g_logo.fade_array)); // Arrayを初期化
            }
        }
    }
    g_logo.fade_array_index = target_frame;

    //-------------------------------------------------------
    // (2015/03/15:+h31) 編集点付近でのFade値の処理
    /*
    前のFrameとのVideoIndexの差が1でない場合は編集点orSourceChangeを含むので
    参照しないようにする.
    */
    func_SearchChangeScene(target_frame, -2);   // Table変換後に現在のFrameに合わせて再度参照できないTableを検出する.
}

//--------------------------------------------------------------------
// ロゴの輝度成分の抽出
//--------------------------------------------------------------------
void extractLogoY(short *buffer, const int buf_pitch,
    const PIXEL_YC *src, const int max_w, const int logo_x, const int logo_y,
    const int logo_w, const int logo_h) {
    src += logo_y * max_w + logo_x;
    for (int y = 0; y < logo_h; y++, src += max_w, buffer += buf_pitch) {
        for (int x = 0; x < logo_w; x++) {
            buffer[x] = src[x].y;
        }
    }
}

//--------------------------------------------------------------------
// 自動Fade値の計算
//--------------------------------------------------------------------
int CalcAutoFade(int *nr_value, int * pFadePreAdjusted /*= nullptr*/,
    FILTER *const fp, FILTER_PROC_INFO *const fpip,
    const int target_frame, PIXEL_YC *src_pos /*= nullptr*/, BOOL bEditing /*= FALSE*/) {
    int fade = 0;

    realignFadeArray(fp, fpip, target_frame);

    //-------------------------------------------------------
    const LOGO_HEADER *logo4work = (LOGO_HEADER *)adjdata.get();
    const int logo_pitch = PITCH(logo4work->w);

    //-------------------------------------------------------
    // 前後のFrameで取得されていないFade値の計算
    //fp->exfunc->set_ycp_filtering_cache_size(fp, buffer_width, buffer_valid_height, 3, nullptr);
    bool bNeedFillFadeArray = false;
    for (int i = 0; i < 7; i++) {
        if (g_logo.fade_array[i].fade == -1 && i != 3) { // Fade値が取得されていないFrameの場合
            const int referFrame = (i - 3) + target_frame;    // 取得対象のFrameIndex(0～)
            if (referFrame >= 0 && referFrame < fpip->frame_n) {
                const PIXEL_YC *refer_pos = (const PIXEL_YC *)fp->exfunc->get_ycp_filtering_cache_ex(fp, fpip->editp, referFrame, 0, 0);
                extractLogoY(g_logo.logo_y.get(), logo_pitch,
                    refer_pos, fpip->max_w, logo4work->x, logo4work->y, logo4work->w, logo4work->h);
                int refer_nr_value = fp->track[LOGO_TRACK_NR_VALUE];
                int refer_fade = calc_auto_fade(&refer_nr_value, fp, logo4work, false);
                g_logo.fade_array[i].fade = refer_fade;
                g_logo.fade_array[i].nNR = refer_nr_value;
            } else {
                bNeedFillFadeArray = true;  // FadeArrayが埋まっていないので調整が必要
            }
        } else if (g_logo.fade_array[i].fade == -2) {
            // (2015/03/15:+h31) FadeArrayが埋まっていないので調整が必要
            bNeedFillFadeArray = true;
        }
    }

    bool bNeedCalcThisFrame = (g_logo.fade_array[3].fade < 0) ? true : false;
    if (bEditing && fp->check[LOGO_CHECK_DEBUG]) {
        memset(result_list, 0, sizeof(result_list));
        bNeedCalcThisFrame = true;  // DebugModeの場合はDebug情報を取得するために必ず再取得する.
    }

    //-------------------------------------------------------
    // 当該FrameのFade値の計算
    if (bNeedCalcThisFrame) {
        if (src_pos == nullptr) {
            src_pos = (PIXEL_YC *)fp->exfunc->get_ycp_filtering_cache_ex(fp, fpip->editp, target_frame, 0, 0);
        }

        // 自動Fade値計算用にbufferの準備をする: logo_fade_bufferの前半分に処理対象FrameのLogo位置のY成分を格納する.
        extractLogoY(g_logo.logo_y.get(), logo_pitch,
            src_pos, fpip->max_w, logo4work->x, logo4work->y, logo4work->w, logo4work->h);
        fade = calc_auto_fade(nr_value, fp, logo4work, true);
        g_logo.fade_array[3].fade = fade;
        g_logo.fade_array[3].nNR = *nr_value;
    } else {
        fade = g_logo.fade_array[3].fade;
        *nr_value = g_logo.fade_array[3].nNR;
    }

    if (pFadePreAdjusted != nullptr) {
        *pFadePreAdjusted = fade;   // 調整前のFade値を保存
    }

    //---------------------------------------------------------
    // 未取得のFadeArrayの調整(先頭/末尾のFrameが含まれる場合)
    if (bNeedFillFadeArray) {
        for (int i = 2; i >= 0; i--) {
            if (g_logo.fade_array[i].fade < 0)
                g_logo.fade_array[i] = g_logo.fade_array[i + 1];
        }
        for (int i = 4; i < 7; i++) {
            if (g_logo.fade_array[i].fade < 0)
                g_logo.fade_array[i] = g_logo.fade_array[i - 1];
        }
    }

    //-------------------------------------------------------
    // 前後のFrameのFade値からFade値を調整する
    bool bNeedAdjust = true;
    int past_frame_fade = (g_logo.fade_array[0].fade + g_logo.fade_array[1].fade + g_logo.fade_array[2].fade) / 3;
    int future_frame_fade = (g_logo.fade_array[4].fade + g_logo.fade_array[5].fade + g_logo.fade_array[6].fade) / 3;
    int current_frame_fade = (g_logo.fade_array[2].fade + g_logo.fade_array[3].fade + g_logo.fade_array[4].fade) / 3;

    const int adjustCoef = 7;                 // 0～10の補正係数
    const int fade_shreshold = LOGO_FADE_MAX * 85 / 100;    // 調整を施す閾値.
    const int fade_min_limit = LOGO_FADE_MAX / 10;          //      V
    if (adjustCoef > 0) {   // (2015/10/19:+h38) 補正係数=0の場合は補正しない
        if (fade < fade_min_limit) {
            if (past_frame_fade < fade_min_limit || future_frame_fade < fade_min_limit || current_frame_fade < fade_min_limit) {
                fade = fade * (LOGO_FADE_AD_MAX - adjustCoef) / LOGO_FADE_AD_MAX;
                bNeedAdjust = false;
            }
        } else if (fade > fade_shreshold) {
            if (past_frame_fade > fade_shreshold || future_frame_fade > fade_shreshold || current_frame_fade > fade_shreshold) {
                // Fade値が前後のFamreで継続して最大値の85%以上で推移している場合の調整.
                fade += ((LOGO_FADE_MAX - fade) * adjustCoef / LOGO_FADE_AD_MAX);
                bNeedAdjust = false;
            }
        } else {
            // (2015/03/15:+h31) 前後の平均との誤差が少ない場合は調整せずにそのまま判定値を採用する
            double rate = (double)std::min(std::abs(fade - past_frame_fade), std::abs(fade - future_frame_fade)) / (double)fade;
            if (rate <= 0.03) { // 誤差が3%以下ならば調整しない
                bNeedAdjust = false;
            }
        }
    } else {
        bNeedAdjust = false;    // (2015/10/19:+h38) 補正係数=0の場合は補正しない
    }

    if (bNeedAdjust) { // 調整が必要な場合
        // 前後2Frameの合計5Frameの中で最大/最小のFade値を除外した平均値を求める.
        int max_fade = -1;
        int min_fade = 0x7fffffff;
        int total = 0;
        for (int i = 1; i < 6; i++) {
            max_fade = std::max(max_fade, g_logo.fade_array[i].fade);
            min_fade = std::min(min_fade, g_logo.fade_array[i].fade);
            total += g_logo.fade_array[i].fade;
        }
        total -= (max_fade + min_fade);
        int ave_fade = total / 3;

        if (fade < ave_fade) {
            // 方針としては、Fade値が調整Fade値よりも小さい場合はできるだけ調整Fade値に置き換えてより大きなFade値にする。
            if (ave_fade >= LOGO_FADE_MAX) {
                fade = LOGO_FADE_MAX;
            }  else if (ave_fade > fade_shreshold) {
                // 閾値以上のFade値が継続する場合はFade値を引き上げる
                fade += ((LOGO_FADE_MAX - fade) * adjustCoef / LOGO_FADE_AD_MAX);
            } else if (fade < (double)ave_fade * 0.98) {
                fade = ave_fade;
            }
        } else if (fade > ave_fade) {
            //方針としては、Fade値が調整Fade値よりも大きい場合はできるだけそのまま採用する。
            if (fade >= LOGO_FADE_MAX) {
                if (ave_fade < LOGO_FADE_MAX)
                    fade = LOGO_FADE_MAX;
                else if (fade > (double)ave_fade * 1.03)
                    fade = ave_fade;
            } else if (ave_fade > fade_shreshold) {
                // 閾値以上のFade値が継続する場合はFade値を引き上げる
                fade += ((LOGO_FADE_MAX - fade) * adjustCoef / LOGO_FADE_AD_MAX);
            } else if (fade < LOGO_FADE_MAX * 0.80  // LOGO_FADE_MAXの80%以上の場合はFade値をそのまま採用する.
                && fade >(double)ave_fade * 1.15) { // 平均よりも15%以上大きい場合
                fade = ave_fade;
            }
        }
    }

    return fade;
}

#endif  /* LOGO_FADE_FAST_ANALYZE */

/*********************************************************************
*   DLL Export
*********************************************************************/
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable( void ) {
    return &filter;
}

/*====================================================================
*   開始時に呼ばれる関数
*===================================================================*/
BOOL func_init( FILTER *fp ) {
    //使用する関数を取得
    func_logo = get_delogo_func();

    // INIからロゴデータファイル名を読み込む
    fp->exfunc->ini_load_str(fp, LDP_KEY, logodata_file, nullptr);

    if (lstrlen(logodata_file) == 0) { // ロゴデータファイル名がなかったとき
        // 読み込みダイアログ
        if (!fp->exfunc->dlg_get_load_name(logodata_file, LDP_FILTER, LDP_DEFAULT)) {
            // キャンセルされた
            MessageBox(fp->hwnd, "ロゴデータファイルがありません", filter_name, MB_OK|MB_ICONWARNING);
            return FALSE;
        }
    } else { // ロゴデータファイル名があるとき
        // 存在を調べる
        HANDLE hFile = CreateFile(logodata_file, 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

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
*   終了時に呼ばれる関数
*===================================================================*/
#pragma warning (push)
#pragma warning (disable: 4100) //'fp' : 引数は関数の本体部で 1 度も参照されません。
BOOL func_exit( FILTER *fp ) {
    // ロゴデータ開放
    if (logodata) {
        for (int i = LOGO_AUTO_SELECT_USED; i < logodata_n; i++) {
            if (logodata[i]) {
                free(logodata[i]);
                logodata[i] = nullptr;
            }
        }
        free(logodata);
        logodata = nullptr;
    }

    adjdata.reset();
    adjdata_size = 0;

    func_logo = nullptr;

    g_logo.mask.reset();
    g_logo.mask_nr.reset();
    g_logo.mask_adjusted.reset();
    g_logo.mask_nr_adjusted.reset();
    g_logo.logo_y.reset();
    g_logo.logo_y_delogo.reset();
    g_logo.logo_y_delogo_nr.reset();
    g_logo.temp.reset();
    for (int i = 0; i < _countof(g_logo.buf_eval); i++) {
        g_logo.buf_eval[i].reset();
    }
    return TRUE;
}
#pragma warning (pop)
/*====================================================================
*   フィルタ処理関数
*===================================================================*/
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip) {

    // (2015/03/04:2015/03/05:+h25) 現在の表示Frameを取得
    //後段の処理で複数のFrameを参照する場合、表示Frame以外のFrameの処理も要求されるので、
    //その場合にはGUIの更新は行わないようにする
    const int nCurrentDispFrame = fp->exfunc->get_frame(fpip->editp);

    // ロゴ検索
    int logo_idx = find_logo((const char *)fp->ex_data_ptr);
    if (logo_idx < 0) return FALSE;

#if LOGO_AUTO_SELECT
    //ロゴ自動選択
    if (logo_select.count) {
        if (logo_idx == 0) {
            if (0 <= (logo_idx = logo_auto_select(fp, fpip))) {
                //"0"あるいは正の値なら変更あり (負なら以前から変更なし)
                logo_auto_select_apply(fp, logo_idx);
            }
            logo_idx = std::abs(logo_idx);
            if (logo_idx == LOGO_AUTO_SELECT_NONE) return TRUE; //選択されたロゴがなければ終了
        } else {
            logo_auto_select_remove(fp);
        }
    }
#endif

    {
        unsigned int new_adjdata_size = sizeof(LOGO_HEADER)
            + (logodata[logo_idx]->h + 1) * (logodata[logo_idx]->w + 1) * sizeof(LOGO_PIXEL);
        if (new_adjdata_size > adjdata_size) {
            adjdata.reset((LOGO_HEADER *)_aligned_malloc(new_adjdata_size, 16));
            adjdata_size = new_adjdata_size;
        }
        if (!adjdata) { //確保失敗
            adjdata_size = 0;
            return FALSE;
        }
    }

    int fade = LOGO_FADE_MAX;

    if (fp->track[LOGO_TRACK_X]%4 || fp->track[LOGO_TRACK_Y]%4) {
        // 位置調整が4の倍数でないとき、1/4ピクセル単位調整
        if (!create_adj_exdata(fp, adjdata.get(), logodata[logo_idx]))
            return FALSE;
    } else {
        // 4の倍数のときはx,yのみ書き換え
        memcpy(adjdata.get(), logodata[logo_idx], adjdata_size);
        adjdata->x += (short)(fp->track[LOGO_TRACK_X] / 4);
        adjdata->y += (short)(fp->track[LOGO_TRACK_Y] / 4);
    }
    const LOGO_HEADER *logo4work = (LOGO_HEADER *)adjdata.get();

    const bool bEditing = !g_logo.bSaving
            && fpip->frame == nCurrentDispFrame     // 表示Frameの処理を行う場合以外は編集中とはみなさない
            && fp->exfunc->is_editing(fpip->editp); // Save処理中を除外

    // Auto NRの設定が変化した場合はFadeArrayは再取得が必要
    if (g_logo.enable_auto_nr != fp->check[LOGO_CHECK_AUTO_NR]) {
        g_logo.fade_array_index = -1;
        g_logo.enable_auto_nr = fp->check[LOGO_CHECK_AUTO_NR];
    }

    // Logo深度の設定が変化した場合はFadeArrayは再取得が必要
    if (g_logo.y_depth != fp->track[LOGO_TRACK_YDP]) {
        g_logo.fade_array_index = -1;
        g_logo.y_depth = fp->track[LOGO_TRACK_YDP];
    }

    //-------------------------------------------------------
    // 処理の準備
    //-------------------------------------------------------
    // 自動除去Mode
    const int logo_pitch = PITCH(logo4work->w);
    bool bUpdateBuffer = false;
    if ((fp->check[LOGO_CHECK_AUTO_FADE] && fp->check[LOGO_CHECK_DELMODE])
        || fp->track[LOGO_TRACK_NR_VALUE] > 0
        || fp->check[LOGO_CHECK_AUTO_NR])  {
        //SIMDを使用するため、アライメントの確保できる幅を使用する
        if (g_logo.mask_logo_index != logo_idx) {
            g_logo.fade_array_index = -1;
#if defined(LOGO_FADE_FAST_ANALYZE) /* (2015/03/15:+h31) 関数化 */
            g_logo.mask_logo_index = logo_idx;
            BOOL brc = CreateLogoMask();    // LogoMaskの作成
            bUpdateBuffer = true;
#else   /* LOGO_FADE_FAST_ANALYZE */
            //---------------------------------------------------------------------------------
            // LogoMaskの作成
            //---------------------------------------------------------------------------------
            // Memoryの準備

            int new_mask_count = logo4work->h * logo_pitch; // Maskに格納する要素数
            if (new_mask_count > logo_mask_count) {
                //SIMDを使用するため、_aligned_mallocを使用する
                /*
                    #1 評価用Mask(Original)
                    #2 評価用Mask(Frame毎の調整後)
                    #3 NR用Mask
                */
                logo_mask = (short *)_aligned_realloc(logo_mask, new_mask_count * sizeof(short) * 3, 32);
                /*
                    #1 処理対象FrameのLogo位置のY成分を格納する(Y成分のみで評価するため)
                    #2 評価用の処理結果格納Buffer
                    #3 作業用Buffer
                    #4 調整後のMask
                    #5～9 評価結果格納用
                */
                logo_fade_buffer = (short *)_aligned_realloc(logo_fade_buffer, new_mask_count * sizeof(short) * (4 + (PRE_DIV_COUNT + 1)), 32);
                if (logo_mask != nullptr && logo_fade_buffer != nullptr) {
                    FillMemory(logo_mask, new_mask_count * sizeof(short), 0);
                    logo_mask_count = new_mask_count;
                } else {
                    if (logo_mask) {
                        _aligned_free(logo_mask);
                        logo_mask = nullptr;
                    }
                    if (logo_fade_buffer) {
                        _aligned_free(logo_fade_buffer);
                        logo_fade_buffer = nullptr;
                    }
                    logo_mask_count = 0;
                    logo_mask_index = -1;
                    return FALSE;   // 初期化失敗
                }
            }
            logo_mask_index = num;
            bUpdateBuffer = true;

            //---------------------------------------------------------------------------------
 #ifdef __LOGO_AUTO_ERASE_SIMD__
            memset(logo_mask, 0, sizeof(short) * logo_pitch);
            func_logo->prewitt_filter(logo_mask, logodata[logo_mask_index]);
            memset(logo_mask + logo_pitch * (logo4work->h - 1), 0, sizeof(short) * logo_pitch);
 #else
            // Prewittフィルタの処理結果をlogo_maskに格納する
            for (int y = 1; y <  logo4work->h - 1; y++) {
                for (int x = 1; x < logo4work->w - 1; x++) {
                    int y_sum_h, y_sum_v;
                    y_sum_h = -GetLogoY(x - 1, y - 1);
                    y_sum_h -= GetLogoY(x - 1, y);
                    y_sum_h -= GetLogoY(x - 1, y + 1);
                    y_sum_h += GetLogoY(x + 1, y - 1);
                    y_sum_h += GetLogoY(x + 1, y);
                    y_sum_h += GetLogoY(x + 1, y + 1);
                    y_sum_v = -GetLogoY(x - 1, y - 1);
                    y_sum_v -= GetLogoY(x    , y - 1);
                    y_sum_v -= GetLogoY(x + 1, y - 1);
                    y_sum_v += GetLogoY(x - 1, y + 1);
                    y_sum_v += GetLogoY(x    , y + 1);
                    y_sum_v += GetLogoY(x + 1, y + 1);
                    logo_mask[x + y * logo_pitch] = (short)(sqrt( (double)(y_sum_h * y_sum_h + y_sum_v * y_sum_v)));
                }
            }
 #endif
            // LogoMaskの有効要素数を数える.
 #ifdef __LOGO_AUTO_ERASE_SIMD__
            logo_mask_valid_pixels = func_logo->count_valid_pixels(logo_mask, logo4work->w, logo_pitch, logo4work->h);
 #else
            logo_mask_valid_pixels = 0;
            for (int y = 1; y <  logo4work->h - 1; y++) {
                for (int x = 1; x < logo4work->w - 1; x++) {
                    logo_mask_valid_pixels += (logo_mask[x + y * logo_pitch] > 1024);
                }
            }
 #endif
#endif  /* LOGO_FADE_FAST_ANALYZE */
        }
    }

    if (fp->track[LOGO_TRACK_NR_VALUE] > 0
        || fp->check[LOGO_CHECK_AUTO_FADE]
        || fp->check[LOGO_CHECK_AUTO_NR]) {
#if defined(LOGO_FADE_FAST_ANALYZE) /* (2015/03/15:+h31) 関数化 */
        CreateNRMask(g_logo.mask_nr.get(), g_logo.mask.get(), fp->track[LOGO_TRACK_NR_AREA], bUpdateBuffer); // NR処理maskの生成
#else   /* LOGO_FADE_FAST_ANALYZE */
        if (bUpdateBuffer
            || (mask_coef > 0 && NR_area_coef != mask_coef)
            )
        {
            NR_area_coef = mask_coef;   // NR処理maskを作成したNR領域係数を保存.
            //-------------------------------------
            // NR処理maskの生成
            //-------------------------------------
            short * target = logo_mask + (logo_pitch * logo4work->h) * 2;   // 作業用bufferはmaskの後半.
            int pos = 0;
            for (int i = 0; i < logo4work->h; ++i) {
                for (int j = 0; j < logo4work->w; ++j) {
                    short * refer = logo_mask + pos - (mask_coef * logo_pitch) - mask_coef; // 参照点から(-mask_coef, -mask_coef)の位置
                    for (int y = -mask_coef; y <= mask_coef; y++) {
                        int referpos = 0;
                        target[pos] = 0;
                        for (int x = -mask_coef; x <= mask_coef; x++) {
                            if (x + j >= 0 && x + j < logo4work->w && y + i >= 0 && y + i < logo4work->h) {
                                if (refer[referpos] > 1024) {
                                    target[pos] = 1025;
                                    break;
                                }
                            }
                            if (target[pos] > 1024) break;
                            referpos ++;
                        }
                        refer += logo_pitch;
                    }
                    ++pos;
                }
            }
        }
#endif  /* LOGO_FADE_FAST_ANALYZE */
    }

    char newCaption[256];
    //-------------------------------------------------------
    // Fade値の計算
    //-------------------------------------------------------
    int nr_value = fp->track[LOGO_TRACK_NR_VALUE];
    bool bNeedCalcThisFrame = false;    // 当該FrameのFade値の計算処理を行うかどうか.
    if (fp->check[LOGO_CHECK_AUTO_FADE]) {
#if defined(LOGO_FADE_FAST_ANALYZE) /* (2015/03/15:+h31) 関数化 */
        int fade_pre_adjusted = 0;  // 調整前のFade値を取得する
        fade = CalcAutoFade(&nr_value, &fade_pre_adjusted, fp, fpip, fpip->frame, fpip->ycp_edit, bEditing);
#else   /* LOGO_FADE_FAST_ANALYZE */
        //-------------------------------------------------------
        /*
            (2015/03/15:+h31) 編集点付近でのFade値の処理
            前のFrameとのVideoIndexの差が1でない場合は編集点 or SourceChangeを含むので
            参照しないようにする.(Tableのfade値=nValをセットする)
        */
        auto func_SearchChangeScene = [&](int nCenterFrame, int nVal) {
            int nCurrentVideoIndex = -1;
            BOOL bFoundSC = FALSE;
            FRAME_STATUS fs;
            if (m_fp->exfunc->get_frame_status(fpip->editp, nCenterFrame, &fs))
                nCurrentVideoIndex = fs.video;
            int nPrevVideoIndex = nCurrentVideoIndex;
            for (int i = 4; i < 7; i++) {
                if (bFoundSC)
                    g_logo.fade_array[i].fade = nVal;
                else if (m_fp->exfunc->get_frame_status(fpip->editp, nCenterFrame + i - 3, &fs)) {
                    if (fs.video - nPrevVideoIndex != 1) {
                        g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                        bFoundSC = TRUE;
                    }
                } else {
                    g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                    bFoundSC = TRUE;
                }
                nPrevVideoIndex = fs.video;
            }

            nPrevVideoIndex = nCurrentVideoIndex;
            bFoundSC = FALSE;
            for (int i = 2; i >= 0; i--) {
                if (bFoundSC)
                    g_logo.fade_array[i].fade = nVal;
                else if (m_fp->exfunc->get_frame_status(fpip->editp, nCenterFrame + i - 3, &fs)) {
                    if (fs.video - nPrevVideoIndex != -1) {
                        g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                        bFoundSC = TRUE;
                    }
                } else {
                    g_logo.fade_array[i].fade = nVal;   // 編集点を挟んで不連続な場合
                    bFoundSC = TRUE;
                }
                nPrevVideoIndex = fs.video;
            }
        };

        //-------------------------------------------------------
        if (g_logo.fade_array_index == -1)
            memset(g_logo.fade_array, 0xff, sizeof(g_logo.fade_array)); // Arrayを初期化
        else {
            // これまでに取得したFade値を再配置して再利用する.
            int delta = fpip->frame - g_logo.fade_array_index;
            if (delta != 0) {
                if (abs(delta) <= 6) {
                    func_SearchChangeScene(g_logo.fade_array_index, -1);    // (2015/03/15:+h31) Tableを再利用する前に参照できないTableのFade値=-1にする

                    if (delta > 0) {
                        for (int i = 0; i < 7 - delta; i++)
                            g_logo.fade_array[i] = g_logo.fade_array[i + delta];
                        for (int i = 7 - delta; i < 7; i++)
                            g_logo.fade_array[i].fade = -1;
                    } else {
                        for (int i = 6; i >= -delta; i--)
                            g_logo.fade_array[i] = g_logo.fade_array[i + delta];
                        for (int i = -delta - 1; i >= 0; i--)
                            g_logo.fade_array[i].fade = -1;
                    }
                } else
                    memset(g_logo.fade_array, 0xff, sizeof(g_logo.fade_array)); // Arrayを初期化
            }
        }
        g_logo.fade_array_index = fpip->frame;

        //-------------------------------------------------------
        // (2015/03/15:+h31) 編集点付近でのFade値の処理
        /*
            前のFrameとのVideoIndexの差が1でない場合は編集点orSourceChangeを含むので
            参照しないようにする.
        */
        func_SearchChangeScene(fpip->frame, -2);    // Table変換後に現在のFrameに合わせて再度参照できないTableを検出する.

        //-------------------------------------------------------
        // 前後のFrameで取得されていないFade値の計算
        bool bNeedFillFadeArray = false;
        for (int i = 0; i < 7; i++) {
            if (g_logo.fade_array[i].fade == -1 && i != 3) // Fade値が取得されていないFrameの場合
            {
                int referFrame = (i - 3) + fpip->frame; // 取得対象のFrameIndex(0～)
                if (referFrame >= 0 && referFrame < fpip->frame_n) {
                    fp->exfunc->set_ycp_filtering_cache_size(fp, fpip->max_w, fpip->h, 3, nullptr);
                    PIXEL_YC * refer_pos = (PIXEL_YC *)fp->exfunc->get_ycp_filtering_cache_ex(fp, fpip->editp, referFrame, 0, 0);
                    refer_pos += logo4work->x + logo4work->y * fpip->max_w;
                    int buffer_pos = 0;
                    for (int y = 0; y < logo4work->h; y++, refer_pos += fpip->max_w, buffer_pos += logo_pitch) {
                        for (int x = 0; x < logo4work->w; x++) {
                            logo_fade_buffer[buffer_pos + x] = refer_pos[x].y;
                        }
                    }
                    int refer_nr_value = extra.nNRValue;
                    int refer_fade = calc_auto_fade(fp,logo4work,logo_mask,logo_fade_buffer, refer_nr_value, false);
                    g_logo.fade_array[i].fade = refer_fade;
                    g_logo.fade_array[i].nNR = refer_nr_value;
                } else
                    bNeedFillFadeArray = true;  // FadeArrayが埋まっていないので調整が必要
            }
            // (2015/03/15:+h31) FadeArrayが埋まっていないので調整が必要
            else if (g_logo.fade_array[i].fade == -2)
                bNeedFillFadeArray = true;
        }

//      bNeedCalcThisFrame = (g_logo.fade_array[3].fade == -1)? true: false;
        bNeedCalcThisFrame = (g_logo.fade_array[3].fade < 0)? true: false;      // (2015/03/15:+h31)
        if (bEditing && extra.bDebug)
        {
            memset(result_list, 0, sizeof(result_list));
            bNeedCalcThisFrame = true;  // DebugModeの場合はDebug情報を取得するために必ず再取得する.
        }

        //-------------------------------------------------------
        // 当該FrameのFade値の計算
        if (bNeedCalcThisFrame) {
            // 自動Fade値計算用にbufferの準備をする: logo_fade_bufferの前半分に処理対象FrameのLogo位置のY成分を格納する.
            PIXEL_YC * src_pos = fpip->ycp_edit;
            src_pos += logo4work->x + logo4work->y * fpip->max_w;
            int buffer_pos = 0;
            for (int y = 0; y < logo4work->h; y++, src_pos += fpip->max_w, buffer_pos += logo_pitch) {
                for (int x = 0; x < logo4work->w; x++) {
                    logo_fade_buffer[buffer_pos + x] = src_pos[x].y;
                }
            }
            fade = calc_auto_fade(fp,logo4work,logo_mask,logo_fade_buffer, nr_value, true);
            g_logo.fade_array[3].fade = fade;
            g_logo.fade_array[3].nNR = nr_value;
        } else {
            fade = g_logo.fade_array[3].fade;
            nr_value = g_logo.fade_array[3].nNR;
        }

        int fade_pre_adjusted = fade;   // 調整前のFade値を保存.

        //---------------------------------------------------------
        // 未取得のFadeArrayの調整(先頭/末尾のFrameが含まれる場合)
        if (bNeedFillFadeArray) {
            for (int i = 2; i >= 0; i--) {
//              if (g_logo.fade_array[i].fade == -1)
                if (g_logo.fade_array[i].fade < 0)  // (2015/03/15:+h31)
                    g_logo.fade_array[i] = g_logo.fade_array[i + 1];
            }
            for (int i = 4; i < 7; i++) {
//              if (g_logo.fade_array[i].fade == -1)
                if (g_logo.fade_array[i].fade < 0)  // (2015/03/15:+h31)
                    g_logo.fade_array[i] = g_logo.fade_array[i - 1];
            }
        }

        //-------------------------------------------------------
        // 前後のFrameのFade値からFade値を調整する
        bool bNeedAdjust = true;
        int past_frame_fade = (g_logo.fade_array[0].fade + g_logo.fade_array[1].fade + g_logo.fade_array[2].fade) / 3;
        int future_frame_fade = (g_logo.fade_array[4].fade + g_logo.fade_array[5].fade + g_logo.fade_array[6].fade) / 3;
        int current_frame_fade = (g_logo.fade_array[2].fade + g_logo.fade_array[3].fade + g_logo.fade_array[4].fade) / 3;

        int adjustCoef = extra.nFadeCorrection;                 // 0～10の補正係数
        const int fade_shreshold = LOGO_FADE_MAX * 85 / 100;    // 調整を施す閾値.
        const int fade_min_limit = LOGO_FADE_MAX / 10;          //      V
        if (fade < fade_min_limit) {
            if (past_frame_fade < fade_min_limit || future_frame_fade < fade_min_limit || current_frame_fade < fade_min_limit) {
                fade = fade * (LOGO_FADE_AD_MAX - adjustCoef) / LOGO_FADE_AD_MAX;
                bNeedAdjust = false;
            }
        }
        else if (fade > fade_shreshold) {
            if (past_frame_fade > fade_shreshold || future_frame_fade > fade_shreshold || current_frame_fade > fade_shreshold) {
                // Fade値が前後のFamreで継続して最大値の85%以上で推移している場合の調整.
                fade += ((LOGO_FADE_MAX - fade) * adjustCoef / LOGO_FADE_AD_MAX);
                bNeedAdjust = false;
            }
        }
        // (2015/03/15:+h31) 前後の平均との誤差が少ない場合は調整せずにそのまま判定値を採用する
        else {
            double rate = (double)min(abs(fade - past_frame_fade), abs(fade - future_frame_fade)) / (double)fade;
            if (rate <= 0.03)   // 誤差が3%以下ならば調整しない
                bNeedAdjust = false;
        }


        if (bNeedAdjust) // 調整が必要な場合
        {
            // 前後2Frameの合計5Frameの中で最大/最小のFade値を除外した平均値を求める.
            int max_fade = -1;
            int min_fade = 0x7fffffff;
            int total = 0;
            for (int i = 1; i < 6; i++) {
                if (max_fade < g_logo.fade_array[i].fade)
                    max_fade = g_logo.fade_array[i].fade;
                if (min_fade > g_logo.fade_array[i].fade)
                    min_fade = g_logo.fade_array[i].fade;
                total += g_logo.fade_array[i].fade;
            }
            total -= (max_fade + min_fade);
            int ave_fade = total / 3;

            if (fade < ave_fade) {
                /*
                    方針としては、Fade値が調整Fade値よりも小さい場合はできるだけ調整Fade値に置き換えてより大きなFade値にする。
                */
                if (ave_fade >= LOGO_FADE_MAX)
                    fade = LOGO_FADE_MAX;
                /* 閾値以上のFade値が継続する場合はFade値を引き上げる */
                else if (ave_fade > fade_shreshold) {
                    fade += ((LOGO_FADE_MAX - fade) * adjustCoef / LOGO_FADE_AD_MAX);
                }
                else if (fade < (double)ave_fade * 0.98)
                    fade = ave_fade;
            } else if (fade > ave_fade) {
                /*
                    方針としては、Fade値が調整Fade値よりも大きい場合はできるだけそのまま採用する。
                */
                if (fade >= LOGO_FADE_MAX)
                {
                    if (ave_fade < LOGO_FADE_MAX)
                        fade = LOGO_FADE_MAX;
                    else if (fade > (double)ave_fade * 1.03)
                        fade = ave_fade;
                }
                /* 閾値以上のFade値が継続する場合はFade値を引き上げる */
                else if (ave_fade > fade_shreshold)
                    fade += ((LOGO_FADE_MAX - fade) * adjustCoef / LOGO_FADE_AD_MAX);
                else if (fade < LOGO_FADE_MAX * 0.80    // LOGO_FADE_MAXの80%以上の場合はFade値をそのまま採用する.
                    && fade > (double)ave_fade * 1.15)  // 平均よりも15%以上大きい場合
                    fade = ave_fade;
            }
        }
#endif  /* LOGO_FADE_FAST_ANALYZE */

        //-------------------------------------------------------
        // 設定Dialogに表示するCaption文字列の作成
        if (bEditing) {
            // Debug情報をセット
            if (fp->check[LOGO_CHECK_DEBUG]) {
                if (fade_pre_adjusted == fade)
                    sprintf_s(newCaption, 255, "Debug: Fade=%d,NR=%d,PW=%d @%d", fade, nr_value, debug_ave_result, fpip->frame + 1);
                else
                    sprintf_s(newCaption, 255, "Debug: Fade=%d(%d),NR=%d,PW=%d @%d", fade, fade_pre_adjusted, nr_value, debug_ave_result, fpip->frame + 1);
            } else
                sprintf_s(newCaption, 255, "透過性ロゴ[自動除去]: Fade=%d,NR=%d @%d", fade, nr_value, fpip->frame + 1);
        } else
            sprintf_s(newCaption, 255, "透過性ロゴ[自動除去]");
    } else {
        fade = calc_fade(fp, fpip);

#if defined(_h39_AUTONR_)   /* (2016/02/14:+h39) 自動NRを自動Fadeから独立させて単独で使用できるように変更 */
        if (fp->check[LOGO_CHECK_AUTO_NR] && fp->check[LOGO_CHECK_DELMODE]) {
            create_adjusted_logo_mask(g_logo.mask_adjusted.get(), g_logo.mask_nr.get(), g_logo.mask.get(), fp, logo4work);    // Frame毎に調整したMaskの作成
            unsigned int minRresult = 0xffffffff;
            nr_value = 0;
            for (int nh = 0; nh <= LOGO_NR_MAX; nh ++) {// NRのLimitに達しても処理を継続してFade値の決定には上限を超えたNRも指定する.
                unsigned int result = calc_auto_fade_coef2(nullptr,
                    g_logo.logo_y_delogo_nr.get(), g_logo.logo_y_delogo.get(), g_logo.logo_y.get(),
                    fp, logo4work, (nh == 0)? g_logo.mask_nr.get() : g_logo.mask.get(), g_logo.mask.get(),
                    fade, nh, fp->track[LOGO_TRACK_NR_AREA], 0xffffffff);
                if (result < minRresult) {
                    minRresult = result;
                    nr_value = nh;
                }
            }
        }
#endif  /* _h39_AUTONR_ */

        // 設定Dialogに表示するCaption文字列の作成
        if (bEditing) {
            if (fp->check[LOGO_CHECK_DELMODE]) {
                if (fp->check[LOGO_CHECK_AUTO_FADE] && fp->track[LOGO_TRACK_YDP] > 0)
                    sprintf_s(newCaption, "透過性ロゴ[除去]: 自動=OFF, Fade=%d/256", fade);
                else
                    sprintf_s(newCaption, "透過性ロゴ[除去]: Fade=%d/256", fade);
#if defined(_h39_AUTONR_)   /* (2016/02/14:+h39) 自動NRを自動Fadeから独立させて単独で使用できるように変更 */
                if (fp->check[LOGO_CHECK_AUTO_NR]) {
                    int len = strlen(newCaption);
                    sprintf_s(&(newCaption[len]), sizeof(newCaption)-1-len, ",NR=%d", nr_value);
                }
#endif  /* _h39_AUTONR_ */
            } else {
                sprintf_s(newCaption, "透過性ロゴ[付加]: Fade=%d/256", fade);
            }
        } else {
            if (fp->check[LOGO_CHECK_DELMODE])
                sprintf_s(newCaption, "透過性ロゴ[除去]");
            else
                sprintf_s(newCaption, "透過性ロゴ[付加]");
        }
    }

    //-------------------------------------------------------
    // Logoの除去/付加
    //-------------------------------------------------------
    BOOL rc = FALSE;
    if (fp->check[LOGO_CHECK_DELMODE]) {   // 除去モードチェック
        rc = func_logo->delogo(fp,fpip,logo4work,fade); // ロゴ除去モード

        /* Logo部 NR処理 */
        if (rc && nr_value > 0) {
            const int logo_pitch = PITCH(logo4work->w);
            /* NRも調整Maskを適用対象とする */
            if (!bNeedCalcThisFrame) {
                extractLogoY(g_logo.logo_y.get(), PITCH(logo4work->w), fpip->ycp_edit, fpip->max_w, logo4work->x, logo4work->y, logo4work->w, logo4work->h);
                create_adjusted_logo_mask(g_logo.mask_adjusted.get(), g_logo.mask_nr.get(), g_logo.mask.get(), fp, logo4work);
            }
            rc = func_logo_NR1(g_logo.mask_nr_adjusted.get(), g_logo.mask_adjusted.get(), nr_value, fp, fpip, logo4work);
        }
    } else {
        rc = func_proc_add_logo(fp, fpip, logo4work, fade);    // ロゴ付加モード
    }

    //-------------------------------------------------------
    // GUIの更新
    //-------------------------------------------------------
    // 設定DialogのTitle文字列の取得と更新
    if (fpip->frame == nCurrentDispFrame) {  // (2015/03/04:2015/03/05:+h25) 表示Frameの処理を行う場合以外はTitlebarは更新しない.
        char prevCaption[256];
        int nCount = GetWindowText(fp->hwnd, prevCaption, 255);
        if (nCount > 0 && strcmp(prevCaption, newCaption) != 0) {
            SetWindowText(fp->hwnd, newCaption);
        }
    }

    // Debug情報の表示
    if (bEditing && fp->check[LOGO_CHECK_AUTO_FADE] && fp->check[LOGO_CHECK_DEBUG]) {
        const int logo_pitch = PITCH(logo4work->w);
        func_debug_adjusted_mask(fp, fpip, logo4work, g_logo.mask_adjusted.get(), g_logo.mask.get());
        func_debug_evaluate_value(fp, fpip, (fp->check[LOGO_CHECK_AUTO_NR]) ? LOGO_NR_MAX+1 : 1, fade);
    }
    return rc;
}

#if LOGO_AUTO_SELECT
/*--------------------------------------------------------------------
*   logo_auto_select_apply()
*-------------------------------------------------------------------*/
static void logo_auto_select_apply(FILTER *fp, int num) {
    SetWindowPos(fp->hwnd, 0, 0, 0, 320, WND_Y + 20, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    SendMessage(dialog.lb_auto_select, WM_SETTEXT, 0, ((num == LOGO_AUTO_SELECT_NONE) ? (LPARAM)"なし" : (LPARAM)logodata[num]));
}

/*--------------------------------------------------------------------
*   logo_auto_select_remove()
*-------------------------------------------------------------------*/
static void logo_auto_select_remove(FILTER *fp) {
    char buf[LOGO_MAX_NAME] = { 0 };
    GetWindowText(dialog.lb_auto_select, buf, _countof(buf));
    if (strlen(buf)) {
        SendMessage(dialog.lb_auto_select, WM_SETTEXT, 0, (LPARAM)"");
        SetWindowPos(fp->hwnd, 0, 0, 0, 320, WND_Y, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        fp->track[LOGO_TRACK_START] = 0;
        fp->track[LOGO_TRACK_FADE_IN]  = 0;
        fp->track[LOGO_TRACK_FADE_OUT] = 0;
        fp->track[LOGO_TRACK_END]  = 0;
        fp->exfunc->filter_window_update(fp);
    }
}

/*--------------------------------------------------------------------
*   logo_auto_select() 自動選択のロゴ名を取得
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
*   func_proc_eraze_logo()  ロゴ除去モード
*-------------------------------------------------------------------*/
BOOL func_proc_eraze_logo(FILTER* const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *const lgh, int fade)
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
                dp = (lgp->dp_y * fp->track[LOGO_TRACK_YDP] * fade +64)/128 /LOGO_FADE_MAX;
                if (dp) {
                    if(dp==LOGO_MAX_DP) dp--; // 0での除算回避
                    int yc = lgp->y + fp->track[LOGO_TRACK_PY]*16;
                    yc = (ptr->y*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);  // 逆算
                    ptr->y = (short)Clamp(yc,-128,4096+128);
                }

                // 色差(青)
                dp = (lgp->dp_cb * fp->track[LOGO_TRACK_CBDP] * fade +64)/128 /LOGO_FADE_MAX;
                if (dp) {
                    if(dp==LOGO_MAX_DP) dp--; // 0での除算回避
                    int yc = lgp->cb + fp->track[LOGO_TRACK_CB]*16;
                    yc = (ptr->cb*LOGO_MAX_DP - yc*dp +(LOGO_MAX_DP-dp)/2) /(LOGO_MAX_DP - dp);
                    ptr->cb = (short)Clamp(yc,-2048-128,2048+128);
                }

                // 色差(赤)
                dp = (lgp->dp_cr * fp->track[LOGO_TRACK_CRDP] * fade +64)/128 /LOGO_FADE_MAX;
                if (dp) {
                    if(dp==LOGO_MAX_DP) dp--; // 0での除算回避
                    int yc = lgp->cr + fp->track[LOGO_TRACK_CR]*16;
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
*   func_proc_add_logo()    ロゴ付加モード
*-------------------------------------------------------------------*/
BOOL func_proc_add_logo(FILTER *const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *const lgh, int fade)
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
                dp = (lgp->dp_y * fp->track[LOGO_TRACK_YDP] *fade +64)/128 /LOGO_FADE_MAX;
                if (dp) {
                    int yc = lgp->y    + fp->track[LOGO_TRACK_PY]*16;
                    yc = (ptr->y*(LOGO_MAX_DP-dp) + yc*dp +(LOGO_MAX_DP/2)) /LOGO_MAX_DP; // ロゴ付加
                    ptr->y = (short)Clamp(yc,-128,4096+128);
                }


                // 色差(青)
                dp = (lgp->dp_cb * fp->track[LOGO_TRACK_CBDP] *fade +64)/128 /LOGO_FADE_MAX;
                if (dp) {
                    int yc = lgp->cb   + fp->track[LOGO_TRACK_CB]*16;
                    yc = (ptr->cb*(LOGO_MAX_DP-dp) + yc*dp +(LOGO_MAX_DP/2)) /LOGO_MAX_DP;
                    ptr->cb = (short)Clamp(yc,-2048-128,2048+128);
                }

                // 色差(赤)
                dp = (lgp->dp_cr * fp->track[LOGO_TRACK_CRDP] * fade +64)/128 /LOGO_FADE_MAX;
                if (dp) {
                    int yc = lgp->cr   + fp->track[LOGO_TRACK_CR]*16;
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
*   find_logo()     ロゴ名からロゴデータを検索
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
*   calc_fade()     フェード不透明度計算
*-------------------------------------------------------------------*/
static int calc_fade(FILTER *fp, FILTER_PROC_INFO *fpip)
{
    int s, e;

    if (fp->check[LOGO_CHECK_BASEPROFILE]) {
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
    if (fpip->frame < s+fp->track[LOGO_TRACK_START]+fp->track[LOGO_TRACK_FADE_IN]) {
        if (fpip->frame < s+fp->track[LOGO_TRACK_START])
            return 0; // フェードイン前
        else // フェードイン
            fade = ((fpip->frame-s-fp->track[LOGO_TRACK_START])*2 +1)*LOGO_FADE_MAX / (fp->track[LOGO_TRACK_FADE_IN]*2);
    } else if (fpip->frame > e-fp->track[LOGO_TRACK_FADE_OUT]-fp->track[LOGO_TRACK_END]) {
        if (fpip->frame > e-fp->track[LOGO_TRACK_END])
            return 0; // フェードアウト後
        else // フェードアウト
            fade = ((e-fpip->frame-fp->track[LOGO_TRACK_END])*2+1)*LOGO_FADE_MAX / (fp->track[LOGO_TRACK_FADE_OUT]*2);
    } else {
        fade = LOGO_FADE_MAX; // 通常
    }

    return fade;
}


/*--------------------------------------------------------------------
*   GetLogoY()  LogoのY成分の取得
*-------------------------------------------------------------------*/
static int GetLogoY(int x, int y)
{
    int Y;
    LOGO_PIXEL * pLogoPixel = (LOGO_PIXEL *)((char *)logodata[g_logo.mask_logo_index] + sizeof(LOGO_HEADER) + (x + logodata[g_logo.mask_logo_index]->w * y) * sizeof(LOGO_PIXEL));
    Y = (int)pLogoPixel->y * pLogoPixel->dp_y / LOGO_FADE_MAX;
    return Y;
}

/*--------------------------------------------------------------------
*   prewitt_filter_3x3()
*    srcに対し3x3のprewitt filterをかけdstに結果を格納する
*-------------------------------------------------------------------*/
static void prewitt_filter_3x3(short *dst, const short *src, int width, int pitch, int height) {
    memset(dst, 0, sizeof(short) * pitch);
    short *dst_line = dst + pitch;
    const short *src_line = src + pitch;

    for (int y = 1; y < height - 1; y++, dst_line += pitch, src_line += pitch) {
        dst_line[0] = 0;
        const short *src_ptr = src_line;
        src_ptr++;

        for (int x = 1; x < width - 1; x++, src_ptr++) {
            int y_sum_h = 0, y_sum_v = 0;
            y_sum_h -= src_ptr[-1 + pitch * -1];
            y_sum_h -= src_ptr[-1             ];
            y_sum_h -= src_ptr[-1 + pitch *  1];
            y_sum_h += src_ptr[ 1 + pitch * -1];
            y_sum_h += src_ptr[ 1             ];
            y_sum_h += src_ptr[ 1 + pitch *  1];
            y_sum_v -= src_ptr[-1 + pitch * -1];
            y_sum_v -= src_ptr[ 0 + pitch * -1];
            y_sum_v -= src_ptr[ 1 + pitch * -1];
            y_sum_v += src_ptr[-1 + pitch *  1];
            y_sum_v += src_ptr[ 0 + pitch *  1];
            y_sum_v += src_ptr[ 1 + pitch *  1];
            /* Prewitt値が輝度に依存する分の補正
                0～4096 => x1.0～5.78に補正される.
            */
            short nVal = (short)(sqrt((double)y_sum_h * (double)y_sum_h + (double)y_sum_v * (double)y_sum_v));
            short nCoef = std::min((short)4096 , *src_ptr);
            short nValAdjusted = nVal * 2200 / (2200- (nCoef * 4 / 9));
            dst_line[x] = nValAdjusted;
        }
        dst_line[width - 1] = 0;

    }
    memset(dst_line, 0, sizeof(short) * pitch);
}

/*--------------------------------------------------------------------
*   get_avg_prewitt_value_of_area()
*    位置(x,y)を中心とするarea_rangeの範囲で、
*    ロゴの輪郭以外のフレームのprewitt値の平均を求める
*-------------------------------------------------------------------*/
template<int area_range> static short get_avg_prewitt_value_of_area(const short *mask, const short *src_prewitt, int x, int y, int width, int pitch, int height) {
    const int j_start = std::max(-y, -area_range);
    const int j_fin   = std::min(area_range, height - y - 1);
    const int i_start = std::max(-x, -area_range);
    const int i_fin   = std::min(area_range, width - x - 1);
    int pixel_count   = 0;
    int area_sum      = 0;
    for (int j = j_start; j <= j_fin; j++) {
        for (int i = i_start; i <= i_fin; i++) {
            int offset = (x+i) + (y+j)*pitch;
            if (mask[offset] <= 1024) { //ロゴの輪郭の部分を除く
                area_sum += src_prewitt[offset];
                pixel_count++;
            }
        }
    }
    return (short)((pixel_count > 0) ? area_sum / pixel_count : -1);
}

/*--------------------------------------------------------------------
*   get_avg_prewitt_value_of_area()
*    ロゴの輪郭に相当する各ピクセルについて、area_rangeの範囲で、
*    ロゴの輪郭以外のフレームのprewitt値の平均を求める
*-------------------------------------------------------------------*/
template<int area_range> static void get_avg_prewitt_value_of_area(short *dst, const short *mask, const short *src_prewitt, int width, int pitch, int height) {
    short *dst_line = dst + pitch;
    const short *mask_line = mask + pitch;
    const int y_fin = height - 1;
    const int x_fin = width - 1;
    for (int y = 1; y < y_fin; y++, dst_line += pitch, mask_line += pitch) {
        dst_line[0] = MAXSHORT;
        for (int x = 1; x < x_fin; x++) {
            short result = MAXSHORT;
            if (mask_line[x] > 1024) {
                short avg = get_avg_prewitt_value_of_area<area_range>(mask, src_prewitt, x, y, width, pitch, height);
                if (avg >= 0) {
                    result = avg;
                }
            }
            dst_line[x] = result;
        }
        dst_line[x_fin] = MAXSHORT;
    }
}

/*--------------------------------------------------------------------
*   get_min_avg_prewitt_value_of_area()
*    ロゴの輪郭に相当する各ピクセルについて求めた
*    周辺prewitt値の平均値の最小値を返す
*-------------------------------------------------------------------*/
static short get_min_avg_prewitt_value_of_area(const short *avg_prewitt, int width, int pitch, int height) {
    const short *ptr_line = avg_prewitt + pitch;
    const int y_fin = height - 1;
    const int x_fin = width - 1;
    short min_value = MAXSHORT;
    for (int y = 1; y < y_fin; y++, ptr_line += pitch) {
        for (int x = 1; x < x_fin; x++) {
            min_value = std::min(min_value, ptr_line[x]);
        }
    }
    return (min_value == MAXSHORT) ? -1 : min_value;
}

/*--------------------------------------------------------------------
*   create_adjusted_mask()
*    背景の輪郭は評価対象としないように、新たな調整されたマスクを生成する
*-------------------------------------------------------------------*/
static void create_adjusted_mask(short *const dst, const short *const mask, const short *const src_prewitt, const short *src, int width, int pitch, int height) {
    //ロゴの輪郭に相当する各ピクセルについて、ロゴの輪郭以外のフレームのprewitt値の平均を求める
    get_avg_prewitt_value_of_area<3>(dst, mask, src_prewitt, width, pitch, height);

    //ロゴの輪郭に相当する各ピクセルのうち、周辺prewitt値の平均値の最小値を得る
    //この値が大雑把にそのフレームにおけるもっとも平らな部分の値とみなせる
    //ロゴの背景が複雑なら、自動的にこの値は大きくなる
    short avg_prewitt_of_area_min_pos = get_min_avg_prewitt_value_of_area(dst, width, pitch, height);

    //しきい値を適当に定める
    short prewitt_threshold = std::min(MAXSHORT, 100 + avg_prewitt_of_area_min_pos * 2);

    //しきい値によっては、輪郭部の評価点が極めて少なくなることがある
    //あまりに少ない場合には、しきい値を緩めて(大きくして)、再調整する
    for (int mask_edge_count = 0, mask_edge_prewitt_big_count = 0;
//      mask_edge_count * 0.5 <= mask_edge_prewitt_big_count;
        mask_edge_count * 0.5 <= mask_edge_prewitt_big_count && prewitt_threshold > 0;  // (2014/12/23)
        prewitt_threshold += prewitt_threshold / 20) {
        memset(dst, 0, pitch * sizeof(short));
        const short *src_line     = src         + pitch;
        short       *dst_line     = dst         + pitch;
        const short *prewitt_line = src_prewitt + pitch;
        const short *mask_line    = mask        + pitch;
        const int y_fin = height - 1;
        const int x_fin = width - 1;
        mask_edge_count = 0;
        mask_edge_prewitt_big_count = 0;
        for (int y = 1; y < y_fin; y++, dst_line += pitch, mask_line += pitch, prewitt_line += pitch, src_line += pitch) {
            dst_line[0] = 0;
            for (int x = 1; x < x_fin; x++) {
                //自分のピクセルの周辺 [-area_range ～ +area_range] の範囲で、
                //ロゴの輪郭部に相当せず、かつしきい値以上(-> 背景の輪郭)のピクセルがあれば、1を返す
                auto area_prewitt_is_high = [&](int area_range) {
                    const int j_start = std::max(-y, -area_range);
                    const int j_fin   = std::min(area_range, height - y - 1);
                    const int i_start = std::max(-x, -area_range);
                    const int i_fin   = std::min(area_range, width - x - 1);
                    for (int j = j_start; j <= j_fin; j++) {
                        for (int i = i_start; i <= i_fin; i++) {
                            int offset = x+i+j*pitch;
                            if (mask_line[offset] < 1024 && prewitt_line[offset] >= prewitt_threshold) {
                                return 1;
                            }
                        }
                    }
                    return 0;
                };
                int check = area_prewitt_is_high(3);
                //背景が輪郭であれば、マスクを反転させる(-> 評価対象外とする)
                dst_line[x] = (check) ? -mask_line[x] : mask_line[x];
                if (mask_line[x] > 1024) {
                    mask_edge_count ++; //ロゴの輪郭の総数
                    mask_edge_prewitt_big_count += check; //ロゴの輪郭で評価対象外の数
                }
            }
            dst_line[x_fin] = 0;
        }
        memset(dst_line, 0, pitch * sizeof(short));
    }
}

/*--------------------------------------------------------------------
*   calc_auto_fade4()   4分割による最適fade値の収束計算.
*-------------------------------------------------------------------*/
int calc_auto_fade4(unsigned int &result, FILTER *const fp, const LOGO_HEADER *lgh, const short * new_mask_buf, int nNR, int nr_area, int fadeMax) {
    int minFade = 0;
    int maxFade = fadeMax;
    unsigned int results[5];
    int n1st = 1;   // 初回の調整用.

    {
        unsigned int minResult = 0xffffffff;    // (2015/08/08:+h36)
        int minPos = -1;                        //      V
        while (maxFade - minFade > 4) {
//          unsigned int minResult = 0xffffffff;    // (2015/08/08:+h36) 上に移動
//          int minPos = -1;                        //      V
            double devide = (double)(maxFade - minFade) / 4;
            /*
                fade値は大きな方から調査する.
                初回は4～0, 2回目以降は3～1のBlock境界を調査する.
            */
            for (int i = 3 + n1st; i >= 1 - n1st; i--) {
                int fade = minFade + (int)(devide * (double)i);
                unsigned int result2 = calc_auto_fade_coef2(nullptr,
                    g_logo.logo_y_delogo_nr.get(), g_logo.logo_y_delogo.get(), g_logo.logo_y.get(),
                    fp, lgh, new_mask_buf, new_mask_buf, fade, nNR, nr_area, 0xffffffff);
                results[i] = result2;
                if (result2 < minResult) {
                    minResult = result2;
                    minPos = i;
                }
            }
            if (n1st) n1st = 0; // ２回目以降は3～1のみ調査する.
            if (minPos == 0) {
                // 次に0～2を調査する場合
                maxFade = minFade + (int)devide;
                results[4] = results[2];
            } else if (minPos == 4) {
                // 次に2～4を調査する場合
                minFade += (int)(devide * 2.0);
                results[0] = results[2];
            } else if (minPos == -2) {
                //最小値が更新されなかった場合は中央部を拡大して調査する
                maxFade -= (int)devide;
                minFade += (int)devide;
                results[0] = results[1];
                results[4] = results[3];
            } else {
                // 次に(minPos-1)～(minPos+1)を調査する場合
                maxFade = minFade + (int)(devide * (double)(minPos + 1));
                minFade += (int)(devide * (double)(minPos - 1));
                results[0] = results[minPos - 1];
                results[4] = results[minPos + 1];
                minPos = -2;    // (2015/08/08:+h36)
            }
        }
    }

    // 3要素以下になったら全ての要素を調査する
    unsigned int minResult = (results[4] <= results[0]) ? results[4]: results[0];
    int auto_fade = (results[4] <= results[0]) ? maxFade : minFade;
    for (int fade = maxFade - 1; fade >= minFade + 1; fade--) {
        unsigned int result2 = calc_auto_fade_coef2(nullptr,
            g_logo.logo_y_delogo_nr.get(), g_logo.logo_y_delogo.get(), g_logo.logo_y.get(),
            fp, lgh, new_mask_buf, new_mask_buf, fade, nNR, nr_area, minResult);
        if (result2 < minResult) {
            minResult = result2;
            auto_fade = fade;
        }
    }
    result = minResult;
    return auto_fade;
}

/*--------------------------------------------------------------------
*   calc_fade_max() 探索用Fade値の最大値の計算
*-------------------------------------------------------------------*/
int calc_fade_max(FILTER *fp) {
    /* 深度の15%増し相当の結果まで導くように改良 */
    int fadeMax = 0;
    const int depth = fp->track[LOGO_TRACK_YDP];
    if (depth > 0) {
        const int adjustedDepth = std::min(256, depth * 23 / 20);
        const int nStep = (adjustedDepth * LOGO_FADE_MAX / depth) / FADE_DIV_COUNT;
        fadeMax = nStep * FADE_DIV_COUNT;
    }
    return fadeMax;
}

/*--------------------------------------------------------------------
*   create_adjusted_logo_mask() 調整Maskの作成
*-------------------------------------------------------------------*/
void create_adjusted_logo_mask(short *mask_adjusted, const short *mask_nr, const short *mask, FILTER *const fp, const LOGO_HEADER *const lgh) {
    const int logo_pitch = PITCH(lgh->w);
    const int logo_buf_size = logo_pitch * lgh->h;

    const int fadeMax = calc_fade_max(fp);
    int nFadeStep = fadeMax / PRE_DIV_COUNT;
    int temp_fade = 0;
    long results[PRE_DIV_COUNT + 1];
    for (int i = 0; i <= PRE_DIV_COUNT; i++, temp_fade += nFadeStep) {
        results[i] = calc_auto_fade_coef2(g_logo.buf_eval[i].get(),
            g_logo.logo_y_delogo_nr.get(), g_logo.logo_y_delogo.get(), g_logo.logo_y.get(),
            fp, lgh, mask_nr, mask,
            temp_fade, 0, fp->track[LOGO_TRACK_NR_AREA], 0xffffffff); // NR=0で評価する.
    }

    //--------------------------------------------------------------------------
    int each_fade_count[PRE_DIV_COUNT + 1];
    memset(each_fade_count, 0, sizeof(each_fade_count));

    long whole_min_result = 0x7fffffff; // 全体の最小値.
    int org_mask_count = 0;             // 調整前のMaskの構成点数.
    int valid_mask_count = 0;
    short *min_fade_index_buf = g_logo.temp.get();
    {
        memset(min_fade_index_buf, 0, sizeof(min_fade_index_buf[0] * logo_pitch * 2)); //maskの周辺部は0で埋める
        int mask_pos = logo_pitch * 2;  // y = 2から開始する.
        for (int y = 2; y < lgh->h - 2; y++) {
            memset(&min_fade_index_buf[mask_pos], 0, sizeof(min_fade_index_buf[0]) * 2); //maskの周辺部は0で埋める
            mask_pos += 2;  // x = 2から開始する.
            for (int x = 2; x < lgh->w - 2; x++) {
                int val = -1;   // (2015/01/14:t14) 各構成点の評価値.
                if (mask[mask_pos] > 1024) { // Mask値が1024を超える領域(=Logoの輪郭部)のみを評価する.
                    // 評価値の最大/最小値を求める
                    int min_fade = -1;
                    long min_result = 0x7fffffff;
                    long max_result = -1;
                    for (int i = 0; i <= PRE_DIV_COUNT; i++) {
                        short * target = g_logo.buf_eval[i].get();    // 評価結果格納用
                        if (target[mask_pos] < min_result) {
                            min_result = target[mask_pos];
                            min_fade = i;
                        }
                        if (target[mask_pos] > max_result)
                            max_result = target[mask_pos];
                    }

                    // 各構成点の評価
                    if (max_result > 0 && (min_result * 100 / max_result) < 70) {
                        each_fade_count[min_fade] ++;
                        min_fade_index_buf[mask_pos] = min_fade;
                        if (whole_min_result > min_result)
                            whole_min_result = min_result;
                        valid_mask_count++;
                    } else
                        min_fade_index_buf[mask_pos] = -2;    // Maskから除外.

                    org_mask_count++;
                } else
                    min_fade_index_buf[mask_pos] = -1;    // Maskではない.
                mask_pos++;
            }
            memset(&min_fade_index_buf[mask_pos], 0, sizeof(min_fade_index_buf[0]) * (logo_pitch - lgh->w + 2)); //maskの周辺部は0で埋める
            mask_pos += 2;
            mask_pos += (logo_pitch - lgh->w);
        }
        memset(&min_fade_index_buf[mask_pos], 0, sizeof(min_fade_index_buf[0] * logo_pitch * 2)); //maskの周辺部は0で埋める
    }

    //--------------------------------------------------------------------------
    // 背景の精細度の計測.
    /*
        9つのFade値の中で最も評価値の低かったFadeでの平均評価値を計算する.
    */
    int min_fade_index = -1;
    int max_fade_count = -1;
    for (int i = 0; i <= PRE_DIV_COUNT; i++) {
        if (each_fade_count[i] > max_fade_count) {
            max_fade_count = each_fade_count[i];
            min_fade_index = i;
        }
    }
//      int aveResult = results[min_fade_index] / logo_mask_valid_pixels;
    int aveResult = (g_logo.mask_valid_pixels > 0)? results[min_fade_index] / g_logo.mask_valid_pixels: 0;  // (2015/03/03:+h24)

    //--------------------------------------------------------------------------
    long prewitt_threshold = whole_min_result * 2 + 100;        // 基準値.
    int mask_count = 0; // 調整後のMaskの構成点数.
    double rate = 0.5 + (double)std::min((aveResult - 400) / 200, 4) * 0.08;
    int target_count = (int)((double)valid_mask_count * (1.0 - rate));

    memcpy(mask_adjusted, min_fade_index_buf, logo_buf_size);
    while (mask_count < target_count) {
        int mask_pos = logo_pitch * 2;  // y = 2から開始する.
        for (int y = 2; y < lgh->h - 2; y++) {
            mask_pos += 2;  // x = 2から開始する.
            for (int x = 2; x < lgh->w - 2; x++) {
                int min_fade_index = min_fade_index_buf[mask_pos];
                if (min_fade_index >= 0 && min_fade_index <= 1024) {
                    short *target = g_logo.buf_eval[min_fade_index].get();    // 評価結果格納用
                    if (target[mask_pos] < prewitt_threshold) {
                        mask_count++;
                        mask_adjusted[mask_pos] = 1025;  // 調整後のMaskとして採用.
                    }
                }
                mask_pos++;
            }
            mask_pos += 2;
            mask_pos += (logo_pitch - lgh->w);
        }
        prewitt_threshold += (prewitt_threshold / 20);
    }
}

/*--------------------------------------------------------------------
*   calc_auto_fade()    fade値の自動計算
*-------------------------------------------------------------------*/
#define AVE_MODE_NONE       (-1)
#define AVE_MODE_3X3        (-2)
#define AVE_MODE_5X5        (-3)
#define AVE_MODE_7X7        (-4)
int calc_auto_fade(int *auto_nr, // 計算された自動NR値を返す.
    FILTER *const fp, const LOGO_HEADER *const lgh, bool bEnableDebug) {
    /* 深度の15%増し相当の結果まで導くように改良 */
    const int fadeMax = calc_fade_max(fp);
    int fade = fadeMax;
    unsigned int result;
    unsigned int auto_result = 0xffffffff; //unsigned int にして一回で大小比較できるようにする
    int auto_index = -1;
    int auto_fade = -1;

    const int logo_pitch = PITCH(lgh->w);

    // Frame毎に調整したMaskの作成
    create_adjusted_logo_mask(g_logo.mask_adjusted.get(),
        g_logo.mask_nr.get(), g_logo.mask.get(), fp, lgh);

    const double fLimitRate = 0.7;
    if (fp->check[LOGO_CHECK_AUTO_NR]) {
        /* fade値が最大となるNR値を採用する */
        unsigned int temp_result = 0xffffffff;
        unsigned int limitResult = 0;
        int nh_limit = LOGO_NR_MAX;
        for (int nh = 0; nh <= LOGO_NR_MAX; nh ++) { // NRのLimitに達しても処理を継続してFade値の決定には上限を超えたNRも指定する.
            int temp_fade = calc_auto_fade4(temp_result, fp, lgh, g_logo.mask_adjusted.get(), nh, fp->track[LOGO_TRACK_NR_AREA], fadeMax);
            if (temp_fade > auto_fade) {
                auto_fade = temp_fade;
                *auto_nr = nh;
            }
            if (nh == 0) {
                // 調整していないロゴMaskで評価する.
                unsigned int result = calc_auto_fade_coef2(nullptr,
                    g_logo.logo_y_delogo_nr.get(), g_logo.logo_y_delogo.get(), g_logo.logo_y.get(),
                    fp, lgh, g_logo.mask_nr.get(), g_logo.mask.get(), temp_fade, 0, 0, 0xffffffff);
                int aveResult = result / g_logo.mask_valid_pixels;
                nh_limit = (aveResult) ? std::min(LOGO_NR_REF / aveResult, LOGO_NR_MAX) : 0; // 最大NR強度を決定する.
                debug_ave_result = aveResult;   // MaskのPixelあたりの平均Prewitt値.
            }
        }

    } else {
        *auto_nr = fp->track[LOGO_TRACK_NR_VALUE];
        auto_fade = calc_auto_fade4(auto_result, fp, lgh, g_logo.mask_adjusted.get(), fp->track[LOGO_TRACK_NR_VALUE], fp->track[LOGO_TRACK_NR_AREA], fadeMax);
    }
    if (!g_logo.bSaving && bEnableDebug) {
        // 総当りで探索を行う(検算)
        unsigned int auto_result2 = 0xffffffff; //unsigned int にして一回で大小比較できるようにする
        int auto_fade2 = -1;
        int auto_nr2 = -1;
        if (fp->check[LOGO_CHECK_AUTO_NR]) {
            /* fade値が最大となるNR値を採用する */
            unsigned int limitResult = 0;
            bool bContinue = true;
            for (int nh = 0; nh <= LOGO_NR_MAX; nh ++) {
                unsigned int temp_result = 0xffffffff;
                int temp_fade = -1;
                for (int i_fade = 0; i_fade <= fadeMax; i_fade++) {
                    result = calc_auto_fade_coef2(nullptr,
                        g_logo.logo_y_delogo_nr.get(), g_logo.logo_y_delogo.get(), g_logo.logo_y.get(),
                        fp, lgh, g_logo.mask_nr.get(), g_logo.mask_adjusted.get(),
                        i_fade, nh, fp->track[LOGO_TRACK_NR_AREA], 0xffffffff);
                    result_list[nh][i_fade] = result;
                    if (result < limitResult)
                        bContinue = false;  // ここで中断してよいが、グラフ作成のために処理を続ける.
                    else if (result < temp_result) {
                        temp_result = result;
                        temp_fade = i_fade;
                    }
                }
                if (nh == 0)
                    limitResult = (unsigned int)((double)temp_result * fLimitRate); // NR=0の最小評価値の規定割合を下回る評価値は採用しない.
                if (bContinue
                    && temp_fade > auto_fade2) {
                    auto_fade2 = temp_fade;
                    auto_nr2 = nh;
                }
            }
        } else {
            for (int i_fade = 0; i_fade <= fadeMax; i_fade++) {
                result = calc_auto_fade_coef2(nullptr,
                    g_logo.logo_y_delogo_nr.get(), g_logo.logo_y_delogo.get(), g_logo.logo_y.get(),
                    fp, lgh, g_logo.mask_nr.get(), g_logo.mask_adjusted.get(),
                    i_fade, fp->track[LOGO_TRACK_NR_VALUE], fp->track[LOGO_TRACK_NR_AREA], 0xffffffff);
                result_list[fp->track[LOGO_TRACK_NR_VALUE]][i_fade] = result;
                if (result < auto_result2) {
                    auto_result2 = result;
                    auto_fade2 = i_fade;
                }
            }
        }
    }

    return auto_fade;
}

unsigned int calc_auto_fade_coef2(
    short *fade_eval, short *logo_y_delogo_nr, short *logo_y_delogo,
    const short *logo_y, FILTER *const fp, const LOGO_HEADER *const lgh, const short *mask_nr, const short *mask,
    int fade, int nr_value, int nr_area, unsigned int auto_result) {

    func_logo->delogo_y(logo_y_delogo, logo_y, fp, lgh, fade);

    //-------------------------------------
    // NR処理.
    //-------------------------------------
    short *logo_eval_src = logo_y_delogo;
    if (nr_value > 0) {
#if defined(__LOGO_AUTO_ERASE_SIMD__)
        func_logo->smooth[nr_value-1](logo_y_delogo_nr, logo_y_delogo, (nr_area > 0) ? mask_nr : mask, lgh->w, PITCH(lgh->w), lgh->h);
#else   /* __LOGO_AUTO_ERASE_SIMD__ */
        int n = nh * 2 + 1;

        // LOGO_PIXELデータへのポインタ
        LOGO_PIXEL *lgp = (LOGO_PIXEL *)(lgh + 1);

        // 左上の位置へ移動
        short * ptr = dst;
        int pos = 0;
        for (int i = 0; i < lgh->h; ++i) {
            for (int j = 0; j < lgh->w; ++j) {
                if (nr_mask[pos] > 1024)    // Mask値が1024を超える領域(=Logoの輪郭部)のみNRをかける.
                {
                    // 輝度を平均化する
                    int total = 0;
                    int count = 0;
                    short * refer = ptr - logo_pitch * nh - nh; // 評価点の(-nh, -nh)の位置
                    for (int y = 0; y < n; y++) {
                        for (int x = 0; x < n; x++) {
                            // 実際に参照する座標(xr, yr)
                            int xr = j + x - nh;
                            int yr = i + y - nh;
                            if (xr >= 0 && xr < lgh->w && yr >= 0 && yr < lgh->h) {
                                count++;    // 参照Point数.
                                total += refer[x];
                            }
                        }
                        refer += logo_pitch;
                    }
                    nr_dst[pos] = (short)Clamp(total / count, -128, 4096+128);
                } else
                    nr_dst[pos] = *ptr; // NR処理しない元の値.

                ++ptr; // 隣りへ
                ++lgp;
                ++pos;
            }
            // 次のラインへ
            ptr += (logo_pitch - lgh->w);
            pos += (logo_pitch - lgh->w);
        }
#endif  /* __LOGO_AUTO_ERASE_SIMD__ */
        logo_eval_src = logo_y_delogo_nr;
    }

#if defined(__LOGO_AUTO_ERASE_SIMD__) && defined(__LOGO_MASK_CREATE_SIMD__)
    //渡すmaskの端(上下左右2pixel + 右 logo_pitch - lgh->w)は0で埋める
    return func_logo->prewitt_evaluate[fade_eval != nullptr](fade_eval, mask, logo_eval_src, lgh, auto_result);
#else
    unsigned int result = 0;
    int mask_pos = logo_pitch * 2;  // y = 2から開始する.
    for (int y = 2; y < lgh->h - 2; y++) {
        mask_pos += 2;  // x = 2から開始する.
        for (int x = 2; x < lgh->w - 2; x++) {
            int val = -1;   // (2015/01/14:h14) 各構成点の評価値.
            if (mask[mask_pos] > 1024) { // Mask値が1024を超える領域(=Logoの輪郭部)のみを評価する.
                int y_sum_h = 0, y_sum_v = 0;
                // 5x5 Prewitt filter
                // +----------------+  +----------------+
                // | -1 -1 -1 -1 -1 |  | -1 -1  0  1  1 |
                // | -1 -1 -1 -1 -1 |  | -1 -1  0  1  1 |
                // |  0  0  0  0  0 |  | -1 -1  0  1  1 |
                // |  1  1  1  1  1 |  | -1 -1  0  1  1 |
                // |  1  1  1  1  1 |  | -1 -1  0  1  1 |
                // +----------------+  +----------------+
                y_sum_h -= src[x - 2 + (y - 2) * logo_pitch];
                y_sum_h -= src[x - 2 + (y - 1) * logo_pitch];
                y_sum_h -= src[x - 2 + (y    ) * logo_pitch];
                y_sum_h -= src[x - 2 + (y + 1) * logo_pitch];
                y_sum_h -= src[x - 2 + (y + 2) * logo_pitch];
                y_sum_h -= src[x - 1 + (y - 2) * logo_pitch];
                y_sum_h -= src[x - 1 + (y - 1) * logo_pitch];
                y_sum_h -= src[x - 1 + (y    ) * logo_pitch];
                y_sum_h -= src[x - 1 + (y + 1) * logo_pitch];
                y_sum_h -= src[x - 1 + (y + 2) * logo_pitch];
                y_sum_h += src[x + 1 + (y - 2) * logo_pitch];
                y_sum_h += src[x + 1 + (y - 1) * logo_pitch];
                y_sum_h += src[x + 1 + (y    ) * logo_pitch];
                y_sum_h += src[x + 1 + (y + 1) * logo_pitch];
                y_sum_h += src[x + 1 + (y + 2) * logo_pitch];
                y_sum_h += src[x + 2 + (y - 2) * logo_pitch];
                y_sum_h += src[x + 2 + (y - 1) * logo_pitch];
                y_sum_h += src[x + 2 + (y    ) * logo_pitch];
                y_sum_h += src[x + 2 + (y + 1) * logo_pitch];
                y_sum_h += src[x + 2 + (y + 2) * logo_pitch];
                y_sum_v -= src[x - 2 + (y - 1) * logo_pitch];
                y_sum_v -= src[x - 1 + (y - 1) * logo_pitch];
                y_sum_v -= src[x     + (y - 1) * logo_pitch];
                y_sum_v -= src[x + 1 + (y - 1) * logo_pitch];
                y_sum_v -= src[x + 2 + (y - 1) * logo_pitch];
                y_sum_v -= src[x - 2 + (y - 2) * logo_pitch];
                y_sum_v -= src[x - 1 + (y - 2) * logo_pitch];
                y_sum_v -= src[x     + (y - 2) * logo_pitch];
                y_sum_v -= src[x + 1 + (y - 2) * logo_pitch];
                y_sum_v -= src[x + 2 + (y - 2) * logo_pitch];
                y_sum_v += src[x - 2 + (y + 1) * logo_pitch];
                y_sum_v += src[x - 1 + (y + 1) * logo_pitch];
                y_sum_v += src[x     + (y + 1) * logo_pitch];
                y_sum_v += src[x + 1 + (y + 1) * logo_pitch];
                y_sum_v += src[x + 2 + (y + 1) * logo_pitch];
                y_sum_v += src[x - 2 + (y + 2) * logo_pitch];
                y_sum_v += src[x - 1 + (y + 2) * logo_pitch];
                y_sum_v += src[x     + (y + 2) * logo_pitch];
                y_sum_v += src[x + 1 + (y + 2) * logo_pitch];
                y_sum_v += src[x + 2 + (y + 2) * logo_pitch];

                y_sum_v >>= 2;
                y_sum_h >>= 2;

#if 1   /* (2015/01/15:h14) Alpha_max_plus_beta_min_algorithmにてSQRTを近似する */
                const int alpha = 123;  // 0.96043387 ≒ 123/128
                const int beta  = 51;   // 0.3978273 ≒ 51/128
                val = (alpha * max(abs(y_sum_h), abs(y_sum_v)) + beta * min(abs(y_sum_h), abs(y_sum_v))) / 128;
#else
                val = (int)sqrt(((double)y_sum_h * (double)y_sum_h + (double)y_sum_v * (double)y_sum_v));
#endif

                result += val;
            }
            // (2015/01/14:h14) 評価結果を格納する.
            if (target != nullptr)
                target[mask_pos] = (short)min(val, 0x7fffffff);

            mask_pos ++;
        }

        mask_pos += 2;  // x < lgh->w - 2の分の調整.
        mask_pos += (logo_pitch - lgh->w);
    }
    return result;
#endif //defined(__LOGO_AUTO_ERASE_SIMD__) && defined(__LOGO_MASK_CREATE_SIMD__)
}

BOOL func_logo_NR1(short *mask_nr_adjusted, const short *mask_adjusted, int nr_value, FILTER *const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *lgh) {
    //-------------------------------------
    // 処理maskの生成
    //-------------------------------------
    if (fp->track[LOGO_TRACK_NR_AREA] > 0) {
#if 1   /* (2016/02/20:+h40) */
        CreateNRMask(mask_nr_adjusted, mask_adjusted, fp->track[LOGO_TRACK_NR_AREA], true); // 各frame調整後のNR処理maskの生成
#endif  /* (2016/02/20:+h40) */
    }

    // LOGO_PIXELデータへのポインタ
    LOGO_PIXEL *lgp = (LOGO_PIXEL *)(lgh + 1);

    // 左上の位置へ移動
    PIXEL_YC *ptr = fpip->ycp_edit;
    ptr += lgh->x + lgh->y * fpip->max_w;
    int pos = 0;

    //-------------------------------------
    // ぼかし処理.
    //-------------------------------------
#if defined(__LOGO_AUTO_ERASE_SIMD__)
    func_logo->smooth_yc48[nr_value-1](ptr, mask_nr_adjusted, lgh->w, fpip->max_w, lgh->h);
#else   /* __LOGO_AUTO_ERASE_SIMD__ */
    for (int i = 0; i < lgh->h; ++i) {
        for (int j = 0; j < lgh->w; ++j) {
            if (ptr >= fpip->ycp_edit                           // 画面内の時のみ処理
                && ptr < fpip->ycp_edit + fpip->max_w*fpip->h   //  V
                && mask[pos] > 1024                             // Mask値が1024を超える領域(=Logoの輪郭部)のみNRをかける.
               ) {
                // 輝度を平均化する
                int total = 0;
                int count = 0;
                int nh = nr_value;
                int n = nh * 2 + 1;
                PIXEL_YC * refer = ptr - (fpip->max_w) * nh - nh;   // 評価点の(-nh, -nh)の位置
                for (int y = 0; y < n; y++) {
                    for (int x = 0; x < n; x++) {
                        // 実際に参照する座標(xr, yr)
                        int xr = j + lgh->x + x - nh;
                        int yr = i + lgh->y + y - nh;
                        if (xr >= 0 && xr < fpip->w && yr >= 0 && yr < fpip->h) {
                            count++;    // 参照Point数.
                            total += refer[x].y;
                        }
                    }
                    refer += fpip->max_w;
                }
                buffer[pos] = (short)Clamp(total / count, -128, 4096+128);
            } else
                buffer[pos] = 0x7fff;   // Invalidな値を入れておく.

            ++ptr; // 隣りへ
            ++lgp;
            ++pos;
        }
        // 次のラインへ
        ptr += fpip->max_w - lgh->w;
        pos += (logo_pitch - lgh->w);
    }

    //-------------------------------------
    // 計算結果の格納
    //-------------------------------------
    ptr = fpip->ycp_edit;
    ptr += lgh->x + lgh->y * fpip->max_w;
    pos = 0;
    for (int i = 0; i < lgh->h - 1; ++i) {
        for (int j = 0; j < lgh->w; ++j) {
            if (buffer[pos] != 0x7fff)  // Invalidな値以外の場合.
                ptr->y = buffer[pos];

            ++ptr; // 隣りへ
            ++lgp;
            ++pos;
        }
        // 次のラインへ
        ptr += (fpip->max_w - lgh->w);
        pos += (logo_pitch - lgh->w);
    }
#endif  /* __LOGO_AUTO_ERASE_SIMD__ */
    return TRUE;
}

//-------------------------------------
// 調整後のMaskの可視表示.
//-------------------------------------
BOOL func_debug_adjusted_mask(FILTER *const fp, FILTER_PROC_INFO *const fpip, const LOGO_HEADER *lgh, const short *mask_adjusted, const short *mask) {
    const int logo_pitch = PITCH(lgh->w);
    const short * adjustedMask = mask + (logo_pitch * lgh->h);
    PIXEL_YC * ptr = fpip->ycp_edit;
    ptr += lgh->x + lgh->y * fpip->max_w;
    int pos = 0;
    for (int i = 0; i < lgh->h - 1; ++i) {
        for (int j = 0; j < lgh->w; ++j) {
            if (mask[pos] > 1024 &&  mask_adjusted[pos] > 1024) {
                ptr->cr = 4096; // Mask部を赤く表示する.
            }

            ++ptr; // 隣りへ
            ++pos;
        }
        // 次のラインへ
        ptr += (fpip->max_w - lgh->w);
        pos += (logo_pitch - lgh->w);
    }

    return TRUE;
}
//-------------------------------------
// 評価値の可視表示.
//-------------------------------------
BOOL func_debug_evaluate_value(FILTER *const fp, FILTER_PROC_INFO *const fpip, const int max_nr, const int fade) {
    PIXEL_YC black = { 256, 0, 0 };
    PIXEL_YC grey =  { 2048, 0, 0 };
    PIXEL_YC nr_color[5] ={ { 1300, -320, 1650 }, { 2150, -1200, 1100 }, { 2400, -1350, -1000 }, { 2500, 550, -1750 }, { 1900, 1187, 126 } };
    PIXEL_YC *ptr = fpip->ycp_edit;
    const int pixel_offset_x = 100;
    const int pixel_offset_y = 100;
    const int max_w = fpip->max_w;
    const int pixel_range_x = std::min(512, fpip->max_w - pixel_offset_x - 100);
    const int pixel_range_y = std::min(600, fpip->max_h - pixel_offset_y - 100);
    const int pixel_max_x = pixel_offset_x + pixel_range_x;
    const int pixel_max_y = pixel_offset_y + pixel_range_y;
    //x軸
    for (int x = pixel_offset_x; x < pixel_max_x; x++)
        ptr[pixel_max_y * max_w + x] = black;

    //y軸
    for (int y = pixel_offset_y; y < pixel_max_y + 50; y++)
        ptr[y * max_w + pixel_offset_x] = black;

    //y軸 (256)
    for (int y = pixel_offset_y; y < pixel_max_y + 50; y++)
        ptr[y * max_w + pixel_offset_x + 256] = grey;

    //y軸 (fade値)
    PIXEL_YC red =  { 2048, 0, 2048 };
    for (int y = pixel_offset_y; y < pixel_max_y + 50; y++)
        ptr[y * max_w + pixel_offset_x + fade] = red;

    // 自動NR=OFFの場合への対応.
    int nr_sta = 0;
    int nr_end = max_nr;
    if (fp->check[LOGO_CHECK_AUTO_NR] > 0) {
        nr_sta = fp->track[LOGO_TRACK_NR_VALUE];
        nr_end = nr_sta + 1;
    }

    unsigned int value_max = 0;
    unsigned int value_min = MAXDWORD;
    for (int i_nr = nr_sta; i_nr < nr_end; i_nr++) { // 自動NR=OFFの場合への対応.
        for (int i = 0; i < 512; i++) {
            value_max = std::max(value_max, result_list[i_nr][i]);
            if (result_list[i_nr][i]) {
                value_min = std::min(value_min, result_list[i_nr][i]);
            }
        }
    }

    int value_range = value_max - value_min;
    value_max += value_range / 20;
    value_min -= value_range / 20;
    value_range = value_max - value_min;

    for (int i_nr = nr_end-1; i_nr >= nr_sta; i_nr--) { // 自動NR=OFFの場合への対応.
        const PIXEL_YC color = nr_color[i_nr];
        unsigned int color_min = MAXDWORD;
        int min_pos = 0;
        for (int i = 0; i < pixel_range_x; i++) {
            unsigned int value = result_list[i_nr][i];
            if (value) {
                int x = i + pixel_offset_x;
                int y = std::max(0, -(((int)value - (int)value_min) / (value_range / pixel_range_y)) + pixel_max_y);
                ptr[y * max_w + x] = color;
                if (color_min > value) {
                    color_min = value;
                    min_pos = i;
                }
            }
        }
        for (int y = pixel_max_y - 4; y <= pixel_max_y + 4; y++) {
            ptr[y * max_w + min_pos + pixel_offset_x] = color;
        }
    }

    return TRUE;
}

/*====================================================================
*   設定ウィンドウプロシージャ
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

                case ID_BUTTON_SYNTH:   // AviSynthボタン
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
*   on_wm_filter_init()     設定ウィンドウ初期化
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
*   on_wm_filter_exit()     終了処理
*       読み込まれているロゴデータをldpに保存
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
        HANDLE hFile = CreateFile(logodata_file, 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

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
    HANDLE hFile = CreateFile(logodata_file, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    SetFilePointer(hFile, 0, 0, FILE_BEGIN); // 先頭へ

    // ヘッダ書き込み
    LOGO_FILE_HEADER logo_file_header = { 0 };
    strcpy_s(logo_file_header.str, LOGO_FILE_HEADER_STR);
    DWORD data_written = 0;
    WriteFile(hFile, &logo_file_header, sizeof(LOGO_FILE_HEADER), &data_written, nullptr);
    if (data_written != 32) {   // 書き込み失敗
        MessageBox(fp->hwnd, "ロゴデータ保存に失敗しました(1)", filter_name, MB_OK|MB_ICONERROR);
    } else { // 成功
        int total_count = 0;
        // 各データ書き込み
        for (int i = LOGO_AUTO_SELECT_USED; i < num; i++) {
            data_written = 0;
            LOGO_HEADER *data = (LOGO_HEADER *)SendMessage(dialog.cb_logo, CB_GETITEMDATA, i, 0); // データのポインタ取得
            WriteFile(hFile, data, logo_data_size(data), &data_written, nullptr);
            if ((int)data_written != logo_data_size(data)) {
                MessageBox(fp->hwnd,"ロゴデータ保存に失敗しました(2)", filter_name, MB_OK|MB_ICONERROR);
                break;
            }
            total_count++;
        }

        logo_file_header.logonum.l = SWAP_ENDIAN(total_count);
        SetFilePointer(hFile,0, 0, FILE_BEGIN); // 先頭へ
        data_written = 0;
        WriteFile(hFile, &logo_file_header, sizeof(logo_file_header), &data_written, nullptr);
        if (data_written != sizeof(logo_file_header))
            MessageBox(fp->hwnd, "ロゴデータ保存に失敗しました(3)", filter_name, MB_OK|MB_ICONERROR);
    }

    CloseHandle(hFile);

    // INIにロゴデータファイル名保存
    fp->exfunc->ini_save_str(fp, LDP_KEY, logodata_file);
}

#if LOGO_AUTO_SELECT
/*--------------------------------------------------------------------
*   on_wm_filter_file_close() ファイルのクローズ
*       ロゴの自動選択になっていたら、画面を元に戻す
*-------------------------------------------------------------------*/
static void on_wm_filter_file_close(FILTER* fp) {
    logo_auto_select_remove(fp);
    logo_select.src_filename[0] = '\0';
    logo_select.num_selected = 0;
}
#endif

/*--------------------------------------------------------------------
*   init_dialog()       ダイアログアイテムを作る
*       ・コンボボックス
*       ・オプションボタン
*-------------------------------------------------------------------*/
static void init_dialog(HWND hwnd, HINSTANCE hinst)
{
#define ITEM_Y (19+24*track_N+20*check_N)

    // フォント作成
    dialog.font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    // コンボボックス
    dialog.cb_logo = CreateWindow("COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL,
                                    8,ITEM_Y, 300,100, hwnd, (HMENU)ID_COMBO_LOGO, hinst, nullptr);
    SendMessage(dialog.cb_logo, WM_SETFONT, (WPARAM)dialog.font, 0);

    // オプションボタン
    dialog.bt_opt = CreateWindow("BUTTON", "オプション", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_VCENTER,
                                    240,ITEM_Y-30, 66,24, hwnd, (HMENU)ID_BUTTON_OPTION, hinst, nullptr);
    SendMessage(dialog.bt_opt, WM_SETFONT, (WPARAM)dialog.font, 0);

    // AviSynthボタン
    dialog.bt_synth = CreateWindow("BUTTON", "AviSynth", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_VCENTER,
                                    240,ITEM_Y-60, 66,24, hwnd, (HMENU)ID_BUTTON_SYNTH, hinst, nullptr);
    SendMessage(dialog.bt_synth, WM_SETFONT, (WPARAM)dialog.font, 0);

#if LOGO_AUTO_SELECT
    //自動選択ロゴ表示
    dialog.lb_auto_select = CreateWindow("static", "", SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                                    20,ITEM_Y+23, 288,24, hwnd, (HMENU)ID_LABEL_LOGO_AUTO_SELECT, hinst, nullptr);
    SendMessage(dialog.lb_auto_select, WM_SETFONT, (WPARAM)dialog.font, 0);
#endif
}

/*--------------------------------------------------------------------
*   create_adj_exdata()     位置調整ロゴデータ作成
*-------------------------------------------------------------------*/
#pragma warning (push)
#pragma warning (disable: 4244) //C4244: '=' : 'int' から 'short' への変換です。データが失われる可能性があります。
static BOOL create_adj_exdata(FILTER *fp, LOGO_HEADER *adjdata, const LOGO_HEADER *data)
{
    int i, j;

    if (data == nullptr)
        return FALSE;

    // ロゴ名コピー
    memcpy(adjdata->name, data->name, LOGO_MAX_NAME);

    // 左上座標設定（位置調整後）
    adjdata->x = data->x + (int)(fp->track[LOGO_TRACK_X]-LOGO_XY_MIN)/4 + LOGO_XY_MIN/4;
    adjdata->y = data->y + (int)(fp->track[LOGO_TRACK_Y]-LOGO_XY_MIN)/4 + LOGO_XY_MIN/4;

    const int w = data->w + 1; // 1/4単位調整するため
    const int h = data->h + 1; // 幅、高さを１増やす
    adjdata->w = w;
    adjdata->h = h;

    // LOGO_PIXELの先頭
    LOGO_PIXEL *df = (LOGO_PIXEL *)(data +1);
    LOGO_PIXEL *ex = (LOGO_PIXEL *)(adjdata +1);

    const int adjx = (fp->track[LOGO_TRACK_X]-LOGO_XY_MIN) % 4; // 位置端数
    const int adjy = (fp->track[LOGO_TRACK_Y]-LOGO_XY_MIN) % 4;

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
            ex[i].y  = (df[i-1].y*std::abs(df[i-1].dp_y)*adjx*(4-adjy)
                    + df[i].y * std::abs(df[i].dp_y)*(4-adjx)*(4-adjy))
                /(std::abs(df[i-1].dp_y)*adjx*(4-adjy) + std::abs(df[i].dp_y)*(4-adjx)*(4-adjy));
        // Cb
        ex[i].dp_cb = (df[i-1].dp_cb*adjx*(4-adjy)
                            + df[i].dp_cb*(4-adjx)*(4-adjy)) /16;
        if (ex[i].dp_cb)
            ex[i].cb = (df[i-1].cb*std::abs(df[i-1].dp_cb)*adjx*(4-adjy)
                    + df[i].cb * std::abs(df[i].dp_cb)*(4-adjx)*(4-adjy))
                / (std::abs(df[i-1].dp_cb)*adjx*(4-adjy)+std::abs(df[i].dp_cb)*(4-adjx)*(4-adjy));
        // Cr
        ex[i].dp_cr = (df[i-1].dp_cr*adjx*(4-adjy)
                            + df[i].dp_cr*(4-adjx)*(4-adjy)) /16;
        if (ex[i].dp_cr)
            ex[i].cr = (df[i-1].cr*std::abs(df[i-1].dp_cr)*adjx*(4-adjy)
                    + df[i].cr * std::abs(df[i].dp_cr)*(4-adjx)*(4-adjy))
                / (std::abs(df[i-1].dp_cr)*adjx*(4-adjy)+std::abs(df[i].dp_cr)*(4-adjx)*(4-adjy));
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
            ex[j*w].y = (df[(j-1)*data->w].y*std::abs(df[(j-1)*data->w].dp_y)*(4-adjx)*adjy
                        + df[j*data->w].y*std::abs(df[j*data->w].dp_y)*(4-adjx)*(4-adjy))
                / (std::abs(df[(j-1)*data->w].dp_y)*(4-adjx)*adjy+std::abs(df[j*data->w].dp_y)*(4-adjx)*(4-adjy));
        // 色差(青)Cb
        ex[j*w].dp_cb = (df[(j-1)*data->w].dp_cb*(4-adjx)*adjy
                        + df[j*data->w].dp_cb*(4-adjx)*(4-adjy)) / 16;
        if (ex[j*w].dp_cb)
            ex[j*w].cb = (df[(j-1)*data->w].cb*std::abs(df[(j-1)*data->w].dp_cb)*(4-adjx)*adjy
                        + df[j*data->w].cb*std::abs(df[j*data->w].dp_cb)*(4-adjx)*(4-adjy))
                / (std::abs(df[(j-1)*data->w].dp_cb)*(4-adjx)*adjy+std::abs(df[j*data->w].dp_cb)*(4-adjx)*(4-adjy));
        // 色差(赤)Cr
        ex[j*w].dp_cr = (df[(j-1)*data->w].dp_cr*(4-adjx)*adjy
                        + df[j*data->w].dp_cr*(4-adjx)*(4-adjy)) / 16;
        if (ex[j*w].dp_cr)
            ex[j*w].cr = (df[(j-1)*data->w].cr*std::abs(df[(j-1)*data->w].dp_cr)*(4-adjx)*adjy
                        + df[j*data->w].cr*std::abs(df[j*data->w].dp_cr)*(4-adjx)*(4-adjy))
                / (std::abs(df[(j-1)*data->w].dp_cr)*(4-adjx)*adjy+std::abs(df[j*data->w].dp_cr)*(4-adjx)*(4-adjy));
        for (i = 1; i < w-1; ++i) { //------------------ 中
            // Y
            ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y*adjx*adjy
                            + df[(j-1)*data->w+i].dp_y*(4-adjx)*adjy
                            + df[j*data->w+i-1].dp_y*adjx*(4-adjy)
                            + df[j*data->w+i].dp_y*(4-adjx)*(4-adjy) ) /16;
            if (ex[j*w+i].dp_y)
                ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*std::abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
                            + df[(j-1)*data->w+i].y*std::abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy
                            + df[j*data->w+i-1].y*std::abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy)
                            + df[j*data->w+i].y*std::abs(df[j*data->w+i].dp_y)*(4-adjx)*(4-adjy) )
                    / (std::abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy + std::abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy
                        + std::abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy)+std::abs(df[j*data->w+i].dp_y)*(4-adjx)*(4-adjy));
            // Cb
            ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb*adjx*adjy
                            + df[(j-1)*data->w+i].dp_cb*(4-adjx)*adjy
                            + df[j*data->w+i-1].dp_cb*adjx*(4-adjy)
                            + df[j*data->w+i].dp_cb*(4-adjx)*(4-adjy) ) /16;
            if (ex[j*w+i].dp_cb)
                ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*std::abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
                            + df[(j-1)*data->w+i].cb*std::abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy
                            + df[j*data->w+i-1].cb*std::abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy)
                            + df[j*data->w+i].cb*std::abs(df[j*data->w+i].dp_cb)*(4-adjx)*(4-adjy) )
                    / (std::abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy + std::abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy
                        + std::abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy) + std::abs(df[j*data->w+i].dp_cb)*(4-adjx)*(4-adjy));
            // Cr
            ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr*adjx*adjy
                            + df[(j-1)*data->w+i].dp_cr*(4-adjx)*adjy
                            + df[j*data->w+i-1].dp_cr*adjx*(4-adjy)
                            + df[j*data->w+i].dp_cr*(4-adjx)*(4-adjy) ) /16;
            if (ex[j*w+i].dp_cr)
                ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*std::abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
                            + df[(j-1)*data->w+i].cr*std::abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy
                            + df[j*data->w+i-1].cr*std::abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy)
                            + df[j*data->w+i].cr*std::abs(df[j*data->w+i].dp_cr)*(4-adjx)*(4-adjy) )
                    / (std::abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy +std::abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy
                        + std::abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy)+std::abs(df[j*data->w+i].dp_cr)*(4-adjx)*(4-adjy));
        }
        // Y //----------------------- 右端
        ex[j*w+i].dp_y = (df[(j-1)*data->w+i-1].dp_y*adjx*adjy
                        + df[j*data->w+i-1].dp_y*adjx*(4-adjy)) / 16;
        if (ex[j*w+i].dp_y)
            ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*std::abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
                        + df[j*data->w+i-1].y*std::abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy))
                / (std::abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy+std::abs(df[j*data->w+i-1].dp_y)*adjx*(4-adjy));
        // Cb
        ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb*adjx*adjy
                        + df[j*data->w+i-1].dp_cb*adjx*(4-adjy)) / 16;
        if (ex[j*w+i].dp_cb)
            ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*std::abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
                        + df[j*data->w+i-1].cb*std::abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy))
                / (std::abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy+std::abs(df[j*data->w+i-1].dp_cb)*adjx*(4-adjy));
        // Cr
        ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr*adjx*adjy
                        + df[j*data->w+i-1].dp_cr*adjx*(4-adjy)) / 16;
        if (ex[j*w+i].dp_cr)
            ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*std::abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
                        + df[j*data->w+i-1].cr*std::abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy))
                / (std::abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy+std::abs(df[j*data->w+i-1].dp_cr)*adjx*(4-adjy));
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
            ex[j*w+i].y = (df[(j-1)*data->w+i-1].y*std::abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy
                        + df[(j-1)*data->w+i].y*std::abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy)
                / (std::abs(df[(j-1)*data->w+i-1].dp_y)*adjx*adjy +std::abs(df[(j-1)*data->w+i].dp_y)*(4-adjx)*adjy);
        // Cb
        ex[j*w+i].dp_cb = (df[(j-1)*data->w+i-1].dp_cb * adjx * adjy
                                + df[(j-1)*data->w+i].dp_cb * (4-adjx) *adjy) /16;
        if (ex[j*w+i].dp_cb)
            ex[j*w+i].cb = (df[(j-1)*data->w+i-1].cb*std::abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy
                        + df[(j-1)*data->w+i].cb*std::abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy )
                / (std::abs(df[(j-1)*data->w+i-1].dp_cb)*adjx*adjy +std::abs(df[(j-1)*data->w+i].dp_cb)*(4-adjx)*adjy);
        // Cr
        ex[j*w+i].dp_cr = (df[(j-1)*data->w+i-1].dp_cr * adjx * adjy
                                + df[(j-1)*data->w+i].dp_cr * (4-adjx) *adjy) /16;
        if (ex[j*w+i].dp_cr)
            ex[j*w+i].cr = (df[(j-1)*data->w+i-1].cr*std::abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy
                        + df[(j-1)*data->w+i].cr*std::abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy)
                / (std::abs(df[(j-1)*data->w+i-1].dp_cr)*adjx*adjy +std::abs(df[(j-1)*data->w+i].dp_cr)*(4-adjx)*adjy);
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
*   update_combobox()       コンボボックスの選択を更新
*       拡張データ領域のロゴ名にコンボボックスのカーソルを合わせる
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
*   load_logo_param()
*       指定したロゴデータを拡張データ領域にコピー
*-------------------------------------------------------------------*/
static void load_logo_param(FILTER* fp, int num) {
    if (logodata[num]->fi || logodata[num]->fo || logodata[num]->st || logodata[num]->ed) {
        fp->track[LOGO_TRACK_START] = logodata[num]->st;
        fp->track[LOGO_TRACK_FADE_IN]  = logodata[num]->fi;
        fp->track[LOGO_TRACK_FADE_OUT] = logodata[num]->fo;
        fp->track[LOGO_TRACK_END]  = logodata[num]->ed;
        fp->exfunc->filter_window_update(fp);
    }
}

/*--------------------------------------------------------------------
*   change_param()      パラメータの変更
*       選択されたロゴデータを拡張データ領域にコピー
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
*   set_combo_item()        コンボボックスにアイテムをセット
*           dataはmallocで確保したポインタ
*           個別にlogodata配列に書き込む必要がある
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
*   del_combo_item()        コンボボックスからアイテムを削除
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
*   get_logo_file_header_ver() LOGO_HEADERのバージョンを取得
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
*   convert_logo_header_v1_to_v2() LOGO_HEADERをv1からv2に変換
*-------------------------------------------------------------------*/
void convert_logo_header_v1_to_v2(LOGO_HEADER *logo_header) {
    LOGO_HEADER_OLD old_header;
    memcpy(&old_header,       logo_header,      sizeof(old_header));
    memset(logo_header,       0,                sizeof(logo_header[0]));
    memcpy(logo_header->name, &old_header.name, sizeof(old_header.name));
    memcpy(&logo_header->x,   &old_header.x,    sizeof(short) * 8);
}

/*--------------------------------------------------------------------
*   read_logo_pack()        ロゴデータ読み込み
*       ファイルからロゴデータを読み込み
*       コンボボックスにセット
*       拡張領域にコピー
*-------------------------------------------------------------------*/
static void read_logo_pack(char *fname, FILTER *fp)
{
    // ファイルオープン
    HANDLE hFile = CreateFile(fname,GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox(fp->hwnd, "ロゴデータファイルが見つかりません", filter_name, MB_OK|MB_ICONERROR);
        return;
    }
    if (GetFileSize(hFile, nullptr) < sizeof(LOGO_FILE_HEADER)) { // サイズ確認
        CloseHandle(hFile);
        MessageBox(fp->hwnd, "ロゴデータファイルが不正です", filter_name, MB_OK|MB_ICONERROR);
        return;
    }

//  SetFilePointer(hFile,31, 0, FILE_BEGIN);    // 先頭から31byteへ
    DWORD readed = 0;
    LOGO_FILE_HEADER logo_file_header = { 0 };
    ReadFile(hFile, &logo_file_header, sizeof(logo_file_header), &readed, nullptr); // ファイルヘッダ取得

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
        ReadFile(hFile, &logo_header, logo_header_size, &readed, nullptr);
        if (readed != logo_header_size) {
            MessageBox(fp->hwnd, "ロゴデータの読み込みに失敗しました", filter_name, MB_OK|MB_ICONERROR);
            break;
        }
        if (logo_header_ver == 1) {
            convert_logo_header_v1_to_v2(&logo_header);
        }

//  ldpには基本的に同名のロゴは存在しない
//
//      // 同名ロゴがあるか
//      int same = find_logodata(lgh.name);
//      if (same>0) {
//          wsprintf(message,"同名のロゴがあります\n置き換えますか？\n\n%s",lgh.name);
//          if (MessageBox(fp->hwnd, message, filter_name, MB_YESNO|MB_ICONQUESTION) == IDYES) {
//              // 削除
//              del_combo_item(same);
//          } else {    // 上書きしない
//              // ファイルポインタを進める
//              SetFilePointer(hFile, LOGO_PIXELSIZE(&lgh), 0, FILE_CURRENT);
//              continue;
//          }
//      }

        // メモリ確保
        BYTE *data = (BYTE *)calloc(logo_data_size(&logo_header), 1);
        if (data == nullptr) {
            MessageBox(fp->hwnd, "メモリが足りません", filter_name, MB_OK|MB_ICONERROR);
            break;
        }

        // LOGO_HEADERコピー
        memcpy(data, &logo_header, sizeof(logo_header));

        LOGO_HEADER* ptr = (LOGO_HEADER *)(data + sizeof(LOGO_HEADER));

        // LOGO_PIXEL読み込み
        readed = 0;
        ReadFile(hFile, ptr, logo_pixel_size(&logo_header), &readed, nullptr);

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
*   on_option_button()      オプションボタン動作
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

        if (logodata_n) // 拡張データ初期値設定
            fp->ex_data_def = logodata[0];
        else
            fp->ex_data_def = nullptr;
    }

    return TRUE;
}

/*--------------------------------------------------------------------
*   set_sended_data()       受信したロゴデータをセット
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
    if (ptr == nullptr) {
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
*   on_avisynth_button()    AviSynthボタン動作
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

    if (fp->track[LOGO_TRACK_X] || fp->track[LOGO_TRACK_Y])
        wsprintf(str, "%s,\r\n\\           pos_x=%d, pos_y=%d",
                    str, fp->track[LOGO_TRACK_X], fp->track[LOGO_TRACK_Y]);

    if (fp->track[LOGO_TRACK_YDP]!=128 || fp->track[LOGO_TRACK_PY] || fp->track[LOGO_TRACK_CB] || fp->track[LOGO_TRACK_CR])
        wsprintf(str, "%s,\r\n\\           depth=%d, yc_y=%d, yc_u=%d, yc_v=%d",
                    str, fp->track[LOGO_TRACK_YDP], fp->track[LOGO_TRACK_PY], fp->track[LOGO_TRACK_CB], fp->track[LOGO_TRACK_CR]);


    if (fp->exfunc->get_frame_n(editp)) { // 画像が読み込まれているとき
        int s, e;
        fp->exfunc->get_select_frame(editp, &s, &e); // 選択範囲取得
        wsprintf(str, "%s,\r\n\\           start=%d", str, s + fp->track[LOGO_TRACK_START]);

        if (fp->track[LOGO_TRACK_FADE_IN] || fp->track[LOGO_TRACK_FADE_OUT])
            wsprintf(str, "%s, fadein=%d, fadeout=%d", str, fp->track[LOGO_TRACK_FADE_IN], fp->track[LOGO_TRACK_FADE_OUT]);

        wsprintf(str, "%s, end=%d", str, e - fp->track[LOGO_TRACK_END]);
    } else {
        if (fp->track[LOGO_TRACK_FADE_IN] || fp->track[LOGO_TRACK_FADE_OUT])
            wsprintf(str, "%s,\r\n\\           fadein=%d, fadeout=%d", str, fp->track[LOGO_TRACK_FADE_IN], fp->track[LOGO_TRACK_FADE_OUT]);
    }

    wsprintf(str,"%s)\r\n", str);


    EnableWindow(dialog.bt_synth, FALSE); // synthボタン無効化

    // ダイアログ呼び出し
    DialogBoxParam(fp->dll_hinst, "STR_DLG", GetWindow(fp->hwnd, GW_OWNER), StrDlgProc, (LPARAM)str);

    EnableWindow(dialog.bt_synth, TRUE); // synthボタン無効化解除

    return TRUE;
}



/*********************************************************************
*   DLLMain
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
        if (strings[0] == nullptr) break;
        GetPrivateProfileString("string", "name", filter.name, strings[0], FILTER_NAME_MAX, ini_name);
        filter.name = strings[0];

        // トラック名
        for (int i = 0; i < TRACK_N; i++) {
            strings[i+1] = (char *)calloc(FILTER_TRACK_MAX, 1);
            if (strings[i+1] == nullptr) break;
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
            if (strings[i+TRACK_N+1] == nullptr) break;
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
            strings[i] = nullptr;
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
