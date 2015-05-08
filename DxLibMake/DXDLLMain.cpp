// -------------------------------------------------------------------------------
// 
// 	ＤＸライブラリ		エントリポイント
// 
// ● [ ライブラリリンク定義 ]
// ┗ エントリポイントは実際の実行ファイルを
// 　 作るプロジェクト同様にパスを通す必要がある。
// -------------------------------------------------------------------------------
// 
// ※Makeプロジェクトには付いていない「最新版」のライブラリのパスを通して置く事。
// 　drawfuncもあらかじめ最新版でライブラリを作っておく必要がある。
//
// 　DxDrawFunc.libとDxUseCLib.libのリンクエラーに注意
//
// →Dxlibの中から取ってくればよい。
// 　極論から行くと、すべてのスタティックライブラリをここに含めてDLL化するのが早い。。

//#define DX_NON_BULLET_PHYSICS				// 3.07から除外できなくなった
//#define DX_USE_VC8_BULLET_PHYSICS_LIB		// vc8用のライブラリは梱包されてない

// ライブラリリンク定義--------------------------------------------------------
#ifndef __DX_MAKE
	#ifndef DX_LIB_NOT_DEFAULTPATH
		#ifndef DX_SRC_COMPILE
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "DxDrawFunc_x64_d.lib"		)		//  描画部分の抜き出し
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW_x64_d.lib"		)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLibW_x64_d.lib"	)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#else
							#pragma comment( lib, "DxLib_x64_d.lib"			)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLib_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#endif
					#else
						#pragma comment( lib, "DxDrawFunc_x64.lib"			)		//  描画部分の抜き出し
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW_x64.lib"			)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLibW_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#else
							#pragma comment( lib, "DxLib_x64.lib"			)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLib_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#endif
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "DxDrawFunc_d.lib"		)		//  描画部分の抜き出し
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW_d.lib"		)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLibW_d.lib"	)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#else
							#pragma comment( lib, "DxLib_d.lib"			)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLib_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#endif
					#else
						#pragma comment( lib, "DxDrawFunc.lib"			)		//  描画部分の抜き出し
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW.lib"			)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLibW.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#else
							//#pragma comment( lib, "DxLib.lib"			)		//  ＤＸライブラリ使用指定
							#pragma comment( lib, "DxUseCLib.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
						#endif
					#endif
				#endif
			#else
				#pragma comment( lib, "DxDrawFunc.lib"		)			//  描画部分の抜き出し
				#ifdef UNICODE
					#pragma comment( lib, "DxLibW.lib"		)			//  ＤＸライブラリ使用指定
					#pragma comment( lib, "DxUseCLibW.lib"	)			//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
				#else
					#pragma comment( lib, "DxLib.lib"		)			//  ＤＸライブラリ使用指定
					#pragma comment( lib, "DxUseCLib.lib"	)			//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
				#endif
			#endif

			#ifdef _DEBUG
				#pragma comment( linker, "/NODEFAULTLIB:libcmt.lib" )
				#pragma comment( linker, "/NODEFAULTLIB:libc.lib" )
				#pragma comment( linker, "/NODEFAULTLIB:libcd.lib" )
	//			#pragma comment( linker, "/NODEFAULTLIB:msvcrt.lib" )
	//			#pragma comment( linker, "/NODEFAULTLIB:msvcrtd.lib" )
			#else
				#pragma comment( linker, "/NODEFAULTLIB:libcmtd.lib" )
				#pragma comment( linker, "/NODEFAULTLIB:libc.lib" )
				#pragma comment( linker, "/NODEFAULTLIB:libcd.lib" )
	//			#pragma comment( linker, "/NODEFAULTLIB:msvcrt.lib" )
	//			#pragma comment( linker, "/NODEFAULTLIB:msvcrtd.lib" )
			#endif
		#endif
		//#pragma comment( lib, "libcmt.lib"		)				//  C標準マルチスレッド対応ライブラリ
		#pragma comment( lib, "kernel32.lib"		)			//  Win32カーネルライブラリ
		//#pragma comment( lib, "comctl32.lib"		)			//　Win32APIライブラリ
		#pragma comment( lib, "user32.lib"		)				//  Win32APIライブラリ
		#pragma comment( lib, "gdi32.lib"		)				//  Win32APIライブラリ
		#pragma comment( lib, "advapi32.lib"		)			//  Win32APIライブラリ
		//#pragma comment( lib, "ole32.lib"		)				//  Win32APIライブラリ
		#pragma comment( lib, "shell32.lib"		)				//  マルチメディアライブラリ
		//#pragma comment( lib, "winmm.lib"		)				//  マルチメディアライブラリ
		#ifndef DX_NON_MOVIE
			//#pragma comment( lib, "Strmiids.lib" )			//　DirectShowライブラリ
		#endif
		#ifndef DX_NON_NETWORK
			//#pragma comment( lib, "wsock32.lib" )				//  WinSocketsライブラリ
		#endif
		#ifndef DX_NON_KEYEX
			//#pragma comment( lib, "imm32.lib" )					// ＩＭＥ操作用ライブラリ
		#endif
		#ifndef DX_NON_ACM
			//#pragma comment( lib, "msacm32.lib" )				// ＡＣＭ操作用ライブラリ 
		#endif
		#ifndef DX_NON_BULLET_PHYSICS
			#ifdef _MSC_VER
				#ifdef DX_USE_VC8_BULLET_PHYSICS_LIB
					#ifdef _WIN64
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc8_x64_d.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc8_x64_d.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc8_x64.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc8_x64.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64.lib" )
						#endif
					#else
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc8_d.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc8_d.lib" )
							#pragma comment( lib, "libbulletmath_vc8_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc8.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc8.lib" )
							#pragma comment( lib, "libbulletmath_vc8.lib" )
						#endif
					#endif
				#else
					#ifdef _WIN64
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc8_x64_d.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc8_x64_d.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc8_x64.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc8_x64.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64.lib" )
						#endif
					#else
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc6_d.lib" )	// Visual C++ 6.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc6_d.lib" )
							#pragma comment( lib, "libbulletmath_vc6_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc6.lib" )	// Visual C++ 6.0 でコンパイルした Bullet Physics ライブラリ 
							#pragma comment( lib, "libbulletdynamics_vc6.lib" )
							#pragma comment( lib, "libbulletmath_vc6.lib" )
						#endif
					#endif
				#endif
			#else
				#pragma comment( lib, "libbulletcollision.lib" )	// Bullet Physics ライブラリ 
				#pragma comment( lib, "libbulletdynamics.lib" )
				#pragma comment( lib, "libbulletmath.lib" )
			#endif
		#endif
		#ifndef DX_NON_PNGREAD
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "libpng_x64_d.lib" )		// ＰＮＧライブラリ
						#pragma comment( lib, "zlib_x64_d.lib" )
					#else
						#pragma comment( lib, "libpng_x64.lib" )		// ＰＮＧライブラリ
						#pragma comment( lib, "zlib_x64.lib" )
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "libpng_d.lib" )			// ＰＮＧライブラリ
						#pragma comment( lib, "zlib_d.lib" )
					#else
						#pragma comment( lib, "libpng.lib" )			// ＰＮＧライブラリ
						#pragma comment( lib, "zlib.lib" )
					#endif
				#endif
			#else
				#pragma comment( lib, "libpng.lib" )			// ＰＮＧライブラリ
				#pragma comment( lib, "zlib.lib" )
			#endif
		#endif
		#ifndef DX_NON_JPEGREAD
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "libjpeg_x64_d.lib" )		// ＪＰＥＧライブラリ
					#else
						#pragma comment( lib, "libjpeg_x64.lib" )		// ＪＰＥＧライブラリ
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "libjpeg_d.lib" )			// ＪＰＥＧライブラリ
					#else
						#pragma comment( lib, "libjpeg.lib" )			// ＪＰＥＧライブラリ
					#endif
				#endif
			#else
				#pragma comment( lib, "libjpeg.lib" )			// ＪＰＥＧライブラリ
			#endif
		#endif
		#ifndef DX_NON_OGGVORBIS								// ＯｇｇＶｏｒｂｉｓライブラリ
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "ogg_static_x64_d.lib" )
						#pragma comment( lib, "vorbis_static_x64_d.lib" )
						#pragma comment( lib, "vorbisfile_static_x64_d.lib" )
					#else
						#pragma comment( lib, "ogg_static_x64.lib" )
						#pragma comment( lib, "vorbis_static_x64.lib" )
						#pragma comment( lib, "vorbisfile_static_x64.lib" )
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "ogg_static_d.lib" )
						#pragma comment( lib, "vorbis_static_d.lib" )
						#pragma comment( lib, "vorbisfile_static_d.lib" )
					#else
						#pragma comment( lib, "ogg_static.lib" )
						#pragma comment( lib, "vorbis_static.lib" )
						#pragma comment( lib, "vorbisfile_static.lib" )
					#endif
				#endif
			#else
				#pragma comment( lib, "ogg_static.lib" )
				#pragma comment( lib, "vorbis_static.lib" )
				#pragma comment( lib, "vorbisfile_static.lib" )
			#endif
		#endif
		#ifndef DX_NON_OGGTHEORA								// ＯｇｇＴｈｅｏｒａライブラリ
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "ogg_static_x64_d.lib" )
						#pragma comment( lib, "vorbis_static_x64_d.lib" )
						#pragma comment( lib, "vorbisfile_static_x64_d.lib" )

						#pragma comment( lib, "libtheora_static_x64_d.lib" )
					#else
						#pragma comment( lib, "ogg_static_x64.lib" )
						#pragma comment( lib, "vorbis_static_x64.lib" )
						#pragma comment( lib, "vorbisfile_static_x64.lib" )

						#pragma comment( lib, "libtheora_static_x64.lib" )
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "ogg_static_d.lib" )
						#pragma comment( lib, "vorbis_static_d.lib" )
						#pragma comment( lib, "vorbisfile_static_d.lib" )

						#pragma comment( lib, "libtheora_static_d.lib" )
					#else
						#pragma comment( lib, "ogg_static.lib" )
						#pragma comment( lib, "vorbis_static.lib" )
						#pragma comment( lib, "vorbisfile_static.lib" )

						#pragma comment( lib, "libtheora_static.lib" )
					#endif
				#endif
			#else
				#pragma comment( lib, "ogg_static.lib" )
				#pragma comment( lib, "vorbis_static.lib" )
				#pragma comment( lib, "vorbisfile_static.lib" )

				#pragma comment( lib, "libtheora_static.lib" )
			#endif
		#endif
	#endif
#endif


#include <windows.h>
BOOL WINAPI DllMain( HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved )
{
	return TRUE;
}
