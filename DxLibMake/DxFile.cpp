// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		�t�@�C���A�N�Z�X�v���O����
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h ------------------------------------------------------------------
#include "DxFile.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxUseCLib.h"
#include "DxASyncLoad.h"
#include "DxSystem.h"
#include "DxLog.h"

#ifndef DX_NON_DXA
#include "DxArchive_.h"
#endif

#ifndef DX_NON_PRINTF_DX
#include "DxFont.h"
#endif

namespace DxLib
{

// �}�N����` --------------------------------------------------------------------

// �t�@�C���A�N�Z�X�n���h���L�����`�F�b�N
#define FILEHCHK( HAND, FPOINT )		HANDLECHK(       DX_HANDLETYPE_FILE, HAND, *( ( HANDLEINFO ** )&FPOINT ) )
#define FILEHCHK_ASYNC( HAND, FPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_FILE, HAND, *( ( HANDLEINFO ** )&FPOINT ) )

// �\���̒�` --------------------------------------------------------------------

// �������ɒu���ꂽ�f�[�^���t�@�C���Ƃ��Ĉ������߂̃f�[�^�\����
typedef struct tagMEMSTREAMDATA
{
	unsigned char			*DataBuffer ;
	size_t					DataSize ;
	size_t					DataPoint ;
	int						EOFFlag ;
} MEMSTREAMDATA, *LPMEMSTREAMDATA ;

// �t�@�C���A�N�Z�X�������g�p����O���[�o���f�[�^�̍\����
struct FILEMANAGEDATA
{
	int						InitializeFlag ;					// �������t���O
} ;

// �t�@�C����񃊃X�g�\����
struct FILEINFOLIST
{
	int Num;		// �t�@�C���̐�
					// ������ FILEINFO �� Num �̐��������݂���
} ;


// �������ϐ��錾 --------------------------------------------------------------

// �t�@�C���A�N�Z�X�������g�p����O���[�o���f�[�^�̍\����
FILEMANAGEDATA GFileData ;

// �A�[�J�C�u�t�@�C����ǂݍ��ދ@�\���g�p���Ȃ����ǂ����̃t���O
int NotUseDXArchive ;


// �֐��v���g�^�C�v�錾 ----------------------------------------------------------

// �t�@�C���n���h���֐�
static	int			InitializeFileHandle( HANDLEINFO *HandleInfo ) ;								// �t�@�C���A�N�Z�X�n���h��������������
static	int			TerminateFileHandle( HANDLEINFO *HandleInfo ) ;									// �t�@�C���A�N�Z�X�n���h���̌�n�����s��


// �������ɒu���ꂽ�f�[�^���t�@�C���̃f�[�^�ɗႦ�Ă������߂̊֐�
static	LONGLONG	MemStreamTell( void *StreamDataPoint ) ;
static	int			MemStreamSeek( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType ) ;
static	size_t		MemStreamRead( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) ;
static	size_t		MemStreamWrite( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) ;
static	int			MemStreamEof( void *StreamDataPoint ) ;
static	int			MemStreamIdleCheck( void *StreamDataPoint ) ;


// �f�[�^�錾 --------------------------------------------------------------------

// �X�g���[���f�[�^�A�N�Z�X�p�֐�
STREAMDATASHREDTYPE2 StreamFunction =
{
#ifndef DX_NON_DXA
	DXA_DIR_Open,
	DXA_DIR_Close,
	DXA_DIR_Tell,
	DXA_DIR_Seek,
	DXA_DIR_Read,
	DXA_DIR_Eof,
	DXA_DIR_IdleCheck,
	DXA_DIR_ChDir,
	DXA_DIR_GetDir,
	DXA_DIR_FindFirst,
	DXA_DIR_FindNext,
	DXA_DIR_FindClose,
#else
	ReadOnlyFileAccessOpen,
	ReadOnlyFileAccessClose,
	ReadOnlyFileAccessTell,
	ReadOnlyFileAccessSeek,
	ReadOnlyFileAccessRead,
	ReadOnlyFileAccessEof,
	ReadOnlyFileAccessIdleCheck,
	ReadOnlyFileAccessChDir,
	ReadOnlyFileAccessGetDir,
	ReadOnlyFileAccessFindFirst,
	ReadOnlyFileAccessFindNext,
	ReadOnlyFileAccessFindClose,
#endif
/*
	_FileOpen,
	_FileClose,
	_FileTell,
	_FileSeek,
	_FileRead,
	_FileEof,
	_FileIdleCheck,
	_FileChDir,
	_FileGetDir,
*/
} ;

// �X�g���[���f�[�^����p�֐��|�C���^�̃f�t�H���g�z��

// �t�@�C���̃X�g���[���f�[�^����p�|�C���^�\����
STREAMDATASHRED StreamFileFunction =
{
	( LONGLONG ( * )( void *StreamDataPoint ) )StreamTell,
	( int ( * )( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType ) )StreamSeek,
	( size_t ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )StreamRead,
	( int ( * )( void *StreamDataPoint ) )StreamEof,
	( int ( * )( void *StreamDataPoint ) )StreamIdleCheck,
	( int ( * )( void *StreamDataPoint ) )StreamClose,
} ;

// �������̃X�g���[���f�[�^����p�|�C���^�\����
STREAMDATASHRED StreamMemFunction =
{
	MemStreamTell,
	MemStreamSeek,
	MemStreamRead,
	MemStreamEof,
	MemStreamIdleCheck,
	MemStreamClose,
} ;

// �v���O���� --------------------------------------------------------------------

// �t�@�C���A�N�Z�X�����̏������E�I���֐�

// �t�@�C���A�N�Z�X�����̏�����
extern int InitializeFile( void )
{
	// ���ɏ���������Ă����牽�����Ȃ�
	if( GFileData.InitializeFlag == TRUE )
		return -1 ;

	// �t�@�C���ǂݍ��݃n���h���Ǘ���������������
	InitializeHandleManage( DX_HANDLETYPE_FILE, sizeof( FILEACCESSINFO ), MAX_FILE_NUM, InitializeFileHandle, TerminateFileHandle, DXSTRING( _T( "�t�@�C��" ) ) ) ;

	// �X�g���[���f�[�^�ǂ݂��ݐ���p�|�C���^�z��̃f�t�H���g�l���Z�b�g
	{
		StreamFileFunction.Read      = ( size_t   ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )StreamRead ;
		StreamFileFunction.Eof       = ( int      ( * )( void *StreamDataPoint ) )StreamEof ;
		StreamFileFunction.IdleCheck = ( int      ( * )( void *StreamDataPoint ) )StreamIdleCheck ;
		StreamFileFunction.Seek      = ( int      ( * )( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType ) )StreamSeek ;
		StreamFileFunction.Tell      = ( LONGLONG ( * )( void *StreamDataPoint ) )StreamTell ;
		StreamFileFunction.Close     = ( int      ( * )( void *StreamDataPoint ) )StreamClose ;
/*
		StreamFileFunction.Read  = ( size_t ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )fread ;
//		StreamFileFunction.Write = ( size_t ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )fwrite ;
		StreamFileFunction.Eof   = ( int ( * )( void *StreamDataPoint ) )feof ;
		StreamFileFunction.Seek  = ( int ( * )( void *StreamDataPoint, long SeekPoint, int SeekType ) )fseek ;
		StreamFileFunction.Tell  = ( long ( * )( void *StreamDataPoint ) )ftell ;
		StreamFileFunction.Close = ( int ( * )( void *StreamDataPoint ) )fclose ;
*/

		StreamMemFunction.Read      = MemStreamRead ;
//		StreamMemFunction.Write     = MemStreamWrite;
		StreamMemFunction.Eof       = MemStreamEof ;
		StreamMemFunction.IdleCheck = MemStreamIdleCheck ;
		StreamMemFunction.Seek      = MemStreamSeek ;
		StreamMemFunction.Tell      = MemStreamTell ;
		StreamMemFunction.Close     = MemStreamClose ;
	}

	// �������t���O�𗧂Ă�
	GFileData.InitializeFlag = TRUE ;

	// ����I��
	return 0 ;
}

// �t�@�C���A�N�Z�X�����̌�n��
extern int TerminateFile( void )
{
	// ���Ɍ�n������Ă����牽�����Ȃ�
	if( GFileData.InitializeFlag == FALSE )
		return -1 ;

	// �t�@�C���n���h���Ǘ����̌�n��
	TerminateHandleManage( DX_HANDLETYPE_FILE ) ;

	// �������t���O��|��
	GFileData.InitializeFlag = FALSE ;

	// ����I��
	return 0 ;
}

// �A�[�J�C�u�t�@�C���̓ǂݍ��݋@�\���g�����ǂ�����ݒ肷��( FALSE:�g�p���Ȃ�  TRUE:�g�p���� )
extern int NS_SetUseDXArchiveFlag( int Flag )
{
	int DefaultFlag ;

	// �t���O�̏�Ԃ������ꍇ�͉������Ȃ�
	if( NotUseDXArchive == !Flag ) return 0 ;

	// �W���̓ǂݍ��݋@�\���g�p����Ă��邩�ǂ������擾���Ă���
	DefaultFlag = NS_GetStreamFunctionDefault() ;

	// �t���O���X�V
	NotUseDXArchive = !Flag ;

	// ���ݕW���̓ǂݍ��݋@�\���g���Ă���ꍇ�͊֐��|�C���^���X�V����
	if( DefaultFlag == TRUE )
		NS_ChangeStreamFunction( NULL ) ;

	// �I��
	return 0 ;
}

// �c�w���C�u�����ŃX�g���[���f�[�^�A�N�Z�X�Ɏg�p����֐���ύX����
extern int NS_ChangeStreamFunction( const STREAMDATASHREDTYPE2 *StreamThread )
{
	// �֐��|�C���^���X�V����
	if( StreamThread == NULL )
	{
		// �A�[�J�C�u�t�@�C����ǂݍ��ދ@�\���g�p���邩�ǂ����ŃZ�b�g����֐��|�C���^��ύX
#ifndef DX_NON_DXA
		if( NotUseDXArchive == TRUE )
#endif
		{
			// �g�p���Ȃ��ꍇ
			StreamFunction.Open			= ReadOnlyFileAccessOpen ;
			StreamFunction.Close		= ReadOnlyFileAccessClose ;
			StreamFunction.Tell			= ReadOnlyFileAccessTell ;
			StreamFunction.Seek			= ReadOnlyFileAccessSeek ;
			StreamFunction.Read			= ReadOnlyFileAccessRead ;
			StreamFunction.Eof 			= ReadOnlyFileAccessEof ;
			StreamFunction.IdleCheck	= ReadOnlyFileAccessIdleCheck ;
			StreamFunction.ChDir		= ReadOnlyFileAccessChDir ;
			StreamFunction.GetDir		= ReadOnlyFileAccessGetDir ;
			StreamFunction.FindFirst	= ReadOnlyFileAccessFindFirst ;
			StreamFunction.FindNext		= ReadOnlyFileAccessFindNext ;
			StreamFunction.FindClose	= ReadOnlyFileAccessFindClose ;
		}
#ifndef DX_NON_DXA
		else
		{
			// �g�p����ꍇ
			StreamFunction.Open			= DXA_DIR_Open ;
			StreamFunction.Close		= DXA_DIR_Close ;
			StreamFunction.Tell			= DXA_DIR_Tell ;
			StreamFunction.Seek			= DXA_DIR_Seek ;
			StreamFunction.Read			= DXA_DIR_Read ;
			StreamFunction.Eof 			= DXA_DIR_Eof ;
			StreamFunction.IdleCheck	= DXA_DIR_IdleCheck ;
			StreamFunction.ChDir		= DXA_DIR_ChDir ;
			StreamFunction.GetDir		= DXA_DIR_GetDir ;
			StreamFunction.FindFirst	= DXA_DIR_FindFirst ;
			StreamFunction.FindNext		= DXA_DIR_FindNext ;
			StreamFunction.FindClose	= DXA_DIR_FindClose ;
		}
#endif
	}
	else
	{
		StreamFunction = *StreamThread ;
	}
	
	// �I��
	return 0 ;
}

// �c�w���C�u�����ŃX�g���[���f�[�^�A�N�Z�X�Ɏg�p����֐����f�t�H���g�̂��̂����ׂ�( TRUE:�f�t�H���g  FALSE:�f�t�H���g�ł͂Ȃ� )
extern int NS_GetStreamFunctionDefault( void )
{
	// �A�[�J�C�u�t�@�C����ǂݍ��ދ@�\���g�p���邩�ǂ����Ŕ�r����֐���ύX
#ifndef DX_NON_DXA
	if( NotUseDXArchive == TRUE )
#endif
	{
		// �g�p���Ȃ��ꍇ
		if( StreamFunction.Open			!= ReadOnlyFileAccessOpen       ) return FALSE ;
		if( StreamFunction.Close		!= ReadOnlyFileAccessClose      ) return FALSE ;
		if( StreamFunction.Tell			!= ReadOnlyFileAccessTell       ) return FALSE ;
		if( StreamFunction.Seek			!= ReadOnlyFileAccessSeek       ) return FALSE ;
		if( StreamFunction.Read			!= ReadOnlyFileAccessRead       ) return FALSE ;
		if( StreamFunction.Eof 			!= ReadOnlyFileAccessEof        ) return FALSE ;
		if( StreamFunction.ChDir		!= ReadOnlyFileAccessChDir      ) return FALSE ;
		if( StreamFunction.GetDir		!= ReadOnlyFileAccessGetDir     ) return FALSE ;
		if( StreamFunction.FindFirst 	!= ReadOnlyFileAccessFindFirst  ) return FALSE ;
		if( StreamFunction.FindNext 	!= ReadOnlyFileAccessFindNext   ) return FALSE ;
		if( StreamFunction.FindClose 	!= ReadOnlyFileAccessFindClose  ) return FALSE ;
	}
#ifndef DX_NON_DXA
	else
	{
		// �g�p����ꍇ
		if( StreamFunction.Open			!= DXA_DIR_Open ) return FALSE ;
		if( StreamFunction.Close		!= DXA_DIR_Close ) return FALSE ;
		if( StreamFunction.Tell			!= DXA_DIR_Tell ) return FALSE ;
		if( StreamFunction.Seek			!= DXA_DIR_Seek ) return FALSE ;
		if( StreamFunction.Read			!= DXA_DIR_Read ) return FALSE ;
		if( StreamFunction.Eof 			!= DXA_DIR_Eof ) return FALSE ;
		if( StreamFunction.ChDir		!= DXA_DIR_ChDir ) return FALSE ;
		if( StreamFunction.GetDir 		!= DXA_DIR_GetDir ) return FALSE ;
		if( StreamFunction.FindFirst 	!= DXA_DIR_FindFirst ) return FALSE ;
		if( StreamFunction.FindNext 	!= DXA_DIR_FindNext  ) return FALSE ;
		if( StreamFunction.FindClose 	!= DXA_DIR_FindClose ) return FALSE ;
	}
#endif
	
	return TRUE ;
}











// �⏕�֌W�֐�

// �t���p�X�ł͂Ȃ��p�X��������t���p�X�ɕϊ�����
// ( CurrentDir �̓t���p�X�ł���K�v������(����Ɂw\�x�������Ă������Ă��ǂ�) )
// ( CurrentDir �� NULL �̏ꍇ�͌��݂̃J�����g�f�B���N�g�����g�p���� )
extern int ConvertFullPath( const TCHAR *Src, TCHAR *Dest, const TCHAR *CurrentDir )
{
	return ConvertFullPathT_( Src, Dest, CurrentDir ) ;
}



















// �X�g���[���f�[�^�A�N�Z�X�֐�
extern DWORD_PTR StreamOpen( const TCHAR *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	return StreamFunction.Open( Path, UseCacheFlag, BlockReadFlag, UseASyncReadFlag ) ;
}

extern int StreamClose( DWORD_PTR Handle )
{
	return StreamFunction.Close( Handle ) ;
}

extern LONGLONG StreamTell( DWORD_PTR Handle )
{
	return StreamFunction.Tell( Handle ) ;
}

extern int StreamSeek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType )
{
	return StreamFunction.Seek( Handle, SeekPoint, SeekType ) ;
}

extern size_t StreamRead( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle )
{
	return StreamFunction.Read( Buffer, BlockSize, DataNum, Handle ) ;
}

extern int StreamEof( DWORD_PTR Handle )
{
	return StreamFunction.Eof( Handle ) ;
}

extern int StreamIdleCheck( DWORD_PTR Handle )
{
	return StreamFunction.IdleCheck( Handle ) ;
}

extern int StreamChDir( const TCHAR *Path )
{
	return StreamFunction.ChDir( Path ) ;
}

extern int StreamGetDir( TCHAR *Buffer )
{
	return StreamFunction.GetDir( Buffer ) ;
}

extern DWORD_PTR StreamFindFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	return StreamFunction.FindFirst( FilePath, Buffer ) ;
}

extern int StreamFindNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	return StreamFunction.FindNext( FindHandle, Buffer ) ;
}

extern int StreamFindClose( DWORD_PTR FindHandle )
{
	return StreamFunction.FindClose( FindHandle ) ;
}

extern	const STREAMDATASHREDTYPE2 *StreamGetStruct( void )
{
	return &StreamFunction ;
}












// �X�g���[���f�[�^�A�N�Z�X�p�֐��\���̊֌W

// �X�g���[���f�[�^�ǂ݂��ݐ���p�֐��|�C���^�\���̂̃t�@�C���p�\���̂𓾂�
extern STREAMDATASHRED *GetFileStreamDataShredStruct( void )
{
	return &StreamFileFunction ;
}

// �X�g���[���f�[�^�ǂ݂��ݐ���p�֐��|�C���^�\���̂���ؗp�\���̂𓾂�
extern STREAMDATASHRED *GetMemStreamDataShredStruct( void )
{
	return &StreamMemFunction ;
}













// �t�@�C���n���h���֌W

// �t�@�C���A�N�Z�X�n���h��������������
static int InitializeFileHandle( HANDLEINFO * )
{
	// ���ɉ������Ȃ�
	return 0 ;
}

// �t�@�C���A�N�Z�X�n���h���̌�n�����s��
static int TerminateFileHandle( HANDLEINFO *HandleInfo )
{
	FILEACCESSINFO *FileInfo = ( FILEACCESSINFO * )HandleInfo ;

	// �n���h���̃^�C�v�ɂ���ď����𕪊�
	switch( FileInfo->HandleType )
	{
	case FILEHANDLETYPE_NORMAL :
		// �t�@�C�������
		if( FileInfo->FilePointer )
		{
			FCLOSE( FileInfo->FilePointer ) ;
			FileInfo->FilePointer = 0 ;
		}
		break ;

	case FILEHANDLETYPE_FULLYLOAD :
		// �t�@�C���̓��e���i�[���Ă��������������
		if( FileInfo->FileImage != NULL )
		{
			DXFREE( FileInfo->FileImage ) ;
			FileInfo->FileImage = NULL ;
		}
		break ;
	}

	// �I��
	return 0 ;
}

// FileRead_open �̎������֐�
static int FileRead_open_Static(
	int FileHandle,
	const TCHAR *FilePath,
	int ASync,
	int ASyncThread
)
{
	FILEACCESSINFO *FileInfo ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}
	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	// �t�@�C�����J��
	if( ASync )
	{
		FileInfo->FilePointer = FOPEN_ASYNC( FilePath ) ;
	}
	else
	{
		FileInfo->FilePointer = FOPEN( FilePath ) ;
	}

	// �J���Ȃ�������G���[
	if( FileInfo->FilePointer == 0 )
		return -1 ;

	// ����I��
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_open �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void FileRead_open_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int FileHandle ;
	const TCHAR *FilePath ;
	int ASync ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FilePath = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	ASync = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = FileRead_open_Static( FileHandle, FilePath, ASync, TRUE ) ;

	DecASyncLoadCount( FileHandle ) ;
	if( Result < 0 )
	{
		SubHandle( FileHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// �t�@�C�����J��
extern int FileRead_open_UseGParam(
	const TCHAR *FilePath,
	int ASync,
	int ASyncLoadFlag
)
{
	int FileHandle ;
	FILEACCESSINFO *FileInfo ;

	// ����������Ă��Ȃ������珉��������
	if( GFileData.InitializeFlag == FALSE )
	{
		InitializeFile() ;
	}

	// �n���h���̍쐬
	FileHandle = AddHandle( DX_HANDLETYPE_FILE ) ;
	if( FileHandle == -1 ) return -1 ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;
	FileInfo->HandleType = FILEHANDLETYPE_NORMAL ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FilePath, FullPath ) ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;
		AddASyncLoadParamInt( NULL, &Addr, ASync ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = FileRead_open_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ASync ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( FileHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( FileRead_open_Static( FileHandle, FilePath, ASync, FALSE ) < 0 )
			goto ERR ;
	}

	// �I��
	return FileHandle ;

ERR :
	SubHandle( FileHandle ) ;

	return 0 ;
}

// �t�@�C���A�N�Z�X�֐�
extern int NS_FileRead_open( const TCHAR *FilePath , int ASync )
{
	return FileRead_open_UseGParam( FilePath, ASync, GetASyncLoadFlag() ) ;
}

extern LONGLONG NS_FileRead_size( const TCHAR *FilePath )
{
	LONGLONG Result ;
	DWORD_PTR fp ;
	fp = FOPEN( FilePath ) ;
	if( fp == 0 ) return -1 ;
	FSEEK( fp, 0L, SEEK_END ) ;
	Result = FTELL( fp ) ;
	FCLOSE( fp ) ;
	return Result ;
}

extern int NS_FileRead_close( int FileHandle )
{
	return SubHandle( FileHandle ) ;
}

extern LONGLONG NS_FileRead_tell( int FileHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	return FTELL( FileInfo->FilePointer ) ;
}

// FileRead_seek �̎������֐�
static int FileRead_seek_Static( int FileHandle, LONGLONG Offset, int Origin, int ASyncThread )
{
	FILEACCESSINFO *FileInfo ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	return FSEEK( FileInfo->FilePointer, Offset, Origin ) ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_seek �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void FileRead_seek_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	FILEACCESSINFO *FileInfo ;
	int FileHandle ;
	LONGLONG Offset ;
	int Origin ;
	int Addr ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Offset = GetASyncLoadParamLONGLONG( AParam->Data, &Addr ) ;
	Origin = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	if( !FILEHCHK_ASYNC( FileHandle, FileInfo ) )
	{
		FileInfo->HandleInfo.ASyncLoadResult = FileRead_seek_Static( FileHandle, Offset, Origin, TRUE ) ;
	}

	DecASyncLoadCount( FileHandle ) ;
}

#endif // DX_NON_ASYNCLOAD

// �t�@�C���|�C���^�̈ʒu��ύX����
extern int FileRead_seek_UseGParam( int FileHandle, LONGLONG Offset, int Origin, int ASyncLoadFlag )
{
	FILEACCESSINFO *FileInfo ;
	int Result = -1 ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamLONGLONG( NULL, &Addr, Offset ) ;
		AddASyncLoadParamInt( NULL, &Addr, Origin ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto END ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = FileRead_seek_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamLONGLONG( AParam->Data, &Addr, Offset ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Origin ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto END ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( FileHandle, AParam->Index ) ;

		Result = 0 ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		Result = FileRead_seek_Static( FileHandle, Offset, Origin, FALSE ) ;
	}

#ifndef DX_NON_ASYNCLOAD
END :
#endif

	// �I��
	return Result ;
}

extern int NS_FileRead_seek( int FileHandle, LONGLONG Offset, int Origin )
{
	return FileRead_seek_UseGParam( FileHandle, Offset, Origin, GetASyncLoadFlag() ) ;
}

// FileRead_read �̎������֐�
static int FileRead_read_Static(
	int FileHandle,
	void *Buffer,
	int ReadSize,
	int ASyncThread
)
{
	FILEACCESSINFO *FileInfo ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	return ( int )FREAD( Buffer, 1, ReadSize, FileInfo->FilePointer ) ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_read �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void FileRead_read_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int FileHandle ;
	void *Buffer ;
	int ReadSize ;
	int Addr ;
	FILEACCESSINFO *FileInfo ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Buffer = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	ReadSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	if( !FILEHCHK_ASYNC( FileHandle, FileInfo ) )
	{
		FileInfo->HandleInfo.ASyncLoadResult = FileRead_read_Static( FileHandle, Buffer, ReadSize, TRUE ) ;
	}

	DecASyncLoadCount( FileHandle ) ;
}

#endif // DX_NON_ASYNCLOAD


// �t�@�C������f�[�^��ǂݍ���
extern int FileRead_read_UseGParam(
	int FileHandle,
	void *Buffer,
	int ReadSize,
	int ASyncLoadFlag
)
{
	int Result = -1 ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, Buffer ) ;
		AddASyncLoadParamInt( NULL, &Addr, ReadSize ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto END ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = FileRead_read_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, Buffer ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ReadSize ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto END ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( FileHandle, AParam->Index ) ;

		Result = 0 ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		Result = FileRead_read_Static( FileHandle, Buffer, ReadSize, FALSE ) ;
	}

#ifndef DX_NON_ASYNCLOAD
END :
#endif

	// �I��
	return Result ;
}

extern int NS_FileRead_read( void *Buffer, int ReadSize, int FileHandle )
{
	return FileRead_read_UseGParam( FileHandle, Buffer, ReadSize, GetASyncLoadFlag() ) ;
}

extern int NS_FileRead_idle_chk( int FileHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( FileInfo->HandleInfo.ASyncLoadCount != 0 )
		return FALSE ;
#endif // DX_NON_ASYNCLOAD

	return FIDLECHK( FileInfo->FilePointer ) ;
}

// �t�@�C���̏I�[���ǂ����𓾂�
extern int NS_FileRead_eof( int FileHandle )
{
	LONGLONG FileSize, NowPoint ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	NowPoint = FTELL( FileInfo->FilePointer                     ) ;
	           FSEEK( FileInfo->FilePointer,        0, SEEK_END ) ;
	FileSize = FTELL( FileInfo->FilePointer                     ) ;
	           FSEEK( FileInfo->FilePointer, NowPoint, SEEK_SET ) ;

	return NowPoint == FileSize ;
}

extern int NS_FileRead_gets( TCHAR *Buffer, int BufferSize, int FileHandle )
{
	LONGLONG i, ReadSize ;
	LONGLONG NowPos ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	if( FEOF( FileInfo->FilePointer ) != 0 )
	{
		return -1 ;
	}

	NowPos   = FTELL( FileInfo->FilePointer ) ;
	ReadSize = FREAD( Buffer, sizeof( TCHAR ), BufferSize - 1, FileInfo->FilePointer ) ;FSYNC( FileInfo->FilePointer )
	if( ReadSize == 0 )
	{
		return -1 ;
	}

#ifdef UNICODE
	if( *( ( WORD * )&Buffer[ 0 ] ) == 0xfeff )
	{
		FSEEK( FileInfo->FilePointer, 2, SEEK_SET ) ;
		NowPos   = FTELL( FileInfo->FilePointer ) ;
		ReadSize = FREAD( Buffer, sizeof( TCHAR ), BufferSize - 1, FileInfo->FilePointer ) ;FSYNC( FileInfo->FilePointer )
	}
#endif

	for( i = 0 ; i < ReadSize ; )
	{
		if( Buffer[i] == _T( '\0' ) ) break ;
		if( _TMULT( Buffer[i], _GET_CHARSET() ) == TRUE )
		{
			if( i + 2 > ReadSize )
			{
				FSEEK( FileInfo->FilePointer, NowPos + i * sizeof( TCHAR ), SEEK_SET ) ;
				break ;
			}
			i += 2 ;
		}
		else
		{
			if( Buffer[i] == _T( '\r' ) && Buffer[i+1] == _T( '\n' ) )
			{
				FSEEK( FileInfo->FilePointer, NowPos + ( i + 2 ) * sizeof( TCHAR ), SEEK_SET ) ;
				break ;
			}
			i ++ ;
		}
	}
	Buffer[i] = _T( '\0' ) ;

	return ( int )i ;
}

// �t�@�C������ꕶ���ǂݏo��
extern TCHAR NS_FileRead_getc( int FileHandle )
{
	size_t ReadSize ;
	TCHAR c ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return _T( '\0' ) ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	if( FEOF( FileInfo->FilePointer ) != 0 )
	{
		return ( TCHAR )EOF ;
	}

	c = 0 ;
	ReadSize = FREAD( &c, sizeof( TCHAR ), 1, FileInfo->FilePointer ) ;
	FSYNC( FileInfo->FilePointer )
	if( ReadSize == 0 )
	{
		return ( TCHAR )EOF ;
	}

	return c ;
}

// �t�@�C�����珑�������ꂽ�f�[�^��ǂݏo��
extern int NS_FileRead_scanf( int FileHandle, const TCHAR *Format, ... )
{
	va_list param;
	int Result;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	va_start( param, Format );
#ifdef UNICODE
	Result = FileRead_scanf_baseW( FileInfo->FilePointer, _DXWTP( Format ), param ) ;
#else
	Result = FileRead_scanf_base( FileInfo->FilePointer, Format, param ) ;
#endif
	va_end( param );

	return Result;
}

// �t�@�C���̏����擾����(�߂�l  -1:�G���[  -1�ȊO:�t�@�C�����n���h��  )
extern DWORD_PTR NS_FileRead_createInfo( const TCHAR *ObjectPath )
{
	int Num, i ;
	DWORD_PTR FindHandle;
	FILEINFO *FileInfo;
	FILEINFOLIST *FileInfoList;

	FindHandle = FFINDFIRST( ObjectPath, NULL );
	if( FindHandle == ( DWORD_PTR )-1 ) return ( DWORD_PTR )-1;

	Num = 0;
	do
	{
		Num ++ ;
	}while( FFINDNEXT( FindHandle, NULL ) == 0 );

	FFINDCLOSE( FindHandle );

	FileInfoList = (FILEINFOLIST *)DXALLOC( sizeof( FILEINFOLIST ) + sizeof( FILEINFO ) * Num );
	if( FileInfoList == NULL ) return ( DWORD_PTR )-1;

	FileInfoList->Num = Num;
	FileInfo = (FILEINFO *)( FileInfoList + 1 );
	FindHandle = FFINDFIRST( ObjectPath, FileInfo );
	FileInfo ++ ;
	for( i = 1; i < Num; i ++, FileInfo ++ )
		FFINDNEXT( FindHandle, FileInfo );

	FFINDCLOSE( FindHandle );

	return (DWORD_PTR)FileInfoList;
}

// �t�@�C�����n���h�����̃t�@�C���̐����擾����
extern int NS_FileRead_getInfoNum( DWORD_PTR FileInfoHandle )
{
	FILEINFOLIST *FileInfoList;

	FileInfoList = (FILEINFOLIST *)FileInfoHandle;

	return FileInfoList->Num;
}

// �t�@�C�����n���h�����̃t�@�C���̏����擾����
extern int NS_FileRead_getInfo( int Index, FILEINFO *Buffer, DWORD_PTR FileInfoHandle )
{
	FILEINFOLIST *FileInfoList;

	FileInfoList = (FILEINFOLIST *)FileInfoHandle;
	if( Index < 0 || FileInfoList->Num <= Index ) return -1;

	*Buffer = ((FILEINFO *)( FileInfoList + 1 ))[Index];

	return 0;
}

// �t�@�C�����n���h�����폜����
extern int NS_FileRead_deleteInfo( DWORD_PTR FileInfoHandle )
{
	DXFREE( (FILEINFOLIST *)FileInfoHandle );

	return 0;
}

// �w��̃t�@�C�����̓t�H���_�̏����擾���A�t�@�C�������n���h�����쐬����( �߂�l: -1=�G���[  -1�ȊO=�t�@�C�������n���h�� )
extern DWORD_PTR NS_FileRead_findFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	return FFINDFIRST( FilePath, Buffer );
}

// �����̍��v���鎟�̃t�@�C���̏����擾����( �߂�l: -1=�G���[  0=���� )
extern int NS_FileRead_findNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	return FFINDNEXT( FindHandle, Buffer );
}

// �t�@�C�������n���h�������( �߂�l: -1=�G���[  0=���� )
extern int NS_FileRead_findClose( DWORD_PTR FindHandle )
{
	return FFINDCLOSE( FindHandle );
}








// FileRead_fullyLoad �̎������֐�
static int FileRead_fullyLoad_Static(
	int FileHandle,
	const TCHAR *FilePath,
	int ASyncThread
)
{
	FILEACCESSINFO *FileInfo ;
	DWORD_PTR FilePointer ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}
	if( FileInfo->HandleType != FILEHANDLETYPE_FULLYLOAD )
		return -1 ;

	// �t�@�C�����J��
	FilePointer = FOPEN( FilePath ) ;

	// �J���Ȃ�������G���[
	if( FilePointer == 0 )
		return -1 ;

	// �t�@�C���̃T�C�Y���ۂ��Ɠǂݍ���
	FSEEK( FilePointer, 0, SEEK_END ) ;
	FileInfo->FileSize = FTELL( FilePointer ) ;
	FSEEK( FilePointer, 0, SEEK_SET ) ;
	FileInfo->FileImage = DXALLOC( ( size_t )FileInfo->FileSize ) ;
	if( FileInfo->FileImage == NULL )
	{
		FCLOSE( FilePointer ) ;
		FilePointer = 0 ;
		return -1 ;
	}
	FREAD( FileInfo->FileImage, FileInfo->FileSize, 1, FilePointer ) ;

	// �t�@�C�������
	FCLOSE( FilePointer ) ;
	FilePointer = 0 ;

	// ����I��
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_fullyLoad �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void FileRead_fullyLoad_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int FileHandle ;
	const TCHAR *FilePath ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FilePath = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = FileRead_fullyLoad_Static( FileHandle, FilePath, TRUE ) ;

	DecASyncLoadCount( FileHandle ) ;
	if( Result < 0 )
	{
		SubHandle( FileHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// �w��̃t�@�C���̓��e��S�ă������ɓǂݍ��݁A���̏��̃A�N�Z�X�ɕK�v�ȃn���h����Ԃ�( �߂�l  -1:�G���[  -1�ȊO:�n���h�� )�A�g���I�������n���h���� FileRead_fullyLoad_delete �ō폜����K�v������܂�
extern	int FileRead_fullyLoad_UseGParam( const TCHAR *FilePath, int ASyncLoadFlag )
{
	int FileHandle ;
	FILEACCESSINFO *FileInfo ;

	// �n���h���̍쐬
	FileHandle = AddHandle( DX_HANDLETYPE_FILE ) ;
	if( FileHandle == -1 ) return -1 ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;
	FileInfo->HandleType = FILEHANDLETYPE_FULLYLOAD ;

	// ���̑��f�[�^��������
	FileInfo->FileImage = NULL ;
	FileInfo->FileSize = -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FilePath, FullPath ) ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = FileRead_fullyLoad_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( FileHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( FileRead_fullyLoad_Static( FileHandle, FilePath, FALSE ) < 0 )
			goto ERR ;
	}

	// �I��
	return FileHandle ;

ERR :
	SubHandle( FileHandle ) ;

	return -1 ;
}

extern	int NS_FileRead_fullyLoad( const TCHAR *FilePath )
{
	return FileRead_fullyLoad_UseGParam( FilePath, GetASyncLoadFlag() ) ;
}

// FileRead_fullyLoad �œǂݍ��񂾃t�@�C��������������폜����
extern	int NS_FileRead_fullyLoad_delete( int FLoadHandle )
{
	return SubHandle( FLoadHandle ) ;
}

// FileRead_fullyLoad �œǂݍ��񂾃t�@�C���̓��e���i�[�����������A�h���X���擾����
extern	const void* NS_FileRead_fullyLoad_getImage( int FLoadHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FLoadHandle, FileInfo ) )
		return NULL ;

	if( FileInfo->HandleType != FILEHANDLETYPE_FULLYLOAD )
		return NULL ;

	// �t�@�C�����i�[���Ă��郁�����A�h���X��Ԃ�
	return FileInfo->FileImage ;
}

// FileRead_fullyLoad �œǂݍ��񂾃t�@�C���̃T�C�Y���擾����
extern	LONGLONG NS_FileRead_fullyLoad_getSize(	int FLoadHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FLoadHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_FULLYLOAD )
		return -1 ;

	// �t�@�C���̃T�C�Y��Ԃ�
	return FileInfo->FileSize ;
}







































// �������ɒu���ꂽ�f�[�^���t�@�C���̃f�[�^�ɗႦ�Ă������߂̊֐�
extern void *MemStreamOpen( const void *DataBuffer, unsigned int DataSize )
{
	MEMSTREAMDATA *NewMemData ;

	// �������̊m��
	if( ( NewMemData = ( MEMSTREAMDATA * )DXALLOC( sizeof( MEMSTREAMDATA ) ) ) == NULL ) return NULL ;
	_MEMSET( NewMemData, 0, sizeof( MEMSTREAMDATA) ) ;

	// �f�[�^�̃Z�b�g
	NewMemData->DataBuffer = ( unsigned char * )DataBuffer ;
	NewMemData->DataPoint = 0 ;
	NewMemData->DataSize = DataSize ;
	NewMemData->EOFFlag = FALSE ;

	// �I��
	return NewMemData ;
}


extern int MemStreamClose( void *StreamDataPoint )
{
	// �������̉��
	DXFREE( StreamDataPoint ) ;

	// �I��
	return 0 ;
}

static LONGLONG MemStreamTell( void *StreamDataPoint )
{
	// �f�[�^�|�C���^��Ԃ�
	return ( LONGLONG )( ( MEMSTREAMDATA * )StreamDataPoint )->DataPoint ;
}

static int MemStreamSeek( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;
	LONGLONG NewPoint = 0 ;

	// �V�[�N�^�C�v�ɂ���ď����𕪊�
	switch( SeekType )
	{
	case STREAM_SEEKTYPE_SET :
		NewPoint = SeekPoint ;
		break ;

	case STREAM_SEEKTYPE_END :
		NewPoint = ( LONGLONG )( SeekPoint + MemData->DataSize ) ;
		break ;

	case STREAM_SEEKTYPE_CUR :
		NewPoint = ( LONGLONG )( SeekPoint + MemData->DataPoint ) ;
		break ;
	}

	// �␳
	if( NewPoint > ( LONGLONG )MemData->DataSize )
	{
		MemData->DataPoint = MemData->DataSize ;
	}
	else
	{
		if( NewPoint < 0 )
		{
			MemData->DataPoint = 0 ;
		}
		else
		{
			MemData->DataPoint = ( size_t )NewPoint ;
		}
	}

	MemData->EOFFlag = FALSE ;

	// �I��
	return 0 ;
} ;

static size_t MemStreamRead( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;

	// EOF �̌��o
	if( MemData->DataPoint == ( int )MemData->DataSize )
	{
		MemData->EOFFlag = TRUE ;
		return 0 ;
	}
	else
	{
		// �ǂ݂��ރf�[�^�̐��̕␳
		if( BlockSize * DataNum > ( int )( MemData->DataSize - MemData->DataPoint ) )
		{
			DataNum = ( ( MemData->DataSize - MemData->DataPoint ) / BlockSize ) * BlockSize ;
		}

		if( DataNum != 0 )
		{
			_MEMCPY( Buffer, MemData->DataBuffer + MemData->DataPoint, DataNum * BlockSize ) ;
			MemData->DataPoint += BlockSize * DataNum ;
		}
		else
		{
			size_t MoveSize ;

			MoveSize = MemData->DataSize - MemData->DataPoint ;
			if( MoveSize != 0 )
			{
				_MEMCPY( Buffer, MemData->DataBuffer + MemData->DataPoint, MoveSize ) ;
				MemData->DataPoint += MoveSize ;
				DataNum = 1 ;
			}
		}

		MemData->EOFFlag = FALSE ;
	}

	return DataNum ;
}
	
static size_t MemStreamWrite( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;

	// EOF �̌��o
	if( MemData->DataPoint == ( int )MemData->DataSize )
	{
		MemData->EOFFlag = TRUE ;
		return 0 ;
	}
	else
	{
		// �������ރf�[�^�̐��̕␳
		if( BlockSize * DataNum > ( int )( MemData->DataSize - MemData->DataPoint ) )
		{
			DataNum = ( ( MemData->DataSize - MemData->DataPoint ) / BlockSize ) * BlockSize ;
		}

		if( DataNum != 0 )
		{
			_MEMCPY( MemData->DataBuffer + MemData->DataPoint, Buffer, DataNum * BlockSize ) ;
			MemData->DataPoint += BlockSize * DataNum ;
		}

		MemData->EOFFlag = FALSE ;
	}

	return DataNum ;
}

static int MemStreamEof( void *StreamDataPoint )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;

	// EOF ��Ԃ�
	return MemData->EOFFlag ;
}

static int MemStreamIdleCheck( void * /*StreamDataPoint*/ )
{
	// ��ɃA�C�h�����O���
	return TRUE ;
}























// �t���p�X�ł͂Ȃ��p�X��������t���p�X�ɕϊ�����
// ( CurrentDir �̓t���p�X�ł���K�v������(����Ɂw\�x�������Ă������Ă��ǂ�) )
// ( CurrentDir �� NULL �̏ꍇ�͌��݂̃J�����g�f�B���N�g�����g�p���� )
extern int ConvertFullPathT_( const TCHAR *Src, TCHAR *Dest, const TCHAR *CurrentDir )
{
#ifdef UNICODE
	return ConvertFullPathW_( Src, Dest, CurrentDir ) ;
#else
	return ConvertFullPath_( Src, Dest, CurrentDir ) ;
#endif
}

// �t���p�X�ł͂Ȃ��p�X��������t���p�X�ɕϊ�����
// ( CurrentDir �̓t���p�X�ł���K�v������(����Ɂw\�x�������Ă������Ă��ǂ�) )
// ( CurrentDir �� NULL �̏ꍇ�͌��݂̃J�����g�f�B���N�g�����g�p���� )
extern	int ConvertFullPath_( const char *Src, char *Dest, const char *CurrentDir )
{
	int i, j, k ;
	char iden[256], cur[MAX_PATH] ;

	if( CurrentDir == NULL )
	{
#ifdef UNICODE
		wchar_t curW[ MAX_PATH ];

		FGETDIR( curW ) ;
		WCharToMBChar( _GET_CODEPAGE(), ( DXWCHAR * )curW, cur, MAX_PATH ) ;
#else
		FGETDIR( cur ) ;
#endif
		CurrentDir = cur ;
	}

	if( Src == NULL )
	{
		_STRCPY( Dest, CurrentDir ) ;
		goto END ;
	}

	i = 0 ;
	j = 0 ;
	k = 0 ;
	
	// �ŏ��Ɂw\�x���́w/�x���Q��A���ő����Ă���ꍇ�̓l�b�g���[�N����Ă���Ɣ��f
	if( ( Src[0] == '\\' && Src[1] == '\\' ) ||
		( Src[0] == '/'  && Src[1] == '/'  ) )
	{
		Dest[0] = '\\';
		Dest[1] = '\0';

		i += 2;
		j ++ ;
	}
	else
	// �ŏ����w\�x���́w/�x�̏ꍇ�̓J�����g�h���C�u�̃��[�g�f�B���N�g���܂ŗ�����
	if( Src[0] == '\\' || Src[0] == '/' )
	{
		Dest[0] = CurrentDir[0] ;
		Dest[1] = CurrentDir[1] ;
		Dest[2] = '\0' ;

		i ++ ;
		j = 2 ;
	}
	else
	// �h���C�u����������Ă����炻�̃h���C�u��
	if( Src[1] == ':' )
	{
		Dest[0] = Src[0] ;
		Dest[1] = Src[1] ;
		Dest[2] = '\0' ;

		i = 2 ;
		j = 2 ;

		if( Src[i] == '\\' || Src[i] == '/' ) i ++ ;
	}
	else
	// ����ȊO�̏ꍇ�̓J�����g�f�B���N�g��
	{
		_STRCPY( Dest, CurrentDir ) ;
		j = _STRLEN( Dest ) ;
		if( Dest[j-1] == '\\' || Dest[j-1] == '/' )
		{
			Dest[j-1] = '\0' ;
			j -- ;
		}
	}

	for(;;)
	{
		switch( Src[i] )
		{
		case '\0' :
			if( k != 0 )
			{
				Dest[j] = '\\' ; j ++ ;
				_STRCPY( &Dest[j], iden ) ;
				j += k ;
				k = 0 ;
			}
			goto END ;

		case '\\' :
		case '/' :
			// �����񂪖���������X�L�b�v
			if( k == 0 )
			{
				i ++ ;
				break ;
			}
			if( _STRCMP( iden, "." ) == 0 )
			{
				// �Ȃɂ����Ȃ�
			}
			else
			if( _STRCMP( iden, ".." ) == 0 )
			{
				// ����̃f�B���N�g����
				j -- ;
				for(;;)
				{
					if( Dest[j] == '\\' || Dest[j] == '/' || Dest[j] == ':' ) break ;
					j -= CheckMultiByteString( Dest, j - 1, _GET_CHARSET() ) == 2 ? 2 : 1 ;
				}
//				while( Dest[j] != '\\' && Dest[j] != '/' && Dest[j] != ':' ) j -- ;
				if( Dest[j] != ':' ) Dest[j] = '\0' ;
				else j ++ ;
			}
			else
			{
				Dest[j] = '\\' ; j ++ ;
				_STRCPY( &Dest[j], iden ) ;
				j += k ;
			}

			k = 0 ;
			i ++ ;
			break ;
		
		default :
			if( CheckMultiByteChar( Src[i], _GET_CHARSET() ) == FALSE )
			{
				iden[k] = Src[i] ;
				iden[k+1] = 0 ; 
				k ++ ;
				i ++ ;
			}
			else
			{
				*(( unsigned short * )&iden[k] ) = *(( unsigned short * )&Src[i] ) ;
				iden[k+2] = '\0' ;
				k += 2 ;
				i += 2 ;
			}
			break ;
		}
	}
	
END :
	// ����I��
	return 0 ;
}

// �t���p�X�ł͂Ȃ��p�X��������t���p�X�ɕϊ�����
// ( CurrentDir �̓t���p�X�ł���K�v������(����Ɂw\�x�������Ă������Ă��ǂ�) )
// ( CurrentDir �� NULL �̏ꍇ�͌��݂̃J�����g�f�B���N�g�����g�p���� )
extern	int ConvertFullPathW_( const wchar_t *Src, wchar_t *Dest, const wchar_t *CurrentDir )
{
	int i, j, k ;
	wchar_t iden[256], cur[MAX_PATH] ;

	if( CurrentDir == NULL )
	{
#ifdef UNICODE
		FGETDIR( cur ) ;
#else
		char curA[ MAX_PATH ] ;
		FGETDIR( curA ) ;
		MBCharToWChar( _GET_CODEPAGE(), curA, ( DXWCHAR * )cur, MAX_PATH ) ;
#endif
		CurrentDir = cur ;
	}

	if( Src == NULL )
	{
		_WCSCPY( Dest, CurrentDir ) ;
		goto END ;
	}

	i = 0 ;
	j = 0 ;
	k = 0 ;
	
	// �ŏ��Ɂw\�x���́w/�x���Q��A���ő����Ă���ꍇ�̓l�b�g���[�N����Ă���Ɣ��f
	if( ( Src[0] == L'\\' && Src[1] == L'\\' ) ||
		( Src[0] == L'/'  && Src[1] == L'/'  ) )
	{
		Dest[0] = L'\\';
		Dest[1] = L'\0';

		i += 2;
		j ++ ;
	}
	else
	// �ŏ����w\�x���́w/�x�̏ꍇ�̓J�����g�h���C�u�̃��[�g�f�B���N�g���܂ŗ�����
	if( Src[0] == L'\\' || Src[0] == L'/' )
	{
		Dest[0] = CurrentDir[0] ;
		Dest[1] = CurrentDir[1] ;
		Dest[2] = L'\0' ;

		i ++ ;
		j = 2 ;
	}
	else
	// �h���C�u����������Ă����炻�̃h���C�u��
	if( Src[1] == L':' )
	{
		Dest[0] = Src[0] ;
		Dest[1] = Src[1] ;
		Dest[2] = L'\0' ;

		i = 2 ;
		j = 2 ;

		if( Src[i] == L'\\' || Src[i] == L'/' ) i ++ ;
	}
	else
	// ����ȊO�̏ꍇ�̓J�����g�f�B���N�g��
	{
		_WCSCPY( Dest, CurrentDir ) ;
		j = _WCSLEN( Dest ) ;
		if( Dest[j-1] == L'\\' || Dest[j-1] == L'/' )
		{
			Dest[j-1] = L'\0' ;
			j -- ;
		}
	}

	for(;;)
	{
		switch( Src[i] )
		{
		case L'\0' :
			if( k != 0 )
			{
				Dest[j] = L'\\' ; j ++ ;
				_WCSCPY( &Dest[j], iden ) ;
				j += k ;
				k = 0 ;
			}
			goto END ;

		case L'\\' :
		case L'/' :
			// �����񂪖���������X�L�b�v
			if( k == 0 )
			{
				i ++ ;
				break ;
			}
			if( _WCSCMP( iden, L"." ) == 0 )
			{
				// �Ȃɂ����Ȃ�
			}
			else
			if( _WCSCMP( iden, L".." ) == 0 )
			{
				// ����̃f�B���N�g����
				j -- ;
//				while( Dest[j] != L'\\' && Dest[j] != L'/' && Dest[j] != L':' ) j -- ;
				for(;;)
				{
					if( Dest[j] == L'\\' || Dest[j] == L'/' || Dest[j] == L':' ) break ;
					j -= CheckUTF16HChar( Dest, j - 1 ) == 2 ? 2 : 1 ;
				}
				if( Dest[j] != L':' ) Dest[j] = L'\0' ;
				else j ++ ;
			}
			else
			{
				Dest[j] = L'\\' ; j ++ ;
				_WCSCPY( &Dest[j], iden ) ;
				j += k ;
			}

			k = 0 ;
			i ++ ;
			break ;
		
		default :
			if( CheckUTF16H( Src[i] ) == FALSE )
			{
				iden[k] = Src[i] ;
				iden[k+1] = 0 ; 
				k ++ ;
				i ++ ;
			}
			else
			{
				iden[k]   = Src[i] ;
				iden[k+1] = Src[i+1] ;
				iden[k+2] = '\0' ;
				k += 2 ;
				i += 2 ;
			}
			break ;
		}
	}
	
END :
	// ����I��
	return 0 ;
}

// �w��̃t�@�C���p�X���w��̃t�H���_�p�X���瑊�΃A�N�Z�X���邽�߂̑��΃p�X���쐬����
// ( FilePath �� StartFolderPath ���t���p�X�ł͂Ȃ������ꍇ�͊֐����Ńt���p�X������܂� )
// StartFolderPath �̖��[�� / or \ �������Ă���肠��܂���
extern int CreateRelativePath_( const char *FilePath, const char *StartFolderPath, char *Dest )
{
	char filefull[ 512 ], filename[ 256 ], endfull[ 512 ], startfull[ 512 ] ;
	char endstr[ 256 ], startstr[ 256 ] ;
	int endlen, startlen, destlen ;
	char *ep, *sp ;

	// �t���p�X��
	ConvertFullPath_( FilePath, filefull ) ;
	AnalysisFileNameAndDirPath_( filefull, filename, endfull ) ;
	ConvertFullPath_( StartFolderPath, startfull ) ;
	startlen = _STRLEN( startfull ) ;
	if( startfull[ startlen - 1 ] == '\\' || startfull[ startlen - 1 ] == '/' )
		startfull[ startlen - 1 ] = '\0' ;

	// �h���C�u��������擾����
	endlen = AnalysisDriveName_( endfull, endstr ) ;
	startlen = AnalysisDriveName_( startfull, startstr ) ;

	// �h���C�u���������t�@�C���p�X�̃t���p�X��������
	if( _STRCMP( endstr, startstr ) != 0 )
	{
		_STRCPY( Dest, filefull ) ;
		return 0 ;
	}
	ep = &endfull[ endlen ] ;
	sp = &startfull[ startlen ] ;

	// �t�H���_�����Ⴄ�Ƃ���܂œǂݐi�߂�
	for(;;)
	{
		// \ or / ���΂�
		while( *ep == '\\' || *ep == '/' ) ep ++ ;
		while( *sp == '\\' || *sp == '/' ) sp ++ ;

		// �I�[�����������烋�[�v�𔲂���
		if( *ep == '\0' || *sp == '\0' ) break ;

		// �t�H���_�p�X���擾����
		endlen = AnalysisDirectoryName_( ep, endstr ) ;
		startlen = AnalysisDirectoryName_( sp, startstr ) ;

		// �t�H���_�p�X��������烋�[�v�𔲂���
		if( _STRCMP( endstr, startstr ) != 0 ) break ;
		ep += endlen ;
		sp += startlen ;
	}

	// ���݂������t�H���_�������ꍇ�̓t�@�C�������o��
	if( *ep == '\0' && *sp == '\0' )
	{
		_STRCPY( Dest, filename ) ;
		return 0 ;
	}

	// �J�n�t�H���_�ƈႤ�t�H���_�ɂ���ꍇ�͂܂��J�n�t�H���_�����݂̃t�H���_�܂ō~���p�X�̍쐬
	destlen = 0 ;
	if( *sp != '\0' )
	{
		Dest[ 0 ] = '\0' ;
		destlen = 0 ;
		for(;;)
		{
			startlen = AnalysisDirectoryName_( sp, startstr ) ;
			sp += startlen ;
			while( *sp == '\\' || *sp == '/' ) sp ++ ;
			_STRCPY( Dest + destlen, "..\\" ) ;
			destlen += 3 ;
			if( *sp == '\0' ) break ;
		}
	}

	// �ړI�̃t�@�C��������t�H���_�܂łƃt�@�C�������Ȃ��Ċ���
	_SPRINTF( Dest + destlen, "%s\\%s", ep, filename ) ;
	return 0 ;
}


// �w��̃t�@�C���p�X���w��̃t�H���_�p�X���瑊�΃A�N�Z�X���邽�߂̑��΃p�X���쐬����
// ( FilePath �� StartFolderPath ���t���p�X�ł͂Ȃ������ꍇ�͊֐����Ńt���p�X������܂� )
// StartFolderPath �̖��[�� / or \ �������Ă���肠��܂���
extern int CreateRelativePathW_( const wchar_t *FilePath, const wchar_t *StartFolderPath, wchar_t *Dest )
{
	wchar_t filefull[ 512 ], filename[ 256 ], endfull[ 512 ], startfull[ 512 ] ;
	wchar_t endstr[ 256 ], startstr[ 256 ] ;
	int endlen, startlen, destlen ;
	wchar_t *ep, *sp ;

	// �t���p�X��
	ConvertFullPathW_( FilePath, filefull ) ;
	AnalysisFileNameAndDirPathW_( filefull, filename, endfull ) ;
	ConvertFullPathW_( StartFolderPath, startfull ) ;
	startlen = _WCSLEN( startfull ) ;
	if( startfull[ startlen - 1 ] == L'\\' || startfull[ startlen - 1 ] == L'/' )
		startfull[ startlen - 1 ] = L'\0' ;

	// �h���C�u��������擾����
	endlen = AnalysisDriveNameW_( endfull, endstr ) ;
	startlen = AnalysisDriveNameW_( startfull, startstr ) ;

	// �h���C�u���������t�@�C���p�X�̃t���p�X��������
	if( _WCSCMP( endstr, startstr ) != 0 )
	{
		_WCSCPY( Dest, filefull ) ;
		return 0 ;
	}
	ep = &endfull[ endlen ] ;
	sp = &startfull[ startlen ] ;

	// �t�H���_�����Ⴄ�Ƃ���܂œǂݐi�߂�
	for(;;)
	{
		// \ or / ���΂�
		while( *ep == L'\\' || *ep == L'/' ) ep ++ ;
		while( *sp == L'\\' || *sp == L'/' ) sp ++ ;

		// �I�[�����������烋�[�v�𔲂���
		if( *ep == L'\0' || *sp == L'\0' ) break ;

		// �t�H���_�p�X���擾����
		endlen = AnalysisDirectoryNameW_( ep, endstr ) ;
		startlen = AnalysisDirectoryNameW_( sp, startstr ) ;

		// �t�H���_�p�X��������烋�[�v�𔲂���
		if( _WCSCMP( endstr, startstr ) != 0 ) break ;
		ep += endlen ;
		sp += startlen ;
	}

	// ���݂������t�H���_�������ꍇ�̓t�@�C�������o��
	if( *ep == L'\0' && *sp == L'\0' )
	{
		_WCSCPY( Dest, filename ) ;
		return 0 ;
	}

	// �J�n�t�H���_�ƈႤ�t�H���_�ɂ���ꍇ�͂܂��J�n�t�H���_�����݂̃t�H���_�܂ō~���p�X�̍쐬
	destlen = 0 ;
	if( *sp != L'\0' )
	{
		Dest[ 0 ] = L'\0' ;
		destlen = 0 ;
		for(;;)
		{
			startlen = AnalysisDirectoryNameW_( sp, startstr ) ;
			sp += startlen ;
			while( *sp == L'\\' || *sp == L'/' ) sp ++ ;
			_WCSCPY( Dest + destlen, L"..\\" ) ;
			destlen += 3 ;
			if( *sp == L'\0' ) break ;
		}
	}

	// �ړI�̃t�@�C��������t�H���_�܂łƃt�@�C�������Ȃ��Ċ���
	_SWPRINTF( ( WORD * )( Dest + destlen ), ( WORD * )( L"%s\\%s" ), ep, filename ) ;
	return 0 ;
}

// ����̃p�X�����񂩂�F�X�ȏ����擾����
// ( CurrentDir �̓t���p�X�ł���K�v������(����Ɂw\�x�������Ă������Ă��ǂ�) )
// ( CurrentDir �� 0 �̏ꍇ�͎��ۂ̃J�����g�f�B���N�g�����g�p���� )
extern int AnalyseFilePath_(
	const char *Src,
	char *FullPath, char *DirPath, char *FileName, char *Name, char *ExeName, const char *CurrentDir )
{
	char full[256], dir[256], fname[256], name[256], exename[256];

	ConvertFullPath_( Src, full, CurrentDir );
	AnalysisFileNameAndDirPath_( full, fname, dir );
	AnalysisFileNameAndExeName_( fname, name, exename );
	if( FullPath ) _STRCPY( FullPath, full    );
	if( DirPath  ) _STRCPY( DirPath,  dir     );
	if( FileName ) _STRCPY( FileName, fname   );
	if( Name     ) _STRCPY( Name,     name    );
	if( ExeName  ) _STRCPY( ExeName,  exename );

	return 0;
}


// �t�@�C�������ꏏ�ɂȂ��Ă���ƕ������Ă���p�X������t�@�C�����ƃf�B���N�g���p�X�𕪊�����
// �t���p�X�ł���K�v�͖����A�t�@�C���������ł��ǂ�
extern int AnalysisFileNameAndDirPath_( const char *Src, char *FileName, char *DirPath )
{
	int i, Last ;
	
	// �t�@�C�����𔲂��o��
	i = 0 ;
	Last = -1 ;
	while( Src[i] != '\0' )
	{
		if( CheckMultiByteChar( Src[i], _GET_CODEPAGE() ) == FALSE )
		{
			if( Src[i] == '\\' || Src[i] == '/' || Src[i] == '\0' || Src[i] == ':' ) Last = i ;
			i ++ ;
		}
		else
		{
			i += 2 ;
		}
	}
	if( FileName != NULL )
	{
		if( Last != -1 ) _STRCPY( FileName, &Src[Last+1] ) ;
		else _STRCPY( FileName, Src ) ;
	}
	
	// �f�B���N�g���p�X�𔲂��o��
	if( DirPath != NULL )
	{
		if( Last != -1 )
		{
			_STRNCPY( DirPath, Src, Last ) ;
			DirPath[Last] = '\0' ;
		}
		else
		{
			DirPath[0] = '\0' ;
		}
	}
	
	// �I��
	return 0 ;
}

// �t�@�C�������ꏏ�ɂȂ��Ă���ƕ������Ă���p�X������t�@�C�����ƃf�B���N�g���p�X�𕪊�����
// �t���p�X�ł���K�v�͖����A�t�@�C���������ł��ǂ�
extern int AnalysisFileNameAndDirPathW_( const wchar_t *Src, wchar_t *FileName, wchar_t *DirPath )
{
	int i, Last ;
	
	// �t�@�C�����𔲂��o��
	i = 0 ;
	Last = -1 ;
	while( Src[i] != L'\0' )
	{
		if( CheckUTF16H( Src[i] ) == FALSE )
		{
			if( Src[i] == L'\\' || Src[i] == L'/' || Src[i] == L'\0' || Src[i] == L':' ) Last = i ;
			i ++ ;
		}
		else
		{
			i += 2 ;
		}
	}
	if( FileName != NULL )
	{
		if( Last != -1 ) _WCSCPY( FileName, &Src[Last+1] ) ;
		else             _WCSCPY( FileName, Src ) ;
	}
	
	// �f�B���N�g���p�X�𔲂��o��
	if( DirPath != NULL )
	{
		if( Last != -1 )
		{
			_WCSNCPY( DirPath, Src, Last ) ;
			DirPath[ Last ] = L'\0' ;
		}
		else
		{
			DirPath[0] = L'\0' ;
		}
	}
	
	// �I��
	return 0 ;
}

// �t�@�C���p�X����t�@�C�����Ɗg���q���擾����
extern int AnalysisFileNameAndExeName_( const char *Src, char *Name, char *ExeName )
{
	char FileName[256], *p, *p2, ename[128], name[128] ;

	// �t�@�C�����݂̂��擾
	AnalysisFileNameAndDirPath_( Src, FileName, 0 ) ;

	// �w.�x�����邩�ǂ����ŏ����𕪊�
	if( ( p = _STRCHR( FileName, '.' ) ) == NULL )
	{
		_STRCPY( name, FileName ) ;
		ename[0] = '\0' ;
	}
	else
	{
		for(;;)
		{
			p2 = _STRCHR( p + 1, '.' ) ;
			if( p2 == NULL ) break ;
			p = p2 ;
		}

		_STRNCPY( name, FileName, ( int )( p - FileName ) ) ;
		name[p - FileName] = '\0' ;
		_STRCPY( ename, p + 1 ) ;
	}

	if( Name != NULL ) _STRCPY( Name, name ) ;
	if( ExeName != NULL ) _STRCPY( ExeName, ename ) ;

	// �I��
	return 0 ;
}

// �t�@�C���p�X����t�@�C�����Ɗg���q���擾����
extern int AnalysisFileNameAndExeNameW_( const wchar_t *Src, wchar_t *Name, wchar_t *ExeName )
{
	wchar_t FileName[256], *p, *p2, ename[128], name[128] ;

	// �t�@�C�����݂̂��擾
	AnalysisFileNameAndDirPathW_( Src, FileName, 0 ) ;

	// �w.�x�����邩�ǂ����ŏ����𕪊�
	if( ( p = _WCSCHR( FileName, L'.' ) ) == NULL )
	{
		_WCSCPY( name, FileName ) ;
		ename[0] = L'\0' ;
	}
	else
	{
		for(;;)
		{
			p2 = _WCSCHR( p + 1, L'.' ) ;
			if( p2 == NULL ) break ;
			p = p2 ;
		}

		_WCSNCPY( name, FileName, ( int )( p - FileName ) ) ;
		name[p - FileName] = L'\0' ;
		_WCSCPY( ename, p + 1 ) ;
	}

	if( Name    != NULL ) _WCSCPY( Name, name ) ;
	if( ExeName != NULL ) _WCSCPY( ExeName, ename ) ;

	// �I��
	return 0 ;
}

// �t�@�C���p�X�̊g���q��ς���������𓾂�
extern int GetChangeExeNamePath_( const char *Src, char *Dest, const char *ExeName )
{
	char DirPath[256], FileName[128] ;

	AnalysisFileNameAndDirPath_( Src, NULL, DirPath ) ;
	AnalysisFileNameAndExeName_( Src, FileName, 0 ) ;
	SetEnMark_( DirPath ) ;
	_STRCPY( Dest, DirPath ) ;
	_STRCAT( Dest, FileName ) ;
	_STRCAT( Dest, "." ) ;
	_STRCAT( Dest, ExeName ) ;
	
	// �I��
	return 0 ;
}


// ����Ɂw\�x�����Ă��Ȃ��ꍇ�͕t����
extern void SetEnMark_( char *PathBuf )
{
	int Len = _STRLEN( PathBuf ) ;
	
	if( PathBuf[Len-1] != '\\' )
	{
		PathBuf[Len] = '\\' ;
		PathBuf[Len+1] = '\0' ;
	}
}

// �n���ꂽ��������t���p�X������Ƃ��Ĉ����A�h���C�u��( \ or / �̑O�܂� )
// ���̓l�b�g���[�N�t�H���_��( \ or / �̑O�܂� )���擾����A
// �h���C�u���������ꍇ�͍Ō�� : ���܂܂�A�l�b�g���[�N�t�H���_�������ꍇ�͍ŏ��� \\ ���܂܂�܂�
// �߂�l�͎擾����������̒���( �h���C�u���̏ꍇ�͍Ō�� :�A�l�b�g���[�N�t�H���_�̏ꍇ�� \\ ��������ʂɊ܂܂�܂� )
// Src �̓t���p�X�ł���K�v������܂�
extern int AnalysisDriveName_( const char *Src, char *Dest )
{
	int len ;

	// �l�b�g���[�N�t�H���_�̏ꍇ�͍ŏ��� \\ ��������
	len = 0 ;
	if( Src[ 0 ] == '\\' || Src[ 0 ] == '/' )
	{
		// �l�b�g���[�N�t�H���_
		Dest[ 0 ] = Src[ 0 ] ;
		Dest[ 1 ] = Src[ 1 ] ;
		Dest += 2 ;
		Src += 2 ;
		len += 2 ;
	}

	// ��͕�����̖��[�� \ or / ���o��܂ŃR�s�[
	while( *Src != '\0' && *Src != '\\' && *Src != '/' )
	{
		if( CheckMultiByteChar( *Src, _GET_CHARSET() ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// �I�[�������Z�b�g
	*Dest = '\0' ;

	// �����񒷂�Ԃ�
	return len ;
}

// �n���ꂽ��������t���p�X������Ƃ��Ĉ����A�h���C�u��( \ or / �̑O�܂� )
// ���̓l�b�g���[�N�t�H���_��( \ or / �̑O�܂� )���擾����A
// �h���C�u���������ꍇ�͍Ō�� : ���܂܂�A�l�b�g���[�N�t�H���_�������ꍇ�͍ŏ��� \\ ���܂܂�܂�
// �߂�l�͎擾����������̒���( �h���C�u���̏ꍇ�͍Ō�� :�A�l�b�g���[�N�t�H���_�̏ꍇ�� \\ ��������ʂɊ܂܂�܂� )
// Src �̓t���p�X�ł���K�v������܂�
extern int AnalysisDriveNameW_( const wchar_t *Src, wchar_t *Dest )
{
	int len ;

	// �l�b�g���[�N�t�H���_�̏ꍇ�͍ŏ��� \\ ��������
	len = 0 ;
	if( Src[ 0 ] == L'\\' || Src[ 0 ] == L'/' )
	{
		// �l�b�g���[�N�t�H���_
		Dest[ 0 ] = Src[ 0 ] ;
		Dest[ 1 ] = Src[ 1 ] ;
		Dest += 2 ;
		Src += 2 ;
		len += 2 ;
	}

	// ��͕�����̖��[�� \ or / ���o��܂ŃR�s�[
	while( *Src != L'\0' && *Src != L'\\' && *Src != L'/' )
	{
		if( CheckUTF16H( *Src ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// �I�[�������Z�b�g
	*Dest = L'\0' ;

	// �����񒷂�Ԃ�
	return len ;
}


// �n���ꂽ��������t�H���_�p�X������Ƃ��Ĉ����A�t�H���_��( \ or / �̑O�܂� )���擾���܂�
// �n�������񂪃t���p�X�ŁA�ŏ��Ƀh���C�u����������Ă����琳��Ȍ��ʂ������܂���
// ../ ���̉��ʃt�H���_�ɍ~��镶���񂪂������ꍇ�� .. �����o�͂���܂�
// �߂�l�͎擾����������̒����ł�
extern int AnalysisDirectoryName_( const char *Src, char *Dest )
{
	int len ;

	len = 0 ;
	while( *Src != '\0' && *Src != '\\' && *Src != '/' )
	{
		if( CheckMultiByteChar( *Src, _GET_CHARSET() ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// �I�[�������Z�b�g
	*Dest = '\0' ;

	// �����񒷂�Ԃ�
	return len ;
}

// �n���ꂽ��������t�H���_�p�X������Ƃ��Ĉ����A�t�H���_��( \ or / �̑O�܂� )���擾���܂�
// �n�������񂪃t���p�X�ŁA�ŏ��Ƀh���C�u����������Ă����琳��Ȍ��ʂ������܂���
// ../ ���̉��ʃt�H���_�ɍ~��镶���񂪂������ꍇ�� .. �����o�͂���܂�
// �߂�l�͎擾����������̒����ł�
extern int AnalysisDirectoryNameW_( const wchar_t *Src, wchar_t *Dest )
{
	int len ;

	len = 0 ;
	while( *Src != L'\0' && *Src != L'\\' && *Src != L'/' )
	{
		if( CheckUTF16H( *Src ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// �I�[�������Z�b�g
	*Dest = L'\0' ;

	// �����񒷂�Ԃ�
	return len ;
}



}
