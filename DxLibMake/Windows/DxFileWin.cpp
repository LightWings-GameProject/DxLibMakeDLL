//-----------------------------------------------------------------------------
// 
// 		�c�w���C�u����		Windows�p�t�@�C���֌W�v���O����
// 
//  	Ver 3.12a
// 
//-----------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h ---------------------------------------------------------------
#include "DxFileWin.h"
#include "../DxFile.h"
#include "../DxBaseFunc.h"
#include "DxWindow.h"
#include "../DxLog.h"
#include <windows.h>

namespace DxLib
{

// �}�N����` -----------------------------------------------------------------

// �\���̐錾 -----------------------------------------------------------------

// �f�[�^��` -----------------------------------------------------------------

// �֐��錾 -------------------------------------------------------------------

DWORD	WINAPI		FileAccessThreadFunction( void *FileAccessThreadData ) ;					// �t�@�C���A�N�Z�X��p�X���b�h�p�֐�
int					SetFilePointer64( HANDLE FileHandle, ULONGLONG Position ) ;					// �t�@�C���̃A�N�Z�X�ʒu��ݒ肷��( 0:����  -1:���s )

// �v���O���� -----------------------------------------------------------------

// �t�@�C���̃A�N�Z�X�ʒu��ݒ肷��
int SetFilePointer64( HANDLE FileHandle, ULONGLONG Position )
{
	DWORD High, Low ;
	DWORD Result ;

	Low = ( DWORD )Position ;
	High = ( DWORD )( Position >> 32 ) ;
	Result = SetFilePointer( FileHandle, ( LONG )Low, ( LONG * )&High, FILE_BEGIN ) ;
	if( Result == 0xffffffff && GetLastError() != NO_ERROR )
	{
		return -1 ;
	}

	return 0 ;
}

// �e���|�����t�@�C�����쐬����
extern HANDLE CreateTemporaryFile( TCHAR *TempFileNameBuffer )
{
	TCHAR String1[MAX_PATH], String2[MAX_PATH] ;
	HANDLE FileHandle ;
	int Length ;

	// �e���|�����t�@�C���̃f�B���N�g���p�X���擾����
	if( GetTempPath( 256, String1 ) == 0 ) return NULL ;

	// ������̍Ō�Ɂ��}�[�N������
	Length = lstrlen( String1 ) ;
	if( String1[Length-1] != _T( '\\' ) )
	{
		String1[Length] = _T( '\\' ) ;
		String1[Length+1] = _T( '\0' ) ;
	}

	// �e���|�����t�@�C���̃t�@�C�������쐬����
	if( GetTempFileName( String1, _T( "tmp" ), 0, String2 ) == 0 ) return NULL ;

	// �t���p�X�ɕϊ�
	ConvertFullPathT_( String2, String1 ) ;

	// �e���|�����t�@�C�����J��
	DeleteFile( String1 ) ;
	FileHandle = CreateFile( String1, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( FileHandle == NULL ) return NULL ;

	// �e���|�����t�@�C������ۑ�
	if( TempFileNameBuffer != NULL ) lstrcpy( TempFileNameBuffer, String1 ) ;

	// �n���h����Ԃ�
	return FileHandle ;
}



// �t�@�C���A�N�Z�X��p�X���b�h�p�֐�
DWORD WINAPI FileAccessThreadFunction( void *FileAccessThreadData )
{
	FILEACCESSTHREAD *dat = (FILEACCESSTHREAD *)FileAccessThreadData ;
	DWORD res, ReadSize ;

	for(;;)
	{
		for(;;)
		{
			// �L���b�V�����g�p�����ǂ����ŏ����𕪊�
			if( dat->CacheBuffer )
			{
				// �w�߂�����܂ł�����Ƒ҂�
				res = WaitForSingleObject( dat->FuncEvent, 100 ) ;

				// �w�߂����Ă������ꍇ�Ńt�@�C�����J���Ă���ꍇ�̓L���b�V���O�������s��
				if( res == WAIT_TIMEOUT && dat->Handle != NULL )
				{
					// �����L���b�V������t�������牽�����Ȃ�
					if( dat->CacheSize != FILEACCESSTHREAD_DEFAULT_CACHESIZE )
					{
						// �ǂݍ��݊J�n�ʒu�Z�b�g
						SetFilePointer64( dat->Handle, dat->CachePosition + dat->CacheSize ) ;

						// �ǂݍ���
						ReadFile( dat->Handle, &dat->CacheBuffer[dat->CacheSize], ( DWORD )( FILEACCESSTHREAD_DEFAULT_CACHESIZE - dat->CacheSize ), &ReadSize, NULL ) ;
						
						// �L���ȃT�C�Y�𑝂₷
						dat->CacheSize += ( LONGLONG )ReadSize ;
					}
				}
				else
				{
					break ;
				}
			}
			else
			{
				// �w�߂�����܂ő҂�
				res = WaitForSingleObject( dat->FuncEvent, INFINITE ) ;
				if( res == WAIT_TIMEOUT && dat->Handle != NULL ) continue;
				break;
			}
		}

//		WaitForSingleObject( dat->FuncEvent, INFINITE ) ;

		// �C�x���g�̃V�O�i����Ԃ���������
		ResetEvent( dat->FuncEvent ) ;
		ResetEvent( dat->CompEvent ) ;

		// �w�߂������画�f����
		switch( dat->Function )
		{
		case FILEACCESSTHREAD_FUNCTION_OPEN :
			dat->Handle = CreateFile( dat->FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			if( dat->Handle == INVALID_HANDLE_VALUE )
			{
				dat->ErrorFlag = TRUE ;
				goto END ;
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_CLOSE :
			CloseHandle( dat->Handle ) ;
			dat->Handle = NULL ;
			break ;

		case FILEACCESSTHREAD_FUNCTION_READ :
			// �L���b�V���Ɠǂݍ��݈ʒu����v���Ă���ꍇ�̓L���b�V������f�[�^��]������
			if( dat->CacheBuffer && dat->ReadPosition == dat->CachePosition && dat->CacheSize != 0 )
			{
				DWORD MoveSize ;

				// �]������T�C�Y�𒲐�
				MoveSize = dat->ReadSize ;
				if( MoveSize > dat->CacheSize ) MoveSize = ( DWORD )dat->CacheSize ;

				// �]��
				_MEMCPY( dat->ReadBuffer, dat->CacheBuffer, MoveSize ) ;

				// �ǂݍ��݃T�C�Y�Ɠǂݍ��݈ʒu���ړ�����
				dat->ReadBuffer = (void *)( (BYTE *)dat->ReadBuffer + MoveSize ) ;
				dat->ReadPosition += MoveSize ;
				dat->ReadSize -= MoveSize ;
				
				// �L���b�V���̏����X�V
				dat->CachePosition += MoveSize ;
				dat->CacheSize     -= MoveSize ;
				if( dat->CacheSize != 0 ) _MEMMOVE( &dat->CacheBuffer[0], &dat->CacheBuffer[MoveSize], ( size_t )dat->CacheSize ) ;
			}

			// ��]�̃f�[�^���S�ēǂ߂Ă��Ȃ��ꍇ�͍X�Ƀt�@�C������ǂݍ���
			if( dat->ReadSize != 0 )
			{
				SetFilePointer64( dat->Handle, dat->ReadPosition ) ;
				ReadFile( dat->Handle, dat->ReadBuffer, dat->ReadSize, &dat->ReadSize, NULL ) ;

				// �L���b�V��������������
				if( dat->CacheBuffer )
				{
					dat->CachePosition = dat->ReadPosition + dat->ReadSize ;
					dat->CacheSize = 0 ;
				}
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_SEEK :
			SetFilePointer64( dat->Handle, dat->SeekPoint ) ;

			// �L���b�V��������������
			if( dat->CacheBuffer )
			{
				dat->CachePosition = (DWORD)dat->SeekPoint ;
				dat->CacheSize = 0 ;
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_EXIT :
			if( dat->Handle != NULL ) CloseHandle( dat->Handle ) ;
			dat->Handle = NULL ;
			goto END ;
		}

		// �w�߂����������犮���C�x���g���V�O�i����Ԃɂ���
		SetEvent( dat->CompEvent ) ;
	}

END:
	// �G���[���ׂ̈Ɋ����C�x���g���V�O�i����Ԃɂ���
	SetEvent( dat->CompEvent ) ;
	dat->EndFlag = TRUE ;
	ExitThread( 1 ) ;

	return 0 ;
}



// �t�@�C���A�N�Z�X�֐�
extern DWORD_PTR ReadOnlyFileAccessOpen( const TCHAR *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	WINFILEACCESS *FileAccess ;
	DWORD Code ;
	DWORD High ;
	
//	UseCacheFlag = UseCacheFlag ;
	BlockReadFlag = BlockReadFlag ;

	FileAccess = (WINFILEACCESS *)DXALLOC( sizeof( WINFILEACCESS ) ) ;
	if( FileAccess == NULL ) return 0 ;

	_MEMSET( FileAccess, 0, sizeof( WINFILEACCESS ) ) ;

//	// �L���b�V�����g�p���邩�ǂ������X���b�h���g�p���邩�ǂ����ɂ��Ă��܂�
//	FileAccess->UseThread = UseCacheFlag ;

	// �L���b�V�����g�p���邩�ǂ����̃t���O���Z�b�g
	FileAccess->UseCacheFlag = UseCacheFlag ;
	FileAccess->ThreadData.CacheBuffer = NULL;

	// �񓯊��ǂݏ����t���O���Z�b�g
	FileAccess->UseASyncReadFlag = UseASyncReadFlag ;

	// �L���b�V���A�Ⴕ���͔񓯊��ǂݏ������s���ꍇ�̓X���b�h���g�p����
	FileAccess->UseThread = FileAccess->UseCacheFlag || FileAccess->UseASyncReadFlag ;

	// �X���b�h���g�p����ꍇ�Ƃ��Ȃ��ꍇ�ŏ����𕪊�
	if( FileAccess->UseThread == TRUE )
	{
		// �X���b�h���g�p����ꍇ�̓t�@�C���A�N�Z�X��p�X���b�h�𗧂Ă�

		// �ŏ��Ƀt�@�C�����J���邩�ǂ����m���߂Ă���
		FileAccess->Handle = CreateFile( Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( FileAccess->Handle == INVALID_HANDLE_VALUE )
		{
			DXFREE( FileAccess ) ;
			return 0 ;
		}
		FileAccess->Size = GetFileSize( FileAccess->Handle, &High ) ;
		FileAccess->Size |= ( ( ULONGLONG )High ) << 32 ;
		CloseHandle( FileAccess->Handle ) ;
		FileAccess->Handle = NULL ;

		// �L���b�V���p�������̊m��
		if( FileAccess->UseCacheFlag )
		{
			FileAccess->ThreadData.CacheBuffer = (BYTE *)DXALLOC( FILEACCESSTHREAD_DEFAULT_CACHESIZE );
			if( FileAccess->ThreadData.CacheBuffer == NULL )
			{
				DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
				DXFREE( FileAccess ) ;
				DXST_ERRORLOG_ADD( _T( "�t�@�C���ǂݍ��݃L���b�V���p�������̊m�ۂɎ��s���܂���\n" ) ) ;
				return 0 ;
			}
		}

		// ��p�X���b�h�f�[�^������������
		FileAccess->ThreadData.Handle = NULL ;
		FileAccess->ThreadData.ThreadHandle = NULL ;
		FileAccess->ThreadData.FuncEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
		FileAccess->ThreadData.CompEvent = CreateEvent( NULL, TRUE, TRUE, NULL ) ;

		FileAccess->ThreadData.ThreadHandle = CreateThread(
												NULL,
												0,
												(LPTHREAD_START_ROUTINE)FileAccessThreadFunction, 
												&FileAccess->ThreadData,
												0,
												&FileAccess->ThreadData.ThreadID ) ;
		if( FileAccess->ThreadData.ThreadHandle == NULL )
		{
			if( FileAccess->ThreadData.CacheBuffer ) DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
			CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
			CloseHandle( FileAccess->ThreadData.CompEvent ) ;
			DXFREE( FileAccess ) ;
			DXST_ERRORLOG_ADD( _T( "�t�@�C���A�N�Z�X��p�X���b�h�̍쐬�Ɏ��s���܂���\n" ) ) ;
			return 0 ;
		}
		SetThreadPriority( FileAccess->ThreadData.ThreadHandle, THREAD_PRIORITY_LOWEST ) ;

		// �t�@�C���I�[�v���w�߂͂����Ŋ������Ă��܂�
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_OPEN ;
		lstrcpy( FileAccess->ThreadData.FilePath, Path ) ;

		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// �w�߂��I������܂ő҂�
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;
		if( FileAccess->ThreadData.ErrorFlag == TRUE )
		{
			if( FileAccess->ThreadData.CacheBuffer ) DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
			CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
			CloseHandle( FileAccess->ThreadData.CompEvent ) ;
			do
			{
				Sleep(0);
				GetExitCodeThread( FileAccess->ThreadData.ThreadHandle, &Code );
			}while( Code == STILL_ACTIVE );
			CloseHandle( FileAccess->ThreadData.ThreadHandle ) ;
			DXFREE( FileAccess ) ;
			DXST_ERRORLOG_ADD( _T( "�t�@�C���̃I�[�v���Ɏ��s���܂���\n" ) ) ;
			return 0 ;
		}
	}
	else
	{
		// �X���b�h���g�p���Ȃ��ꍇ�͂��̏�Ńt�@�C�����J��
		FileAccess->Handle = CreateFile( Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( FileAccess->Handle == INVALID_HANDLE_VALUE )
		{
			DXFREE( FileAccess ) ;
			return 0 ;
		}
		FileAccess->Size = GetFileSize( FileAccess->Handle, &High ) ;
		FileAccess->Size |= ( ( ULONGLONG )High ) << 32 ;
	}
	FileAccess->EofFlag = FALSE ;
	FileAccess->Position = 0 ;

	return (DWORD_PTR)FileAccess ;
}

extern int ReadOnlyFileAccessClose( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	BOOL Result;
	DWORD Code ;

	// �X���b�h���g�p����ꍇ�Ƃ��Ȃ��ꍇ�ŏ����𕪊�
	if( FileAccess->UseThread == TRUE )
	{
		// ����ȑO�̎w�߂��o�Ă����ꍇ�ׂ̈Ɏw�ߊ����C�x���g���V�O�i����ԂɂȂ�܂ő҂�
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// �X���b�h�ɏI���w�߂��o��
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_EXIT ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// �w�߂��I������܂ő҂�
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// �X���b�h���I������̂�҂�
		do
		{
			Sleep(1);
			GetExitCodeThread( FileAccess->ThreadData.ThreadHandle, &Code );
		}while( Code == STILL_ACTIVE );

		// �L���b�V�����g�p���Ă����ꍇ�̓L���b�V���p���������J������
		if( FileAccess->ThreadData.CacheBuffer )
			DXFREE( FileAccess->ThreadData.CacheBuffer ) ;

		// �C�x���g��X���b�h�����
		CloseHandle( FileAccess->ThreadData.ThreadHandle ) ;
		CloseHandle( FileAccess->ThreadData.CompEvent ) ;
		CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
		Result = 0 ;
	}
	else
	{
		// �g�p���Ă��Ȃ��ꍇ�͂��̏�Ńn���h������ďI��
		Result = CloseHandle( FileAccess->Handle ) ;
	}
	DXFREE( FileAccess ) ;

	return Result != 0 ? 0 : -1/*EOF*/ ;
}

extern LONGLONG ReadOnlyFileAccessTell( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;

	return ( LONGLONG )FileAccess->Position ;
}

extern int ReadOnlyFileAccessSeek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	ULONGLONG Pos = 0 ;
	int Result ;

	switch( SeekType )
	{
	case SEEK_CUR :
		if( FileAccess->Position + SeekPoint < 0 )
		{
			Pos = 0 ;
		}
		else
		{
			Pos = FileAccess->Position + SeekPoint ;
		}
		break ;

	case SEEK_END :
		if( FileAccess->Size + SeekPoint < 0 )
		{
			Pos = 0 ;
		}
		else
		{
			Pos = FileAccess->Size + SeekPoint ;
		}
		break ;

	case SEEK_SET :
		if( SeekPoint < 0 )
		{
			Pos = 0 ;
		}
		else
		{
			Pos = SeekPoint ;
		}
		break ;
	}

	// �X���b�h���g�p���Ă��邩�ǂ����ŏ����𕪊�
	if( FileAccess->UseThread == TRUE )
	{
		// ����ȑO�̎w�߂��o�Ă����ꍇ�ׂ̈Ɏw�ߊ����C�x���g���V�O�i����ԂɂȂ�܂ő҂�
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// �X���b�h�Ƀt�@�C���ʒu�ύX�w�߂��o��
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_SEEK ;
		FileAccess->ThreadData.SeekPoint = Pos ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;
	}
	else
	{
		// �t�@�C���A�N�Z�X�ʒu��ύX����
		Result = SetFilePointer64( FileAccess->Handle, Pos ) ;
		if( Result == -1 ) return -1 ;
	}

	// �ʒu��ۑ����Ă���
	FileAccess->Position = Pos ;

	// �I�[�`�F�b�N�t���O��|��
	FileAccess->EofFlag = FALSE ;

	// �I��
	return 0 ;
}

extern size_t ReadOnlyFileAccessRead( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	DWORD Result, BytesRead ;

	if( BlockSize == 0 ) return 0 ;

	// �I�[�`�F�b�N
	if( FileAccess->Position == FileAccess->Size )
	{
		FileAccess->EofFlag = TRUE ;
		return 0 ;
	}

	// ���ڐ�����
	if( BlockSize * DataNum + FileAccess->Position > FileAccess->Size )
	{
		DataNum = ( size_t )( ( FileAccess->Size - FileAccess->Position ) / BlockSize ) ;
	}
	
	if( DataNum == 0 )
	{
		FileAccess->EofFlag = TRUE ;
		return 0 ;
	}

	// �X���b�h���g�p���Ă��邩�ǂ����ŏ����𕪊�
	if( FileAccess->UseThread == TRUE )
	{
		// ����ȑO�̎w�߂��o�Ă����ꍇ�ׂ̈Ɏw�ߊ����C�x���g���V�O�i����ԂɂȂ�܂ő҂�
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// �X���b�h�Ƀt�@�C���ǂݍ��ݎw�߂��o��
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_READ ;
		FileAccess->ThreadData.ReadBuffer = Buffer ;
		FileAccess->ThreadData.ReadPosition = FileAccess->Position ;
		FileAccess->ThreadData.ReadSize = ( DWORD )( BlockSize * DataNum ) ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// �񓯊����ǂ����ŏ����𕪊�
		if( FileAccess->UseASyncReadFlag == FALSE )
		{
			// �����ǂݍ��݂̏ꍇ�͎w�߂���������܂ő҂�
			WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;
		}

		BytesRead = ( DWORD )( BlockSize * DataNum ) ;
		Result = 1 ;
	}
	else
	{
		Result = ReadFile( FileAccess->Handle, Buffer, DWORD( BlockSize * DataNum ), &BytesRead, NULL ) ;
	}

	FileAccess->Position += ( DWORD )( DataNum * BlockSize ) ;
	return Result != 0 ? BytesRead / BlockSize : 0 ;
}

extern int ReadOnlyFileAccessEof( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = (WINFILEACCESS *)Handle ;

	return FileAccess->EofFlag ? EOF : 0 ;
}

extern int ReadOnlyFileAccessIdleCheck( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = (WINFILEACCESS *)Handle ;

	if( FileAccess->UseThread == TRUE )
	{
		return WaitForSingleObject( FileAccess->ThreadData.CompEvent, 0 ) == WAIT_TIMEOUT ? FALSE : TRUE ;
	}
	else
	{
		return TRUE ;
	}
}

extern int ReadOnlyFileAccessChDir( const TCHAR *Path )
{
	return SetCurrentDirectory( Path ) ;
}

extern int ReadOnlyFileAccessGetDir( TCHAR *Buffer )
{
	return GetCurrentDirectory( MAX_PATH, Buffer ) ;
}

static void _WIN32_FIND_DATA_To_FILEINFO( WIN32_FIND_DATA *FindData, FILEINFO *FileInfo )
{
	// �t�@�C�����̃R�s�[
	lstrcpy( FileInfo->Name, FindData->cFileName );

	// �f�B���N�g�����ǂ����̃t���O���Z�b�g
	FileInfo->DirFlag = (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;

	// �t�@�C���̃T�C�Y���Z�b�g
	FileInfo->Size = ( LONGLONG )( ( ( ( ULONGLONG )FindData->nFileSizeHigh ) << 32 ) + FindData->nFileSizeLow ) ;

	// �t�@�C���^�C����ۑ�
	_FileTimeToLocalDateData( &FindData->ftCreationTime, &FileInfo->CreationTime );
	_FileTimeToLocalDateData( &FindData->ftLastWriteTime, &FileInfo->LastWriteTime );
}

// �߂�l: -1=�G���[  -1�ȊO=FindHandle
extern DWORD_PTR ReadOnlyFileAccessFindFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	WIN32_FIND_DATA FindData;
	HANDLE FindHandle;

	FindHandle = FindFirstFile( FilePath, &FindData );
	if( FindHandle == INVALID_HANDLE_VALUE ) return ( DWORD_PTR )-1;

	if( Buffer ) _WIN32_FIND_DATA_To_FILEINFO( &FindData, Buffer );

	return (DWORD_PTR)FindHandle;
}

// �߂�l: -1=�G���[  0=����
extern int ReadOnlyFileAccessFindNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	WIN32_FIND_DATA FindData;

	if( FindNextFile( (HANDLE)FindHandle, &FindData ) == 0 ) return -1;

	if( Buffer ) _WIN32_FIND_DATA_To_FILEINFO( &FindData, Buffer );

	return 0;
}

// �߂�l: -1=�G���[  0=����
extern int ReadOnlyFileAccessFindClose( DWORD_PTR FindHandle )
{
	// �O�ȊO���Ԃ��Ă����琬��
	return FindClose( (HANDLE)FindHandle ) != 0 ? 0 : -1;
}







}

