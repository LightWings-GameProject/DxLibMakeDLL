// -------------------------------------------------------------------------------
// 
// 	�c�w���C�u����		�G���g���|�C���g
// 
// �� [ ���C�u���������N��` ]
// �� �G���g���|�C���g�͎��ۂ̎��s�t�@�C����
// �@ ���v���W�F�N�g���l�Ƀp�X��ʂ��K�v������B
// -------------------------------------------------------------------------------
// 
// ��Make�v���W�F�N�g�ɂ͕t���Ă��Ȃ��u�ŐV�Łv�̃��C�u�����̃p�X��ʂ��Ēu�����B
// �@drawfunc�����炩���ߍŐV�łŃ��C�u����������Ă����K�v������B
//
// �@DxDrawFunc.lib��DxUseCLib.lib�̃����N�G���[�ɒ���
//
// ��Dxlib�̒��������Ă���΂悢�B
// �@�ɘ_����s���ƁA���ׂẴX�^�e�B�b�N���C�u�����������Ɋ܂߂�DLL������̂������B�B

//#define DX_NON_BULLET_PHYSICS				// 3.07���珜�O�ł��Ȃ��Ȃ���
//#define DX_USE_VC8_BULLET_PHYSICS_LIB		// vc8�p�̃��C�u�����͍����ĂȂ�

// ���C�u���������N��`--------------------------------------------------------
#ifndef __DX_MAKE
	#ifndef DX_LIB_NOT_DEFAULTPATH
		#ifndef DX_SRC_COMPILE
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "DxDrawFunc_x64_d.lib"		)		//  �`�敔���̔����o��
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW_x64_d.lib"		)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLibW_x64_d.lib"	)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#else
							#pragma comment( lib, "DxLib_x64_d.lib"			)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLib_x64_d.lib"		)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#endif
					#else
						#pragma comment( lib, "DxDrawFunc_x64.lib"			)		//  �`�敔���̔����o��
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW_x64.lib"			)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLibW_x64.lib"		)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#else
							#pragma comment( lib, "DxLib_x64.lib"			)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLib_x64.lib"		)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#endif
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "DxDrawFunc_d.lib"		)		//  �`�敔���̔����o��
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW_d.lib"		)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLibW_d.lib"	)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#else
							#pragma comment( lib, "DxLib_d.lib"			)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLib_d.lib"		)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#endif
					#else
						#pragma comment( lib, "DxDrawFunc.lib"			)		//  �`�敔���̔����o��
						#ifdef UNICODE
							#pragma comment( lib, "DxLibW.lib"			)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLibW.lib"		)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#else
							//#pragma comment( lib, "DxLib.lib"			)		//  �c�w���C�u�����g�p�w��
							#pragma comment( lib, "DxUseCLib.lib"		)		//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
						#endif
					#endif
				#endif
			#else
				#pragma comment( lib, "DxDrawFunc.lib"		)			//  �`�敔���̔����o��
				#ifdef UNICODE
					#pragma comment( lib, "DxLibW.lib"		)			//  �c�w���C�u�����g�p�w��
					#pragma comment( lib, "DxUseCLibW.lib"	)			//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
				#else
					#pragma comment( lib, "DxLib.lib"		)			//  �c�w���C�u�����g�p�w��
					#pragma comment( lib, "DxUseCLib.lib"	)			//  �W���b���C�u�������g�p���镔���� lib �t�@�C���̎g�p�w��
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
		//#pragma comment( lib, "libcmt.lib"		)				//  C�W���}���`�X���b�h�Ή����C�u����
		#pragma comment( lib, "kernel32.lib"		)			//  Win32�J�[�l�����C�u����
		//#pragma comment( lib, "comctl32.lib"		)			//�@Win32API���C�u����
		#pragma comment( lib, "user32.lib"		)				//  Win32API���C�u����
		#pragma comment( lib, "gdi32.lib"		)				//  Win32API���C�u����
		#pragma comment( lib, "advapi32.lib"		)			//  Win32API���C�u����
		//#pragma comment( lib, "ole32.lib"		)				//  Win32API���C�u����
		#pragma comment( lib, "shell32.lib"		)				//  �}���`���f�B�A���C�u����
		//#pragma comment( lib, "winmm.lib"		)				//  �}���`���f�B�A���C�u����
		#ifndef DX_NON_MOVIE
			//#pragma comment( lib, "Strmiids.lib" )			//�@DirectShow���C�u����
		#endif
		#ifndef DX_NON_NETWORK
			//#pragma comment( lib, "wsock32.lib" )				//  WinSockets���C�u����
		#endif
		#ifndef DX_NON_KEYEX
			//#pragma comment( lib, "imm32.lib" )					// �h�l�d����p���C�u����
		#endif
		#ifndef DX_NON_ACM
			//#pragma comment( lib, "msacm32.lib" )				// �`�b�l����p���C�u���� 
		#endif
		#ifndef DX_NON_BULLET_PHYSICS
			#ifdef _MSC_VER
				#ifdef DX_USE_VC8_BULLET_PHYSICS_LIB
					#ifdef _WIN64
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc8_x64_d.lib" )	// Visual C++ 8.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc8_x64_d.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc8_x64.lib" )	// Visual C++ 8.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc8_x64.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64.lib" )
						#endif
					#else
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc8_d.lib" )	// Visual C++ 8.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc8_d.lib" )
							#pragma comment( lib, "libbulletmath_vc8_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc8.lib" )	// Visual C++ 8.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc8.lib" )
							#pragma comment( lib, "libbulletmath_vc8.lib" )
						#endif
					#endif
				#else
					#ifdef _WIN64
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc8_x64_d.lib" )	// Visual C++ 8.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc8_x64_d.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc8_x64.lib" )	// Visual C++ 8.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc8_x64.lib" )
							#pragma comment( lib, "libbulletmath_vc8_x64.lib" )
						#endif
					#else
						#ifdef _DEBUG
							#pragma comment( lib, "libbulletcollision_vc6_d.lib" )	// Visual C++ 6.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc6_d.lib" )
							#pragma comment( lib, "libbulletmath_vc6_d.lib" )
						#else
							#pragma comment( lib, "libbulletcollision_vc6.lib" )	// Visual C++ 6.0 �ŃR���p�C������ Bullet Physics ���C�u���� 
							#pragma comment( lib, "libbulletdynamics_vc6.lib" )
							#pragma comment( lib, "libbulletmath_vc6.lib" )
						#endif
					#endif
				#endif
			#else
				#pragma comment( lib, "libbulletcollision.lib" )	// Bullet Physics ���C�u���� 
				#pragma comment( lib, "libbulletdynamics.lib" )
				#pragma comment( lib, "libbulletmath.lib" )
			#endif
		#endif
		#ifndef DX_NON_PNGREAD
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "libpng_x64_d.lib" )		// �o�m�f���C�u����
						#pragma comment( lib, "zlib_x64_d.lib" )
					#else
						#pragma comment( lib, "libpng_x64.lib" )		// �o�m�f���C�u����
						#pragma comment( lib, "zlib_x64.lib" )
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "libpng_d.lib" )			// �o�m�f���C�u����
						#pragma comment( lib, "zlib_d.lib" )
					#else
						#pragma comment( lib, "libpng.lib" )			// �o�m�f���C�u����
						#pragma comment( lib, "zlib.lib" )
					#endif
				#endif
			#else
				#pragma comment( lib, "libpng.lib" )			// �o�m�f���C�u����
				#pragma comment( lib, "zlib.lib" )
			#endif
		#endif
		#ifndef DX_NON_JPEGREAD
			#ifdef _MSC_VER
				#ifdef _WIN64
					#ifdef _DEBUG
						#pragma comment( lib, "libjpeg_x64_d.lib" )		// �i�o�d�f���C�u����
					#else
						#pragma comment( lib, "libjpeg_x64.lib" )		// �i�o�d�f���C�u����
					#endif
				#else
					#ifdef _DEBUG
						#pragma comment( lib, "libjpeg_d.lib" )			// �i�o�d�f���C�u����
					#else
						#pragma comment( lib, "libjpeg.lib" )			// �i�o�d�f���C�u����
					#endif
				#endif
			#else
				#pragma comment( lib, "libjpeg.lib" )			// �i�o�d�f���C�u����
			#endif
		#endif
		#ifndef DX_NON_OGGVORBIS								// �n�����u�������������C�u����
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
		#ifndef DX_NON_OGGTHEORA								// �n�����s�������������C�u����
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
