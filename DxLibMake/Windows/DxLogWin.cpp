//-----------------------------------------------------------------------------
// 
// 		�c�w���C�u����		Windows�p���O�v���O����
// 
//  	Ver 3.12a
// 
//-----------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h ---------------------------------------------------------------
#include "DxLogWin.h"

#ifndef DX_NON_LOG

#include "../DxStatic.h"
#include "../DxLib.h"
#include "../DxLog.h"
#include "../DxBaseFunc.h"
#include "../DxMemory.h"
#include "DxWindow.h"
#include "DxWinAPI.h"

namespace DxLib
{

// �}�N����` -----------------------------------------------------------------

// �\���̐錾 -----------------------------------------------------------------

// �f�[�^��` -----------------------------------------------------------------

// �֐��錾 -------------------------------------------------------------------

// �v���O���� -----------------------------------------------------------------

// �G���[�����o���t�@�C�������������鏈���̊��ˑ�����
extern int ErrorLogInitialize_Native( void )
{
	HANDLE fp ;
	TCHAR String[ 512 ] ;
	int Len ;

	// ���O�o�͗}���t���O�������Ă��邩�A���[�U�[�w��̃E�C���h�E��
	// �g�p���Ă���ꍇ�͏o�͂��s��Ȃ�
	if( LogData.NotLogOutFlag == TRUE /* || WinData.UserWindowFlag == TRUE */ ) return 0 ;

	// ���t���̃��O�o�͂��w�肳��Ă���ꍇ�̓t�@�C�������쐬
	if( LogData.UseLogDateName == TRUE )
	{
		DATEDATA Date ;
		TCHAR String[128] ;

		// ���t���擾
		NS_GetDateTime( &Date ) ;

		// ������̍쐬
		lstrcpy( LogData.LogFileName, _T( "Log" ) ) ;
		_ITOT( Date.Year, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Mon, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Day, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Hour, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Min, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Sec, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( ".txt" ) ) ;
	}
	else
	{
		// ����ȊO�̏ꍇ�͒ʏ�̃t�@�C����
		lstrcpy( LogData.LogFileName, _T( "Log.txt" ) ) ;
	}

	Len = lstrlen( LogData.LogOutDirectory ) ;
	_TSTRNCPY( String, LogData.LogOutDirectory, Len ) ;
	if( String[ Len - 1 ] != _T( '\\' ) )
	{
		String[ Len ] = _T( '\\' ) ;
		Len ++ ;
	}
	lstrcpy( String + Len, LogData.LogFileName ) ;

	// �G���[���O�t�@�C�����č쐬����
	DeleteFile( String ) ;
	fp = CreateFile( String, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL ) ;

#ifdef UNICODE
	BYTE BOM[ 2 ] = { 0xff, 0xfe } ;
	DWORD WriteSize ;
	WriteFile( fp, &BOM, 2, &WriteSize, NULL ) ;
#endif

	if( fp != NULL ) CloseHandle( fp ) ;

	// �I��
	return 0 ;
}

// �G���[�����������o��
extern int NS_ErrorLogAdd( const TCHAR *ErrorStr )
{
	HANDLE fp ;
	DWORD WriteSize, StringLength ;
	TCHAR *Temp, DefaultTemp[256], *d, *AllocTemp = NULL ;
	const TCHAR *p1, *p2 ;

	if( WinAPIData.Win32Func.WinMMDLL == NULL )
		return -1 ;

	StringLength = lstrlen( ErrorStr ) ;
	Temp = DefaultTemp ;

	// ���������񂪗]��ɂ�����������e���|�����̈�������Ŋm�ۂ���
	if( StringLength > 255 )
	{
		Temp = AllocTemp = ( TCHAR * )NS_DxAlloc( StringLength * 2 * sizeof( TCHAR ) ) ;
	}

	// \n �� \r\n �ɕϊ����Ȃ���R�s�[
	{
		Temp[0] = _T( '\0' ) ;
		p1 = ErrorStr ;
		d = Temp ;

		p2 = _TSTRCHR( p1, _T( '\n' ) ) ;
		while( p2 != NULL && ( ErrorStr == p2 || p2[-1] != _T( '\r' ) ) )
		{
			_MEMCPY( d, p1, ( p2 - p1 ) * sizeof( TCHAR ) ) ;
			d += p2 - p1 ;
			d[0] = _T( '\r' ) ;
			d[1] = _T( '\n' ) ;
			d[2] = _T( '\0' ) ;
			d += 2 ;
			p1 = p2 + 1 ;

			p2 = _TSTRCHR( p1, _T( '\n' ) ) ;
		}
		lstrcpy( d, p1 ) ;
		ErrorStr = Temp ;
	}

	// �c�w���C�u�������쐬�����E�C���h�E�ł���ꍇ�̂݃t�@�C���ɏo�͂���
	if( /*WinData.MainWindow != NULL && WinData.UserWindowFlag == FALSE */ true )
	{
		// ���O�o�͗}���t���O�������Ă����ꍇ�͏o�͂��s��Ȃ�
		if( LogData.NotLogOutFlag == FALSE && LogData.LogOutDirectory[0] != _T( '\0' ) )
		{
//			TCHAR MotoPath[MAX_PATH] ;
			TCHAR String[ 512 ] ;
			int Len ;

			// �G���[���O�t�@�C�����J��
//			FGETDIR( MotoPath ) ;
//			FSETDIR( LogData.LogOutDirectory ) ;
			Len = lstrlen( LogData.LogOutDirectory ) ;
			_TSTRNCPY( String, LogData.LogOutDirectory, Len ) ;
//			lstrcpy( String, LogData.LogOutDirectory ) ;
			if( String[ Len - 1 ] != _T( '\\' ) )
			{
				String[ Len ] = _T( '\\' ) ;
				Len ++ ;
			}
			lstrcpy( String + Len, LogData.LogFileName ) ;
			fp = CreateFile( String, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			if( fp != NULL )
			{
				SetFilePointer( fp, 0, NULL, FILE_END ) ;

				// �K�蕪�����^�u��r�o
				if( LogData.ErTabStop == FALSE )
				{
					int i ;	

					// �^�C���X�^���v���o��
					if( LogData.NonUseTimeStampFlag == 0 )
					{
						_TSPRINTF( _DXWTP( String ), _DXWTR( "%d:" ), WinAPIData.Win32Func.timeGetTimeFunc() - LogData.LogStartTime ) ;
						WriteFile( fp, String, lstrlen( String ) * sizeof( TCHAR ), &WriteSize, NULL ) ;
						OutputDebugString( String ) ;
					}

					if( LogData.ErTabNum != 0 )
					{
						for( i = 0 ; i < LogData.ErTabNum ; i ++ )
							String[i] = _T( '\t' ) ;
						String[i] = _T( '\0' ) ;
						WriteFile( fp, String, LogData.ErTabNum * sizeof( TCHAR ), &WriteSize, NULL ) ;
						OutputDebugString( String ) ;
					}
				}

				// �G���[���O�t�@�C���ɏ����o��
				WriteFile( fp, ErrorStr, lstrlen( ErrorStr ) * sizeof( TCHAR ), &WriteSize, NULL ) ;

				// �G���[���O���A�E�g�v�b�g�ɏ����o��
				OutputDebugString( ErrorStr ) ;

				// �Ō�̕��������s�ӊO�������ꍇ�̓^�u�X�g�b�v�t���O�𗧂Ă�
				LogData.ErTabStop = ErrorStr[ lstrlen( ErrorStr ) - 1 ] != _T( '\n' ) ;

				CloseHandle( fp ) ;
			}
//			FSETDIR( MotoPath ) ;
		}
	}
	else
	{
		// �G���[���O���A�E�g�v�b�g�ɏ����o��
		OutputDebugString( ErrorStr ) ;
	}

	// �e���|�����̈���m�ۂ��Ă�����J������
	if( AllocTemp != NULL )
	{
		NS_DxFree( AllocTemp ) ;
	}
	
	// �I��
	return -1 ;
}


// ���O�@�\�̏��������s�����ǂ������擾����
extern int IsInitializeLog( void )
{
	// �c�w���C�u�������I����Ԃɓ����Ă����珉�������Ȃ�
	return NS_GetUseDDrawObj() == NULL || WinData.MainWindow == NULL ? FALSE : TRUE ;
}


}

#endif // DX_NON_LOG

