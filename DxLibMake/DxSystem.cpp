// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		システムプログラム
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード ------------------------------------------------------------------
#include "DxSystem.h"
#include "DxBaseFunc.h"

namespace DxLib
{

// マクロ定義 --------------------------------------------------------------------

// 構造体定義 --------------------------------------------------------------------

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

DXSYSTEMDATA DxSysData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// プログラム --------------------------------------------------------------------

// 初期化・終了関係

// DxSysData 関係の初期化を行う
extern int DxLib_SysInit( void )
{
	// 終了リクエストのフラグを倒す
	DxSysData.EndRequestFlag = FALSE ;

	// 終了
	return 0 ;
}

// 終了リクエストを行う
extern int DxLib_EndRequest( void )
{
	DxSysData.EndRequestFlag = TRUE ;

	// 終了
	return 0 ;
}

// 終了リクエストの状態を取得する
extern int DxLib_GetEndRequest( void )
{
	return DxSysData.EndRequestFlag ;
}







// ライブラリが初期化されているかどうかを取得する( 戻り値: TRUE=初期化されている  FALSE=されていない )
extern int NS_DxLib_IsInit( void )
{
	return DxSysData.DxLib_InitializeFlag ;
}






// エラー処理関数

// 書式付きライブラリのエラー処理を行う
extern int DxLib_FmtError( const TCHAR *FormatString , ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ];

	// ログ出力用のリストをセットする
	va_start( VaList , FormatString ) ;

	// 編集語の文字列を取得する
	_TVSPRINTF( String , FormatString , VaList ) ;

	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// エラー処理にまわす
	return DxLib_Error( String ) ;
}






// 読み込み処理系の関数で非同期読み込みを行うかどうかを設定する( 非同期読み込みに対応している関数のみ有効 )( TRUE:非同期読み込みを行う  FALSE:非同期読み込みを行わない( デフォルト ) )
extern int NS_SetUseASyncLoadFlag( int Flag )
{
	DxSysData.ASyncLoadFlag = Flag ;

	// 終了
	return 0 ;
}

// 非同期読み込みを行うかどうかを取得する( TRUE:非同期読み込みを行う   FALSE:非同期読み込みを行わない )
extern int GetASyncLoadFlag( void )
{
	return DxSysData.ASyncLoadFlag ? TRUE : FALSE ;
}

// ＤＸライブラリのウインドウ関連の機能を使用しないフラグ
extern int NS_SetNotWinFlag( int Flag )
{
	if( Flag == TRUE ) DxSysData.NotDrawFlag = TRUE;
	DxSysData.NotWinFlag = Flag ;

	return 0 ;
}

// 描画機能を使うかどうかのフラグをセットする
extern int NS_SetNotDrawFlag( int Flag )
{
	DxSysData.NotDrawFlag = Flag ;

	return 0 ;
}

// 描画機能を使うかどうかのフラグを取得する
extern int NS_GetNotDrawFlag( void )
{
	return DxSysData.NotDrawFlag ;
}

// サウンド機能を使うかどうかのフラグをセットする
extern int NS_SetNotSoundFlag( int Flag )
{
	DxSysData.NotSoundFlag = Flag ;

	return 0;
}

// 入力状態取得機能を使うかどうかのフラグをセットする
extern int NS_SetNotInputFlag( int Flag )
{
	DxSysData.NotInputFlag = Flag ;

	return 0;
}


}





