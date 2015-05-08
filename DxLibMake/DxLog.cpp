// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		���O�v���O����
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h ------------------------------------------------------------------
#include "DxLog.h"

#ifndef DX_NON_LOG

#include "DxLib.h"
#include "DxFont.h"
#include "DxStatic.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"

namespace DxLib
{

// �}�N����` --------------------------------------------------------------------

// �\���̒�` --------------------------------------------------------------------

// �������ϐ��錾 --------------------------------------------------------------

LOGDATA LogData ;

// �֐��v���g�^�C�v�錾-----------------------------------------------------------

extern	int			ErrorLogInitialize_Native( void ) ;										// �G���[�����o���t�@�C�������������鏈���̊��ˑ�����

#ifndef DX_NON_PRINTF_DX

// ���O�o�͋@�\�֐�
static	int			RefreshLogFont( void ) ;												// ���O�����Ŏg�p���Ă���t�H���g���X�V����
static	int			CrLog( void ) ;															// ���s����
static	int			AddCharLog( const TCHAR *C ) ;											// �ꕶ�����邩���ׂāA�K�v�Ȃ���s����
static	int			AddLog( const TCHAR *String ) ;											// ���O�o��
static	int			ClrLog( void ) ;														// ���O�̏�����

#endif // DX_NON_PRINTF_DX

// �v���O���� --------------------------------------------------------------------

// �G���[�����o���t�@�C��������������
extern int ErrorLogInitialize( void )
{
	if( ErrorLogInitialize_Native() < 0 )
		return -1 ;

	// �^�u����������
	LogData.ErTabNum = 0 ; 

	// �^�u�r�o�X�g�b�v�t���O��|��
	LogData.ErTabStop = FALSE ;

	// �X�^�[�g���̃^�C���X�^���v���擾
	LogData.LogStartTime = NS_GetNowCount() ;

	// �I��
	return 0 ;
}

// �G���[�����������o��
extern int ErrorLogAddA( const char *ErrorStr )
{
#ifdef UNICODE
	wchar_t ErrorStrW[ 1024 ] ;
	MBCharToWChar( 932, ErrorStr, ( DXWCHAR * )ErrorStrW, 1024 ) ;
	return NS_ErrorLogAdd( ErrorStrW ) ;
#else
	return NS_ErrorLogAdd( ErrorStr ) ;
#endif
}

// �G���[�����������o��
extern int ErrorLogAddW( const wchar_t *ErrorStr )
{
#ifdef UNICODE
	return NS_ErrorLogAdd( ErrorStr ) ;
#else
	char ErrorStrA[ 1024 ] ;
	WCharToMBChar( 932, ( DXWCHAR * )ErrorStr, ErrorStrA, 1024 ) ;
	return NS_ErrorLogAdd( ErrorStrA ) ;
#endif
}

// �����t���G���[�����������o��
extern int ErrorLogFmtAddA( const char *FormatString , ... )
{
	va_list VaList ;
	char String[ 1024 ] ;
	
	// ���O�o�͗p�̃��X�g���Z�b�g����
	va_start( VaList , FormatString ) ;

	// �ҏW��̕�������擾����
	_VSPRINTF( String , FormatString , VaList ) ;
	
	// �ϒ����X�g�̃|�C���^�����Z�b�g����
	va_end( VaList ) ;

	// ���s������ǉ�����
	_STRCAT( String , "\n" ) ;

	// ���O�o�͂���
	return ErrorLogAddA( String ) ;
}

// �����t���G���[�����������o��
extern int ErrorLogFmtAddW( const wchar_t *FormatString , ... )
{
	va_list VaList ;
	wchar_t String[ 1024 ] ;
	
	// ���O�o�͗p�̃��X�g���Z�b�g����
	va_start( VaList , FormatString ) ;

	// �ҏW��̕�������擾����
	_VSWPRINTF( ( WORD * )String, ( WORD * )FormatString, VaList ) ;
	
	// �ϒ����X�g�̃|�C���^�����Z�b�g����
	va_end( VaList ) ;

	// ���s������ǉ�����
	_WCSCAT( String , L"\n" ) ;

	// ���O�o�͂���
	return ErrorLogAddW( String ) ;
}

// �����t���G���[�����������o��
extern int NS_ErrorLogFmtAdd( const TCHAR *FormatString , ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	int Result ;

	// ���O�o�͗p�̃��X�g���Z�b�g����
	va_start( VaList , FormatString ) ;

	// �ҏW��̕�������擾����
	_TVSPRINTF( String , FormatString , VaList ) ;
	
	// �ϒ����X�g�̃|�C���^�����Z�b�g����
	va_end( VaList ) ;

	// ���s������ǉ�����
	_TSTRCAT( String , _T( "\n" ) ) ;

	// ���O�o�͂���
	Result = DXST_ERRORLOG_ADD( String ) ;
	
	return Result ;
}


// �^�u���𑝂₷
extern int NS_ErrorLogTabAdd( void )
{
	LogData.ErTabNum ++ ;

	return 0 ;
}

// �^�u�������炷
extern int NS_ErrorLogTabSub( void )
{
	if( LogData.ErTabNum != 0 ) LogData.ErTabNum -- ;

	return 0 ;
}

// �^�C���X�^���v�̗L����ݒ肷��
extern int NS_SetUseTimeStampFlag( int UseFlag )
{
	LogData.NonUseTimeStampFlag = UseFlag != 0 ? 0 : 1 ;

	return 0 ;
}

// �����t�����O������������o��
extern int NS_AppLogAdd( const TCHAR *String, ... )
{
	va_list VaList ;
	TCHAR StringBuf[ 1024 ] ;

	va_start( VaList, String ) ;
	_TVSPRINTF( StringBuf, String, VaList ) ;
	va_end( VaList ) ;

	return NS_ErrorLogAdd( StringBuf ) ;
}

// �G���[�����o���t�@�C���̌�n��
extern int ErrorLogTerminate( void )
{
	// �I��
	return 0 ; 
}

// ���O�t�@�C�����ɓ��t�����邩�ǂ������Z�b�g����
extern int NS_SetUseDateNameLogFile( int Flag )
{
	LogData.UseLogDateName = Flag ;

	// �I��
	return 0 ;
}

// ���O�o�͂��s�����ۂ��̃Z�b�g
extern int NS_SetOutApplicationLogValidFlag( int Flag )
{
	// �t���O�Z�b�g
	LogData.NotLogOutFlag = !Flag ;

	// �I��
	return 0 ;
}

// ���O�t�@�C����ۑ�����f�B���N�g���p�X��ݒ肷��
extern int NS_SetApplicationLogSaveDirectory( const TCHAR *DirectoryPath )
{
	// �p�X�̕ۑ�
	_TSTRCPY( LogData.LogOutDirectory, DirectoryPath ) ;

	// �I��
	return 0 ;
}










#ifndef DX_NON_PRINTF_DX

// ���O�o�͋@�\�֐�

// ���O�@�\�̏�����
extern int InitializeLog( void )
{
	if( IsInitializeLog() == FALSE ) return -1 ;

	// ���ɏ���������Ă����牽�����Ȃ�
	if( LogData.LogInitializeFlag == TRUE ) return 0 ;

	// �������t���O�𗧂Ă�
	LogData.LogInitializeFlag = TRUE ;

	// ���O�t�H���g�̃T�C�Y���Z�b�g
	LogData.LogFontSize = LOG_FONTSIZE ;

	// �t�H���g�̍쐬
	LogData.LogFontHandle = -1 ;
	LogData.LogFontHandleLostFlag = TRUE ;

	// ���O�̗L�薳���t���O���Z�b�g
	LogData.LogDrawFlag = FALSE ;

	// ���O�̃t�@�C���o�͗L�薳���t���O���Z�b�g
	LogData.LogFileOutFlag = FALSE ;

	// ���O�̕����Z�b�g
	LogData.LogTabWidth = LOG_TABWIDTH ;

	// �J�[�\���̈ʒu���Z�b�g
	LogData.LogX = LogData.LogY = 0 ;

	// �X�g�����O��������
	_MEMSET( LogData.LogString, 0, sizeof( LogData.LogString ) ) ;

	// �I��
	return 0 ;
}

// ���O�����̌�n�����s��
extern int TerminateLog( void )
{
	// ���O����������������Ă��Ȃ������Ȃɂ����Ȃ�
	if( LogData.LogInitializeFlag == FALSE ) return 0 ;

	// �t�H���g�n���h�����폜����
	if( LogData.LogFontHandleLostFlag == FALSE )
	{
		NS_DeleteFontToHandle( LogData.LogFontHandle ) ;
	}
	LogData.LogFontHandleLostFlag = TRUE ;
	LogData.LogFontHandle = -1 ;

	// �������t���O��|��
	LogData.LogInitializeFlag = FALSE ;

	// �I��
	return 0 ;
}

// ���O�����Ŏg�p���Ă���t�H���g���X�V����
static int RefreshLogFont( void )
{
	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	if( LogData.LogFontHandleLostFlag == FALSE ) return 0 ;

	// �t�H���g�n���h�����쐬����
	CREATEFONTTOHANDLE_GPARAM GParam ;
	InitCreateFontToHandleGParam( &GParam ) ;
	LogData.LogFontHandle = CreateFontToHandle_UseGParam( &GParam, NULL, LogData.LogFontSize, LOG_FONTTICK, DX_FONTTYPE_EDGE, -1, -1, FALSE, -1, FALSE ) ;
	if( LogData.LogFontHandle >= 0 )
	{
		// �t�H���g�n���h�����폜���ꂽ�ۂɗ��t���O�̃|�C���^���Z�b�g����
		LogData.LogFontHandleLostFlag = FALSE ;
		NS_SetFontLostFlag( LogData.LogFontHandle, &LogData.LogFontHandleLostFlag  ) ;
	}

	return 0 ;
}

// ���s����
static int CrLog( void )
{
	int ScWidth, ScHeight, StrHeight ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// �t�H���g���폜����Ă���ꍇ�͍č쐬����
	RefreshLogFont() ;

	// ��ʂ̃T�C�Y���擾
	NS_GetDrawScreenSize( &ScWidth , &ScHeight ) ;
	
	// ��ʓ��Ɏ��܂�s�����Z�o
	StrHeight = ScHeight / LogData.LogFontSize ;
	if( StrHeight > LOG_MAXHEIGHT ) StrHeight = LOG_MAXHEIGHT ;

	// ���s�������ʂ���O��Ă��܂�Ȃ������ׁA�O��邩�ǂ����ŏ����𕪊�
	if( StrHeight <= LogData.LogY + 1 )
	{
		// �O���ꍇ�͈�s���f�[�^��S�ďグ��

		// ��������P�s�����炷
		_MEMMOVE( LogData.LogString[0], LogData.LogString[1], sizeof( TCHAR ) * LOG_MAXLENGTH * LogData.LogY ) ;
	}
	else
	{
		// �x���W���C���N�������g
		LogData.LogY ++ ;
	}

	// �V�����s�̕����������������
	_MEMSET( LogData.LogString[ LogData.LogY ], 0, sizeof( TCHAR ) * LOG_MAXLENGTH ) ;

	// �`�敝��������
	LogData.LogDrawWidth = 0 ;

	// �w���W������������
	LogData.LogX = 0 ;

	// �I��
	return 0 ;
}

// �ꕶ�����O�ɒǉ�����A�K�v�Ȃ���s����
static int AddCharLog( const TCHAR *C )
{
	int ScWidth, ScHeight ;
	int Width = 0, Length, i ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// �t�H���g���폜����Ă���ꍇ�͍č쐬����
	RefreshLogFont() ;

	// ��ʂ̃T�C�Y���擾
	NS_GetDrawScreenSize( &ScWidth , &ScHeight ) ;

	// �V���������̕`�敝���擾
//	Length = _mbclen( (unsigned char *)C ) ;
	if( _TMULT( *C, _GET_CODEPAGE() ) == TRUE )	Length = 2 ;
	else										Length = 1 ;

	// �����񂪃o�b�t�@���I�[�o�[���Ȃ������ׂ�A�o��ꍇ�͉��s����
	if( LogData.LogX + Length >= LOG_MAXLENGTH )
	{
		CrLog() ;
	}
	else
	{
		// ��ʊO�ɏo�Ȃ������ׂ�A�o��ꍇ�͉��s����
		Width = NS_GetDrawStringWidthToHandle( C , Length , LogData.LogFontHandle ) ;
		if( Width + LogData.LogDrawWidth >= ScWidth ) 
		{
			CrLog() ;
		}
	}

	// �`�敝�����Z
	LogData.LogDrawWidth += Width ;

	// ������ǉ�
	for( i = 0 ; i < Length ; i ++ )
	{
		LogData.LogString[ LogData.LogY ][ LogData.LogX + i ] = C[ i ]  ;
	}
	LogData.LogString[ LogData.LogY ][ LogData.LogX + i ] = _T( '\0' ) ;

	// �J�[�\���ʒu���C���N�������g
	LogData.LogX += Length  ;

	// �I��
	return Length ;
}


// ���O�o��
static int AddLog( const TCHAR *String )
{
	int StrLen ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// �t�H���g���폜����Ă���ꍇ�͍č쐬����
	RefreshLogFont() ;

	// �ꕶ���Âǉ����Ă���
	{
		int i ;
		const TCHAR *C ;

		// �������r�؂��܂ŌJ��Ԃ�
		StrLen = lstrlen( String ) ;
		for( i = 0 ; i < StrLen ; i ++ )
		{
			C = &String[ i ] ;

			// ���̕����̃^�C�v�ɂ���ď����𕪊�
			switch( *C )
			{
			case _T( '\n' ) :
				// ���s�R�[�h�̏ꍇ�͉��s�������s��
				CrLog() ;
				break ;

			case _T( '\t' ) :
				{
					int j ;

					// �^�u�̏ꍇ�̓^�u�̕��������X�y�[�X��ǉ�
					for( j = 0 ; j < LogData.LogTabWidth ; j ++ )
						AddCharLog( _T( " " ) )  ;
				}
				break ;

			case _T( '\a' ) : case _T( '\b' ) : case _T( '\?' ) : 
			case _T( '\f' ) : case _T( '\r' ) : case _T( '\v' ) :
				break ;

			default :
				// ����ȊO�̏ꍇ�͕��ʂɒǉ�
				if( AddCharLog( C ) == 2 ) i ++ ;
				break ;
			}
		}
	}

	// �I��
	return 0 ;
}

// ���O�̏�����
static int ClrLog( void )
{
	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// �t�H���g���폜����Ă���ꍇ�͍č쐬����
	RefreshLogFont() ;

	// ���O��������
	_MEMSET( LogData.LogString, 0, sizeof( LogData.LogString ) ) ;

	// �J�[�\���ʒu��������
	LogData.LogX = LogData.LogY = 0 ;
	LogData.LogDrawWidth = 0 ;

	// �I��
	return 0 ;
}

// ���O�o�̓t���O���Z�b�g����
extern int NS_SetLogDrawOutFlag( int DrawFlag )
{
	// ���O�`��o�̓t���O���Z�b�g����
	LogData.LogDrawFlag = DrawFlag ;

	// �I��
	return 0 ;
}

// ���O�o�͂����邩�t���O�̎擾
extern int NS_GetLogDrawFlag( void )
{
	return LogData.LogDrawFlag ;
}

// printfDx �ŉ�ʂɏo�͂��郍�O�t�H���g�̃T�C�Y��ύX����
extern int NS_SetLogFontSize( int Size )
{
	if( Size < 0 ) Size = LOG_FONTSIZE;

	LogData.LogFontSize = Size;

	DeleteFontToHandle( LogData.LogFontHandle );
	LogData.LogFontHandle = -1;
	LogData.LogFontHandleLostFlag = TRUE;

	RefreshLogFont();

	// �I��
	return 0;
}

// ���O��`�悷��
extern int DrawLog( void )
{
	int ScWidth , ScHeight ;
	int StrHeight , i ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// �t�H���g���폜����Ă���ꍇ�͍č쐬����
	RefreshLogFont() ;

	// ��ʂ̃T�C�Y���擾����
	NS_GetDrawScreenSize( &ScWidth , &ScHeight ) ;
	
	// ��ʂɃ��O��`�悷��
	StrHeight = ScHeight / LogData.LogFontSize  ;
	if( StrHeight >= LOG_MAXHEIGHT ) StrHeight = LOG_MAXHEIGHT ;
	for( i = 0 ; i < StrHeight ; i ++ )
	{
		NS_DrawStringToHandle( 0 , i * LogData.LogFontSize , LogData.LogString[ i ] , NS_GetColor( 255 , 255 , 255 ) , LogData.LogFontHandle , NS_GetColor( 0 , 0 , 0 ) ) ;
	}

	// �I��
	return 0 ;
}








// �ȈՉ�ʏo�͊֐�
extern int NS_printfDx( const TCHAR *FormatString , ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	int Result ;

	// ���O�o�͗p�̃��X�g���Z�b�g����
	va_start( VaList , FormatString ) ;

	// �ҏW��̕�������擾����
	_TVSPRINTF( String , FormatString , VaList ) ;
	
	// �ϒ����X�g�̃|�C���^�����Z�b�g����
	va_end( VaList ) ;

	// ���O�o�̓t���O�𗧂Ă�
	LogData.LogDrawFlag = TRUE ;

	// ���O�o�͊֐����Ă�
	Result = AddLog( String ) ;

	// �I��
	return Result ;
}

// �ȈՉ�ʏo�͂��N���A����
extern int NS_clsDx( void )
{
	// ���O������
	ClrLog() ;

	// ���O�o�̓t���O��|��
	LogData.LogDrawFlag = FALSE ;

	// �I��
	return 0 ;
}

#endif // DX_NON_PRINTF_DX


}

#endif // DX_NON_LOG

