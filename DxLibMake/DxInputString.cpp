// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		��������̓v���O����
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

#include "DxInputString.h"

#ifndef DX_NON_INPUTSTRING

// �C���N���[�h ------------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxGraphicsBase.h"
#include "DxFont.h"
#include "DxUseCLib.h"
#include "DxLog.h"
#include "Windows/DxWindow.h"
#include "Windows/DxWinAPI.h"

namespace DxLib
{

// �}�N����` --------------------------------------------------------------------

// ��������̓n���h���̗L�����`�F�b�N
#define KEYHCHK( HAND, KPOINT )																			\
	  ( ( ( (HAND) & DX_HANDLEERROR_MASK ) != 0 ) ||													\
		( ( (HAND) & DX_HANDLETYPE_MASK ) != DX_HANDLETYPE_MASK_KEYINPUT ) ||								\
		( ( (HAND) & DX_HANDLEINDEX_MASK ) >= MAX_INPUT_NUM ) ||										\
		( ( KPOINT = &CharBuf.InputData[ (HAND) & DX_HANDLEINDEX_MASK ] )->UseFlag == FALSE ) ||		\
		( ( (KPOINT)->ID << DX_HANDLECHECK_ADDRESS ) != ( (HAND) & DX_HANDLECHECK_MASK ) ) )


#define CURSORBRINKWAIT				(500)				// �J�[�\���_�ł̊Ԋu

// �\���̒�` --------------------------------------------------------------------

// �e�[�u��-----------------------------------------------------------------------

// ���z�L�[�R�[�h���R���g���[�������R�[�h�ɕϊ����邽�߂̃e�[�u��
char CtrlCode[ 10 ][ 2 ] =
{
	VK_DELETE	,	CTRL_CODE_DEL		,	// �c�d�k�L�[

	VK_LEFT		,	CTRL_CODE_LEFT		,	// ���L�[
	VK_RIGHT	,	CTRL_CODE_RIGHT		,	// ���L�[
	VK_UP		,	CTRL_CODE_UP		,	// ���L�[
	VK_DOWN		,	CTRL_CODE_DOWN		,	// ���L�[

	VK_HOME		,	CTRL_CODE_HOME		,	// �g�n�l�d�L�[
	VK_END		,	CTRL_CODE_END		,	// �d�m�c�L�[
	VK_PRIOR	,	CTRL_CODE_PAGE_UP	,	// �o�`�f�d�@�t�o�L�[
	VK_NEXT		,	CTRL_CODE_PAGE_DOWN	,	// �o�`�f�d�@�c�n�v�m�L�[
	0			,	0
} ;

// ���̓��[�h�e�[�u��
static INPUTMODE InputModeTable[] =
{
	{ IME_CMODE_CHARCODE											, _T( "�R�[�h����" ) } ,
	{ IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE						, _T( "�S�p�Ђ炪��" ) } ,
	{ IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA	, _T( "�S�p�J�^�J�i" ) } ,
	{ IME_CMODE_NATIVE | IME_CMODE_KATAKANA							, _T( "���p�J�^�J�i" ) } ,
	{ IME_CMODE_FULLSHAPE											, _T( "�S�p�p��" ) } ,
	{ IME_CMODE_ALPHANUMERIC										, _T( "���p�p��" ) } ,
	{ 0 , _T( "" ) } ,
} ;

// �������ϐ��錾 --------------------------------------------------------------

CHARBUFFER CharBuf ;										// �����R�[�h�o�b�t�@

// �֐��v���g�^�C�v�錾-----------------------------------------------------------

// �����R�[�h�o�b�t�@����֌W
#ifndef DX_NON_KEYEX
static	int			ResetKeyInputCursorBrinkCount( void ) ;										// �L�[���͎��̃J�[�\���_�ŏ����̃J�E���^�����Z�b�g
static	int			SetIMEOpenState( int OpenFlag ) ;											// �h�l�d���g�p��Ԃ�ύX����
static	int			KeyInputSelectAreaDelete( INPUTDATA *Input ) ;								// �L�[���͂̑I��͈͂��폜����
#endif

// �v���O���� --------------------------------------------------------------------

// �����R�[�h�o�b�t�@����֌W

// �����R�[�h�o�b�t�@�֌W�̏�����
extern int InitializeInputCharBuf( void )
{
	int IMEUseFlag ;
	int IMEUseFlag_OSSet ;

	DXST_ERRORLOG_ADD( _T( "�����R�[�h�o�b�t�@�̏��������s���܂�... " ) ) ; 

	// �[��������
	IMEUseFlag = CharBuf.IMEUseFlag ;
	IMEUseFlag_OSSet = CharBuf.IMEUseFlag_OSSet ;
	_MEMSET( &CharBuf, 0, sizeof( CharBuf ) ) ;
	CharBuf.IMEUseFlag = IMEUseFlag ;
	CharBuf.IMEUseFlag_OSSet = IMEUseFlag_OSSet ;

	// �F�̃Z�b�g
	CharBuf.StrColor1  = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor3  = NS_GetColor( 255 , 255 ,   0 ) ;
	CharBuf.StrColor2  = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor4  = NS_GetColor(   0 , 255 , 255 ) ;
	CharBuf.StrColor5  = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor6  = NS_GetColor( 255 ,   0 ,   0 ) ;
	CharBuf.StrColor7  = NS_GetColor( 255 , 255 ,  20 ) ;
	CharBuf.StrColor8  = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor9  = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor10 = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor11 = CharBuf.StrColor3;
	CharBuf.StrColor12 = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor13 = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor14 = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor15 = NS_GetColor( 128 , 128 , 128 ) ;
	CharBuf.StrColor16 = NS_GetColor(   0 ,   0 ,   0 ) ;

	CharBuf.StrColor17Enable = FALSE ;
	CharBuf.StrColor17 = NS_GetColor(   0 ,   0 ,   0 ) ;

	// �`��Ɏg�p����t�H���g�̓f�t�H���g�t�H���g
	CharBuf.UseFontHandle = -1 ;

	// ���̓A�N�e�B�u�n���h����������
	CharBuf.ActiveInputHandle = -1 ;
	
	// �J�[�\���_�ŏ����֌W�̏�����
	CharBuf.CBrinkFlag = FALSE ;
	CharBuf.CBrinkCount = 0 ;
	CharBuf.CBrinkWait = CURSORBRINKWAIT ;
	CharBuf.CBrinkDrawFlag = TRUE ;

	DXST_ERRORLOG_ADD( _T( "�������܂���\n" ) ) ;

	// �I��
	return 0 ;
}


// �o�b�t�@�ɃR�[�h���X�g�b�N����
extern int NS_StockInputChar( TCHAR CharCode )
{
	// �o�b�t�@����t�̏ꍇ�͂Ȃɂ����Ȃ�
	if( ( CharBuf.EdPoint + 1 == CharBuf.StPoint ) ||
		( CharBuf.StPoint == 0 && CharBuf.EdPoint == CHARBUFFER_SIZE ) )
	{
		return -1 ;
	}

	// �o�b�t�@�ɕ����R�[�h����
	CharBuf.CharBuffer[ CharBuf.EdPoint ] = CharCode ;
	CharBuf.EdPoint ++ ;
	if( CharBuf.EdPoint == CHARBUFFER_SIZE + 1 ) CharBuf.EdPoint = 0 ;

	// �I��
	return 0 ;
}

// �����R�[�h�o�b�t�@���N���A����
extern int NS_ClearInputCharBuf( void )
{
	while( NS_ProcessMessage() == 0 )
	{
		if( NS_GetInputChar( TRUE ) == 0 ) break ;
	} 

	// �X�^�[�g�|�C���^�ƃG���h�|�C���^��������
	CharBuf.EdPoint = CharBuf.StPoint = 0 ;

#ifndef DX_NON_KEYEX
	// IME���͂̏�Ԃ����Z�b�g����
	if( WinData.MainWindow )
	{
		if( CharBuf.IMEUseFlag_OSSet == TRUE && CharBuf.IMESwitch == TRUE )
		{
			HIMC Imc;

			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( WinData.MainWindow ) ;
			if( Imc )
			{
				_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
				WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc , NI_COMPOSITIONSTR ,  CPS_CANCEL , 0  );

				WinAPIData.ImmFunc.ImmReleaseContextFunc( WinData.MainWindow , Imc );
			}
		}
	}
#endif

	// �I��
	return 0 ;
}

// �����R�[�h�o�b�t�@�ɗ��܂����f�[�^����P�o�C�g���擾����
extern TCHAR NS_GetInputChar( int DeleteFlag )
{
	TCHAR RetChar ;

	// �o�b�t�@�ɕ������Ȃ������ꍇ�͂O��Ԃ�
	if( CharBuf.EdPoint == CharBuf.StPoint ) return 0 ;

	// �Ԃ������R�[�h���Z�b�g
	RetChar = CharBuf.CharBuffer[ CharBuf.StPoint ] ;

	// �L���f�[�^�X�^�[�g�|�C���^���C���N�������g
	if( DeleteFlag )
	{
		CharBuf.StPoint ++ ;
		if( CharBuf.StPoint == CHARBUFFER_SIZE + 1 ) CharBuf.StPoint = 0 ;
	}

	if( RetChar == -1 ) 
	{
		DXST_ERRORLOG_ADD( _T( "�����o�b�t�@�G���[\n" ) ) ;
		return ( TCHAR )-1 ;
	}

	// �����R�[�h��Ԃ�
	return RetChar ;
}

// �����R�[�h�o�b�t�@�ɗ��܂����f�[�^����P�������擾����
extern int NS_GetOneChar( TCHAR *CharBuffer , int DeleteFlag )
{
	int Ret ;

	CharBuffer[0] = 0 ;
	CharBuffer[1] = 0 ;

	Ret = 0 ;

	// �o�b�t�@�ɕ������Ȃ������ꍇ�͂O��Ԃ�
	if( CharBuf.EdPoint == CharBuf.StPoint ) return 0 ;

	// �P�����ڂ��擾
	CharBuffer[0] = CharBuf.CharBuffer[ CharBuf.StPoint ] ;

	Ret ++ ;

	// �L���f�[�^�X�^�[�g�|�C���^���C���N�������g
	CharBuf.StPoint ++ ;
	if( CharBuf.StPoint == CHARBUFFER_SIZE + 1 ) CharBuf.StPoint = 0 ;

	// �ŏ��̕������S�p�����̂P�o�C�g�ڂ����擾���A�����ł������ꍇ�͂Q�o�C�g�ڂ��擾
	if( NS_GetCtrlCodeCmp( CharBuffer[0] ) == 0 &&
		_TMULT( *CharBuffer, _GET_CHARSET() ) == TRUE )
	{
		// �o�b�t�@�ɕ������Ȃ������ꍇ�͂O��Ԃ�	
		if( CharBuf.EdPoint == CharBuf.StPoint )
		{
			// �P�o�C�g���o�b�t�@�|�C���^��߂�
			if( CharBuf.StPoint == 0 )
				CharBuf.StPoint = CHARBUFFER_SIZE ;
			else
				CharBuf.StPoint -- ;
			
			return 0 ;
		}

		// �Q�����ڂ��擾
		CharBuffer[1] = CharBuf.CharBuffer[ CharBuf.StPoint ] ;

		Ret ++ ;

		// �L���f�[�^�X�^�[�g�|�C���^���C���N�������g
		if( DeleteFlag )
		{
			CharBuf.StPoint ++ ;
			if( CharBuf.StPoint == CHARBUFFER_SIZE + 1 ) CharBuf.StPoint = 0 ;
		}
	}

	// �����o�b�t�@�폜�t���O���������Ă��Ȃ���΂P�o�C�g���߂�
	if( !DeleteFlag )
	{
		if( CharBuf.StPoint == 0 )
			CharBuf.StPoint = CHARBUFFER_SIZE ;
		else
			CharBuf.StPoint -- ;
	}

	// �擾�����o�C�g����Ԃ�
	return Ret ;
}

// �����R�[�h�o�b�t�@�ɗ��܂����f�[�^����P�o�C�g���擾����A�o�b�t�@�ɂȂɂ������R�[�h���Ȃ��ꍇ�̓L�[���������܂ő҂�
extern TCHAR NS_GetInputCharWait( int DeleteFlag )
{
	TCHAR RetChar = 0 ;

	// �L�[�o�b�t�@�ɕ����R�[�h�����܂�܂ő҂�
	while( NS_ProcessMessage() == 0 )
	{
		if( ( RetChar = NS_GetInputChar( DeleteFlag ) ) != 0 ) break ;
	}

	// �L�[�R�[�h��Ԃ�
	return RetChar ;
}

// �����R�[�h�o�b�t�@�ɗ��܂����f�[�^����P�������擾����A�o�b�t�@�ɉ��������R�[�h���Ȃ��ꍇ�̓L�[���������܂ő҂�
extern int NS_GetOneCharWait( TCHAR *CharBuffer , int DeleteFlag ) 
{
	int Ret = 0 ;

	// �L�[�o�b�t�@�ɕ����R�[�h�����܂�܂ő҂�
	while( NS_ProcessMessage() == 0 )
	{
		if( ( Ret = NS_GetOneChar( CharBuffer , DeleteFlag ) ) != 0 ) break ;
	}

	// �擾�����o�C�g����Ԃ�
	return Ret ;
}


#ifndef DX_NON_KEYEX

// �h�l�d���b�Z�[�W�̃R�[���o�b�N�֐�
extern LRESULT IMEProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HIMC Imc ;
	static int Flag = FALSE ;

	// �h�l�d������Ԃ������ꍇ�͂Ȃɂ������I��
/*	if( CharBuf.IMEUseFlag == FALSE )
	{
//		DefWindowProc( hWnd , message , wParam , lParam ) ;
		return 0;
	}
*/
	// �ω����������t���O�Z�b�g
	CharBuf.ChangeFlag = TRUE ;

	// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
	ResetKeyInputCursorBrinkCount() ;

	// �X�C�b�`
	switch( message )
	{
	// �h�l�d�E�C���h�E�A�N�e�B�u��ԕω�
	case WM_IME_SETCONTEXT :
//		lParam &= ~ISC_SHOWUICANDIDATEWINDOW ;
		lParam = 0;
		return DefWindowProc( hWnd , message , wParam , lParam ) ;

	// �h�l�d���͊J�n���b�Z�[�W
	case WM_IME_STARTCOMPOSITION :

		CharBuf.InputPoint = 0 ;
		CharBuf.IMEInputFlag = TRUE ;

		// �h�l�d�̎g�p��Ԃ𓾂�
		if( CharBuf.IMESwitch == FALSE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA )
		{
			CharBuf.IMESwitch = TRUE ;
		}
		return 0;

	// �h�l�d���͏I�����b�Z�[�W
	case WM_IME_ENDCOMPOSITION :
		_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
		CharBuf.IMEInputFlag = FALSE ;

		// �h�l�d�̎g�p��Ԃ𓾂�
		if( CharBuf.IMESwitch == TRUE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA )
		{
			CharBuf.IMESwitch = FALSE ;
		}
		break ;


	// �h�l�d�����ҏW�ω����b�Z�[�W
	case WM_IME_COMPOSITION :
		{
//			HWND DefHwnd = ImmGetDefaultIMEWnd( WinData.MainWindow ) ;
			HWND DefHwnd = hWnd;

			// ���̓R���e�L�X�g���擾
//			SendMessage( DefHwnd , WM_CLOSE , 0 , 0 ) ;
			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
//			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( hWnd ) ;

			// �ҏW�����ω����̏���
			if( lParam & GCS_COMPSTR )
			{
				// �ҏW��������̎擾
				_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , CharBuf.InputString , CHARBUFFER_SIZE );
			}

			// �ҏW���������莞�̏���
			if( lParam & GCS_RESULTSTR )
			{
				_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_RESULTSTR , CharBuf.InputString , CHARBUFFER_SIZE );
				// �o�b�t�@�ɃX�g�b�N
				{
					int StrLen , i ;

					StrLen = lstrlen( CharBuf.InputString ) ;
					for( i = 0 ; i < StrLen ; i ++ )
					{
						NS_StockInputChar( CharBuf.InputString[ i ] ) ;
					}
				}

				// �ҏW�������񏉊���
				_MEMSET( CharBuf.InputString, 0, sizeof( CharBuf.InputString ) );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , CharBuf.InputString , CHARBUFFER_SIZE );

				// �ϊ����̏������Z�b�g
				if( CharBuf.CandidateList )
				{
					DXFREE( CharBuf.CandidateList ) ;
					CharBuf.CandidateList = NULL ;
					CharBuf.CandidateListSize = 0 ;
				}
			}

			// �J�[�\���ʒu�ω����̏���
			if( lParam & GCS_CURSORPOS )
			{
				int OlgPoint = CharBuf.InputPoint ;
				CharBuf.InputPoint = NS_GetStringPoint2( CharBuf.InputString , WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_CURSORPOS , NULL , 0 ) ) ;

				// �ϊ����̏������Z�b�g
				if( CharBuf.CandidateList && OlgPoint != CharBuf.InputPoint )
				{
					DXFREE( CharBuf.CandidateList ) ;
					CharBuf.CandidateList = NULL ;
					CharBuf.CandidateListSize = 0 ;
				}
			}

			// ���ߏ��ω���
			if( lParam & GCS_COMPCLAUSE )
			{
				// ���ߏ��̎擾
				_MEMSET( CharBuf.ClauseData, 0, sizeof( CharBuf.ClauseData ) ) ;
				CharBuf.ClauseNum = WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPCLAUSE , ( void * )CharBuf.ClauseData , 1024 * sizeof( int ) ) / 4 ;
			}

			// ���͕��������ω���
			if( lParam & GCS_COMPATTR )
			{
				_MEMSET( CharBuf.CharAttr, 0, sizeof( CharBuf.CharAttr ) ) ;
				CharBuf.CharAttrNum = WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPATTR , ( void * )CharBuf.CharAttr , 1024 * sizeof( BYTE ) ) ;
			}

			// ���̓R���e�L�X�g�̍폜
			WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//			WinAPIData.ImmFunc.ImmReleaseContextFunc( hWnd , Imc ) ;
		}
		break;

	// �h�l�d��ԕω����b�Z�[�W
	case WM_IME_NOTIFY :

		switch( wParam )
		{
		// �h�l�d�̂n�m�A�n�e�e�ω�
		case IMN_SETOPENSTATUS :

			if( Flag ) break ;

			Flag = TRUE ;

			DefWindowProc( hWnd , message , wParam , lParam ) ;

			// �L�[��������Ă���Ԃ����Ŏ~�܂�
			{
				__int64 Time = NS_GetNowHiPerformanceCount() ;

				while( NS_ProcessMessage() == 0 && NS_GetNowHiPerformanceCount() - Time < 2000 ){}
			}

			Flag = FALSE ;

			// �h�l�d�̎g�p��Ԃ𓾂�
			{
				HWND DefHwnd = hWnd;
//				HWND DefHwnd = ImmGetDefaultIMEWnd( hWnd ) ;
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;

//				SendMessage( DefHwnd , WM_CLOSE , 0 , 0 ) ;
//				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( hWnd ) ;

				CharBuf.IMESwitch = WinAPIData.ImmFunc.ImmGetOpenStatusFunc( Imc ) ;
				WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//				WinAPIData.ImmFunc.ImmReleaseContextFunc( hWnd , Imc );
			}

			break ;


		// ���ω����̏���
		case IMN_CHANGECANDIDATE:
		case IMN_OPENCANDIDATE :
		case IMN_SETCANDIDATEPOS :
			{
				DWORD BufSize ;
//				HWND DefHwnd = ImmGetDefaultIMEWnd( hWnd ) ;
				HWND DefHwnd = hWnd;
//				int Result ;

				// ���̓R���e�L�X�g���擾
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
//				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( hWnd ) ;

				// �o�b�t�@�T�C�Y�擾
//				BufSize = ImmGetCandidateListCount( Imc , &ListSize ) ;
				BufSize = WinAPIData.ImmFunc.ImmGetCandidateListFunc( Imc , 0 , NULL , 0 ) ;
				if( BufSize != 0 )
				{
					CharBuf.CandidateListSize = ( int )BufSize ;
					if( BufSize == 0 )
					{
						WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
						DXST_ERRORLOGFMT_ADD(( _T( "�����ϊ����̃f�[�^�T�C�Y���O�ł���  DefHWND:%x IMC:%x\n" ), DefHwnd, Imc )) ;
						return 0 ;
					}

					// �o�b�t�@�p�������̊m��
					if( CharBuf.CandidateList != NULL )
							DXFREE( CharBuf.CandidateList ) ;
					if( ( CharBuf.CandidateList = ( CANDIDATELIST * )DXALLOC( BufSize ) ) == NULL )
					{
						WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
#ifndef DX_NON_LITERAL_STRING
						return DxLib_FmtError( _T( "�����ϊ�����ۑ����郁�����̊m�ۂɎ��s���܂���( �m�ۂ��悤�Ƃ����T�C�Y %d �o�C�g )" ), BufSize ) ;
#else
						return DxLib_FmtError( "" ) ;
#endif
					}

					// �f�[�^�̎擾
					if( WinAPIData.ImmFunc.ImmGetCandidateListFunc( Imc , 0 , CharBuf.CandidateList , BufSize ) == 0 )
					{
						DXFREE( CharBuf.CandidateList ) ;
						CharBuf.CandidateList = NULL ;
						WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );

						DXST_ERRORLOGFMT_ADD(( _T( "�����ϊ����̎擾�Ɏ��s���܂���  BufSize:%d\n" ), BufSize )) ;
						return 0 ;
					}
				}

				// ���̓R���e�L�X�g�̍폜
				WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//				WinAPIData.ImmFunc.ImmReleaseContextFunc( hWnd , Imc );
			}
			break ;

		// ���͌��E�C���h�E����悤�Ƃ��Ă��鎞�̏���
		case IMN_CLOSECANDIDATE :

			// �������̉��
			if( CharBuf.CandidateList )
				DXFREE( CharBuf.CandidateList ) ;
			CharBuf.CandidateList = NULL ;
			CharBuf.CandidateListSize = 0 ;
			break ;

		default :
//			return 0;
			return DefWindowProc( hWnd , message , wParam , lParam ) ;
		}
		break;
	}

	return 0 ;
}



// ��ʏ�ɓ��͒��̕������`�悷��
extern int NS_DrawIMEInputString( int x , int y , int SelectStringNum )
{
	int StrLen , FontSize , FontHandle ;
	int PointX , PointY ;
	int CPointX , CPointY ;
	int StrWidth , Width ;
	RECT DrawRect ;
	int Use3DFlag ;
	static TCHAR StringBuf[2048] ;

	if( CharBuf.IMEUseFlag_OSSet == FALSE || CharBuf.IMESwitch == FALSE ) return -1 ;

	// �g�p����t�H���g�̃n���h�����Z�b�g����
	FontHandle = CharBuf.UseFontHandle == -1 ? NS_GetDefaultFontHandle() : CharBuf.UseFontHandle ;

	// �t�H���g�T�C�Y�𓾂�
	FontSize = NS_GetFontSizeToHandle( FontHandle ) ;

	// �R�c�L���t���O�𓾂�
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// �`��\�̈�𓾂�
	NS_GetDrawArea( &DrawRect ) ;

	// ���͕����񂪂Ȃ��ꍇ�͂����ŏI��
	if( CharBuf.InputString[ 0 ] == 0 )
	{
		// �R�c�L���t���O�����ɖ߂�
		NS_SetUse3DFlag( Use3DFlag ) ;
		return 0 ;
	}

	// �`�敶����̒����𓾂�
	StrWidth = NS_GetDrawStringWidthToHandle( CharBuf.InputString , lstrlen( CharBuf.InputString ) , FontHandle ) ;

	// �`��ʒu��␳
	if( x < DrawRect.left ) x = DrawRect.left ;
	if( y < DrawRect.top  ) y = DrawRect.top ;

	// ���͕������`��
	{
		// ���͕����𕢂���`�̕`��
		NS_DrawObtainsBox( x , y , x + StrWidth , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor3 , TRUE ) ;

		// �ϊ����̑I�𕶐߂̋����\��
//		if( CharBuf.ClauseNum != 2 )
		{
			int i , Point ;

			for( i = 0 ; i < CharBuf.CharAttrNum ; i ++ )
			{
				if( CharBuf.CharAttr[ i ] == ATTR_TARGET_CONVERTED )
				{
					Point = NS_GetDrawStringWidthToHandle( CharBuf.InputString     , i , FontHandle ) ;
					Width = NS_GetDrawStringWidthToHandle( CharBuf.InputString + i , 1 , FontHandle ) ;

					NS_DrawObtainsBox( x + Point , y , x + Point + Width , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor6 , TRUE ) ;
				}

				if( _TMULT( CharBuf.InputString[ i ], _GET_CHARSET() ) )
					i ++ ;
			}
/*
			Point = NS_GetStringPoint( CharBuf.InputString , CharBuf.InputPoint ) ;	

			for( i = 0 ; i < CharBuf.ClauseNum - 1 && CharBuf.ClauseData[ i ] != Point ; i ++ ){}
			if( i != CharBuf.ClauseNum - 1 )
			{
				int Width ;

				Point = NS_GetDrawStringWidthToHandle( CharBuf.InputString , Point , FontHandle ) ;
				Width = CharBuf.ClauseData[ i + 1 ] - CharBuf.ClauseData[ i ] ;
				Width = NS_GetDrawStringWidthToHandle( &CharBuf.InputString[ CharBuf.ClauseData[ i ] ] , Width , FontHandle ) ;

				NS_DrawObtainsBox( x + Point , y , x + Point + Width , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor6 , TRUE ) ;
			}
*/
		}

		NS_DrawObtainsString( x , y , FontSize + FontSize / 10 * 3 , CharBuf.InputString , CharBuf.StrColor16 , CharBuf.StrColor17Enable ? CharBuf.StrColor17 : CharBuf.StrColor3 , FontHandle  ) ;
	}

	// �J�[�\���̂���ʒu���Z�o
	{
		StrLen = NS_GetStringPoint( CharBuf.InputString , CharBuf.InputPoint ) ;

		// �擪�����ʒu�܂ł̕����Z�o
		if( StrLen == 0 )
		{
			CPointX = 0 ;
		}
		else
		{
			// �������𓾂�
			CPointX = NS_GetDrawStringWidthToHandle( CharBuf.InputString , StrLen , FontHandle ) ;
		}
		CPointX += x ;
		CPointY = y ;
	}

	// �J�[�\����`��
	{
		// �I�𕶎��񂪑��݂���ꍇ�̂ݕ`��
		if( !CharBuf.CandidateList && CharBuf.CBrinkDrawFlag == TRUE )
		{
			// �J�[�\���̕`��
			NS_DrawObtainsBox( CPointX , CPointY , CPointX + 2 , CPointY + FontSize , FontSize + FontSize / 10 * 3  , CharBuf.StrColor4 , TRUE ) ;
		}
	}

	// ������`��
	{
		int i , StrNum , StrWidth , PointX , PointY , LineP ;

		StrNum = 0 ;
		PointX = x ;
		PointY = y ;
		LineP = FontSize / 10 ;
		for( i = 0 ; i < CharBuf.ClauseNum ; i ++ )
		{
			StrWidth = NS_GetDrawStringWidthToHandle( &CharBuf.InputString[ StrNum ] , CharBuf.ClauseData[ i ] - StrNum , FontHandle ) ;
			NS_DrawObtainsBox( PointX + LineP * 2				, PointY + FontSize + LineP , 
								PointX + StrWidth - LineP * 2	, PointY + FontSize + LineP * 2 , FontSize + FontSize / 10 * 3 , CharBuf.StrColor5 , TRUE ) ;
			PointX += StrWidth ;
			StrNum = CharBuf.ClauseData[ i ] ;
		}
	}


	// ��⃊�X�g���o�Ă���ꍇ�͂��̕`��
	if( CharBuf.CandidateList )
	{
		DWORD i ;
		RECT SelectRect ;
		int j ,h ;
		int MaxWidth ;
		int SelectionNumWidth ;

		// �Œ��I�����𒲂ׂ�
		MaxWidth = 0 ;
		j = 0 ;
		for( i = ( CharBuf.CandidateList->dwSelection / SelectStringNum ) * SelectStringNum ; j < SelectStringNum ; i ++ , j ++ )
		{
			if( i == CharBuf.CandidateList->dwCount ) break ;

			_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d:%s" ), j + 1, ( TCHAR * )( ( BYTE * )CharBuf.CandidateList + CharBuf.CandidateList->dwOffset[i] ) ) ;
			Width = NS_GetDrawStringWidthToHandle( StringBuf , lstrlen( StringBuf ) , FontHandle ) ;
			if( Width > MaxWidth ) MaxWidth = Width ;
		}
		_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d/%d" ), CharBuf.CandidateList->dwSelection + 1, CharBuf.CandidateList->dwCount ) ;
		SelectionNumWidth = NS_GetDrawStringWidthToHandle( StringBuf , lstrlen( StringBuf ) , FontHandle ) ;
		if( SelectionNumWidth > MaxWidth ) MaxWidth = SelectionNumWidth ;
		j ++ ;

		// �`��͈͂��Z�b�g
		{
			h = FontSize / 3 ;
			SETRECT( SelectRect , CPointX , CPointY + FontSize + FontSize / 2 , CPointX + MaxWidth + h * 2, CPointY + FontSize * ( 1 + j ) + FontSize / 2 + h * 2 ) ;

			// �C���v�b�g�����`��ʒu��艺�̈ʒu�ɔz�u�ł��邩����
			if( SelectRect.bottom > DrawRect.bottom )
			{
				// �o���Ȃ��ꍇ�̓C���v�b�g�����`��ʒu����̈ʒu�ɔz�u�����݂�
				if( y - FontSize * j - FontSize / 2 - h * 2 < DrawRect.top )
				{	
					// ����ł��ʖڂȏꍇ�͖�����艺�̈ʒu�ɕ`�悷��
					j = SelectRect.bottom - DrawRect.bottom ;
				}
				else
				{
					// ��̈ʒu���N�_�ɔz�u
					j = SelectRect.top - ( y - FontSize * j - FontSize / 2 - h * 2 ) ;
				}

				// ������␳
				SelectRect.bottom -= j ;
				SelectRect.top -= j ;
			}

			// �C���v�b�g�����`�敝�̂����ŕ`��\�̈�E�[�ɓ��B���Ă����ꍇ�͕␳
			if( SelectRect.right > DrawRect.right )
			{
				j = SelectRect.right - DrawRect.right ;	

				SelectRect.left -= j ;
				SelectRect.right -= j ;
			}
		}

		// �`��͈͂����œh��Ԃ�
		NS_DrawBox( SelectRect.left , SelectRect.top , SelectRect.right , SelectRect.bottom , CharBuf.StrColor12 , TRUE ) ;
		NS_DrawBox( SelectRect.left , SelectRect.top , SelectRect.right , SelectRect.bottom , CharBuf.StrColor11 , FALSE ) ;

		// ����`��
		PointX = SelectRect.left + h ;
		PointY = SelectRect.top + h ;
		j = 0 ;
		for( i = ( CharBuf.CandidateList->dwSelection / SelectStringNum ) * SelectStringNum ; j < SelectStringNum ; i ++ , j ++ )
		{
			if( i == CharBuf.CandidateList->dwCount ) break ;
			_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d:%s" ), j + 1, ( TCHAR * )( ( BYTE * )CharBuf.CandidateList + CharBuf.CandidateList->dwOffset[i] ) ) ;
			NS_DrawStringToHandle( PointX , PointY + j * FontSize , StringBuf,
				( i == CharBuf.CandidateList->dwSelection ) ? CharBuf.StrColor6 : CharBuf.StrColor1 , FontHandle ,
				( i == CharBuf.CandidateList->dwSelection ) ? CharBuf.StrColor9 : CharBuf.StrColor8 ) ;
		}
 
		// ��⑍���̓����Ԗڂ��A�̏���`�悷��
		_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d/%d" ), CharBuf.CandidateList->dwSelection + 1, CharBuf.CandidateList->dwCount ) ;
		NS_DrawStringToHandle( PointX + MaxWidth - SelectionNumWidth, PointY + j * FontSize , StringBuf, CharBuf.StrColor1 , FontHandle , CharBuf.StrColor8 ) ;

		// ��⃊�X�g�̊J�n�C���f�b�N�X�ƌ�⃊�X�g�̃y�[�W�T�C�Y��ύX
		{
			HWND DefHwnd = WinData.MainWindow;
			HIMC Imc ;

			// ���̓R���e�L�X�g���擾
			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;

			// ��⃊�X�g�̃y�[�W�T�C�Y��ύX
			WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc, NI_SETCANDIDATE_PAGESIZE, 0, SelectStringNum ) ;

			// ��⃊�X�g�̊J�n�C���f�b�N�X�̕ύX���b�Z�[�W�𑗂�
			WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc, NI_SETCANDIDATE_PAGESTART, 0, ( CharBuf.CandidateList->dwSelection / SelectStringNum ) * SelectStringNum ) ;

			// ���̓R���e�L�X�g�̍폜
			WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
		}
	}

	// �R�c�L���t���O�����ɖ߂�
	NS_SetUse3DFlag( Use3DFlag ) ;

	// �I��
	return 0 ;
}

// �h�l�d���g�p���邩�ǂ����̏�Ԃ��X�V����
extern void RefreshIMEFlag( int Always )
{
	int NewUseFlag ;

	NewUseFlag = CharBuf.IMEUseFlag || CharBuf.IMEUseFlag_System ;

	// �t���O���ȑO�Ɠ����ꍇ�͉��������I��
	if( NewUseFlag == CharBuf.IMEUseFlag_OSSet && Always == FALSE ) return ;

	// �L���t���O���Z�b�g
	if( WinAPIData.WINNLSEnableIME_Func )
		WinAPIData.WINNLSEnableIME_Func( WinData.MainWindow , NewUseFlag ) ;

	// �t���O�ۑ�
	CharBuf.IMEUseFlag_OSSet = NewUseFlag ;

	// ����FALSE�������ꍇ�͊e�탁�������������
	if( NewUseFlag == FALSE ) 
	{
		DXFREE( CharBuf.CandidateList ) ;
		CharBuf.CandidateList = NULL ;
		CharBuf.CandidateListSize = 0 ;

		_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE ) ;

		// ���͒����A�t���O���|��
		CharBuf.IMEInputFlag = FALSE ;
	}

	// �I��
	return ;
}

// �h�l�d���g�p���邩�ǂ������Z�b�g����
extern int NS_SetUseIMEFlag( int UseFlag )
{
	// �t���O�ۑ�
	CharBuf.IMEUseFlag = UseFlag ;

	// ��Ԃ��X�V
	RefreshIMEFlag() ;

	// �I��
	return 0 ;
}


// �h�l�d�œ��͂ł���ő啶������ MakeKeyInput �̐ݒ�ɍ��킹�邩�ǂ������Z�b�g����( TRUE:���킹��  FALSE:���킹�Ȃ�(�f�t�H���g) )
extern int NS_SetInputStringMaxLengthIMESync( int Flag )
{
	CharBuf.IMEInputStringMaxLengthIMESync = Flag ;

	// �I��
	return 0 ;
}

// �h�l�d�ň�x�ɓ��͂ł���ő啶������ݒ肷��( 0:�����Ȃ�  1�ȏ�:�w��̕������Ő��� )
extern int NS_SetIMEInputStringMaxLength( int Length )
{
	CharBuf.IMEInputMaxLength = Length ;

	// �I��
	return 0 ;
}

// �h�l�d���g�p��Ԃ�ύX����
static int SetIMEOpenState( int OpenFlag ) 
{
	HIMC Imc ;
	HWND DefHwnd = WinData.MainWindow;

	// ���̓R���e�L�X�g���擾
	Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;

	// �g�p��Ԃ�ύX����
	WinAPIData.ImmFunc.ImmSetOpenStatusFunc( Imc, OpenFlag == TRUE ? true : false ) ;

	// ���̓R���e�L�X�g�̍폜
	WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );

	// �I��
	return 0 ;
}

// �L�[���͂̑I��͈͂��폜����
static int KeyInputSelectAreaDelete( INPUTDATA * Input )
{
	int smin, smax, DelNum ;

	if( Input->SelectStart == -1 ) return 0 ;

	if( Input->SelectStart > Input->SelectEnd )
	{
		smin = Input->SelectEnd ;
		smax = Input->SelectStart ;
	}
	else
	{
		smin = Input->SelectStart ;
		smax = Input->SelectEnd ;
	}
	DelNum = smax - smin ;
	Input->Point = smin ;

	if( smax != Input->StrLength )
	{
		_MEMMOVE( &Input->Buffer[ smin ], &Input->Buffer[ smax ], ( Input->StrLength - smax ) * sizeof( TCHAR ) ) ;
	}
	Input->StrLength -= DelNum ;
	Input->Buffer[ Input->StrLength ] = _T( '\0' ) ;
	Input->SelectStart = -1 ;
	Input->SelectEnd = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// �I��
	return 0 ;
}


#endif

// �S�p�����A���p�������藐��钆����w��̑S�����ݕ������ł̔��p�������𓾂�
extern int NS_GetStringPoint( const TCHAR *String , int Point )
{
	int i , p ;

	p = 0 ;
	for( i = 0 ; i < Point ; i ++ )
	{
		if( *( String + p ) == _T( '\0' ) ) break ; 
		if( _TMULT( String[p], _GET_CHARSET() ) == FALSE )	p ++ ;
		else												p += 2 ;
	}

	return p ;
}

// �S�p�����A���p�������藐��钆����w��̔��p�������ł̑S�p�������𓾂�
extern int NS_GetStringPoint2( const TCHAR *String , int Point )
{
	int i , p ;

	p = 0 ;
	i = 0 ;
	while( i < Point )
	{
		if( *( String + i ) == _T( '\0' ) ) break ; 
		if( _TMULT( String[i], _GET_CHARSET() ) == FALSE )	i ++ ;
		else												i += 2 ;

		p ++ ;
	}

	return p ;
}

// �S�p�����A���p�������藐��钆���當�������擾����
extern int NS_GetStringLength( const TCHAR *String )
{
	int i , p ;

	p = 0 ;
	i = 0 ;
	while( *( String + i ) != _T( '\0' ) )
	{
		if( _TMULT( String[i], _GET_CHARSET() ) == FALSE )	i ++ ;
		else												i += 2 ;

		p ++ ;
	}

	return p ;
}

#ifndef DX_NON_FONT

// �K��̈�Ɏ��߂��������ŕ������`��
extern int NS_DrawObtainsString(
	int x , int y ,
	int AddY ,
	const TCHAR *String ,
	int StrColor ,
	int StrEdgeColor ,
	int FontHandle,
	int SelectBackColor ,
	int SelectStrColor ,
	int SelectStrEdgeColor ,
	int SelectStart ,
	int SelectEnd )
{
	int PointX , PointY ;
	int Width , StrWidth ;
	int Use3DFlag ;
	RECT DrawRect , Rect ;

	// FontHandle �� -1 �̏ꍇ�̓f�t�H���g�̃t�H���g���g�p����
	if( FontHandle == -1 ) FontHandle = NS_GetDefaultFontHandle() ;

	// �R�c�L���t���O�𓾂�
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// �`��\�̈�𓾂�
	NS_GetDrawArea( &DrawRect ) ;

	// �`��ʒu��␳
	if( x < DrawRect.left ) x = DrawRect.left ;
	if( y < DrawRect.top  ) y = DrawRect.top ;

	while( x > DrawRect.right )
	{
		x -= DrawRect.right - DrawRect.left ;
		y += AddY ;
	}

	// ���͕������`��
	if( SelectStart == -1 )
	{
		// �`�敶����̒����𓾂�
		StrWidth = NS_GetDrawStringWidthToHandle( String , lstrlen( String ) , FontHandle ) ;

		Width = StrWidth ;
		PointX = x ;
		PointY = y ;
		for(;;)
		{
			NS_DrawStringToHandle( PointX , PointY , String , StrColor , FontHandle , StrEdgeColor ) ;

			SETRECT( Rect , PointX , PointY , PointX + StrWidth , PointY + AddY ) ;
			RectClipping( &Rect , &DrawRect ) ;

			Width -= Rect.right - Rect.left ;

			if( Width > 0 )
			{
				PointX = DrawRect.left - ( StrWidth - Width ) ;
				PointY += AddY ;
			}
			else break ;
		}
	}
	else
	{
		int i, Num, StrLen, smin, smax ;
		TCHAR TempBuf[ 3 ] ;

		if( SelectStart < SelectEnd )
		{
			smax = SelectEnd ;
			smin = SelectStart ;
		}
		else
		{
			smax = SelectStart ;
			smin = SelectEnd ;
		}

		StrLen = lstrlen( String ) ;
		PointX = x ;
		PointY = y ;
		for( i = 0 ; i < StrLen ; )
		{
			if( _TMULT( String[ i ], _GET_CHARSET() ) )
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = String[ i + 1 ] ;
				TempBuf[ 2 ] = 0 ;
				Num = 2 ;
			}
			else
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = 0 ;
				Num = 1 ;
			}

			StrWidth = NS_GetDrawStringWidthToHandle( TempBuf, lstrlen( TempBuf ), FontHandle ) ;
			if( i >= smin && i < smax )
			{
				NS_DrawBox( PointX, PointY, PointX + StrWidth, PointY + NS_GetFontSizeToHandle( FontHandle ), SelectBackColor, TRUE ) ; 
				NS_DrawStringToHandle( PointX, PointY, TempBuf, SelectStrColor, FontHandle, SelectStrEdgeColor ) ;
			}
			else
			{
				NS_DrawStringToHandle( PointX, PointY, TempBuf, StrColor, FontHandle, StrEdgeColor ) ;
			}

			if( PointX + StrWidth > DrawRect.right )
			{
				PointX = DrawRect.left - ( DrawRect.right - PointX ) ;
				PointY += AddY ;
			}
			else
			{
				PointX += StrWidth ;
				i += Num ;
			}
		}
	}

	// �R�c�`��t���O�����ɖ߂�
	NS_SetUse3DFlag( Use3DFlag ) ;

	// �I��
	return 0 ;
}

// �`��\�̈�Ɏ��܂�悤�ɉ��s���Ȃ��當�����`��( �N���b�v�������P�� )
extern int NS_DrawObtainsString_CharClip(
	int x, int y,
	int AddY,
	const TCHAR *String,
	int StrColor,
	int StrEdgeColor,
	int FontHandle,
	int SelectBackColor,
	int SelectStrColor,
	int SelectStrEdgeColor,
	int SelectStart,
	int SelectEnd
)
{
	int PointX , PointY ;
	int StrWidth ;
	int Use3DFlag ;
	RECT DrawRect ;
	int i, Num, StrLen ;
	TCHAR TempBuf[ 3 ] ;

	// FontHandle �� -1 �̏ꍇ�̓f�t�H���g�̃t�H���g���g�p����
	if( FontHandle == -1 ) FontHandle = NS_GetDefaultFontHandle() ;

	// �R�c�L���t���O�𓾂�
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// �`��\�̈�𓾂�
	NS_GetDrawArea( &DrawRect ) ;

	// �`��͈͂ɕ��������ꍇ�͉��������ɏI��
	if( DrawRect.left == DrawRect.right ||
		DrawRect.top  == DrawRect.bottom )
	{
		return 0 ;
	}

	// �`��ʒu��␳
	if( x < DrawRect.left ) x = DrawRect.left ;
	if( y < DrawRect.top  ) y = DrawRect.top ;

	while( x > DrawRect.right )
	{
		x -= DrawRect.right - DrawRect.left ;
		y += AddY ;
	}

	StrLen = lstrlen( String ) ;

	// ���͕������`��
	if( SelectStart == -1 )
	{
		PointX = x ;
		PointY = y ;
		for( i = 0 ; i < StrLen ; )
		{
			if( _TMULT( String[ i ], _GET_CHARSET() ) )
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = String[ i + 1 ] ;
				TempBuf[ 2 ] = 0 ;
				Num = 2 ;
			}
			else
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = 0 ;
				Num = 1 ;
			}

			StrWidth = NS_GetDrawStringWidthToHandle( TempBuf, Num, FontHandle ) ;
			if( PointX + StrWidth > DrawRect.right )
			{
				PointX = x ;
				PointY += AddY ;
			}

			NS_DrawStringToHandle( PointX, PointY, TempBuf, StrColor, FontHandle, StrEdgeColor ) ;

			PointX += StrWidth ;
			i += Num ;
		}
	}
	else
	{
		int smin, smax ;

		if( SelectStart < SelectEnd )
		{
			smax = SelectEnd ;
			smin = SelectStart ;
		}
		else
		{
			smax = SelectStart ;
			smin = SelectEnd ;
		}

		PointX = x ;
		PointY = y ;
		for( i = 0 ; i < StrLen ; )
		{
			if( _TMULT( String[ i ], _GET_CHARSET() ) )
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = String[ i + 1 ] ;
				TempBuf[ 2 ] = 0 ;
				Num = 2 ;
			}
			else
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = 0 ;
				Num = 1 ;
			}

			StrWidth = NS_GetDrawStringWidthToHandle( TempBuf, Num, FontHandle ) ;
			if( PointX + StrWidth > DrawRect.right )
			{
				PointX = x ;
				PointY += AddY ;
			}

			if( i >= smin && i < smax )
			{
				NS_DrawBox( PointX, PointY, PointX + StrWidth, PointY + NS_GetFontSizeToHandle( FontHandle ), SelectBackColor, TRUE ) ; 
				NS_DrawStringToHandle( PointX, PointY, TempBuf, SelectStrColor, FontHandle, SelectStrEdgeColor ) ;
			}
			else
			{
				NS_DrawStringToHandle( PointX, PointY, TempBuf, StrColor, FontHandle, StrEdgeColor ) ;
			}

			PointX += StrWidth ;
			i += Num ;
		}
	}

	// �R�c�`��t���O�����ɖ߂�
	NS_SetUse3DFlag( Use3DFlag ) ;

	// �I��
	return 0 ;
}

#endif // DX_NON_FONT

// �K��̈�Ɏ��߂��������ŋ�`��`�� 
extern int NS_DrawObtainsBox( int x1 , int y1 , int x2 , int y2 , int AddY , int Color , int FillFlag )
{
	int AllWidth , PointX , PointY ;
	int Width , Height ;
	int Use3DFlag ;
	RECT DrawRect , Rect ;

	// �R�c�L���t���O�𓾂�
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// �`�悷�钷���ƍ����𓾂�
	AllWidth = x2 - x1 ;
	Height = y2 - y1 ;

	// �`��\�̈�𓾂�
	NS_GetDrawArea( &DrawRect ) ;

	// �`��ʒu��␳
	if( x1 < DrawRect.left ){ x2 += DrawRect.left - x1 ; x1 = DrawRect.left ; }
	if( y1 < DrawRect.top  ){ y2 += DrawRect.top  - y1 ; y1 = DrawRect.top  ; }

	while( x1 > DrawRect.right )
	{
		x1 -= DrawRect.right - DrawRect.left ;
		y1 += AddY ;
	}

	// ���͕������`��
	Width = AllWidth ;
	PointX = x1 ;
	PointY = y1 ;
	for(;;)
	{
		NS_DrawBox( PointX , PointY , PointX + AllWidth , PointY + Height , Color , FillFlag ) ;

		SETRECT( Rect , PointX , PointY , PointX + AllWidth , PointY + AddY ) ;
		RectClipping( &Rect , &DrawRect ) ;

		Width -= Rect.right - Rect.left ;

		if( Width > 0 )
		{
			PointX = DrawRect.left - ( AllWidth - Width ) ;
			PointY += AddY ;
		}
		else break ;
	}

	// �R�c�`��t���O�����ɖ߂�
	NS_SetUse3DFlag( Use3DFlag ) ;

	// �I��
	return 0 ;
}



// �A�X�L�[�R���g���[���R�[�h�����ׂ�
extern int NS_GetCtrlCodeCmp( TCHAR Char ) 
{
	int i ;

	for( i = 0 ; CtrlCode[ i ][ 0 ] != 0  &&  ( TCHAR )CtrlCode[ i ][ 1 ] != Char ; i ++ ){}
	
	// ���ʂ�Ԃ�
	return CtrlCode[ i ][ 0 ] != 0 ;
}

// �h�l�d�ɕω������������A�t���O�̎擾
extern int GetIMEChangeFlag( void )
{
	int Flag = CharBuf.ChangeFlag ;

	CharBuf.ChangeFlag = FALSE ;
	return Flag ;
}



#ifndef DX_NON_KEYEX

// ������̓��͎擾
extern int NS_InputStringToCustom( int x , int y , int BufLength , TCHAR *StrBuffer , int CancelValidFlag , int SingleCharOnlyFlag , int NumCharOnlyFlag, int DoubleCharOnlyFlag )
{
	int DrawScreen ;
	int ScreenGraphFront, ScreenGraphBack, Use3DFlag , UseSysMemFlag ;
	int EndFlag = 0 ;
	RECT DrawRect ;
	int FontSize ;
	int DrawWidth , DrawHeight ;
//	TCHAR *Buffer ;
	int Red , Green , Blue ;
	int InputHandle ;
	int ScreenWidth, ScreenHeight ;
	MATRIX ViewMatrix, ProjectionMatrix, ViewportMatrix ;
	MATRIX WorldMatrix ;
	int ProjectionMatrixMode ;
	float ProjFov ;
	float ProjSize ;
	float ProjNear ;
	float ProjFar ;
	float ProjDotAspect ;
	MATRIX ProjMatrix ;

	// �e��f�[�^�ۑ�
	Use3DFlag = GetUse3DFlag() ;
	UseSysMemFlag = GetUseSystemMemGraphCreateFlag() ;
	DrawScreen = GetActiveGraph() ;
	GetTransColor( &Red , &Green , &Blue ) ;
	ViewMatrix           = GBASE.ViewMatrix ;
	ProjectionMatrixMode = GBASE.ProjectionMatrixMode ;
	ProjFov              = GBASE.ProjFov ;
	ProjSize             = GBASE.ProjSize ;
	ProjMatrix           = GBASE.ProjMatrix ;
	ProjNear             = GBASE.ProjNear ;
	ProjFar              = GBASE.ProjFar ;
	ProjDotAspect        = GBASE.ProjDotAspect ;
	ProjectionMatrix     = GBASE.ProjectionMatrix ;
	ViewportMatrix       = GBASE.ViewportMatrix ;
	WorldMatrix          = GBASE.WorldMatrix ;
	
	// ��ʂ̑傫���𓾂�
	GetDrawScreenSize( &ScreenWidth, &ScreenHeight ) ;

	// �`��̈�𓾂�
	GetDrawArea( &DrawRect ) ;

	// �u�q�`�l�̎g�p�A�R�c�`��@�\�̎g�p�ύX
	SetUseSystemMemGraphCreateFlag( CheckFontChacheToTextureFlag( GetDefaultFontHandle() ) == TRUE ? FALSE : TRUE ) ;
	SetUse3DFlag( FALSE ) ;

	// ���ߐF��^�����ɂ���
	SetTransColor( 0 , 0 , 0 ) ;

	// �h�l�d��L���ɂ���
	CharBuf.IMEUseFlag_System = TRUE ;
	RefreshIMEFlag() ;

	// ��ʂ�ۑ����邽�߂̃��������m��
	DrawWidth = DrawRect.right - DrawRect.left ;
	DrawHeight = DrawRect.bottom - DrawRect.top ; 
	ScreenGraphFront = MakeGraph( ScreenWidth, ScreenHeight ) ;
	ScreenGraphBack = MakeGraph( ScreenWidth, ScreenHeight ) ;
	SetDrawScreen( DX_SCREEN_FRONT ) ;
	SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;
	GetDrawScreenGraph( 0, 0, ScreenWidth, ScreenHeight, ScreenGraphFront ) ;
	SetDrawScreen( DX_SCREEN_BACK ) ;
	SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;
	GetDrawScreenGraph( 0, 0, ScreenWidth, ScreenHeight, ScreenGraphBack ) ;

	// �������͗p�o�b�t�@�̊m��
//	if( ( Buffer = ( TCHAR * )DXCALLOC( BufLength + 1 ) ) == NULL ) return -1 ;

	// ���̓n���h���𓾂�
	if( ( InputHandle = MakeKeyInput( BufLength , CancelValidFlag , SingleCharOnlyFlag , NumCharOnlyFlag, DoubleCharOnlyFlag ) ) != -1 )
	{
		// ���͂��A�N�e�B�u�ɂ���
		SetActiveKeyInput( InputHandle ) ;

		// ���͕��������O�ɂ���
		FontSize = GetFontSize() ;

		// ���W�̕␳
//		x -= DrawRect.left ;
//		y -= DrawRect.top ;

		// �P��`��
		DrawKeyInputString( x , y , InputHandle ) ;

		// �`���𗠉�ʂ�
		SetDrawScreen( DX_SCREEN_BACK ) ;
		while( ProcessMessage() == 0 )
		{
			// ���͂��I�����Ă���ꍇ�͏I��
			EndFlag = CheckKeyInput( InputHandle ) ;
			if( EndFlag ) break ;

			// ���͕����̍X�V
			{
				// ���Ɖ�ʂ̕`��
				DrawGraph( 0 , 0 , ScreenGraphFront , FALSE ) ;

				// �h�l�d���̓��[�h�̕`��
				DrawKeyInputModeString( DrawRect.right , DrawRect.bottom ) ; 

				// ���͏�Ԃ̕`��
				SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;
				DrawKeyInputString( x , y , InputHandle ) ;
				SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;

				// �\��ʂɓ]��
				ScreenFlip() ;
			}
		}
	}

	// �E�C���h�E������ꂽ�ꍇ�́|�P��Ԃ�
	if( ProcessMessage() == -1 ) return -1;

	// �L�����Z������Ă��Ȃ��ꍇ�͕�������擾
	if( EndFlag == 1 )
	{
		GetKeyInputString( StrBuffer , InputHandle ) ;
	}

	// ��ʂ����ɂ��ǂ�
	SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;
	DrawGraph( 0, 0, ScreenGraphBack, FALSE ) ;
	SetDrawScreen( DX_SCREEN_FRONT ) ;
	DrawGraph( 0, 0, ScreenGraphFront, FALSE ) ;

	// �O���t�B�b�N�f�[�^������
	DeleteKeyInput( InputHandle ) ;
	DeleteGraph( ScreenGraphFront ) ;
	DeleteGraph( ScreenGraphBack ) ;

	// �f�[�^�����ɖ߂�
	SetDrawScreen( DrawScreen ) ;
	SetUse3DFlag( Use3DFlag ) ;
	SetUseSystemMemGraphCreateFlag( UseSysMemFlag ) ;
	SetTransColor( Red , Green , Blue ) ;
	SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;

	SetTransformToWorld( &WorldMatrix );
	SetTransformToView( &ViewMatrix );
	switch( ProjectionMatrixMode )
	{
	case 0 :	// ���ߖ@
		SetupCamera_Perspective( ProjFov ) ;
		break ;

	case 1 :	// ���ˉe
		SetupCamera_Ortho( ProjSize ) ;
		break ;

	case 2 :	// �s�񒼎w��
		SetupCamera_ProjectionMatrix( ProjMatrix ) ;
		break ;
	}
	SetCameraNearFar( ProjNear, ProjFar ) ;
	SetCameraDotAspect( ProjDotAspect ) ;
//	SetTransformToProjection( &ProjectionMatrix );
	SetTransformToViewport( &ViewportMatrix );

	// �I��
	return EndFlag ;
}


// ������̓��͎擾
extern int NS_KeyInputString( int x , int y , int CharMaxLength , TCHAR *StrBuffer , int CancelValidFlag )
{
	return NS_InputStringToCustom( x , y , CharMaxLength , StrBuffer , CancelValidFlag , FALSE , FALSE, FALSE ) ;
}

// ���p������݂̂̓��͎擾
extern int NS_KeyInputSingleCharString( int x , int y , int CharMaxLength , TCHAR *StrBuffer , int CancelValidFlag )
{
	return NS_InputStringToCustom( x , y, CharMaxLength , StrBuffer , CancelValidFlag , TRUE , FALSE, FALSE ) ;
}

// ���l�̓���
extern int NS_KeyInputNumber( int x , int y , int MaxNum , int MinNum , int CancelValidFlag )
{
	int Num ;
	TCHAR Buffer[ 50 ] ;
	int StrLen , hr ;

	// �ő啶���񕝂��擾
	{
		int MaxLen , MinLen ;

		_TSPRINTF( _DXWTP( Buffer ), _DXWTR( "%d" ), MaxNum ) ;		MaxLen = lstrlen( Buffer ) ;
		_TSPRINTF( _DXWTP( Buffer ), _DXWTR( "%d" ), MinNum ) ;		MinLen = lstrlen( Buffer ) ;

		StrLen = MaxLen > MinLen ? MaxLen : MinLen ;
	}

	// �������擾
	hr = InputStringToCustom( x , y , StrLen , Buffer , CancelValidFlag , TRUE , TRUE ) ;
	if( hr == -1 ) return MinNum - 1 ;
	if( hr == 2 ) return MaxNum + 1 ;

	// ���l�ɕϊ�
	Num = _TTOI( _DXWTP( Buffer ) ) ;

	// �K��͈͂̒��Ɏ擾�������l�����Ă͂܂�Ȃ��Ԃ͌J��Ԃ�
	while( ProcessMessage() == 0 && ( Num > MaxNum || Num < MinNum ) )
	{
		hr = InputStringToCustom( x , y , StrLen , Buffer , CancelValidFlag , TRUE , TRUE ) ;
		if( hr == -1 ) return MinNum - 1 ;
		if( hr == 2 ) return MaxNum + 1 ;

		// ���l�ɕϊ�
		Num = _TTOI( _DXWTP( Buffer ) ) ;
	}

	// �I��
	return Num ;
}

// IME�̓��̓��[�h������̎擾
extern int NS_GetIMEInputModeStr( TCHAR *GetBuffer ) 
{
	HIMC	Imc ;
	DWORD	InputState , SentenceState , Buf ;
	TCHAR	*SelectStr ;
//	int		i ;
/*	DWORD	InputModePal[] =	{
IME_CMODE_ALPHANUMERIC , IME_CMODE_NATIVE , IME_CMODE_KATAKANA ,IME_CMODE_LANGUAGE , 
IME_CMODE_FULLSHAPE ,IME_CMODE_ROMAN , IME_CMODE_CHARCODE , IME_CMODE_HANJACONVERT , 
IME_CMODE_SOFTKBD , IME_CMODE_NOCONVERSION,IME_CMODE_EUDC , IME_CMODE_SYMBOL , 
IME_CMODE_FIXED  	} ;
	int	
IME_CMODE_ALPHANUMERICp ,IME_CMODE_NATIVEp , IME_CMODE_KATAKANAp,IME_CMODE_LANGUAGEp ,
IME_CMODE_FULLSHAPEp ,IME_CMODE_ROMANp ,IME_CMODE_CHARCODEp, IME_CMODE_HANJACONVERTp, 
IME_CMODE_SOFTKBDp ,IME_CMODE_NOCONVERSIONp,IME_CMODE_EUDCp ,IME_CMODE_SYMBOLp, 
IME_CMODE_FIXEDp ;
*/
	// �h�l�d���g���Ă��Ȃ��Ƃ���-1��Ԃ�
	if( CharBuf.IMESwitch == FALSE ) return -1 ;

	// ���̓R���e�L�X�g���擾
	HWND DefHwnd = WinData.MainWindow;
	Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
//	Imc = WinAPIData.ImmFunc.ImmGetContextFunc( WinData.MainWindow ) ;

	// ���̓��[�h�𓾂�
	WinAPIData.ImmFunc.ImmGetConversionStatusFunc( Imc , &InputState , &SentenceState ) ;

/*	IME_CMODE_ALPHANUMERICp = ( ( InputState & IME_CMODE_ALPHANUMERIC ) == IME_CMODE_ALPHANUMERIC ) ;
	IME_CMODE_NATIVEp = ( ( InputState & IME_CMODE_NATIVE ) == IME_CMODE_NATIVE ) ;
	IME_CMODE_KATAKANAp = ( ( InputState & IME_CMODE_KATAKANA ) == IME_CMODE_KATAKANA ) ;
	IME_CMODE_LANGUAGEp = ( ( InputState & IME_CMODE_LANGUAGE ) == IME_CMODE_LANGUAGE ) ;
	IME_CMODE_FULLSHAPEp = ( ( InputState & IME_CMODE_FULLSHAPE ) == IME_CMODE_FULLSHAPE ) ;
	IME_CMODE_ROMANp = ( ( InputState & IME_CMODE_ROMAN ) == IME_CMODE_ROMAN ) ;
	IME_CMODE_CHARCODEp = ( ( InputState & IME_CMODE_CHARCODE ) == IME_CMODE_CHARCODE ) ;
	IME_CMODE_HANJACONVERTp = ( ( InputState & IME_CMODE_HANJACONVERT ) == IME_CMODE_HANJACONVERT ) ;
	IME_CMODE_SOFTKBDp = ( ( InputState & IME_CMODE_SOFTKBD ) == IME_CMODE_SOFTKBD ) ;
	IME_CMODE_NOCONVERSIONp = ( ( InputState & IME_CMODE_NOCONVERSION ) == IME_CMODE_NOCONVERSION ) ;
	IME_CMODE_EUDCp = ( ( InputState & IME_CMODE_EUDC ) == IME_CMODE_EUDC ) ;
	IME_CMODE_SYMBOLp = ( ( InputState & IME_CMODE_SYMBOL ) == IME_CMODE_SYMBOL ) ;
	IME_CMODE_FIXEDp = ( ( InputState & IME_CMODE_FIXED ) == IME_CMODE_FIXED ) ;
*/
	// ���f
	Buf = InputState & ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA ) ;
	if( Buf == ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA ) )
	{
		SelectStr = _T( "�S�p�J�^�J�i" ) ;
	}
	else
	{
		Buf = InputState & ( IME_CMODE_NATIVE | IME_CMODE_KATAKANA ) ;
		if( Buf == ( IME_CMODE_NATIVE | IME_CMODE_KATAKANA ) )
		{
			SelectStr = _T( "���p�J�^�J�i" ) ;
		}
		else
		{
			Buf = InputState & ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE ) ;
			if( Buf == ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE ) )
			{
				SelectStr = _T( "�S�p�Ђ炪��" ) ;
			}
			else
			{
				Buf = InputState & IME_CMODE_FULLSHAPE ;
				if( Buf == IME_CMODE_FULLSHAPE )
				{
					SelectStr = _T( "�S�p�p��" ) ;
				}
				else
				{
					Buf = InputState & IME_CMODE_ALPHANUMERIC ;
					if( Buf == IME_CMODE_ALPHANUMERIC )
					{
						SelectStr = _T( "���p�p��" ) ;
					}
					else
					{
						SelectStr = _T( "�s���ȓ��̓��[�h" ) ;
					}
				}
			}
		}
	}

	lstrcpy( GetBuffer , SelectStr ) ;

/*
	// ���f
	for( i = 0 ; InputModeTable[ i ].InputState != 0 && InputModeTable[ i ].InputState != InputState ; i ++ ){}
	if( InputModeTable[ i ].InputState == 0 )
	{
		_STRCPY( GetBuffer , "�s���ȓ��̓��[�h" ) ;
	}
	else
	{
		_STRCPY( GetBuffer , InputModeTable[ i ].InputName ) ;
	}
*/
	// ���̓R���e�L�X�g�̍폜
	WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//	WinAPIData.ImmFunc.ImmReleaseContextFunc( WinData.MainWindow , Imc );

	// �I��
	return 0 ;
}

// InputString�֐��g�p���̕����̊e�F��ύX����
extern int NS_SetKeyInputStringColor(
	ULONGLONG NmlStr ,				// ���͕�����̐F
	ULONGLONG NmlCur ,				// �h�l�d��g�p���̃J�[�\���̐F
	ULONGLONG IMEStrBack ,			// �h�l�d�g�p���̓��͕�����̔w�i�̐F
	ULONGLONG IMECur ,				// �h�l�d�g�p���̃J�[�\���̐F
	ULONGLONG IMELine ,				// �h�l�d�g�p���̕ϊ�������̉���
	ULONGLONG IMESelectStr ,		// �h�l�d�g�p���̑I��Ώۂ̕ϊ���╶����̐F
	ULONGLONG IMEModeStr  ,			// �h�l�d�g�p���̓��̓��[�h������̐F(�u�S�p�Ђ炪�ȁv��)
	ULONGLONG NmlStrE ,				// ���͕�����̉��̐F
	ULONGLONG IMESelectStrE ,		// �h�l�d�g�p���̑I��Ώۂ̕ϊ���╶����̉��̐F
	ULONGLONG IMEModeStrE	,		// �h�l�d�g�p���̓��̓��[�h������̉��̐F
	ULONGLONG IMESelectWinE ,		// �h�l�d�g�p���̕ϊ����E�C���h�E�̉��̐F
	ULONGLONG IMESelectWinF	,		// �h�l�d�g�p���̕ϊ����E�C���h�E�̉��n�̐F
	ULONGLONG SelectStrBackColor ,	// �I�����ꂽ���͕�����̔w�i�̐F
	ULONGLONG SelectStrColor ,		// �I�����ꂽ���͕�����̐F
	ULONGLONG SelectStrEdgeColor,	// �I�����ꂽ���͕�����̉��̐F
	ULONGLONG IMEStr,				// �h�l�d�g�p���̓��͕�����̐F
	ULONGLONG IMEStrE				// �h�l�d�g�p���̓��͕�����̉��̐F
	)
{
	if( NmlStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor1) = *((DWORD *)&NmlStr) ;
	}

	if( NmlCur != -1 )
	{
		*((DWORD *)&CharBuf.StrColor2) = *((DWORD *)&NmlCur) ;
	}

	if( IMEStrBack != -1 )
	{
		*((DWORD *)&CharBuf.StrColor3) = *((DWORD *)&IMEStrBack) ;
	}

	if( IMECur != -1 )
	{
		*((DWORD *)&CharBuf.StrColor4) = *((DWORD *)&IMECur) ;
	}

	if( IMELine != -1 )
	{
		*((DWORD *)&CharBuf.StrColor5) = *((DWORD *)&IMELine) ;
	}

	if( IMESelectStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor6) = *((DWORD *)&IMESelectStr) ;
	}

	if( IMEModeStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor7) = *((DWORD *)&IMEModeStr) ;
	}

	if( NmlStrE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor8) = *((DWORD *)&NmlStrE) ;
	}

	if( IMESelectStrE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor9) = *((DWORD *)&IMESelectStrE) ;
	}

	if( IMEModeStrE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor10) = *((DWORD *)&IMEModeStrE) ;
	}

	if( IMESelectWinE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor11) = *((DWORD *)&IMESelectWinE) ;
	}
	else
	{
		if( IMEStrBack != -1 )
			CharBuf.StrColor11 = CharBuf.StrColor3;
	}

	if( IMESelectWinF != -1 )
	{
		*((DWORD *)&CharBuf.StrColor12) = *((DWORD *)&IMESelectWinF) ;
	}
	else
	{
		CharBuf.StrColor12 = NS_GetColor( 0, 0, 0 );
	}

	if( IMESelectWinF != -1 )
	{
		*((DWORD *)&CharBuf.StrColor12) = *((DWORD *)&IMESelectWinF) ;
	}
	else
	{
		CharBuf.StrColor12 = NS_GetColor( 0, 0, 0 );
	}

	if( SelectStrBackColor != -1 )
	{
		*((DWORD *)&CharBuf.StrColor13) = *((DWORD *)&SelectStrBackColor) ;
	}
	else
	{
		CharBuf.StrColor13 = NS_GetColor( 255, 255, 255 );
	}

	if( SelectStrColor != -1 )
	{
		*((DWORD *)&CharBuf.StrColor14) = *((DWORD *)&SelectStrColor) ;
	}
	else
	{
		CharBuf.StrColor14 = NS_GetColor( 0, 0, 0 );
	}

	if( SelectStrEdgeColor != -1 )
	{
		*((DWORD *)&CharBuf.StrColor15) = *((DWORD *)&SelectStrEdgeColor) ;
	}
	else
	{
		CharBuf.StrColor15 = NS_GetColor( 128, 128, 128 );
	}

	if( IMEStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor16) = *((DWORD *)&IMEStr) ;
	}

	if( IMEStrE != -1 )
	{
		CharBuf.StrColor17Enable = TRUE ;
		*((DWORD *)&CharBuf.StrColor17) = *((DWORD *)&IMEStrE) ;
	}

	// �I��
	return 0 ;
}

// �L�[���͕�����`��֘A�Ŏg�p����t�H���g�̃n���h����ύX����(-1�Ńf�t�H���g�̃t�H���g�n���h��)
extern int NS_SetKeyInputStringFont( int FontHandle )
{
	CharBuf.UseFontHandle = FontHandle ;

	// �I��
	return 0 ;
}

// ���̓��[�h�������`�悷��
extern int NS_DrawKeyInputModeString( int x , int y )
{
	RECT DrawRect ;
	int FontSize, FontHandle ;

	FontHandle = CharBuf.UseFontHandle == -1 ? NS_GetDefaultFontHandle() : CharBuf.UseFontHandle ;
	FontSize = NS_GetFontSizeToHandle( FontHandle ) ;
	NS_GetDrawArea( &DrawRect ) ;

	// ���̓��[�h��`��
	if( CharBuf.IMESwitch )
	{
		TCHAR InputModeStr[ 20 ] ;
		int Width ;

		// ������擾
		if( NS_GetIMEInputModeStr( InputModeStr ) != -1 )
		{
			// ���擾
			Width = NS_GetDrawStringWidthToHandle( InputModeStr , lstrlen( InputModeStr ) , FontHandle ) ;

			if( Width    + x > DrawRect.right  ) x = DrawRect.right  - Width ;
			if( FontSize + y > DrawRect.bottom ) y = DrawRect.bottom - ( FontSize + 2 ) ;
			if( x < DrawRect.left ) x = DrawRect.left ;
			if( y < DrawRect.top  ) y = DrawRect.top ;

			// �`��
			NS_DrawStringToHandle( x , y , InputModeStr , CharBuf.StrColor7 , FontHandle , CharBuf.StrColor10 )  ;
		}
	}

	// �I��
	return 0 ;
}

// �L�[���̓f�[�^������
extern int NS_InitKeyInput( void )
{
	int i ;
	INPUTDATA * Input ;

	Input = CharBuf.InputData ;
	for( i = 0 ; i < MAX_INPUT_NUM ; i ++, Input ++ )
	{
		if( Input->UseFlag == TRUE )
		{
			NS_DeleteKeyInput( i | DX_HANDLETYPE_MASK_KEYINPUT | ( Input->ID << DX_HANDLECHECK_ADDRESS ) ) ;
		}
	}

	// �h�l�d�̓��͏��擾�p�Ɋm�ۂ��Ă��郁����������ꍇ�͉��
	if( CharBuf.IMEInputData )
	{
		DXFREE( CharBuf.IMEInputData ) ;
		CharBuf.IMEInputData = NULL ;
	}

	// �A�N�e�B�u�n���h����-1�ɂ���
	CharBuf.ActiveInputHandle = -1 ;

	// �I��
	return 0 ;
}

// �V�����L�[���̓f�[�^�̍쐬
extern int NS_MakeKeyInput( int MaxStrLength , int CancelValidFlag , int SingleCharOnlyFlag , int NumCharOnlyFlag, int DoubleCharOnlyFlag )
{
	int i, Result ;
	INPUTDATA * Input ;

	// �g���Ă��Ȃ��L�[���̓f�[�^��T��
	for( i = 0 ; i != MAX_INPUT_NUM && CharBuf.InputData[ i ].UseFlag ; i ++ ){}
	if( i == MAX_INPUT_NUM ) return -1 ;
	Input = &CharBuf.InputData[ i ] ;

	// �f�[�^�̏�����
	_MEMSET( Input, 0, sizeof( *Input ) ) ;
	if( ( Input->Buffer	= ( TCHAR * )DXCALLOC( ( MaxStrLength + 1 ) * sizeof( TCHAR ) ) ) == NULL ) return -1 ;
	Input->MaxStrLength			= MaxStrLength ;
	Input->CancelValidFlag		= CancelValidFlag ;
	Input->SingleCharOnlyFlag	= SingleCharOnlyFlag ;
	Input->NumCharOnlyFlag		= NumCharOnlyFlag ;
	Input->DoubleCharOnlyFlag	= DoubleCharOnlyFlag ;
	Input->UseFlag				= TRUE ;
	Input->ID					= CharBuf.HandleID ;
	Input->SelectStart			= -1 ;
	Input->SelectEnd			= -1 ;
	CharBuf.HandleID ++ ;
	if( CharBuf.HandleID >= ( DX_HANDLECHECK_MASK >> DX_HANDLECHECK_ADDRESS ) )
		CharBuf.HandleID = 0 ;

	// �h�l�d��L���ɂ���
	CharBuf.IMEUseFlag_System = TRUE ;
	RefreshIMEFlag() ;

	// �����R�[�h�o�b�t�@���N���A����
	NS_ClearInputCharBuf() ;

	// �n���h���l�̍쐬
	Result = i | DX_HANDLETYPE_MASK_KEYINPUT | ( Input->ID << DX_HANDLECHECK_ADDRESS ) ;

	// �I��
	return Result ;
}

// �L�[���̓f�[�^�̍폜
extern int NS_DeleteKeyInput( int InputHandle )
{
	INPUTDATA * Input ;
	int i ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// �m�ۂ��Ă����������̉��
	if( Input->Buffer != NULL ) DXFREE( Input->Buffer ) ;

	// �f�[�^�̏�����
	_MEMSET( &CharBuf.InputData[ InputHandle & DX_HANDLEINDEX_MASK ], 0, sizeof( INPUTDATA ) ) ;
	
	if( CharBuf.ActiveInputHandle == InputHandle )
	{
		CharBuf.ActiveInputHandle = -1 ;
	}

	// ���ׂẴL�[���͂������ɂȂ��Ă���΂h�l�d�͖����ɂ���
	for( i = 0 ; i != MAX_INPUT_NUM && !CharBuf.InputData[ i & DX_HANDLEINDEX_MASK ].UseFlag ; i ++ ){}
	if( i == MAX_INPUT_NUM )
	{
		CharBuf.IMEUseFlag_System = FALSE ;
		RefreshIMEFlag() ;
	}

	// �I��
	return 0 ;
}

// �w��̃L�[���͂��A�N�e�B�u�ɂ���
extern int NS_SetActiveKeyInput( int InputHandle )
{
	INPUTDATA * Input ;
	INPUTDATA * ActInput ;

	// �n���h���`�F�b�N
	Input = NULL ;
	if( InputHandle > 0 )
	{
		if( KEYHCHK( InputHandle, Input ) ) return -1 ;
	}

	// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
	ResetKeyInputCursorBrinkCount() ;

	// ���܂łƓ����ꍇ�͉������Ȃ�
	if( InputHandle == CharBuf.ActiveInputHandle &&
		( ( Input != NULL && Input->EndFlag == FALSE ) || ( Input == NULL && InputHandle >= 0 ) ) ) return 0 ;

	// ���܂ŃA�N�e�B�u�������n���h���őI��̈悪����ꍇ�͉�������
	if( CharBuf.ActiveInputHandle != -1 )
	{
		if( KEYHCHK( CharBuf.ActiveInputHandle, ActInput ) ) return -1 ;

		ActInput->SelectStart = -1 ;
		ActInput->SelectEnd = -1 ;
	}

	if( InputHandle < 0 )
	{
		CharBuf.ActiveInputHandle = -1 ;
	}
	else
	{
		// �A�N�e�B�u�ȓ��̓n���h����ύX����O�ɓ��͕�������N���A���Ă���
		NS_ClearInputCharBuf() ;

		CharBuf.ActiveInputHandle = InputHandle ;

		// �G���h�t���O�̗ނ�����������
		Input->EndFlag = FALSE ;
		Input->CancellFlag = FALSE ;
	}

	// �I��
	return 0 ;
}

// ���݃A�N�e�B�u�ɂȂ��Ă���L�[���̓n���h�����擾����
extern int NS_GetActiveKeyInput( void )
{
	return CharBuf.ActiveInputHandle ;
}

// ���͂��I�����Ă��邩�擾����
extern int NS_CheckKeyInput( int InputHandle )
{
	INPUTDATA * Input ;
	int Result ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	Result = Input->CancellFlag ? 2 : ( Input->EndFlag ? 1 : 0 ) ;

	return Result ;
}

// ���͂����������L�[���͂��ēx�ҏW��Ԃɖ߂�
extern int NS_ReStartKeyInput( int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	Input->EndFlag = FALSE ;

	// �I��
	return 0 ;
}

// �L�[���͏����֐�
extern int NS_ProcessActKeyInput( void )
{
	INPUTDATA * Input ;
	TCHAR C[ 3 ] = { 0 , 0 , 0 } ;
	int CharLen, len, DelNum ;
	HIMC Imc;

	// �h�l�d�̃��t���b�V������
	{
		HWND DefHwnd;
		static TCHAR str[256];

		switch( CharBuf.IMERefreshStep )
		{
		case 1:
//			DefHwnd = ImmGetDefaultIMEWnd( WinData.MainWindow ) ;
			DefHwnd = WinData.MainWindow;
			if( DefHwnd )
			{
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
				if( Imc )
				{
					_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
					_MEMSET( CharBuf.InputTempString, 0, CHARBUFFER_SIZE );
					WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , CharBuf.InputTempString , CHARBUFFER_SIZE );
					WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc , NI_COMPOSITIONSTR ,  CPS_CANCEL , 0  );

					WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
				}
			}
			CharBuf.IMERefreshStep ++ ;
//			break;

		case 2:
			SetIMEOpenState( FALSE );
			CharBuf.IMERefreshStep ++ ;
//			break;

		case 3:
			SetIMEOpenState( TRUE );
			CharBuf.IMERefreshStep ++ ;
//			break;

		case 4:
//			DefHwnd = ImmGetDefaultIMEWnd( WinData.MainWindow ) ;
			DefHwnd = WinData.MainWindow;
			if( DefHwnd )
			{
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
				if( Imc )
				{
					WinAPIData.ImmFunc.ImmSetOpenStatusFunc( Imc, TRUE );
					WinAPIData.ImmFunc.ImmSetCompositionStringFunc( Imc, SCS_SETSTR, CharBuf.InputTempString, lstrlen( CharBuf.InputTempString ), 0, 0 );
					WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
				}
			}
			CharBuf.IMERefreshStep = 0 ;
//			break;
		}
	}

	if( CharBuf.ActiveInputHandle == -1 ) return 0 ;
	if( KEYHCHK( CharBuf.ActiveInputHandle, Input ) ) return -1 ;
	if( Input->EndFlag ) return 0 ;

	// �h�l�d�̓��͕�������������
	{
		TCHAR TempString[ CHARBUFFER_SIZE ] ;
		int i, len, maxlen ;
		bool flag ;

		// ���������w��ő�o�C�g���𒴂��Ă����狭���I�ɕ����������炷
		flag = false ;
		len = lstrlen( CharBuf.InputString ) ;
		maxlen = 65536 ;
		if( CharBuf.IMEInputStringMaxLengthIMESync && Input->StrLength + len > Input->MaxStrLength )
		{
			flag = true ;
			maxlen = Input->MaxStrLength - Input->StrLength ;
		}

		if( CharBuf.IMEInputMaxLength && len > CharBuf.IMEInputMaxLength )
		{
			flag = true ;
			if( maxlen > CharBuf.IMEInputMaxLength ) maxlen = CharBuf.IMEInputMaxLength ;
		}

		if( flag )
		{
			// ���̓R���e�L�X�g�̎擾
			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( WinData.MainWindow ) ;
			if( Imc )
			{
				// ���o�b�t�@�ɂ��镶����������Ȃ��Ă��邩���ׂ�
				_MEMSET( TempString, 0, sizeof( TempString ) );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , TempString , CHARBUFFER_SIZE );
				len = lstrlen( TempString ) ;
				if( len > maxlen )
				{
					// �������Z������
					for( i = 0; i < len ; )
					{
						if( _TMULT( TempString[ i ], _GET_CHARSET() ) )
						{
							if( i + 2 > maxlen ) break;
							i += 2 ;
						}
						else
						{
							if( i + 1 > maxlen ) break;
							i ++ ;
						}
					}
					TempString[ i ] = _T( '\0' );

					// �ݒ��������
					WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc , NI_COMPOSITIONSTR ,  CPS_CANCEL , 0  );
					WinAPIData.ImmFunc.ImmSetCompositionStringFunc( Imc, SCS_SETSTR, TempString, lstrlen( TempString ), 0, 0 );
				}

				// ���̓R���e�L�X�g�̍폜
				WinAPIData.ImmFunc.ImmReleaseContextFunc( WinData.MainWindow , Imc );
			}
		}
	}

	// �L�[�R�[�h�o�b�t�@�̑������菈��
	while( Input->EndFlag == FALSE )
	{
		if( ( CharLen = NS_GetOneChar( C , TRUE ) ) == 0 ) break ;

		// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
		ResetKeyInputCursorBrinkCount() ;
		
		switch( C[0] )
		{
			// �㉺�^�u�L�[�͖���
		case CTRL_CODE_UP :
		case CTRL_CODE_DOWN :
		case CTRL_CODE_TAB :
			break ;

			// �z�[���{�^��
		case CTRL_CODE_HOME :
			if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
			{
				if( Input->SelectStart == -1 )
				{
					Input->SelectStart = Input->Point ;
					Input->SelectEnd   = 0 ;
				}
				else
				{
					Input->SelectEnd   = 0 ;
				}
			}
			else
			{
				Input->SelectStart = -1 ;
				Input->SelectEnd = -1 ;
			}
			Input->Point = 0 ;
			break ;

			// �G���h�{�^��
		case CTRL_CODE_END :
			if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
			{
				if( Input->SelectStart == -1 )
				{
					Input->SelectStart = Input->Point ;
					Input->SelectEnd   = Input->StrLength ;
				}
				else
				{
					Input->SelectEnd   = Input->StrLength ;
				}
			}
			else
			{
				Input->SelectStart = -1 ;
				Input->SelectEnd = -1 ;
			}
			Input->Point = Input->StrLength ;
			break ;

			// �J�b�g
		case CTRL_CODE_CUT :
CUT:
			if( Input->SelectStart != -1 )
			{
				TCHAR *Buffer ;
				int smin, smax, size ;

				if( Input->SelectStart > Input->SelectEnd )
				{
					smin = Input->SelectEnd ;
					smax = Input->SelectStart ;
				}
				else
				{
					smin = Input->SelectStart ;
					smax = Input->SelectEnd ;
				}
				size = smax - smin ;
				Buffer = ( TCHAR * )DXALLOC( ( size + 1 ) * sizeof( TCHAR ) ) ;
				if( Buffer )
				{
					_MEMCPY( Buffer, &Input->Buffer[ smin ], size * sizeof( TCHAR ) ) ;
					Buffer[ size ] = _T( '\0' ) ;
					NS_SetClipboardText( Buffer ) ;
					DXFREE( Buffer ) ;
				}
				KeyInputSelectAreaDelete( Input ) ;
			}
			break ;

			// �R�s�[
		case CTRL_CODE_COPY :
			if( Input->SelectStart != -1 )
			{
				TCHAR *Buffer ;
				int smin, smax, size ;

				if( Input->SelectStart > Input->SelectEnd )
				{
					smin = Input->SelectEnd ;
					smax = Input->SelectStart ;
				}
				else
				{
					smin = Input->SelectStart ;
					smax = Input->SelectEnd ;
				}
				size = smax - smin ;
				Buffer = ( TCHAR * )DXALLOC( ( size + 1 ) * sizeof( TCHAR ) ) ;
				if( Buffer )
				{
					_MEMCPY( Buffer, &Input->Buffer[ smin ], size * sizeof( TCHAR ) ) ;
					Buffer[ size ] = _T( '\0' ) ;
					NS_SetClipboardText( Buffer ) ;
					DXFREE( Buffer ) ;
				}
			}
			break ;

			// �y�[�X�g
		case CTRL_CODE_PASTE :
			if( NS_GetClipboardText( NULL ) != -1 )
			{
				int Size ;
				TCHAR *Buffer, *p ;

				Size = NS_GetClipboardText( NULL ) ;
				Buffer = ( TCHAR * )DXALLOC( Size ) ;
				if( Buffer )
				{
					NS_GetClipboardText( Buffer ) ;
					for( p = Buffer ; *p ; p++ )
					{
						if( _TMULT( *p, _GET_CHARSET() ) == TRUE )
						{
							NS_StockInputChar( *p ) ;
							p++ ;
							NS_StockInputChar( *p ) ;
						}
						else
						{
							if( *p != _T( '\r' ) && *p != _T( '\n' ) )
							{
								NS_StockInputChar( *p ) ;
							}
						}
					}
					DXFREE( Buffer ) ;
				}
			}
			break ;

			// �G���^�[�����������ꍇ�͂����ŏI��
		case CTRL_CODE_CR :
			{
				Input->EndFlag = TRUE ;
				CharBuf.ActiveInputHandle = -1 ;
			}
			break ;

			// �G�X�P�[�v�L�[�������ꍇ�̓L�����Z��
		case CTRL_CODE_ESC :
			if( Input->CancelValidFlag )
			{
				Input->CancellFlag = TRUE ;
				Input->EndFlag = TRUE ;
				CharBuf.ActiveInputHandle = -1 ;
			}
			break ;

			// �f���[�g�L�[�������ꍇ�͕����̍폜
		case CTRL_CODE_DEL :
			if( Input->SelectStart == -1 )
			{
				if( Input->Point != Input->StrLength )
				{
					DelNum = _TMULT( Input->Buffer[Input->Point], _GET_CHARSET() ) == TRUE ? 2 : 1 ;  
					_MEMMOVE( &Input->Buffer[ Input->Point ] , &Input->Buffer[ Input->Point + DelNum ] , ( Input->StrLength - Input->Point ) * sizeof( TCHAR ) ) ;
					Input->StrLength -= DelNum ;
				}
			}
			else
			{
				if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
				{
					goto CUT ;
				}
				KeyInputSelectAreaDelete( Input ) ;
			}
			break ;

			// �o�b�N�X�y�[�X�L�[�������ꍇ�͈�O�̕������폜
		case CTRL_CODE_BS :
			if( Input->SelectStart == -1 )
			{
				if( Input->Point > 0 )
				{
					int DelNum ;

					DelNum = 1 ;
					if( Input->Point > 1 )
					{
//						DelNum = _TMULT( Input->Buffer[Input->Point - 2], _GET_CHARSET() ) == TRUE ? 2 : 1 ;
						DelNum = CheckDoubleChar( Input->Buffer, Input->Point - 1, _GET_CHARSET() ) == 2 ? 2 : 1 ;
					}
		
					Input->Point -= DelNum ;
					_MEMMOVE( &Input->Buffer[ Input->Point ] , &Input->Buffer[ Input->Point + DelNum ] , ( Input->StrLength - Input->Point ) * sizeof( TCHAR ) ) ;
					Input->StrLength -= DelNum ;
				}
			}
			else
			{
				KeyInputSelectAreaDelete( Input ) ;
			}
			break ;

			// �ړ��L�[�̏ꍇ�͂��̏���
		case CTRL_CODE_LEFT :
			if( Input->Point > 0 )
			{
				len = 1 ;
//				if( Input->Point > 1 && _TMULT( Input->Buffer[Input->Point - 2], _GET_CHARSET() ) == TRUE )
				if( Input->Point > 1 && CheckDoubleChar( Input->Buffer, Input->Point - 1, _GET_CHARSET() ) == 2 )
					len = 2 ;

				if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
				{
					if( Input->SelectStart == -1 )
					{
						Input->SelectStart = Input->Point ;
						Input->SelectEnd   = Input->Point - len ;
					}
					else
					{
						Input->SelectEnd   = Input->Point - len ;
					}
				}
				else
				{
					Input->SelectStart = -1 ;
					Input->SelectEnd = -1 ;
				}
				Input->Point -= len ;
			}
			break ;

		case CTRL_CODE_RIGHT :
			if( Input->Point < Input->StrLength )
			{
				len = 1 ;
				if( _TMULT( Input->Buffer[Input->Point], _GET_CHARSET() ) == TRUE )
					len = 2 ;

				if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
				{
					if( Input->SelectStart == -1 )
					{
						Input->SelectStart = Input->Point ;
						Input->SelectEnd   = Input->Point + len ;
					}
					else
					{
						Input->SelectEnd   = Input->Point + len ;
					}
				}
				else
				{
					Input->SelectStart = -1 ;
					Input->SelectEnd = -1 ;
				}
				Input->Point += len ;
			}
			break ;

		default:
			KeyInputSelectAreaDelete( Input ) ;

			// ���l�����̂ݎ��̏���
			if( Input->NumCharOnlyFlag )
			{
				// ���l�����łȂ���΃L�����Z��
				if( C[ 0 ] < _T( '0' ) || C[ 0 ] > _T( '9' ) )
				{
					// �}�C�i�X�����ŃJ�[�\���ʒu���O�������ꍇ�͓��͉\
					if( C[ 0 ] != _T( '-' ) || Input->Point != 0 )
					{
						// �s���I�h����܂łn�j
						if( ( C[ 0 ] == _T( '.' ) && _TSTRCHR( Input->Buffer, _T( '.' ) ) != NULL ) || C[ 0 ] != _T( '.' ) ) break ;
					}
				}
			}

			// �P�o�C�g�����̂ݎ��̏���
			if( Input->SingleCharOnlyFlag )
			{
				if( CharLen != 1 ) break ;
			}

			// �Q�o�C�g�����̂ݎ��̏���
			if( Input->DoubleCharOnlyFlag )
			{
				if( CharLen == 1 ) break ;
			}

			// ������̒ǉ�
			{
				// �o�b�t�@�ɋ󂫂��Ȃ����̏���
				while( Input->StrLength + CharLen > Input->MaxStrLength )
				{
					int /*DelNum,*/ Pos, CLen ;

					// �Ō�̕��������
					Pos = 0 ;
					while( Pos < Input->StrLength )
					{
						if( _TMULT( Input->Buffer[Pos], _GET_CHARSET() ) == TRUE )	CLen = 2 ;
						else														CLen = 1 ;
						Pos += CLen ;
					}
					Input->Buffer[Pos - CLen] = _T( '\0' ) ;
					Input->StrLength -= CLen ;

					// �J�[�\�����ړ�����
					if( Input->Point == Pos ) Input->Point -= CLen ;
				}
				
				_MEMMOVE( &Input->Buffer[ Input->Point + CharLen ] , &Input->Buffer[ Input->Point ] , ( Input->StrLength - Input->Point + 1 ) * sizeof( TCHAR ) ) ;
				_MEMMOVE( &Input->Buffer[ Input->Point ] , C , CharLen * sizeof( TCHAR ) ) ;

				Input->Point += CharLen ;
				Input->StrLength += CharLen ;
				break ;
			}
		}
	}

	// �I��
	return 0 ;
}

// �L�[���͒��f�[�^�̕`��
extern int NS_DrawKeyInputString( int x , int y , int InputHandle )
{
	INPUTDATA * Input ;
	int Use3DFlag , FontSize ;
	int PointX , FontHandle ;
	int AddY, StrWidth ;
	RECT DrawRect ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;
	if( Input->EndFlag ) return -1 ;

	// �e��f�[�^�ۑ�
	Use3DFlag = NS_GetUse3DFlag() ;

	// �R�c�`��@�\�̎g�p�ύX
	NS_SetUse3DFlag( FALSE ) ;

	// �g�p����t�H���g�̃n���h�����Z�b�g����
	FontHandle = CharBuf.UseFontHandle == -1 ? NS_GetDefaultFontHandle() : CharBuf.UseFontHandle ;

	// �t�H���g�T�C�Y�̎擾
	FontSize = NS_GetFontSizeToHandle( FontHandle ) ;
	
	// �J�[�\���_�ŏ���
	if( CharBuf.CBrinkFlag == TRUE )
	{
		if( NS_GetNowCount() - CharBuf.CBrinkCount > CharBuf.CBrinkWait )
		{
			CharBuf.CBrinkDrawFlag = CharBuf.CBrinkDrawFlag == TRUE ? FALSE : TRUE ;
			CharBuf.CBrinkCount = NS_GetNowCount() ;
		}
	}
	else
	{
		CharBuf.CBrinkDrawFlag = TRUE ;
	}

	// �`��J�n�����ʒu�̌���
	{
		int DrawWidth, DrawHNum ;
		int tx, ty, w ;

		NS_GetDrawArea( &DrawRect ) ;
		AddY = FontSize + FontSize / 10 * 3 ;

		if( DrawRect.left == DrawRect.right ||
			DrawRect.bottom == DrawRect.top )
			return -1 ;

		tx = x ;
		ty = y ;
		if( tx < DrawRect.left ) tx = DrawRect.left ;
		if( ty < DrawRect.top  ) ty = DrawRect.top ;

		while( tx > DrawRect.right )
		{
			tx -= DrawRect.right - DrawRect.left ;
			ty += AddY ;
		}

		DrawHNum = ( DrawRect.bottom - ty ) / AddY ;
		if( DrawHNum == 0 )
		{
			DrawWidth = DrawRect.right - tx ;
			DrawHNum = 1 ;
		}
		else
		{
			DrawWidth = ( DrawRect.right - tx ) + ( DrawHNum - 1 ) * ( DrawRect.right - DrawRect.left ) ;
		}

		StrWidth = NS_GetDrawStringWidthToHandle( Input->Buffer , Input->StrLength , FontHandle ) + 4 ;

		if( StrWidth < DrawWidth )
		{
			Input->DrawStartPos = 0 ;
		}
		else
		{
			if( Input->DrawStartPos > Input->Point )
			{
				Input->DrawStartPos = Input->Point ;
			}
			else
			{
				w = NS_GetDrawStringWidthToHandle( Input->Buffer + Input->DrawStartPos, Input->Point - Input->DrawStartPos, FontHandle ) ;
				if( w > DrawWidth )
				{
					int TotalWidth, Width, Num ;

					TotalWidth = 0 ;
					Input->DrawStartPos = Input->Point ;
					for(;;)
					{
						if( Input->DrawStartPos <= 0 )
						{
							break ;
						}

						if( Input->DrawStartPos > 1 )
						{
							Num = _TMULT( Input->Buffer[ Input->DrawStartPos - 2 ], _GET_CHARSET() ) == TRUE ? 2 : 1 ;
						}
						else
						{
							Num = 1 ;
						}

						Width = NS_GetDrawStringWidthToHandle( Input->Buffer + Input->DrawStartPos - Num, Num, FontHandle ) ;
						if( TotalWidth + Width > DrawWidth ) break ;
						TotalWidth += Width ;
						Input->DrawStartPos -= Num ;
					}
				}
			}
		}
	}

	// �J�[�\���̕`��
	PointX = NS_GetDrawStringWidthToHandle( Input->Buffer + Input->DrawStartPos, Input->Point - Input->DrawStartPos, FontHandle ) + x ;
	if( CharBuf.CBrinkDrawFlag == TRUE && InputHandle == CharBuf.ActiveInputHandle )
//		|| ( InputHandle == CharBuf.ActiveInputHandle && CharBuf.IMESwitch == TRUE ) )
	{
		NS_DrawObtainsBox( PointX , y , PointX + 2 , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor2 , TRUE ) ;
	}

	// ������̕`��
	NS_DrawObtainsString(
		x,
		y,
		AddY,
		Input->Buffer + Input->DrawStartPos,
		CharBuf.StrColor1,
		CharBuf.StrColor8,
		FontHandle,
		CharBuf.StrColor13,
		CharBuf.StrColor14,
		CharBuf.StrColor15,
		Input->SelectStart - Input->DrawStartPos,
		Input->SelectEnd   - Input->DrawStartPos
	) ;

	// ���͒�������̕`��
	if( InputHandle == CharBuf.ActiveInputHandle )
	{
		NS_DrawIMEInputString( PointX , y , 5 ) ; 
	}

	// �f�[�^�����ɖ߂�
	NS_SetUse3DFlag( Use3DFlag ) ;

	// �I��
	return 0 ;
}

// �L�[���͎��̃J�[�\���̓_�ł��鑁�����Z�b�g����
extern int NS_SetKeyInputCursorBrinkTime( int Time )
{
	CharBuf.CBrinkWait = Time ;

	// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
	ResetKeyInputCursorBrinkCount() ;

	// �I��
	return 0 ;
}

// �L�[���̓f�[�^�̎w��̗̈��I����Ԃɂ���( SelectStart �� SelectEnd �� -1 ���w�肷��ƑI����Ԃ���������܂� )
extern int NS_SetKeyInputSelectArea( int SelectStart, int SelectEnd, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( SelectStart == -1 && SelectEnd == -1 )
	{
		Input->SelectStart = -1 ;
		Input->SelectEnd   = -1 ;
	}
	else
	{
		if( SelectStart < 0                ) SelectStart = 0 ;
		if( SelectStart > Input->StrLength ) SelectStart = Input->StrLength ;
		if( SelectEnd   < 0                ) SelectEnd   = 0 ;
		if( SelectEnd   > Input->StrLength ) SelectEnd   = Input->StrLength ;

		Input->SelectStart = SelectStart ;
		Input->SelectEnd   = SelectEnd ;
	}

	// �I��
	return 0 ;
}

// �L�[���̓f�[�^�̑I��̈���擾����
extern int NS_GetKeyInputSelectArea( int *SelectStart, int *SelectEnd, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( SelectStart ) *SelectStart = Input->SelectStart ;
	if( SelectEnd   ) *SelectEnd   = Input->SelectEnd ;

	// �I��
	return 0 ;
}

// �L�[���͂̕`��J�n�����ʒu���擾����
extern int NS_GetKeyInputDrawStartPos( int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	return Input->DrawStartPos ;
}

// �L�[���͂̕`��J�n�����ʒu��ݒ肷��
extern int NS_SetKeyInputDrawStartPos( int DrawStartPos, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( DrawStartPos < 0                ) DrawStartPos = 0 ;
	if( DrawStartPos > Input->StrLength ) DrawStartPos = Input->StrLength ;

	Input->DrawStartPos = DrawStartPos ;

	return Input->DrawStartPos ;
}

// �L�[���͎��̃J�[�\����_�ł����邩�ǂ������Z�b�g����
extern int NS_SetKeyInputCursorBrinkFlag( int Flag )
{
	CharBuf.CBrinkFlag = Flag ;

	// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
	ResetKeyInputCursorBrinkCount() ;
	
	// �I��
	return 0 ;
}

// �L�[���͎��̃J�[�\���_�ŏ����̃J�E���^�����Z�b�g
static int ResetKeyInputCursorBrinkCount( void )
{
	CharBuf.CBrinkCount = NS_GetNowCount() ;
	CharBuf.CBrinkDrawFlag = TRUE ;
	
	// �I��
	return 0 ;
}
	



// �L�[���̓f�[�^�Ɏw��̕�������Z�b�g����
extern int NS_SetKeyInputString( const TCHAR *String , int InputHandle )
{
	INPUTDATA * Input ;
	int StrLen ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	StrLen = lstrlen( String ) ;
	if( StrLen > Input->MaxStrLength )
		StrLen = Input->MaxStrLength ;

	_TSTRNCPY( Input->Buffer, String, StrLen ) ;
	Input->Buffer[ StrLen ] = _T( '\0' ) ;
	Input->StrLength = StrLen ;
	Input->Point = StrLen ;
	Input->SelectEnd = -1 ;
	Input->SelectStart = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
	ResetKeyInputCursorBrinkCount() ;
	
	// �I��
	return 0 ;
}

// �L�[���̓f�[�^�Ɏw��̐��l�𕶎��ɒu�������ăZ�b�g����
extern int NS_SetKeyInputNumber( int Number , int InputHandle )
{
	INPUTDATA * Input ;
	TCHAR StrBuf[ 512 ] ;
	int StrLen ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// �����ɒu��������
	_ITOT( Number, StrBuf, 10 ) ;

	StrLen = lstrlen( StrBuf ) ;
	if( StrLen > Input->MaxStrLength )
		StrLen = Input->MaxStrLength ;

	_TSTRNCPY( Input->Buffer , StrBuf , StrLen ) ;
	Input->Buffer[ StrLen ] = _T( '\0' ) ;
	Input->StrLength = StrLen ;
	Input->Point = StrLen ;
	Input->SelectEnd = -1 ;
	Input->SelectStart = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
	ResetKeyInputCursorBrinkCount() ;

	// �I��
	return 0 ;
}

// �L�[���̓f�[�^�Ɏw��̕��������_�l�𕶎��ɒu�������ăZ�b�g����
extern int NS_SetKeyInputNumberToFloat( float Number, int InputHandle )
{
	INPUTDATA * Input ;
	TCHAR StrBuf[ 256 ] ;
	int StrLen ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// �����ɒu��������
	_TSPRINTF( _DXWTP( StrBuf ), _DXWTR( "%f" ), Number ) ;

	StrLen = lstrlen( StrBuf ) ;
	if( StrLen > Input->MaxStrLength )
		StrLen = Input->MaxStrLength ;

	_TSTRNCPY( Input->Buffer , StrBuf, StrLen ) ;
	Input->Buffer[ StrLen ] = _T( '\0' ) ;
	Input->StrLength = StrLen ;
	Input->Point = StrLen ;
	Input->SelectEnd = -1 ;
	Input->SelectStart = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// �J�[�\���_�ŏ����̃J�E���^�����Z�b�g
	ResetKeyInputCursorBrinkCount() ;

	// �I��
	return 0 ;
}

// ���͒��̕�����f�[�^���擾����
extern int NS_GetKeyInputString( TCHAR *StrBuffer , int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	lstrcpy( StrBuffer , Input->Buffer ) ;

	// �I��
	return 0 ;
}

// ���̓f�[�^�̕�����𐮐��l�Ƃ��Ď擾����
extern int NS_GetKeyInputNumber( int InputHandle )
{
	INPUTDATA * Input ;
	int Number ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// ���l�ɕϊ�
	if( _TSTRCHR( Input->Buffer, _T( '.' ) ) )
	{
		Number = _FTOL( ( float )_TTOF( _DXWTP( Input->Buffer ) ) ) ;
	}
	else
	{
		Number = _TTOI( _DXWTP( Input->Buffer ) ) ;
	}

	// �I��
	return Number ;
}

// ���̓f�[�^�̕�����𕂓������_�l�Ƃ��Ď擾����
extern float NS_GetKeyInputNumberToFloat( int InputHandle )
{
	INPUTDATA * Input ;
	float Number ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// ���l�ɕϊ�
	if( _TSTRCHR( Input->Buffer, _T( '.' ) ) )
	{
		Number = ( float )_TTOF( _DXWTP( Input->Buffer ) ) ;
	}
	else
	{
		Number = ( float )_TTOI( _DXWTP( Input->Buffer ) ) ;
	}

	// �I��
	return Number ;
}

// �L�[���͂̌��݂̃J�[�\���ʒu��ݒ肷��
extern int NS_SetKeyInputCursorPosition( int Position, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( Position < 0                ) Position = 0 ;
	if( Position > Input->StrLength ) Position = Input->StrLength ;

	Input->Point = Position ;

	// �I��
	return 0 ;
}

// �L�[���͂̌��݂̃J�[�\���ʒu���擾����
extern int NS_GetKeyInputCursorPosition( int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// �I��
	return Input->Point ;
}

// �h�l�d�œ��͒��̕�����̏����擾����
extern const IMEINPUTDATA *NS_GetIMEInputData( void )
{
	LPIMEINPUTDATA IMEInput ;
	int TotalSize, i, Len ;
	LPIMEINPUTCLAUSEDATA IMEClause ;

	// ���Ɋm�ۂ���Ă�������
	if( CharBuf.IMEInputData )
	{
		DXFREE( CharBuf.IMEInputData ) ;
		CharBuf.IMEInputData = NULL ;
	}

	if( CharBuf.IMEUseFlag_OSSet == FALSE || CharBuf.IMESwitch == FALSE /* || CharBuf.ClauseNum == 0 */ ) return NULL ;

	if( CharBuf.InputString[ 0 ] == 0 ) return NULL ;

	// �m�ۂ��ׂ��������e�ʂ��v�Z
	Len = lstrlen( CharBuf.InputString ) ;
	TotalSize = sizeof( IMEINPUTDATA ) ;
	TotalSize += ( Len + 1 ) * sizeof( TCHAR ) ;
	if( CharBuf.ClauseNum > 1 )
	{
		TotalSize += ( CharBuf.ClauseNum - 1 ) * sizeof( IMEINPUTCLAUSEDATA ) ;
	}
	if( CharBuf.CandidateList != NULL )
	{
		TotalSize += CharBuf.CandidateList->dwCount * sizeof( TCHAR ** ) + CharBuf.CandidateListSize ;
	}

	// �������̊m��
	IMEInput = ( LPIMEINPUTDATA )DXALLOC( TotalSize ) ;
	if( IMEInput == NULL )
	{
		DXST_ERRORLOG_ADD( _T( "�h�l�d���͏��p�̃������̊m�ۂɎ��s���܂���\n" ) ) ;
		return NULL ;
	}
	CharBuf.IMEInputData = IMEInput ;

	// ���̃Z�b�g
	{
		const void *NextP ;

		// ��������̃Z�b�g
		IMEInput->InputString = ( TCHAR * )( IMEInput + 1 );
		_TSTRNCPY( ( TCHAR * )IMEInput->InputString, CharBuf.InputString, Len + 1 ) ;

		// �J�[�\���ʒu�̃Z�b�g
		IMEInput->CursorPosition = NS_GetStringPoint( CharBuf.InputString , CharBuf.InputPoint ) ;

		// ���ߏ��̃Z�b�g
		if( CharBuf.ClauseNum > 1 )
		{
			IMEInput->ClauseNum = CharBuf.ClauseNum - 1 ;
			IMEClause = ( LPIMEINPUTCLAUSEDATA )( IMEInput->InputString + Len + 1 ) ;
			IMEInput->ClauseData = IMEClause ;
			IMEInput->SelectClause = -1 ;
			for( i = 0 ; i < IMEInput->ClauseNum ; i ++ , IMEClause ++ )
			{
				IMEClause->Position = CharBuf.ClauseData[ i ] ;
				IMEClause->Length   = CharBuf.ClauseData[ i + 1 ] - IMEClause->Position ;
				if( CharBuf.ClauseData[ i ] == IMEInput->CursorPosition )
				{
					IMEInput->SelectClause = i ;
				}
			}

			NextP = IMEInput->ClauseData + IMEInput->ClauseNum ;
		}
		else
		{
			IMEInput->ClauseNum = 0 ;
			IMEInput->ClauseData = NULL ;
			IMEInput->SelectClause = -1 ;

			NextP = IMEInput->InputString + Len + 1 ;
		}

		// �ϊ������̃Z�b�g
		if( CharBuf.CandidateList != NULL )
		{
			PCANDIDATELIST CandidateListBase ;
			TCHAR **CandidateList ;

			CandidateList = ( TCHAR ** )NextP ;
			IMEInput->CandidateList = ( const TCHAR ** )CandidateList ;
			IMEInput->CandidateNum = CharBuf.CandidateList->dwCount ;
			CandidateListBase = ( PCANDIDATELIST )( IMEInput->CandidateList + IMEInput->CandidateNum ) ;
			_MEMCPY( CandidateListBase, CharBuf.CandidateList, CharBuf.CandidateListSize ) ;
			for( i = 0 ; i < IMEInput->CandidateNum ; i ++ )
			{
				CandidateList[ i ] = ( TCHAR * )( ( BYTE * )CandidateListBase + CandidateListBase->dwOffset[ i ] ) ;
			}
			IMEInput->SelectCandidate = ( int )CharBuf.CandidateList->dwSelection ;
		}
		else
		{
			IMEInput->CandidateList   = NULL ;
			IMEInput->CandidateNum    = 0 ;
			IMEInput->SelectCandidate = 0 ;
		}

		// ������ϊ������ǂ������Z�b�g
		if( CharBuf.CharAttrNum == 0 )
		{
			IMEInput->ConvertFlag = FALSE ;
		}
		else
		{
			for( i = 0 ; i < CharBuf.CharAttrNum && CharBuf.CharAttr[ i ] != ATTR_INPUT ; i ++ ){}
			IMEInput->ConvertFlag = i == CharBuf.CharAttrNum ? TRUE : FALSE ;
		}
	}

	// �쐬��������Ԃ�
	return CharBuf.IMEInputData ;
}

#endif

}

#endif // DX_NON_INPUTSTRING

