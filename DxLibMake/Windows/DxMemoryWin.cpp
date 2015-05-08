//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用メモリ関係プログラム
// 
//  	Ver 3.12a
// 
//-----------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード ---------------------------------------------------------------
#include "DxMemoryWin.h"
#include <windows.h>

namespace DxLib
{

// マクロ定義 -----------------------------------------------------------------

// 構造体宣言 -----------------------------------------------------------------

// データ定義 -----------------------------------------------------------------

// 関数宣言 -------------------------------------------------------------------

// プログラム -----------------------------------------------------------------

// ラッパー関数

// メモリを確保する
extern void *AllocWrap( size_t AllocSize )
{
	void *AllocMemory ;

//	AllocMemory = malloc( AllocSize ) ;
	AllocMemory = HeapAlloc( GetProcessHeap(), 0, AllocSize ) ;

	return AllocMemory ;
}

// メモリを確保して０で初期化する
extern void *CallocWrap( size_t AllocSize )
{
	void *AllocMemory ;

//	AllocMemory = calloc( 1, AllocSize ) ;
	AllocMemory = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, AllocSize ) ;
	
	return AllocMemory ;
}

// メモリの再確保を行う
extern void *ReallocWrap( void *Memory, size_t AllocSize )
{
	void *AllocMemory ;

//	AllocMemory = realloc( Memory, AllocSize ) ;
	AllocMemory = HeapReAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, Memory, AllocSize ) ;
	
	return AllocMemory ;
}

// メモリを解放する
extern void FreeWrap( void *Memory )
{
//	free( Memory ) ;
	HeapFree( GetProcessHeap(), 0, Memory ) ;
}

}

