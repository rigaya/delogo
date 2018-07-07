/*====================================================================
* 				logodef.h
*===================================================================*/
#ifndef ___LOGODEF_H
#define ___LOGODEF_H

#define LOGO_FADE_FAST_ANALYZE	1
#define __LOGO_AUTO_ERASE_SIMD__	/* 自動FadeのSIMD化 */
#define __LOGO_MASK_CREATE_SIMD__	/* (2015/01/15:h14) 新方式の場合のSIMD化 */
#define __LOGO_AUTO_ERASE_			/* Logoの自動消去Modeを有効にする */
#define __LOGO_ADD_NR2__			/* Logo部NR処理 */
#define	_h39_AUTONR_				/* (2016/02/14:+h39) 自動NRを自動Fadeから独立させて単独で使用できるように変更 */

#define LOGO_FADE_MAX    256
#define LOGO_XY_MAX      500
#define LOGO_XY_MIN     -500
#define LOGO_STED_MAX    256
#define LOGO_STED_MIN   -256

#define LOGO_NR_AREA_MAX	(3)
#define LOGO_NR_MAX			(4)
#define LOGO_NR_REF			(500)
#define LOGO_FADE_AD_MAX	(10)
#define LOGO_FADE_AD_DEF	(7)

#if 0
typedef struct _DELOGO_TARGET_FRAME_ELM {
    int				nFrame;				// Frame番号(0～)
    unsigned char	nType;				// 0:先頭, 1:末尾, 2:編集点(Frameの削除位置), 3:任意追加, 4:ソースチェンジ, 5:自動検出ON, 6:自動検出OFF, 7:手動ON, 8:手動OFF, 254:無音Frame(調査開始点), 255:未定義
} DELOGO_TARGET_FRAME_ELM;

typedef struct _DELOGO_SEARCH_FRAME_ELM {
    DELOGO_TARGET_FRAME_ELM elm;
    int				nMuteLength;
} DELOGO_SEARCH_FRAME_ELM;

typedef struct _DELOGO_EXTRA {
    unsigned char	key[16];			// 識別Key
    unsigned char	bAutoFade;			// 自動Fade
    int				nFadeCorrection;	// Fade補正
    unsigned char	bAutoNR;			// 自動NR
    int				nNRArea;			// NR範囲
    int				nNRValue;			// NR強度
    unsigned char	bDebug;				// DebugMode

                                        // TargetFrame情報
    int				nTotalFrames;
    int				nFadeFrames;			// FadeFrame数(1～40, 指定FrameからOffsetした位置から)
    int				nFadeOffset;			// 基準FrameからのFade開始位置(0～200)
    DELOGO_TARGET_FRAME_ELM Target[101];	// TargetのFrameIndex(0～), 最大100コマで格納する. 101個目は作業用.
    unsigned char	TargetCount;			// 指定されているTarget数
    unsigned char	nTargetType;			// 0:All, 1:Specified
    int				nAnalyzeFrames;			// (2015/03/07:+h28) Scne検出用FadeFrame数(0～240) CMなどのChangeSceneからロゴが完全に表示されるまでのFrame数

    char			Reserved[64];
} DELOGO_EXTRA;

typedef struct _DELOGO_EXT_DATA {
    LOGO_HEADER		logoHeader;
    DELOGO_EXTRA	extra;
} DELOGO_EXT_DATA;

enum LOGO_BOOL {
    LB_UNDEFINED = -1
    , LB_FALSE = 0
    , LB_TRUE = 1
};
#endif

#endif
