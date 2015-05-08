// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		�A�[�J�C�u����v���O����
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------


// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h-------------------------------------------------------------------
#include "DxArchive_.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxFile.h"
#include "DxFont.h"
#include "DxLog.h"
#include "DxSystem.h"
#include <stdio.h>

#ifdef __WINDOWS__
#include "Windows/DxWindow.h"
#endif // __WINDOWS__

namespace DxLib
{

#ifndef DX_NON_DXA

// �}�N����` --------------------------------------------------------------------

#define DXARCD						DX_ArchiveDirData
//#define CHECKMULTIBYTECHAR(CP)		(( (unsigned char)*(CP) >= 0x81 && (unsigned char)*(CP) <= 0x9F ) || ( (unsigned char)*(CP) >= 0xE0 && (unsigned char)*(CP) <= 0xFC ))	// TRUE:�Q�o�C�g����  FALSE:�P�o�C�g����
#define CHARUP(C)					((C) >= _T( 'a' ) && (C) <= _T( 'z' ) ? (C) - _T( 'a' ) + _T( 'A' ) : (C))

#define DXARC_ID_AND_VERSION_SIZE	(sizeof( WORD ) * 2)

#define DXARC_HEAD_VER3_SIZE		(24)
#define DXARC_HEAD_VER4_SIZE		(28)
#define DXARC_HEAD_VER6_SIZE		(48)

#define DXARC_FILEHEAD_VER1_SIZE	(40)			// Ver0x0001 �� DXARC_FILEHEAD �\���̂̃T�C�Y
#define DXARC_FILEHEAD_VER2_SIZE	(44)			// Ver0x0002 �� DXARC_FILEHEAD �\���̂̃T�C�Y
#define DXARC_FILEHEAD_VER6_SIZE	(64)			// Ver0x0006 �� DXARC_FILEHEAD �\���̂̃T�C�Y

#if defined( DX_GCC_COMPILE ) || defined( __ANDROID )
#define NONE_PAL		(0xffffffffffffffffULL)
#else
#define NONE_PAL		(0xffffffffffffffff)
#endif

// �\���̒�` --------------------------------------------------------------------

// DXA_DIR_FindFirst ���̏����Ŏg�p����\����
struct DXA_DIR_FINDDATA
{
	int							UseArchiveFlag;					// �A�[�J�C�u�t�@�C�����g�p���Ă��邩�t���O
	int							UseArchiveIndex;				// �A�[�J�C�u���g�p���Ă���ꍇ�A�g�p���Ă���A�[�J�C�u�t�@�C���f�[�^�̃C���f�b�N�X
	DWORD_PTR					FindHandle;						// �t�@�C�������p�n���h��
} ;

// DXA_FindFirst ���̏����Ŏg�p����\����
struct DXA_FINDDATA
{
	DXARC						*Container;						// �����Ώۂ̂c�w�`�t�@�C��
	char						SearchStr[MAX_PATH];			// ����������
	union
	{
		DXARC_DIRECTORY			*Directory;						// �����Ώۂ̃f�B���N�g��
		DXARC_DIRECTORY_VER5	*DirectoryV5;					// �����Ώۂ̃f�B���N�g��(Ver5�ȑO�p)
	};
	DWORD						ObjectCount;					// ���ɓn���f�B���N�g�����I�u�W�F�N�g�̃C���f�b�N�X
} ;

// �������ϐ��錾 --------------------------------------------------------------

// �A�[�J�C�u���f�B���N�g���Ɍ����Ă�ׂ̃f�[�^
DXARC_DIR DX_ArchiveDirData ;

// �֐��v���g�^�C�v�錾-----------------------------------------------------------

static DXARC_FILEHEAD_VER5 *DXA_GetFileInfoV5(		DXARC *DXA, const char *FilePath ) ;													// �t�@�C���̏��𓾂�
static DXARC_FILEHEAD      *DXA_GetFileInfo(		DXARC *DXA, const char *FilePath ) ;													// �t�@�C���̏��𓾂�
static int		DXA_ConvSearchData(					DXARC *DXA, DXARC_SEARCHDATA *Dest, const char *Src, int *Length ) ;					// ������������p�̃f�[�^�ɕϊ�( �k�������� \ ����������I�� )
static int		DXA_ChangeCurrentDirectoryFast(		DXARC *DXA, DXARC_SEARCHDATA *SearchData ) ;											// �A�[�J�C�u���̃f�B���N�g���p�X��ύX����( 0:����  -1:���s )
static int		DXA_ChangeCurrentDirectoryBase(		DXARC *DXA, const char *DirectoryPath, bool ErrorIsDirectoryReset, DXARC_SEARCHDATA *LastSearchData = NULL ) ;		// �A�[�J�C�u���̃f�B���N�g���p�X��ύX����( 0:����  -1:���s )
//static int	DXA_Decode(							void *Src, void *Dest ) ;																// �f�[�^���𓀂���( �߂�l:�𓀌�̃f�[�^�T�C�Y )
static void		DXA_KeyCreate(						const char *Source, unsigned char *Key ) ;												// ����������쐬
static void		DXA_KeyConv(						void *Data, LONGLONG  SizeLL, LONGLONG  PositionLL,  unsigned char *Key ) ;				// ����������g�p���� Xor ���Z( Key �͕K�� DXA_KEYSTR_LENGTH �̒������Ȃ���΂Ȃ�Ȃ� )
static void		DXA_KeyConvFileRead(				void *Data, ULONGLONG Size,   DWORD_PTR FilePointer, unsigned char *Key, LONGLONG Position = -1 ) ;	// �t�@�C������ǂݍ��񂾃f�[�^������������g�p���� Xor ���Z����֐�( Key �͕K�� DXA_KEYSTR_LENGTH �̒������Ȃ���΂Ȃ�Ȃ� )
static int		DXA_FindProcess(					DXA_FINDDATA *FindData, FILEINFO *FileInfo );											// �����ɓK������I�u�W�F�N�g����������(�����Ώۂ� ObjectCount ���C���f�b�N�X�Ƃ����Ƃ��납��)(�߂�l -1:�G���[ 0:����)

static int		DXA_DIR_OpenArchive(				const TCHAR *FilePath, void *FileImage = NULL, int FileSize = -1, int FileImageCopyFlag = FALSE, int FileImageReadOnly = FALSE, int ArchiveIndex = -1, int OnMemory = FALSE, int ASyncThread = FALSE ) ;	// �A�[�J�C�u�t�@�C�����J��
static int		DXA_DIR_GetArchive(					const TCHAR *FilePath, void *FileImage = NULL ) ;										// ���ɊJ����Ă���A�[�J�C�u�̃n���h�����擾����( �߂�l: -1=�������� 0�ȏ�:�n���h�� )
static int		DXA_DIR_CloseArchive(				int ArchiveHandle ) ;																	// �A�[�J�C�u�t�@�C�������
static void		DXA_DIR_CloseWaitArchive(			void ) ;																				// �g�p�����̂�҂��Ă���A�[�J�C�u�t�@�C����S�ĕ���
static int		DXA_DIR_ConvertFullPath(			const TCHAR *Src, TCHAR *Dest ) ;														// �S�Ẳp����������啶���ɂ��Ȃ���A�t���p�X�ɕϊ�����
static int		DXA_DIR_AnalysisFileNameAndDirPath( DXARC *DXA, const char *Src, char *FileName = 0, char *DirPath = 0 ) ;					// �t�@�C�������ꏏ�ɂȂ��Ă���ƕ������Ă���p�X������t�@�C�����ƃf�B���N�g���p�X�𕪊�����B�t���p�X�ł���K�v�͖����A�t�@�C���������ł��ǂ��ADirPath �̏I�[�� �� �}�[�N�͕t���Ȃ�
static int		DXA_DIR_FileNameCmp(				DXARC *DXA, const char *Src, const char *CmpStr );										// CmpStr �̏����� Src ���K�����邩�ǂ����𒲂ׂ�( 0:�K������  -1:�K�����Ȃ� )
static int		DXA_DIR_OpenTest(					const TCHAR *FilePath, int *ArchiveIndex, char *ArchivePath, char *ArchiveFilePath );	// �A�[�J�C�u�t�@�C�����t�H���_�Ɍ����Ăăt�@�C�����J�����̏��𓾂�( -1:�A�[�J�C�u�Ƃ��Ă͑��݂��Ȃ�����  0:���݂��� )

static int		DXA_DirectoryKeyConv(				DXARC *DXA, DXARC_DIRECTORY *Dir ) ;													// �w��̃f�B���N�g���f�[�^�̈Í�������������( �ۂ��ƃ������ɓǂݍ��񂾏ꍇ�p )
static int		DXA_DirectoryKeyConvV5(				DXARC *DXA, DXARC_DIRECTORY_VER5 *Dir ) ;												// �w��̃f�B���N�g���f�[�^�̈Í�������������( �ۂ��ƃ������ɓǂݍ��񂾏ꍇ�p )

// �v���O���� --------------------------------------------------------------------

// �t�@�C���̏��𓾂�
static DXARC_FILEHEAD_VER5 *DXA_GetFileInfoV5( DXARC *DXA, const char *FilePath )
{
	DXARC_DIRECTORY_VER5 *OldDir ;
	DXARC_FILEHEAD_VER5 *FileH ;
	DWORD FileHeadSize ;
	BYTE *NameData ;
	int i, j, k, Num ;
	DXARC_SEARCHDATA SearchData ;

	// ���̃f�B���N�g����ۑ����Ă���
	OldDir = DXA->CurrentDirectoryV5 ;

	// �t�@�C���p�X�� \ or / ���܂܂�Ă���ꍇ�A�f�B���N�g���ύX���s��
	if( _STRCHR( FilePath, '\\' ) != NULL || _STRCHR( FilePath, '/' ) != NULL )
	{
		// �J�����g�f�B���N�g����ړI�̃t�@�C��������f�B���N�g���ɕύX����
		if( DXA_ChangeCurrentDirectoryBase( DXA, FilePath, false, &SearchData ) >= 0 )
		{
			// �G���[���N���Ȃ������ꍇ�̓t�@�C�������f�B���N�g�����������ƂɂȂ�̂ŃG���[
			goto ERR ;
		}
	}
	else
	{
		// �t�@�C�����������p�f�[�^�ɕϊ�����
		DXA_ConvSearchData( DXA, &SearchData, FilePath, NULL ) ;
	}

	// �����̃t�@�C����T��
	FileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
	FileH        = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + DXA->CurrentDirectoryV5->FileHeadAddress ) ;
	Num          = ( int )DXA->CurrentDirectoryV5->FileHeadNum ;
	for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD_VER5 *)( (BYTE *)FileH + FileHeadSize ) )
	{
		// �f�B���N�g���`�F�b�N
		if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) continue ;

		// �����񐔂ƃp���e�B�`�F�b�N
		NameData = DXA->Table.NameTable + FileH->NameAddress ;
		if( SearchData.PackNum != ((WORD *)NameData)[0] || SearchData.Parity != ((WORD *)NameData)[1] ) continue ;

		// ������`�F�b�N
		NameData += 4 ;
		for( j = 0, k = 0 ; j < SearchData.PackNum ; j ++, k += 4 )
			if( *((DWORD *)&SearchData.FileName[k]) != *((DWORD *)&NameData[k]) ) break ;

		// �K�������t�@�C�����������炱���ŏI��
		if( SearchData.PackNum == j ) break ;
	}

	// ����������G���[
	if( i == Num ) goto ERR ;
	
	// �f�B���N�g�������ɖ߂�
	DXA->CurrentDirectoryV5 = OldDir ;
	
	// �ړI�̃t�@�C���̃A�h���X��Ԃ�
	return FileH ;
	
ERR :
	// �f�B���N�g�������ɖ߂�
	DXA->CurrentDirectoryV5 = OldDir ;
	
	// �G���[�I��
	return NULL ;
}

// �t�@�C���̏��𓾂�
static DXARC_FILEHEAD *DXA_GetFileInfo( DXARC *DXA, const char *FilePath )
{
	DXARC_DIRECTORY *OldDir ;
	DXARC_FILEHEAD *FileH ;
	DWORD FileHeadSize ;
	BYTE *NameData ;
	int i, j, k, Num ;
	DXARC_SEARCHDATA SearchData ;

	// ���̃f�B���N�g����ۑ����Ă���
	OldDir = DXA->CurrentDirectory ;

	// �t�@�C���p�X�� \ or / ���܂܂�Ă���ꍇ�A�f�B���N�g���ύX���s��
	if( _STRCHR( FilePath, '\\' ) != NULL || _STRCHR( FilePath, '/' ) != NULL )
	{
		// �J�����g�f�B���N�g����ړI�̃t�@�C��������f�B���N�g���ɕύX����
		if( DXA_ChangeCurrentDirectoryBase( DXA, FilePath, false, &SearchData ) >= 0 )
		{
			// �G���[���N���Ȃ������ꍇ�̓t�@�C�������f�B���N�g�����������ƂɂȂ�̂ŃG���[
			goto ERR ;
		}
	}
	else
	{
		// �t�@�C�����������p�f�[�^�ɕϊ�����
		DXA_ConvSearchData( DXA, &SearchData, FilePath, NULL ) ;
	}

	// �����̃t�@�C����T��
	FileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;
	FileH        = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + DXA->CurrentDirectory->FileHeadAddress ) ;
	Num          = ( int )DXA->CurrentDirectory->FileHeadNum ;
	for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD *)( (BYTE *)FileH + FileHeadSize ) )
	{
		// �f�B���N�g���`�F�b�N
		if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) continue ;

		// �����񐔂ƃp���e�B�`�F�b�N
		NameData = DXA->Table.NameTable + FileH->NameAddress ;
		if( SearchData.PackNum != ((WORD *)NameData)[0] || SearchData.Parity != ((WORD *)NameData)[1] ) continue ;

		// ������`�F�b�N
		NameData += 4 ;
		for( j = 0, k = 0 ; j < SearchData.PackNum ; j ++, k += 4 )
			if( *((DWORD *)&SearchData.FileName[k]) != *((DWORD *)&NameData[k]) ) break ;

		// �K�������t�@�C�����������炱���ŏI��
		if( SearchData.PackNum == j ) break ;
	}

	// ����������G���[
	if( i == Num ) goto ERR ;
	
	// �f�B���N�g�������ɖ߂�
	DXA->CurrentDirectory = OldDir ;
	
	// �ړI�̃t�@�C���̃A�h���X��Ԃ�
	return FileH ;
	
ERR :
	// �f�B���N�g�������ɖ߂�
	DXA->CurrentDirectory = OldDir ;
	
	// �G���[�I��
	return NULL ;
}


// ������������p�̃f�[�^�ɕϊ�( �k�������� \ ����������I�� )
static int DXA_ConvSearchData( DXARC *DXA, DXARC_SEARCHDATA *Dest, const char *Src, int *Length )
{
	int i, StringLength ;
	WORD ParityData ;

	ParityData = 0 ;
	for( i = 0 ; Src[i] != '\0' && Src[i] != '\\' && Src[i] != '/' ; )
	{
		if( CheckMultiByteChar( Src[i], DXA->CharSet ) == TRUE )
		{
			// �Q�o�C�g�����̏ꍇ�͂��̂܂܃R�s�[
			*((WORD *)&Dest->FileName[i]) = *((WORD *)&Src[i]) ;
			ParityData += (BYTE)Src[i] + (BYTE)Src[i+1] ;
			i += 2 ;
		}
		else
		{
			// �������̏ꍇ�͑啶���ɕϊ�
			if( Src[i] >= 'a' && Src[i] <= 'z' )	Dest->FileName[i] = (BYTE)Src[i] - 'a' + 'A' ;
			else									Dest->FileName[i] = Src[i] ;
			ParityData += ( WORD )Dest->FileName[i] ;
			i ++ ;
		}
	}

	// ������̒�����ۑ�
	if( Length != NULL ) *Length = i ;

	// �S�̔{���̈ʒu�܂łO����
	StringLength = ( ( i + 1 ) + 3 ) / 4 * 4 ;
	_MEMSET( &Dest->FileName[i], 0, StringLength - i ) ;

	// �p���e�B�f�[�^�̕ۑ�
	Dest->Parity = ParityData ;

	// �p�b�N�f�[�^���̕ۑ�
	Dest->PackNum = ( WORD )( StringLength / 4 ) ;

	// ����I��
	return 0 ;
}






// ����������쐬
void DXA_KeyCreate( const char *Source, unsigned char *Key )
{
	size_t Len ;

	if( Source == NULL )
	{
		_MEMSET( Key, 0xaa, DXA_KEYSTR_LENGTH ) ;
	}
	else
	{
		Len = strlen( Source ) ;
		if( Len > DXA_KEYSTR_LENGTH )
		{
			_MEMCPY( Key, Source, DXA_KEYSTR_LENGTH ) ;
		}
		else
		{
			// �������� DXA_KEYSTR_LENGTH ���Z�������烋�[�v����
			size_t i ;

			for( i = 0 ; i + Len <= DXA_KEYSTR_LENGTH ; i += Len )
				_MEMCPY( Key + i, Source, Len ) ;
			if( i < DXA_KEYSTR_LENGTH )
				_MEMCPY( Key + i, Source, DXA_KEYSTR_LENGTH - i ) ;
		}
	}

	Key[0] = ( unsigned char )( ~Key[0] ) ;
	Key[1] = ( unsigned char )( ( Key[1] >> 4 ) | ( Key[1] << 4 ) ) ;
	Key[2] = ( unsigned char )( Key[2] ^ 0x8a ) ;
	Key[3] = ( unsigned char )( ~( ( Key[3] >> 4 ) | ( Key[3] << 4 ) ) ) ;
	Key[4] = ( unsigned char )( ~Key[4] ) ;
	Key[5] = ( unsigned char )( Key[5] ^ 0xac ) ;
	Key[6] = ( unsigned char )( ~Key[6] ) ;
	Key[7] = ( unsigned char )( ~( ( Key[7] >> 3 ) | ( Key[7] << 5 ) ) ) ;
	Key[8] = ( unsigned char )( ( Key[8] >> 5 ) | ( Key[8] << 3 ) ) ;
	Key[9] = ( unsigned char )( Key[9] ^ 0x7f ) ;
	Key[10] = ( unsigned char )( ( ( Key[10] >> 4 ) | ( Key[10] << 4 ) ) ^ 0xd6 ) ;
	Key[11] = ( unsigned char )( Key[11] ^ 0xcc ) ;
}

// ����������g�p���� Xor ���Z( Key �͕K�� DXA_KEYSTR_LENGTH �̒������Ȃ���΂Ȃ�Ȃ� )
void DXA_KeyConv( void *Data, LONGLONG SizeLL, LONGLONG PositionLL, unsigned char *Key )
{
	int Position ;

	Position = ( int )( PositionLL % DXA_KEYSTR_LENGTH ) ;

	if( SizeLL <= 0xffffffff )
	{
		DWORD SizeT ;
		SizeT = ( DWORD )SizeLL ;

#ifndef DX_NON_INLINE_ASM
		DWORD DataT ;
		DataT = (DWORD)Data ;

		__asm
		{
			MOV EDI, DataT
			MOV ESI, Key

			MOV EAX, SizeT
			CMP EAX, DXA_KEYSTR_LENGTH
			JB LABEL2


			MOV EAX, Position
			CMP EAX, 0
			JE LABEL1


			MOV EDX, SizeT
	LOOP1:
			MOV BL, [ESI+EAX]
			XOR [EDI], BL
			INC EAX
			INC EDI
			DEC EDX
			CMP EAX, DXA_KEYSTR_LENGTH
			JB LOOP1
			XOR ECX, ECX
			MOV Position, ECX

			MOV SizeT, EDX
			CMP EDX, DXA_KEYSTR_LENGTH
			JB LABEL2


	LABEL1:
			MOV EAX, SizeT
			XOR EDX, EDX
			MOV ECX, DXA_KEYSTR_LENGTH
			DIV ECX
			MOV SizeT, EDX
			MOV ECX, EAX

			MOV EAX, [ESI]
			MOV EBX, [ESI+4]
			MOV EDX, [ESI+8]
	LOOP2:
			XOR [EDI],    EAX
			XOR [EDI+4],  EBX
			XOR [EDI+8],  EDX
			ADD EDI, DXA_KEYSTR_LENGTH
			DEC ECX
			JNZ LOOP2


	LABEL2:
			MOV EDX, SizeT
			CMP EDX, 0
			JE LABEL3


			MOV EAX, Position
	LOOP3:
			MOV BL, [ESI+EAX]
			XOR [EDI], BL
			INC EAX
			CMP EAX, DXA_KEYSTR_LENGTH
			JNE LABEL4
			XOR EAX, EAX
	LABEL4:
			INC EDI
			DEC EDX
			JNZ LOOP3
	LABEL3:
		} ;

#else // DX_NON_INLINE_ASM

		BYTE *DataBP ;
		BYTE *KeyBP ;

		DataBP = ( BYTE * )Data ;
		KeyBP = ( BYTE * )Key ;
		if( SizeT >= DXA_KEYSTR_LENGTH )
		{
			if( Position != 0 )
			{
				do
				{
					*DataBP ^= KeyBP[ Position ] ;
					Position ++ ;
					DataBP ++ ;
					SizeT -- ;
				}while( Position < DXA_KEYSTR_LENGTH ) ;
				Position = 0 ;
			}

			if( SizeT >= DXA_KEYSTR_LENGTH )
			{
				DWORD SetNum ;
				DWORD Key1, Key2, Key3 ;

				SetNum = SizeT / DXA_KEYSTR_LENGTH ;
				SizeT -= SetNum * DXA_KEYSTR_LENGTH ;
				Key1 = ( ( DWORD * )KeyBP )[ 0 ] ;
				Key2 = ( ( DWORD * )KeyBP )[ 1 ] ;
				Key3 = ( ( DWORD * )KeyBP )[ 2 ] ;

				do
				{
					( ( DWORD * )DataBP )[ 0 ] ^= Key1 ;
					( ( DWORD * )DataBP )[ 1 ] ^= Key2 ;
					( ( DWORD * )DataBP )[ 2 ] ^= Key3 ;
					DataBP += DXA_KEYSTR_LENGTH ;
					SetNum -- ;
				}while( SetNum > 0 ) ;
			}
		}

		if( SizeT > 0 )
		{
			do
			{
				*DataBP ^= KeyBP[ Position ] ;
				DataBP ++ ;
				Position ++ ;
				if( Position == DXA_KEYSTR_LENGTH )
					Position = 0 ;
				SizeT -- ;
			}while( SizeT > 0 ) ;
		}
#endif // DX_NON_INLINE_ASM
	}
	else
	{
		BYTE *DataBP ;
		BYTE *KeyBP ;

		DataBP = ( BYTE * )Data ;
		KeyBP = ( BYTE * )Key ;
		if( SizeLL >= DXA_KEYSTR_LENGTH )
		{
			if( Position != 0 )
			{
				do
				{
					*DataBP ^= KeyBP[ Position ] ;
					Position ++ ;
					DataBP ++ ;
					SizeLL -- ;
				}while( Position < DXA_KEYSTR_LENGTH ) ;
				Position = 0 ;
			}

			if( SizeLL >= DXA_KEYSTR_LENGTH )
			{
				DWORD SetNum ;
				DWORD Key1, Key2, Key3 ;

				SetNum = ( DWORD )( SizeLL / DXA_KEYSTR_LENGTH ) ;
				SizeLL -= SetNum * DXA_KEYSTR_LENGTH ;
				Key1 = ( ( DWORD * )KeyBP )[ 0 ] ;
				Key2 = ( ( DWORD * )KeyBP )[ 1 ] ;
				Key3 = ( ( DWORD * )KeyBP )[ 2 ] ;

				do
				{
					( ( DWORD * )DataBP )[ 0 ] ^= Key1 ;
					( ( DWORD * )DataBP )[ 1 ] ^= Key2 ;
					( ( DWORD * )DataBP )[ 2 ] ^= Key3 ;
					DataBP += DXA_KEYSTR_LENGTH ;
					SetNum -- ;
				}while( SetNum > 0 ) ;
			}
		}

		if( SizeLL > 0 )
		{
			do
			{
				*DataBP ^= KeyBP[ Position ] ;
				DataBP ++ ;
				Position ++ ;
				if( Position == DXA_KEYSTR_LENGTH )
					Position = 0 ;
				SizeLL -- ;
			}while( SizeLL > 0 ) ;
		}
	}
}

// �t�@�C������ǂݍ��񂾃f�[�^������������g�p���� Xor ���Z����֐�( Key �͕K�� DXA_KEYSTR_LENGTH �̒������Ȃ���΂Ȃ�Ȃ� )
void DXA_KeyConvFileRead( void *Data, ULONGLONG  Size, DWORD_PTR FilePointer, unsigned char *Key, LONGLONG Position )
{
	LONGLONG pos ;

	// �t�@�C���̈ʒu���擾���Ă���
	if( Position == -1 ) pos = ReadOnlyFileAccessTell( FilePointer ) ;
	else                 pos = Position ;

	// �ǂݍ���
	ReadOnlyFileAccessRead( Data, ( size_t )Size, 1, FilePointer ) ;
	while( ReadOnlyFileAccessIdleCheck( FilePointer ) == FALSE ) Sleep(1);

	// �f�[�^������������g���� Xor ���Z
	DXA_KeyConv( Data, Size, pos, Key ) ;
}

// �����ɓK������I�u�W�F�N�g����������(�����Ώۂ� ObjectCount ���C���f�b�N�X�Ƃ����Ƃ��납��)(�߂�l -1:�G���[ 0:����)
static int DXA_FindProcess( DXA_FINDDATA *FindData, FILEINFO *FileInfo )
{
	BYTE *nameTable;
	DXARC *DXA;
	char *str, *name;
	DWORD fileHeadSize;
	char TempName[ 1024 ] ;

	DXA = FindData->Container;
	nameTable = DXA->Table.NameTable;
	str = FindData->SearchStr;

	if( DXA->V5Flag )
	{
		int i, num, addnum;

		DXARC_DIRECTORY_VER5 *dir;
		DXARC_FILEHEAD_VER5 *file;

		dir = FindData->DirectoryV5;
		num = dir->FileHeadNum;
		addnum = dir->ParentDirectoryAddress == 0xffffffff ? 1 : 2;
		fileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;

		if( FindData->ObjectCount == (DWORD)( num + addnum ) ) return -1;

		file = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + dir->FileHeadAddress + fileHeadSize * ( FindData->ObjectCount - addnum ) ) ;
		for( i = FindData->ObjectCount; i < num + addnum; i ++, file = (DXARC_FILEHEAD_VER5 *)( (BYTE *)file + fileHeadSize ) )
		{
			if( i < addnum )
			{
					 if( i == 0 ){ if( DXA_DIR_FileNameCmp( DXA, ".",  str ) == 0 ) break; }
				else if( i == 1 ){ if( DXA_DIR_FileNameCmp( DXA, "..", str ) == 0 ) break; }
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress + 4 );
				if( DXA_DIR_FileNameCmp( DXA, name, str ) == 0 ) break;
			}
		}
		FindData->ObjectCount = i;
		if( i == num + addnum )
			return -1;

		if( FileInfo )
		{
			if( i < addnum )
			{
				switch( i )
				{
				case 0: _STRCPY( TempName, "."  ); break;
				case 1: _STRCPY( TempName, ".." ); break;
				}
				FileInfo->DirFlag = 1;
				FileInfo->Size    = 0;
				_MEMSET( &FileInfo->CreationTime,  0, sizeof( FileInfo->CreationTime  ) );
				_MEMSET( &FileInfo->LastWriteTime, 0, sizeof( FileInfo->LastWriteTime ) );
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress );
				_STRCPY( TempName, name + ((WORD *)name)[0] * 4 + 4 );
				FileInfo->DirFlag = (file->Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;
				FileInfo->Size = (LONGLONG)file->DataSize;
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.Create,    &FileInfo->CreationTime  );
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.LastWrite, &FileInfo->LastWriteTime );
			}

#ifdef UNICODE
			MBCharToWChar( DXA->HeadV5.CodePage, TempName, ( DXWCHAR * )FileInfo->Name, 260 ) ;
#else
			lstrcpy( FileInfo->Name, TempName );
#endif
		}
	}
	else
	{
		ULONGLONG i, num, addnum;
		DXARC_DIRECTORY *dir;
		DXARC_FILEHEAD *file;

		dir = FindData->Directory;
		num = dir->FileHeadNum;
		addnum = dir->ParentDirectoryAddress == NONE_PAL ? 1 : 2;
		fileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;

		if( FindData->ObjectCount == (DWORD)( num + addnum ) ) return -1;

		file = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + dir->FileHeadAddress + fileHeadSize * ( FindData->ObjectCount - addnum ) ) ;
		for( i = FindData->ObjectCount; i < num + addnum; i ++, file = (DXARC_FILEHEAD *)( (BYTE *)file + fileHeadSize ) )
		{
			if( i < addnum )
			{
					 if( i == 0 ){ if( DXA_DIR_FileNameCmp( DXA, ".",  str ) == 0 ) break; }
				else if( i == 1 ){ if( DXA_DIR_FileNameCmp( DXA, "..", str ) == 0 ) break; }
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress + 4 );
				if( DXA_DIR_FileNameCmp( DXA, name, str ) == 0 ) break;
			}
		}
		FindData->ObjectCount = ( DWORD )i;
		if( i == num + addnum )
			return -1;

		if( FileInfo )
		{
			if( i < addnum )
			{
				switch( i )
				{
				case 0: _STRCPY( TempName, "."  ); break;
				case 1: _STRCPY( TempName, ".." ); break;
				}
				FileInfo->DirFlag = 1;
				FileInfo->Size    = 0;
				_MEMSET( &FileInfo->CreationTime,  0, sizeof( FileInfo->CreationTime  ) );
				_MEMSET( &FileInfo->LastWriteTime, 0, sizeof( FileInfo->LastWriteTime ) );
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress );
				_STRCPY( TempName, name + ((WORD *)name)[0] * 4 + 4 );
				FileInfo->DirFlag = (file->Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;
				FileInfo->Size = (LONGLONG)file->DataSize;
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.Create,    &FileInfo->CreationTime  );
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.LastWrite, &FileInfo->LastWriteTime );
			}

#ifdef UNICODE
			MBCharToWChar( DXA->Head.CodePage, TempName, ( DXWCHAR * )FileInfo->Name, 260 ) ;
#else
			lstrcpy( FileInfo->Name, TempName );
#endif
		}
	}

	return 0;
}



// �A�[�J�C�u�t�@�C���������ׂ̍\���̂�����������
extern	int	DXA_Initialize( DXARC *DXA )
{
	_MEMSET( DXA, 0, sizeof( DXARC ) ) ;

	// �I��
	return 0 ;
}

// �A�[�J�C�u�t�@�C���������ׂ̍\���̂̌�n��������
extern int DXA_Terminate( DXARC *DXA )
{
	DXA_CloseArchive( DXA ) ;

	_MEMSET( DXA, 0, sizeof( DXARC ) ) ;

	// �I��
	return 0 ;
}

// �w��̃f�B���N�g���f�[�^�̈Í�������������( �ۂ��ƃ������ɓǂݍ��񂾏ꍇ�p )
static int DXA_DirectoryKeyConv( DXARC *DXA, DXARC_DIRECTORY *Dir )
{
	// �������C���[�W�ł͂Ȃ��ꍇ�̓G���[
	if( DXA->MemoryOpenFlag == FALSE )
		return -1 ;

	// �o�[�W���� 0x0006 ���O�ł͉������Ȃ�
	if( DXA->V5Flag )
		return 0 ;

	// �Í������������J�n
	{
		DWORD i, FileHeadSize ;
		DXARC_FILEHEAD *File ;

		// �i�[����Ă���t�@�C���̐������J��Ԃ�
		FileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;
		File = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + Dir->FileHeadAddress ) ;
		for( i = 0 ; i < Dir->FileHeadNum ; i ++, File = ( DXARC_FILEHEAD * )( ( BYTE * )File + FileHeadSize ) )
		{
			// �f�B���N�g�����ǂ����ŏ����𕪊�
			if( File->Attributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// �f�B���N�g���̏ꍇ�͍ċA��������
				DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + File->DataAddress ) ) ;
			}
			else
			{
				BYTE *DataP ;

				// �t�@�C���̏ꍇ�͈Í�������������
				
				// �f�[�^������ꍇ�̂ݏ���
				if( File->DataSize != 0 )
				{
					// �f�[�^�ʒu���Z�b�g����
					DataP = ( BYTE * )DXA->MemoryImage + DXA->Head.DataStartAddress + File->DataAddress ;

					// �f�[�^�����k����Ă��邩�ǂ����ŏ����𕪊�
					if( File->PressDataSize != NONE_PAL )
					{
						// ���k����Ă���ꍇ
						DXA_KeyConv( DataP, File->PressDataSize, File->DataSize, DXA->Key ) ;
					}
					else
					{
						// ���k����Ă��Ȃ��ꍇ
						DXA_KeyConv( DataP, File->DataSize, File->DataSize, DXA->Key ) ;
					}
				}
			}
		}
	}

	// �I��
	return 0 ;
}

// �w��̃f�B���N�g���f�[�^�̈Í�������������( �ۂ��ƃ������ɓǂݍ��񂾏ꍇ�p )
static int DXA_DirectoryKeyConvV5( DXARC *DXA, DXARC_DIRECTORY_VER5 *Dir )
{
	// �������C���[�W�ł͂Ȃ��ꍇ�̓G���[
	if( DXA->MemoryOpenFlag == FALSE )
		return -1 ;

	// �o�[�W���� 0x0005 ���O�ł͉������Ȃ�
	if( DXA->HeadV5.Version < 0x0005 )
		return 0 ;
	
	// �Í������������J�n
	{
		DWORD i, FileHeadSize ;
		DXARC_FILEHEAD_VER5 *File ;

		// �i�[����Ă���t�@�C���̐������J��Ԃ�
		FileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
		File = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + Dir->FileHeadAddress ) ;
		for( i = 0 ; i < Dir->FileHeadNum ; i ++, File = ( DXARC_FILEHEAD_VER5 * )( ( BYTE * )File + FileHeadSize ) )
		{
			// �f�B���N�g�����ǂ����ŏ����𕪊�
			if( File->Attributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// �f�B���N�g���̏ꍇ�͍ċA��������
				DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + File->DataAddress ) ) ;
			}
			else
			{
				BYTE *DataP ;

				// �t�@�C���̏ꍇ�͈Í�������������
				
				// �f�[�^������ꍇ�̂ݏ���
				if( File->DataSize != 0 )
				{
					// �f�[�^�ʒu���Z�b�g����
					DataP = ( BYTE * )DXA->MemoryImage + DXA->HeadV5.DataStartAddress + File->DataAddress ;

					// �f�[�^�����k����Ă��邩�ǂ����ŏ����𕪊�
					if( DXA->HeadV5.Version >= 0x0002 && File->PressDataSize != 0xffffffff )
					{
						// ���k����Ă���ꍇ
						DXA_KeyConv( DataP, File->PressDataSize, File->DataSize, DXA->Key ) ;
					}
					else
					{
						// ���k����Ă��Ȃ��ꍇ
						DXA_KeyConv( DataP, File->DataSize, File->DataSize, DXA->Key ) ;
					}
				}
			}
		}
	}

	// �I��
	return 0 ;
}

// ��������ɂ���A�[�J�C�u�t�@�C���C���[�W���J��( 0:����  -1:���s )
extern int DXA_OpenArchiveFromMem( DXARC *DXA, void *ArchiveImage, int ArchiveSize, int ArchiveImageCopyFlag, int ArchiveImageReadOnlyFlag, const char *KeyString, const TCHAR *EmulateArchivePath )
{
	BYTE *datp ;

	// �������C���[�W���R�s�[���Ďg�p����ꍇ�͓ǂݎ���p�ɂ͂��Ȃ�
	if( ArchiveImageCopyFlag )
	{
		ArchiveImageReadOnlyFlag = FALSE ;
	}

	// ���ɂȂ�炩�̃A�[�J�C�u���J���Ă����ꍇ�̓G���[
	if( DXA->WinFilePointer__ != 0 || DXA->MemoryImage != NULL ) return -1 ;

	// ���̍쐬
	DXA_KeyCreate( KeyString, DXA->Key ) ;

	// �t�@�C���p�X��ۑ�
	if( EmulateArchivePath != NULL )
	{
		lstrcpy( DXA->FilePath, EmulateArchivePath ) ;
	}
	else
	{
		_MEMSET( DXA->FilePath, 0, sizeof( DXA->FilePath ) ) ;
	}

	DXA->Table.Top = NULL ;
	DXA->MemoryImage = NULL ;
	if( ArchiveImageCopyFlag )
	{
		// �C���[�W���R�s�[����t���O�������Ă���ꍇ�̓R�s�[��̃������̈���m��
		DXA->MemoryImage = DXALLOC( ( size_t )ArchiveSize ) ;
		if( DXA->MemoryImage == NULL )
			return -1 ;

		// �A�[�J�C�u�t�@�C���C���[�W�̓��e���m�ۂ����������ɃR�s�[
		_MEMCPY( DXA->MemoryImage, ArchiveImage, ArchiveSize ) ;

		// �R�s�[�����������C���[�W���g�p����
		DXA->MemoryImageOriginal = ArchiveImage ;
		ArchiveImage             = DXA->MemoryImage ;
	}
	else
	{
		// �|�C���^��ۑ�
		DXA->MemoryImage         = ArchiveImage ;
		DXA->MemoryImageOriginal = NULL ;
	}

	DXA->V5Flag = FALSE ;

	// �ŏ��Ƀw�b�_�̕����𔽓]����
	_MEMCPY( &DXA->Head, ArchiveImage, DXARC_ID_AND_VERSION_SIZE ) ;
	DXA_KeyConv( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, 0, DXA->Key ) ;

	// �h�c���Ⴄ�ꍇ�̓o�[�W�����Q�ȑO�����ׂ�
	if( DXA->Head.Head != DXAHEAD )
	{
		// �o�[�W�����Q�ȑO�����ׂ�
		_MEMSET( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;

		_MEMCPY( &DXA->Head, ArchiveImage, DXARC_ID_AND_VERSION_SIZE ) ;
		DXA_KeyConv( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, 0, DXA->Key ) ;

		// �o�[�W�����Q�ȑO�ł��Ȃ��ꍇ�̓G���[
		if( DXA->Head.Head != DXAHEAD )
			goto ERR ;
	}

	// �o�[�W����6�ȍ~���ǂ����ŏ����𕪊�
	if( DXA->Head.Version >= 0x0006 )
	{
		DXA->V5Flag = FALSE ;

		DXA->Head.CodePage = 0 ;

		// �w�b�_����͂���
		{
			_MEMCPY( &DXA->Head, ArchiveImage, DXARC_HEAD_VER6_SIZE ) ;
			DXA_KeyConv( &DXA->Head, DXARC_HEAD_VER6_SIZE, 0, DXA->Key ) ;

			// �h�c�̌���
			if( DXA->Head.Head != DXAHEAD ) goto ERR ;
			
			// �o�[�W��������
			if( DXA->Head.Version > DXAVER ) goto ERR ;

			// �R�[�h�y�[�W���Z�b�g
			switch( DXA->Head.CodePage )
			{
			case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
			case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
			case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
			case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
			default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
			}

			// �ǂݎ���p�̏ꍇ�͏��e�[�u���̃T�C�Y���̃��������m�ۂ���
			if( ArchiveImageReadOnlyFlag )
			{
				DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
				if( DXA->Table.Top == NULL ) goto ERR ;

				_MEMCPY( DXA->Table.Top,  (BYTE *)DXA->MemoryImage + DXA->Head.FileNameTableStartAddress, DXA->Head.HeadSize ) ;
			}
			else
			{
				// ���e�[�u���̃A�h���X���Z�b�g����
				DXA->Table.Top          = (BYTE *)DXA->MemoryImage + DXA->Head.FileNameTableStartAddress ;
			}
			DXA_KeyConv( DXA->Table.Top, DXA->Head.HeadSize, 0, DXA->Key ) ;

			DXA->Table.NameTable		= DXA->Table.Top ;
			DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
			DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;
		}

		// �J�����g�f�B���N�g���̃Z�b�g
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;

		DXA->MemoryOpenFlag					= TRUE ;						// �������C���[�W����J���Ă���t���O�𗧂Ă�
		DXA->UserMemoryImageFlag			= TRUE ;						// ���[�U�[�̃C���[�W����J�����t���O�𗧂Ă�
		DXA->MemoryImageSize				= ArchiveSize ;					// �T�C�Y���ۑ����Ă���
		DXA->MemoryImageCopyFlag			= ArchiveImageCopyFlag ;		// �������C���[�W���R�s�[���Ďg�p���Ă��邩�ǂ����̃t���O��ۑ�
		DXA->MemoryImageReadOnlyFlag        = ArchiveImageReadOnlyFlag ;	// �������C���[�W��ǂݎ���p�ɂ��邩�ǂ����̃t���O��ۑ�

		// �S�Ẵt�@�C���f�[�^�̈Í�������������
		if( ArchiveImageReadOnlyFlag == FALSE )
		{
			DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ) ;
		}
	}
	else
	{
		DXA->V5Flag = TRUE ;

		// �o�[�W��������
		if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

		// ��������ǂݎ���p�Ƃ��邩�ǂ����ŏ����𕪊�
		if( ArchiveImageReadOnlyFlag )
		{
			// �o�[�W�������S�ȏォ�ǂ����œǂݍ��ގc��̃w�b�_�T�C�Y��ύX
			if( DXA->HeadV5.Version >= 0x0004 )
			{
				_MEMCPY( &DXA->HeadV5, ArchiveImage, DXARC_HEAD_VER4_SIZE ) ;
				DXA_KeyConv( &DXA->HeadV5, DXARC_HEAD_VER4_SIZE, 0, DXA->Key ) ;
				switch( DXA->HeadV5.CodePage )
				{
				case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
				case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
				case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
				case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
				default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
				}
			}
			else
			{
				_MEMCPY( &DXA->HeadV5, ArchiveImage, DXARC_HEAD_VER3_SIZE ) ;
				DXA_KeyConv( &DXA->HeadV5, DXARC_HEAD_VER3_SIZE, 0, DXA->Key ) ;

				DXA->HeadV5.CodePage = 0 ;
				DXA->CharSet = DX_CHARSET_DEFAULT ;
			}

			// ���e�[�u���̃T�C�Y���̃��������m�ۂ���
			DXA->Table.Top = ( BYTE * )DXALLOC( DXA->HeadV5.HeadSize ) ;
			if( DXA->Table.Top == NULL ) goto ERR ;
			
			// ���e�[�u�����������ɓǂݍ���
			_MEMCPY( DXA->Table.Top, (BYTE *)DXA->MemoryImage + DXA->HeadV5.FileNameTableStartAddress, DXA->HeadV5.HeadSize ) ;
			if( DXA->HeadV5.Version >= 0x0005 )
			{
				DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize,                                     0, DXA->Key ) ;
			}
			else
			{
				DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->HeadV5.FileNameTableStartAddress, DXA->Key ) ;
			}
		}
		else
		{
			// ���ׂẴf�[�^�𔽓]����
			if( DXA->HeadV5.Version < 0x0005 )
			{
				DXA_KeyConv( ArchiveImage, ArchiveSize, 0, DXA->Key ) ;
			}

			datp = (BYTE *)ArchiveImage ;

			// �w�b�_����͂���
			{
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					_MEMCPY( &DXA->HeadV5, datp, DXARC_HEAD_VER3_SIZE ) ;
					DXA_KeyConv( &DXA->HeadV5, DXARC_HEAD_VER3_SIZE, 0, DXA->Key ) ;
				}
				else
				{
					_MEMCPY( &DXA->HeadV5, datp, DXARC_HEAD_VER3_SIZE ) ;
				}
				datp += DXARC_HEAD_VER3_SIZE ;

				// �h�c�̌���
				if( DXA->HeadV5.Head != DXAHEAD ) goto ERR ;
				
				// �o�[�W��������
				if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

				// �o�[�W������ 4�ȏゾ������R�[�h�y�[�W��ǂݍ���
				if( DXA->HeadV5.Version >= 0x0004 )
				{
					DXA->HeadV5.CodePage = *( ( DWORD * )datp ) ;
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( &DXA->HeadV5.CodePage, 4, DXARC_HEAD_VER3_SIZE, DXA->Key ) ;
					}
					switch( DXA->HeadV5.CodePage )
					{
					case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
					case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
					case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
					case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
					default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
					}
				}
				else
				{
					DXA->HeadV5.CodePage = 0 ;
					DXA->CharSet = DX_CHARSET_DEFAULT ;
				}

				// ���e�[�u���̃A�h���X���Z�b�g����
				DXA->Table.Top = (BYTE *)DXA->MemoryImage + DXA->HeadV5.FileNameTableStartAddress ;
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize, 0, DXA->Key ) ;
				}
			}
		}

		// ���e�[�u���̃A�h���X���Z�b�g����
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->HeadV5.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->HeadV5.DirectoryTableStartAddress ;

		// �J�����g�f�B���N�g���̃Z�b�g
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;

		DXA->MemoryOpenFlag					= TRUE ;						// �������C���[�W����J���Ă���t���O�𗧂Ă�
		DXA->UserMemoryImageFlag			= TRUE ;						// ���[�U�[�̃C���[�W����J�����t���O�𗧂Ă�
		DXA->MemoryImageSize				= ArchiveSize ;					// �T�C�Y���ۑ����Ă���
		DXA->MemoryImageCopyFlag			= ArchiveImageCopyFlag ;		// �������C���[�W���R�s�[���Ďg�p���Ă��邩�ǂ����̃t���O��ۑ�
		DXA->MemoryImageReadOnlyFlag        = ArchiveImageReadOnlyFlag ;	// �������C���[�W��ǂݎ���p�ɂ��邩�ǂ����̃t���O��ۑ�

		if( ArchiveImageReadOnlyFlag == FALSE )
		{
			// �S�Ẵt�@�C���f�[�^�̈Í�������������
			if( DXA->HeadV5.Version >= 0x0005 )
			{
				DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ) ;
			}
		}
	}

	// �I��
	return 0 ;

ERR :
	if( ArchiveImageCopyFlag )
	{
		if( DXA->MemoryImage != NULL )
		{
			DXFREE( DXA->MemoryImage ) ;
			DXA->MemoryImage = NULL ;
		}
	}
	else
	if( ArchiveImageReadOnlyFlag )
	{
		if( DXA->Table.Top != NULL )
		{
			DXFREE( DXA->Table.Top ) ;
			DXA->Table.Top = NULL ;
		}
	}
	else
	{
		if( DXA->V5Flag )
		{
			// ���]�����f�[�^�����ɖ߂�
			if( DXA->HeadV5.Version < 0x0005 )
			{
				DXA_KeyConv( ArchiveImage, ArchiveSize, 0, DXA->Key ) ;
			}
		}
	}
	
	// �I��
	return -1 ;
}

// �A�[�J�C�u�t�@�C�����������������������𓾂�( TRUE:�����Ă���  FALSE:�����Ă��Ȃ� )
extern int DXA_CheckIdle( DXARC *DXA )
{
	// �񓯊��I�[�v�����ł͂Ȃ���Γ��ɂ�邱�Ƃ͂Ȃ�
	if( DXA->ASyncOpenFlag == FALSE ) return TRUE ;

	// �t�@�C���ǂݍ��݂��������Ă��邩���ׂ�
	if( ReadOnlyFileAccessIdleCheck( DXA->ASyncOpenFilePointer ) == FALSE ) return FALSE ;

	// �t�@�C�������
	ReadOnlyFileAccessClose( DXA->ASyncOpenFilePointer ) ;
	DXA->ASyncOpenFilePointer = 0;

	// ���ׂẴf�[�^�𔽓]����
	if( DXA->V5Flag == FALSE )
	{
		DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ) ;
	}
	else
	{
		if( DXA->HeadV5.Version >= 0x0005 )
		{
			DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ) ;
		}
		else
		{
			DXA_KeyConv( DXA->MemoryImage, DXA->MemoryImageSize, 0, DXA->Key ) ;
		}
	}

	// �񓯊��I�[�v�����t���O��|��
	DXA->ASyncOpenFlag = FALSE ;

	// �I��
	return TRUE ;
}

// �A�[�J�C�u�t�@�C�����J���ŏ��ɂ��ׂă�������ɓǂݍ���ł��珈������( 0:����  -1:���s )
extern int DXA_OpenArchiveFromFileUseMem( DXARC *DXA, const TCHAR *ArchivePath, const char *KeyString , int ASyncThread )
{
	// ���ɂȂ�炩�̃A�[�J�C�u���J���Ă����ꍇ�̓G���[
	if( DXA->WinFilePointer__ != 0 || DXA->MemoryImage ) return -1 ;

	// ���̍쐬
	DXA_KeyCreate( KeyString, DXA->Key ) ;

	// �w�b�_����������ɓǂݍ���
	DXA->ASyncOpenFilePointer = 0 ;
	DXA->MemoryImage = NULL ;
	DXA->ASyncOpenFilePointer = ReadOnlyFileAccessOpen( ArchivePath, FALSE, TRUE, FALSE ) ;
	if( DXA->ASyncOpenFilePointer == 0 ) return -1 ;

	// �t�@�C���p�X��ۑ�
	lstrcpy( DXA->FilePath, ArchivePath ) ;

	// �t�@�C���̃T�C�Y���擾����
	ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_END ) ;
	DXA->MemoryImageSize = ReadOnlyFileAccessTell( DXA->ASyncOpenFilePointer ) ;
	ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;

	// �t�@�C���̓��e��S�ă������ɓǂݍ��ވׂ̃������̈�̊m��
	DXA->MemoryImage = DXALLOC( ( size_t )DXA->MemoryImageSize ) ;

	// ID�ƃo�[�W������ǂݍ���
	DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;

	// �h�c���Ⴄ�ꍇ�̓o�[�W�����Q�ȑO�����ׂ�
	if( DXA->Head.Head != DXAHEAD )
	{
		// �o�[�W�����Q�ȑO�����ׂ�
		_MEMSET( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;

		ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;
		DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;

		// �o�[�W�����Q�ȑO�ł��Ȃ��ꍇ�̓G���[
		if( DXA->Head.Head != DXAHEAD )
			goto ERR ;
	}

	// �o�[�W����6�ȍ~���ǂ����ŏ����𕪊�
	if( DXA->Head.Version >= 0x0006 )
	{
		DXA->V5Flag = FALSE ;

		// �o�[�W��������
		if( DXA->Head.Version > DXAVER ) goto ERR ;

		// �o�[�W�������S�ȏォ�ǂ����œǂݍ��ގc��̃w�b�_�T�C�Y��ύX
		DXA_KeyConvFileRead( ( BYTE * )&DXA->Head + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER6_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
		switch( DXA->Head.CodePage )
		{
		case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
		case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
		case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
		case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
		default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
		}
		
		// ���e�[�u���̃T�C�Y���̃��������m�ۂ���
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// ���e�[�u�����������ɓǂݍ���
		ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, DXA->Head.FileNameTableStartAddress, SEEK_SET ) ;
		DXA_KeyConvFileRead( DXA->Table.Top, DXA->Head.HeadSize, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;

		// ���e�[�u���̃A�h���X���Z�b�g����
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;

		// �J�����g�f�B���N�g���̃Z�b�g
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
	}
	else
	{
		DXA->V5Flag = TRUE ;

		// �o�[�W��������
		if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

		// �o�[�W�������S�ȏォ�ǂ����œǂݍ��ގc��̃w�b�_�T�C�Y��ύX
		if( DXA->HeadV5.Version >= 0x0004 )
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER4_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			switch( DXA->HeadV5.CodePage )
			{
			case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
			case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
			case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
			case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
			default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
			}
		}
		else
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER3_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			DXA->HeadV5.CodePage = 0 ;
			DXA->CharSet = DX_CHARSET_DEFAULT ;
		}
		
		// ���e�[�u���̃T�C�Y���̃��������m�ۂ���
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->HeadV5.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// ���e�[�u�����������ɓǂݍ���
		ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, DXA->HeadV5.FileNameTableStartAddress, SEEK_SET ) ;
		if( DXA->HeadV5.Version >= 0x0005 )
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;
		}
		else
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->ASyncOpenFilePointer, DXA->Key ) ;
		}

		// ���e�[�u���̃A�h���X���Z�b�g����
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->HeadV5.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->HeadV5.DirectoryTableStartAddress ;

		// �J�����g�f�B���N�g���̃Z�b�g
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;
	}

	// ���߂ăt�@�C�����ۂ��Ɠǂݍ���
	ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;
	ReadOnlyFileAccessRead( DXA->MemoryImage, ( size_t )DXA->MemoryImageSize, 1, DXA->ASyncOpenFilePointer );

	// �t�@�C���񓯊��I�[�v�������Ƃ������Ƃ��Z�b�g���Ă���
	DXA->ASyncOpenFlag = TRUE ;

	DXA->MemoryOpenFlag					= TRUE ;			// �������C���[�W����J���Ă���t���O�𗧂Ă�
	DXA->UserMemoryImageFlag			= FALSE ;			// ���[�U�[�̃C���[�W����J�����킯�ł͂Ȃ��̂Ńt���O��|��
	DXA->MemoryImageCopyFlag			= FALSE ;			// �������C���[�W���R�s�[���Ďg�p���Ă���t���O��|��
	DXA->MemoryImageReadOnlyFlag        = FALSE ;			// �������C���[�W��ǂݎ���p�ɂ��邩�ǂ����̃t���O��|��

	// �����I�[�v���̏ꍇ�͂����ŊJ���I���̂�҂�
	if( ASyncThread == FALSE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// �I��
	return 0 ;

ERR :
	if( DXA->ASyncOpenFilePointer )
	{
		ReadOnlyFileAccessClose( DXA->ASyncOpenFilePointer );
		DXA->ASyncOpenFilePointer = 0;
	}

	if( DXA->MemoryImage )
	{
		DXFREE( DXA->MemoryImage );
		DXA->MemoryImage = 0;
	}

	DXA->ASyncOpenFlag = FALSE ;

	// �I��
	return -1 ;
}

// �A�[�J�C�u�t�@�C�����J��( 0:����  -1:���s )
extern int DXA_OpenArchiveFromFile( DXARC *DXA, const TCHAR *ArchivePath, const char *KeyString )
{
	// ���ɂȂ�炩�̃A�[�J�C�u���J���Ă����ꍇ�̓G���[
	if( DXA->WinFilePointer__ != 0 || DXA->MemoryImage ) return -1 ;

	// �w�b�_�̏�����
	_MEMSET( &DXA->Head, 0, sizeof( DXA->Head ) ) ;

	// �A�[�J�C�u�t�@�C�����J�����Ǝ��݂�
	DXA->WinFilePointer__ = ReadOnlyFileAccessOpen( ArchivePath, FALSE, TRUE, FALSE ) ;
	if( DXA->WinFilePointer__ == 0 ) return -1 ;

	// �t�@�C���p�X��ۑ�
	lstrcpy( DXA->FilePath, ArchivePath ) ;

	// ��������̍쐬
	DXA_KeyCreate( KeyString, DXA->Key ) ;

	// �h�c�ƃo�[�W�����ԍ�����������ǂݍ���
	DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, 0 ) ;

	// �h�c�̌���
	if( DXA->Head.Head != DXAHEAD )
	{
		// �o�[�W�����Q�ȑO�����ׂ�
		_MEMSET( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;

		ReadOnlyFileAccessSeek( DXA->WinFilePointer__, 0L, SEEK_SET ) ;
		DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, 0 ) ;

		// �o�[�W�����Q�ȑO�ł��Ȃ��ꍇ�̓G���[
		if( DXA->Head.Head != DXAHEAD )
			goto ERR ;
	}

	// �o�[�W����6�ȍ~���ǂ����ŏ����𕪊�
	if( DXA->Head.Version >= 0x0006 )
	{
		DXA->V5Flag = FALSE ;

		// �o�[�W��������
		if( DXA->Head.Version > DXAVER ) goto ERR ;

		// �o�[�W�������S�ȏォ�ǂ����œǂݍ��ގc��̃w�b�_�T�C�Y��ύX
		DXA_KeyConvFileRead( ( BYTE * )&DXA->Head + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER6_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
		switch( DXA->Head.CodePage )
		{
		case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
		case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
		case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
		case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
		default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
		}

		// ���e�[�u���̃T�C�Y���̃��������m�ۂ���
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// ���e�[�u�����������ɓǂݍ���
		ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->Head.FileNameTableStartAddress, SEEK_SET ) ;
		DXA_KeyConvFileRead( DXA->Table.Top, DXA->Head.HeadSize, DXA->WinFilePointer__, DXA->Key, 0 ) ;

		// ���e�[�u���̃A�h���X���Z�b�g����
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;

		// �J�����g�f�B���N�g���̃Z�b�g
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
	}
	else
	{
		DXA->V5Flag = TRUE ;

		// �o�[�W��������
		if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

		// �o�[�W�������S�ȏォ�ǂ����œǂݍ��ގc��̃w�b�_�T�C�Y��ύX
		if( DXA->HeadV5.Version >= 0x0004 )
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER4_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			switch( DXA->HeadV5.CodePage )
			{
			case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
			case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
			case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
			case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
			default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
			}
		}
		else
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER3_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			DXA->HeadV5.CodePage = 0 ;
			DXA->CharSet = DX_CHARSET_DEFAULT ;
		}

		// ���e�[�u���̃T�C�Y���̃��������m�ۂ���
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->HeadV5.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// ���e�[�u�����������ɓǂݍ���
		ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->HeadV5.FileNameTableStartAddress, SEEK_SET ) ;
		if( DXA->HeadV5.Version >= 0x0005 )
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->WinFilePointer__, DXA->Key, 0 ) ;
		}
		else
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->WinFilePointer__, DXA->Key ) ;
		}

		// ���e�[�u���̃A�h���X���Z�b�g����
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->HeadV5.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->HeadV5.DirectoryTableStartAddress ;

		// �J�����g�f�B���N�g���̃Z�b�g
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;
	}

	DXA->MemoryOpenFlag					= FALSE ;			// �������C���[�W����J���Ă���t���O��|��
	DXA->UserMemoryImageFlag			= FALSE ;			// ���[�U�[�̃C���[�W����J�����킯�ł͂Ȃ��̂Ńt���O��|��
	DXA->MemoryImageCopyFlag			= FALSE ;			// �������C���[�W���R�s�[���Ďg�p���Ă���t���O��|��
	DXA->MemoryImageReadOnlyFlag        = FALSE ;			// �������C���[�W��ǂݎ���p�ɂ��邩�ǂ����̃t���O��|��

	// �I��
	return 0 ;

ERR :
	if( DXA->WinFilePointer__ != 0 )
	{
		ReadOnlyFileAccessClose( DXA->WinFilePointer__ ) ;
		DXA->WinFilePointer__ = 0 ;
	}

	if( DXA->Table.Top != NULL )
	{
		DXFREE( DXA->Table.Top ) ;
		DXA->Table.Top = NULL ;
	}
	
	// �I��
	return -1 ;
}

// �A�[�J�C�u�t�@�C�������
extern int DXA_CloseArchive( DXARC *DXA )
{
	// ���ɕ��Ă����牽�������I��
	if( DXA->WinFilePointer__ == 0 && DXA->MemoryImage == NULL ) return 0 ;

	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// ����������J���Ă��邩�ǂ����ŏ����𕪊�
	if( DXA->MemoryOpenFlag == TRUE )
	{
		// �A�[�J�C�u�v���O�������������ɓǂݍ��񂾏ꍇ�Ƃ����łȂ��ꍇ�ŏ����𕪊�
		if( DXA->UserMemoryImageFlag == TRUE )
		{
			// ���[�U�[����n���ꂽ�f�[�^�������ꍇ

			// �A�[�J�C�u�C���[�W���R�s�[���Ďg�p���Ă����ꍇ�͒P���ɉ������
			if( DXA->MemoryImageCopyFlag )
			{
				if( DXA->MemoryImage != NULL )
				{
					DXFREE( DXA->MemoryImage ) ;
					DXA->MemoryImage = NULL ;
				}
			}
			else
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// �ǂݎ���p�̏ꍇ�͏��e�[�u���p�Ɋm�ۂ��Ă��������������
					if( DXA->Table.Top != NULL )
					{
						DXFREE( DXA->Table.Top ) ;
						DXA->Table.Top = NULL ;
					}
				}
				else
				{
					// �n���ꂽ�������A�h���X��̓��e�𒼐ڎg�p���Ă����ꍇ��
					// ���]�����f�[�^�����ɖ߂�
					if( DXA->V5Flag == FALSE )
					{
						DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ) ;
						DXA_KeyConv( DXA->Table.Top, DXA->Head.HeadSize, 0, DXA->Key ) ;
					}
					else
					{
						if( DXA->HeadV5.Version >= 0x0005 )
						{
							DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ) ;
							DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize, 0, DXA->Key ) ;
						}
						else
						{
							DXA_KeyConv( DXA->MemoryImage, DXA->MemoryImageSize, 0, DXA->Key ) ;
						}
					}
				}
			}
		}
		else
		{
			// �A�[�J�C�u�v���O�������������ɓǂݍ��񂾏ꍇ

			// �m�ۂ��Ă������������J������
			DXFREE( DXA->Table.Top ) ;
			DXFREE( DXA->MemoryImage ) ;
		}
	}
	else
	{
		// �A�[�J�C�u�t�@�C�������
		ReadOnlyFileAccessClose( DXA->WinFilePointer__ ) ;

		// ���e�[�u�����i�[���Ă����������̈�����
		DXFREE( DXA->Table.Top ) ;
	}

	// ������
	_MEMSET( DXA, 0, sizeof( DXARC ) ) ;

	// �I��
	return 0 ;
}

// �A�[�J�C�u���̃f�B���N�g���p�X��ύX����( 0:����  -1:���s )
static int DXA_ChangeCurrentDirectoryFast( DXARC *DXA, DXARC_SEARCHDATA *SearchData )
{
	int i, j, k, Num ;
	BYTE *NameData, *PathData ;
	WORD PackNum, Parity ;
	DWORD FileHeadSize ;

	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	PackNum  = SearchData->PackNum ;
	Parity   = SearchData->Parity ;
	PathData = SearchData->FileName ;

	// �J�����g�f�B���N�g�����瓯���̃f�B���N�g����T��
	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		FileH = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + DXA->CurrentDirectoryV5->FileHeadAddress ) ;
		Num = (int)DXA->CurrentDirectoryV5->FileHeadNum ;
		FileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
		for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD_VER5 *)( (BYTE *)FileH + FileHeadSize ) )
		{
			// �f�B���N�g���`�F�b�N
			if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) continue ;

			// �����񐔂ƃp���e�B�`�F�b�N
			NameData = DXA->Table.NameTable + FileH->NameAddress ;
			if( PackNum != ((WORD *)NameData)[0] || Parity != ((WORD *)NameData)[1] ) continue ;

			// ������`�F�b�N
			NameData += 4 ;
			for( j = 0, k = 0 ; j < PackNum ; j ++, k += 4 )
				if( *((DWORD *)&PathData[k]) != *((DWORD *)&NameData[k]) ) break ;

			// �K�������f�B���N�g�����������炱���ŏI��
			if( PackNum == j ) break ;
		}

		// ����������G���[
		if( i == Num ) return -1 ;

		// �݂�����J�����g�f�B���N�g����ύX
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + FileH->DataAddress ) ;
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		FileH = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + DXA->CurrentDirectory->FileHeadAddress ) ;
		Num = (int)DXA->CurrentDirectory->FileHeadNum ;
		FileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;
		for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD *)( (BYTE *)FileH + FileHeadSize ) )
		{
			// �f�B���N�g���`�F�b�N
			if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) continue ;

			// �����񐔂ƃp���e�B�`�F�b�N
			NameData = DXA->Table.NameTable + FileH->NameAddress ;
			if( PackNum != ((WORD *)NameData)[0] || Parity != ((WORD *)NameData)[1] ) continue ;

			// ������`�F�b�N
			NameData += 4 ;
			for( j = 0, k = 0 ; j < PackNum ; j ++, k += 4 )
				if( *((DWORD *)&PathData[k]) != *((DWORD *)&NameData[k]) ) break ;

			// �K�������f�B���N�g�����������炱���ŏI��
			if( PackNum == j ) break ;
		}

		// ����������G���[
		if( i == Num ) return -1 ;

		// �݂�����J�����g�f�B���N�g����ύX
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + FileH->DataAddress ) ;
	}

	// ����I��
	return 0 ;
}

// �A�[�J�C�u���̃f�B���N�g���p�X��ύX����( 0:����  -1:���s )
extern int DXA_ChangeCurrentDir( DXARC *DXA, const char *DirPath )
{
	return DXA_ChangeCurrentDirectoryBase( DXA, DirPath, true ) ;
}

// �A�[�J�C�u���̃f�B���N�g���p�X��ύX����( 0:����  -1:���s )
static int DXA_ChangeCurrentDirectoryBase( DXARC *DXA, const char *DirectoryPath, bool ErrorIsDirectoryReset, DXARC_SEARCHDATA *LastSearchData )
{
	DXARC_DIRECTORY *OldDir ;
	DXARC_SEARCHDATA SearchData ;

	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// �����ɗ��܂�p�X�������疳��
	if( _STRCMP( DirectoryPath, "." ) == 0 ) return 0 ;

	// �w\ or /�x�����̏ꍇ�̓��[�g�f�B���N�g���ɖ߂�
	if( _STRCMP( DirectoryPath, "\\" ) == 0 || _STRCMP( DirectoryPath, "/" ) == 0 )
	{
		if( DXA->V5Flag )
		{
			DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;
		}
		else
		{
			DXA->CurrentDirectory   = ( DXARC_DIRECTORY      * )DXA->Table.DirectoryTable ;
		}
		return 0 ;
	}

	// ���Ɉ������p�X�������珈���𕪊�
	if( _STRCMP( DirectoryPath, ".." ) == 0 )
	{
		if( DXA->V5Flag )
		{
			// ���[�g�f�B���N�g���ɋ�����G���[
			if( DXA->CurrentDirectoryV5->ParentDirectoryAddress == 0xffffffff ) return -1 ;
			
			// �e�f�B���N�g�����������炻����Ɉڂ�
			DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + DXA->CurrentDirectoryV5->ParentDirectoryAddress ) ;
		}
		else
		{
			// ���[�g�f�B���N�g���ɋ�����G���[
			if( DXA->CurrentDirectory->ParentDirectoryAddress == NONE_PAL ) return -1 ;
			
			// �e�f�B���N�g�����������炻����Ɉڂ�
			DXA->CurrentDirectory   = ( DXARC_DIRECTORY      * )( DXA->Table.DirectoryTable + DXA->CurrentDirectory->ParentDirectoryAddress ) ;
		}
		return 0 ;
	}

	// ����ȊO�̏ꍇ�͎w��̖��O�̃f�B���N�g����T��
	
	// �ύX�ȑO�̃f�B���N�g����ۑ����Ă���
	OldDir = DXA->CurrentDirectory ;

	// �p�X���Ɂw\�x�����邩�ǂ����ŏ����𕪊�
	if( _STRCHR( DirectoryPath, '\\' ) == NULL && _STRCHR( DirectoryPath, '/' ) == NULL )
	{
		// �t�@�C������������p�̌`���ɕϊ�����
		DXA_ConvSearchData( DXA, &SearchData, DirectoryPath, NULL ) ;

		// �f�B���N�g����ύX
		if( DXA_ChangeCurrentDirectoryFast( DXA, &SearchData ) < 0 ) goto ERR ;
	}
	else
	{
		// \ or / ������ꍇ�͌q�������f�B���N�g������ÂύX���Ă䂭
		int Point, StrLength ;

		Point = 0 ;
		// ���[�v
		for(;;)
		{
			// ��������擾����
			DXA_ConvSearchData( DXA, &SearchData, &DirectoryPath[Point], &StrLength ) ;
			Point += StrLength ;

			// ���������[�� \ or / �������ꍇ�̓��[�g�f�B���N�g���ɗ��Ƃ�
			if( StrLength == 0 && ( DirectoryPath[Point] == '\\' || DirectoryPath[Point] == '/' ) )
			{
				DXA_ChangeCurrentDirectoryBase( DXA, "\\", false ) ;
			}
			else
			{
				// ����ȊO�̏ꍇ�͕��ʂɃf�B���N�g���ύX
				if( DXA_ChangeCurrentDirectoryFast( DXA, &SearchData ) < 0 )
				{
					// �G���[���N���āA�X�ɃG���[���N�������Ɍ��̃f�B���N�g���ɖ߂���
					// �t���O�������Ă���ꍇ�͌��̃f�B���N�g���ɖ߂�
					if( ErrorIsDirectoryReset == true ) DXA->CurrentDirectory = OldDir ;

					// �G���[�I��
					goto ERR ;
				}
			}

			// �����I�[�����ŏI�������ꍇ�̓��[�v���甲����
			// ���͂��� \ or / �����Ȃ��ꍇ�����[�v���甲����
			if( DirectoryPath[Point] == '\0' ||
				( ( DirectoryPath[Point] == '\\' && DirectoryPath[Point+1] == '\0' ) ||
				  ( DirectoryPath[Point] == '/' && DirectoryPath[Point+1] == '\0' ) ) ) break ;
			Point ++ ;
		}
	}

	if( LastSearchData != NULL )
	{
		_MEMCPY( LastSearchData->FileName, SearchData.FileName, SearchData.PackNum * 4 ) ;
		LastSearchData->Parity  = SearchData.Parity ;
		LastSearchData->PackNum = SearchData.PackNum ;
	}

	// ����I��
	return 0 ;

ERR:
	if( LastSearchData != NULL )
	{
		_MEMCPY( LastSearchData->FileName, SearchData.FileName, SearchData.PackNum * 4 ) ;
		LastSearchData->Parity  = SearchData.Parity ;
		LastSearchData->PackNum = SearchData.PackNum ;
	}

	// �G���[�I��
	return -1 ;
}
		
// �A�[�J�C�u���̃J�����g�f�B���N�g���p�X���擾����
extern int DXA_GetCurrentDir( DXARC *DXA, char *DirPathBuffer, int BufferSize )
{
	char DirPath[256] ;
	int Depth, i ;

	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	if( DXA->V5Flag )
	{
		DXARC_DIRECTORY_VER5 *Dir[200], *DirTempP ;

		// ���[�g�f�B���N�g���ɒ����܂Ō�������
		Depth = 0 ;
		DirTempP = DXA->CurrentDirectoryV5 ;
		while( DirTempP->DirectoryAddress != 0xffffffff && DirTempP->DirectoryAddress != 0 )
		{
			Dir[Depth] = DirTempP ;
			DirTempP = ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + DirTempP->ParentDirectoryAddress ) ;
			Depth ++ ;
		}
		
		// �p�X����A������
		DirPath[0] = '\0' ;
		for( i = Depth - 1 ; i >= 0 ; i -- )
		{
			_STRCAT( DirPath, "\\" ) ;
			_STRCAT( DirPath, (char *)DXA->Table.NameTable + ((DXARC_FILEHEAD_VER5 *)( DXA->Table.FileTable + Dir[i]->DirectoryAddress ))->NameAddress ) ;
		}
	}
	else
	{
		DXARC_DIRECTORY *Dir[200], *DirTempP ;

		// ���[�g�f�B���N�g���ɒ����܂Ō�������
		Depth = 0 ;
		DirTempP = DXA->CurrentDirectory ;
		while( DirTempP->DirectoryAddress != 0xffffffff && DirTempP->DirectoryAddress != 0 )
		{
			Dir[Depth] = DirTempP ;
			DirTempP = ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + DirTempP->ParentDirectoryAddress ) ;
			Depth ++ ;
		}
		
		// �p�X����A������
		DirPath[0] = '\0' ;
		for( i = Depth - 1 ; i >= 0 ; i -- )
		{
			_STRCAT( DirPath, "\\" ) ;
			_STRCAT( DirPath, (char *)DXA->Table.NameTable + ((DXARC_FILEHEAD *)( DXA->Table.FileTable + Dir[i]->DirectoryAddress ))->NameAddress ) ;
		}
	}

	// �o�b�t�@�̒������O���A����������Ȃ��Ƃ��̓f�B���N�g�����̒�����Ԃ�
	if( BufferSize == 0 || BufferSize < (int)lstrlenA( DirPath ) )
	{
		return lstrlenA( DirPath ) + 1 ;
	}
	else
	{
		// �f�B���N�g�������o�b�t�@�ɓ]������
		_STRCPY( DirPathBuffer, DirPath ) ;
	}

	// �I��
	return 0 ;
}

// �A�[�J�C�u���̃I�u�W�F�N�g����������( -1:�G���[ -1�ȊO:DXA�����n���h�� )
extern DWORD_PTR DXA_FindFirst( DXARC *DXA, const char *FilePath, FILEINFO *Buffer )
{
	DXA_FINDDATA *find;
	char Dir[256], Name[256];

	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// �������̊m��
	find = (DXA_FINDDATA *)DXALLOC( sizeof( *find ) );
	if( find == NULL ) return ( DWORD_PTR )-1;

	find->Container = DXA;
	DXA_DIR_AnalysisFileNameAndDirPath( DXA, FilePath, Name, Dir );

	// �S�đ啶���ɂ���
	_STRUPR( Dir );
	_STRUPR( Name );

	// �����Ώۂ̃f�B���N�g�����擾
	if( Dir[0] == '\0' )
	{
		find->Directory = DXA->CurrentDirectory;
	}
	else
	{
		DXARC_DIRECTORY *OldDir;

		OldDir = DXA->CurrentDirectory;

		// �w��̃f�B���N�g���������ꍇ�̓G���[
		if( DXA_ChangeCurrentDirectoryBase( DXA, Dir, false ) == -1 )
		{
			DXFREE( find );
			return ( DWORD_PTR )-1;
		}

		find->Directory = DXA->CurrentDirectory;
		DXA->CurrentDirectory = OldDir;
	}

	find->ObjectCount = 0;
	_STRCPY( find->SearchStr, Name );

	// �K������ŏ��̃t�@�C������������
	if( DXA_FindProcess( find, Buffer ) == -1 )
	{
		DXFREE( find );
		return ( DWORD_PTR )-1;
	}
	find->ObjectCount ++ ;

	// �n���h����Ԃ�
	return (DWORD_PTR)find;
}

// �A�[�J�C�u���̃I�u�W�F�N�g����������( -1:�G���[ 0:���� )
extern int DXA_FindNext( DWORD_PTR DxaFindHandle, FILEINFO *Buffer )
{
	DXA_FINDDATA *find;

	find = (DXA_FINDDATA *)DxaFindHandle;
	if( DXA_FindProcess( find, Buffer ) == -1 ) return -1;
	find->ObjectCount ++ ;

	return 0;
}

// �A�[�J�C�u���̃I�u�W�F�N�g�������I������
extern int DXA_FindClose( DWORD_PTR DxaFindHandle )
{
	DXA_FINDDATA *find;

	find = (DXA_FINDDATA *)DxaFindHandle;
	DXFREE( find );

	return 0;
}

// �A�[�J�C�u�t�@�C�����̎w��̃t�@�C�����������ɓǂݍ���( -1:�G���[ 0�ȏ�:�t�@�C���T�C�Y )
extern int DXA_LoadFile( DXARC *DXA, const char *FilePath, void *Buffer, ULONGLONG BufferSize )
{
	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		// �w��̃t�@�C���̏��𓾂�
		FileH = DXA_GetFileInfoV5( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// �t�@�C���T�C�Y������Ă��邩���ׂ�A����Ă��Ȃ����A�o�b�t�@�A���̓T�C�Y���O��������T�C�Y��Ԃ�
		if( BufferSize < FileH->DataSize || BufferSize == 0 || Buffer == NULL )
		{
			return ( int )FileH->DataSize ;
		}
		
		// ����Ă���ꍇ�̓o�b�t�@�[�ɓǂݍ���

		// �t�@�C�������k����Ă��邩�ǂ����ŏ����𕪊�
		if( DXA->HeadV5.Version >= 0x0002 && FileH->PressDataSize != 0xffffffff )
		{
			// ���k����Ă���ꍇ

			// ��������ɓǂݍ���ł��邩�ǂ����ŏ����𕪊�
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					void *temp ;

					// ���k�f�[�^���������ɓǂݍ���ł���𓀂���

					// ���k�f�[�^�����܂郁�����̈�̊m��
					temp = DXALLOC( FileH->PressDataSize ) ;

					// ���k�f�[�^�̓]��
					_MEMCPY( temp, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->DataSize ) ;
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( temp, FileH->PressDataSize,                                   FileH->DataSize, DXA->Key ) ;
					}
					else
					{
						DXA_KeyConv( temp, FileH->PressDataSize, DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXA->Key ) ;
					}
					
					// ��
					DXA_Decode( temp, Buffer ) ;
					
					// �������̉��
					DXFREE( temp ) ;
				}
				else
				{
					// ��������̈��k�f�[�^���𓀂���
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, Buffer ) ;
				}
			}
			else
			{
				void *temp ;

				// ���k�f�[�^���������ɓǂݍ���ł���𓀂���

				// ���k�f�[�^�����܂郁�����̈�̊m��
				temp = DXALLOC( FileH->PressDataSize ) ;

				// ���k�f�[�^�̓ǂݍ���
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->HeadV5.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					DXA_KeyConvFileRead( temp, FileH->PressDataSize, DXA->WinFilePointer__, DXA->Key, FileH->DataSize ) ;
				}
				else
				{
					DXA_KeyConvFileRead( temp, FileH->PressDataSize, DXA->WinFilePointer__, DXA->Key ) ;
				}
				
				// ��
				DXA_Decode( temp, Buffer ) ;
				
				// �������̉��
				DXFREE( temp ) ;
			}
		}
		else
		{
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// �t�@�C���|�C���^���ړ�
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->DataSize ) ;

					// �ǂݍ���
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( Buffer, FileH->DataSize,                                   FileH->DataSize, DXA->Key ) ;
					}
					else
					{
						DXA_KeyConv( Buffer, FileH->DataSize, DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXA->Key ) ;
					}
				}
				else
				{
					// �R�s�[
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->DataSize ) ;
				}
			}
			else
			{
				// �t�@�C���|�C���^���ړ�
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->HeadV5.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;

				// �ǂݍ���
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					DXA_KeyConvFileRead( Buffer, FileH->DataSize, DXA->WinFilePointer__, DXA->Key, FileH->DataSize ) ;
				}
				else
				{
					DXA_KeyConvFileRead( Buffer, FileH->DataSize, DXA->WinFilePointer__, DXA->Key ) ;
				}
			}
		}
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		// �w��̃t�@�C���̏��𓾂�
		FileH = DXA_GetFileInfo( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// �t�@�C���T�C�Y������Ă��邩���ׂ�A����Ă��Ȃ����A�o�b�t�@�A���̓T�C�Y���O��������T�C�Y��Ԃ�
		if( BufferSize < FileH->DataSize || BufferSize == 0 || Buffer == NULL )
		{
			return ( int )FileH->DataSize ;
		}
		
		// ����Ă���ꍇ�̓o�b�t�@�[�ɓǂݍ���

		// �t�@�C�������k����Ă��邩�ǂ����ŏ����𕪊�
		if( FileH->PressDataSize != NONE_PAL )
		{
			// ���k����Ă���ꍇ

			// ��������ɓǂݍ���ł��邩�ǂ����ŏ����𕪊�
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					void *temp ;

					// ���k�f�[�^���������ɓǂݍ���ł���𓀂���

					// ���k�f�[�^�����܂郁�����̈�̊m��
					temp = DXALLOC( ( size_t )FileH->PressDataSize ) ;

					// ���k�f�[�^�̓ǂݍ���
					_MEMCPY( temp, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;
					DXA_KeyConv( temp, FileH->DataSize, ( LONGLONG )FileH->DataSize, DXA->Key ) ;
					
					// ��
					DXA_Decode( temp, Buffer ) ;
					
					// �������̉��
					DXFREE( temp ) ;
				}
				else
				{
					// ��������̈��k�f�[�^���𓀂���
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, Buffer ) ;
				}
			}
			else
			{
				void *temp ;

				// ���k�f�[�^���������ɓǂݍ���ł���𓀂���

				// ���k�f�[�^�����܂郁�����̈�̊m��
				temp = DXALLOC( ( size_t )FileH->PressDataSize ) ;

				// ���k�f�[�^�̓ǂݍ���
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->Head.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;
				DXA_KeyConvFileRead( temp, FileH->PressDataSize, DXA->WinFilePointer__, DXA->Key, ( LONGLONG )FileH->DataSize ) ;
				
				// ��
				DXA_Decode( temp, Buffer ) ;
				
				// �������̉��
				DXFREE( temp ) ;
			}
		}
		else
		{
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// �R�s�[
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;

					DXA_KeyConv( Buffer, FileH->DataSize, FileH->DataSize, DXA->Key ) ;
				}
				else
				{
					// �R�s�[
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;
				}
			}
			else
			{
				// �t�@�C���|�C���^���ړ�
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->Head.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;

				// �ǂݍ���
				DXA_KeyConvFileRead( Buffer, FileH->DataSize, DXA->WinFilePointer__, DXA->Key, FileH->DataSize ) ;
			}
		}
	}
	
	// �I��
	return 0 ;
}

// �A�[�J�C�u�t�@�C�����������ɓǂݍ��񂾏ꍇ�̃t�@�C���C���[�W���i�[����Ă���擪�A�h���X���擾����( DXA_OpenArchiveFromFileUseMem �Ⴕ���� DXA_OpenArchiveFromMem �ŊJ�����ꍇ�ɗL�� )
extern void *DXA_GetFileImage( DXARC *DXA )
{
	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// �������C���[�W����J���Ă��Ȃ�������G���[
	if( DXA->MemoryOpenFlag == FALSE ) return NULL ;

	// �擪�A�h���X��Ԃ�
	return DXA->MemoryImage ;
}

// �A�[�J�C�u�t�@�C�����̎w��̃t�@�C���̃t�@�C�����̈ʒu�ƃt�@�C���̑傫���𓾂�( -1:�G���[ )
extern int DXA_GetFileInfo( DXARC *DXA, const char *FilePath, int *Position, int *Size )
{
	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		// �w��̃t�@�C���̏��𓾂�
		FileH = DXA_GetFileInfoV5( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// �t�@�C���̃f�[�^������ʒu�ƃt�@�C���T�C�Y��ۑ�����
		if( Position != NULL ) *Position = DXA->HeadV5.DataStartAddress + FileH->DataAddress ;
		if( Size     != NULL ) *Size     = FileH->DataSize ;
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		// �w��̃t�@�C���̏��𓾂�
		FileH = DXA_GetFileInfo( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// �t�@�C���̃f�[�^������ʒu�ƃt�@�C���T�C�Y��ۑ�����
		if( Position != NULL ) *Position = ( int )( DXA->Head.DataStartAddress + FileH->DataAddress ) ;
		if( Size     != NULL ) *Size     = ( int )( FileH->DataSize ) ;
	}

	// �����I��
	return 0 ;
}



// �A�[�J�C�u�t�@�C�����̃t�@�C�����J��(�t�@�C�������Ƃ͕K�v�Ȃ�)
extern int DXA_STREAM_Initialize( DXARC_STREAM *DXAStream, DXARC *DXA, const char *FilePath, int UseASyncReadFlag )
{
	// �񓯊������I�[�v�����̏ꍇ�͂����ŊJ���I���̂�҂�
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// �f�[�^�̃Z�b�g
	DXAStream->Archive          = DXA ;
	DXAStream->EOFFlag          = FALSE ;
	DXAStream->FilePoint        = 0 ;
	DXAStream->DecodeDataBuffer = NULL ;
	DXAStream->DecodeTempBuffer = NULL ;
	DXAStream->UseASyncReadFlag = UseASyncReadFlag ;
	DXAStream->ASyncState       = DXARC_STREAM_ASYNCSTATE_IDLE ;

	// �t�@�C������J���Ă���ꍇ�̓A�[�J�C�u�t�@�C���̃t�@�C���|�C���^���쐬
	if( DXA->MemoryOpenFlag == FALSE )
	{
		DXAStream->WinFilePointer = ReadOnlyFileAccessOpen( DXA->FilePath, FALSE, TRUE, FALSE ) ;
		if( DXAStream->WinFilePointer == 0 )
			return -1 ;
	}

	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		// �w��̃t�@�C���̏��𓾂�
		FileH = DXA_GetFileInfoV5( DXA, FilePath ) ;
		if( FileH == NULL )
		{
			if( DXA->MemoryOpenFlag == FALSE )
			{
				ReadOnlyFileAccessClose( DXAStream->WinFilePointer ) ;
				DXAStream->WinFilePointer = 0 ;
			}
			return -1 ;
		}

		// �t�@�C�������Z�b�g
		DXAStream->FileHeadV5 = FileH ;

		// �t�@�C�������k����Ă���ꍇ�͂����œǂݍ���ŉ𓀂��Ă��܂�
		if( DXA->HeadV5.Version >= 0x0002 && FileH->PressDataSize != 0xffffffff )
		{
			// �𓀃f�[�^�����܂郁�����̈�̊m��
			DXAStream->DecodeDataBuffer = DXALLOC( FileH->DataSize ) ;

			// ��������ɓǂݍ��܂�Ă��邩�ǂ����ŏ����𕪊�
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// ���k�f�[�^�����܂郁�����̈�̊m��
					DXAStream->DecodeTempBuffer = DXALLOC( FileH->PressDataSize ) ;

					// ���k�f�[�^�̓ǂݍ���
					_MEMCPY( DXAStream->DecodeTempBuffer, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->PressDataSize ) ;
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( DXAStream->DecodeTempBuffer, FileH->PressDataSize,                                   FileH->DataSize, DXA->Key ) ;
					}
					else
					{
						DXA_KeyConv( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXA->Key ) ;
					}

					// ��
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// �������̉��
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
				else
				{
					// ��
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXAStream->DecodeDataBuffer ) ;
				}
			}
			else
			{
				// ���k�f�[�^�����܂郁�����̈�̊m��
				DXAStream->DecodeTempBuffer = DXALLOC( FileH->PressDataSize ) ;

				// ���k�f�[�^�̓ǂݍ���
				DXAStream->ASyncReadFileAddress = DXA->HeadV5.DataStartAddress + FileH->DataAddress;
				ReadOnlyFileAccessSeek( DXAStream->WinFilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;

				// �񓯊��̏ꍇ�͓ǂݍ��݂ƌ�������ʁX�ɍs��
				if( DXAStream->UseASyncReadFlag == TRUE )
				{
					// �t�@�C������ǂݍ���
					ReadOnlyFileAccessRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, 1, DXAStream->WinFilePointer ) ;
					DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_PRESSREAD ;
				}
				else
				{
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConvFileRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXAStream->WinFilePointer, DXA->Key, FileH->DataSize ) ;
					}
					else
					{
						DXA_KeyConvFileRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXAStream->WinFilePointer, DXA->Key ) ;
					}

					// ��
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// �������̉��
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
			}
		}
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		// �w��̃t�@�C���̏��𓾂�
		FileH = DXA_GetFileInfo( DXA, FilePath ) ;
		if( FileH == NULL )
		{
			if( DXA->MemoryOpenFlag == FALSE )
			{
				ReadOnlyFileAccessClose( DXAStream->WinFilePointer ) ;
				DXAStream->WinFilePointer = 0 ;
			}
			return -1 ;
		}

		// �t�@�C�������Z�b�g
		DXAStream->FileHead = FileH ;

		// �t�@�C�������k����Ă���ꍇ�͂����œǂݍ���ŉ𓀂��Ă��܂�
		if( FileH->PressDataSize != NONE_PAL )
		{
			// �𓀃f�[�^�����܂郁�����̈�̊m��
			DXAStream->DecodeDataBuffer = DXALLOC( ( size_t )FileH->DataSize ) ;

			// ��������ɓǂݍ��܂�Ă��邩�ǂ����ŏ����𕪊�
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// ���k�f�[�^�����܂郁�����̈�̊m��
					DXAStream->DecodeTempBuffer = DXALLOC( ( size_t )FileH->PressDataSize ) ;

					// ���k�f�[�^�̓ǂݍ���
					_MEMCPY( DXAStream->DecodeTempBuffer, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;
					DXA_KeyConv( DXAStream->DecodeTempBuffer, FileH->PressDataSize, FileH->DataSize, DXA->Key ) ;

					// ��
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// �������̉��
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
				else
				{
					// ��
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, DXAStream->DecodeDataBuffer ) ;
				}
			}
			else
			{
				// ���k�f�[�^�����܂郁�����̈�̊m��
				DXAStream->DecodeTempBuffer = DXALLOC( ( size_t )FileH->PressDataSize ) ;

				// ���k�f�[�^�̓ǂݍ���
				DXAStream->ASyncReadFileAddress = DXA->Head.DataStartAddress + FileH->DataAddress;
				ReadOnlyFileAccessSeek( DXAStream->WinFilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;

				// �񓯊��̏ꍇ�͓ǂݍ��݂ƌ�������ʁX�ɍs��
				if( DXAStream->UseASyncReadFlag == TRUE )
				{
					// �t�@�C������ǂݍ���
					ReadOnlyFileAccessRead( DXAStream->DecodeTempBuffer, ( size_t )FileH->PressDataSize, 1, DXAStream->WinFilePointer ) ;
					DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_PRESSREAD ;
				}
				else
				{
					DXA_KeyConvFileRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXAStream->WinFilePointer, DXA->Key, FileH->DataSize ) ;

					// ��
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// �������̉��
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
			}
		}
	}

	// �I��
	return 0 ;
}

// �A�[�J�C�u�t�@�C�����̃t�@�C�������
extern int DXA_STREAM_Terminate( DXARC_STREAM *DXAStream )
{
	// �񓯊��ǂݍ��݂ŏ�Ԃ��܂��ҋ@��Ԃł͂Ȃ�������ҋ@��ԂɂȂ�܂ő҂�
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	// �������̉��
	if( DXAStream->DecodeDataBuffer != NULL )
	{
		DXFREE( DXAStream->DecodeDataBuffer ) ;
		DXAStream->DecodeDataBuffer = NULL ;
	}

	if( DXAStream->DecodeTempBuffer != NULL )
	{
		DXFREE( DXAStream->DecodeTempBuffer ) ;
		DXAStream->DecodeTempBuffer = NULL ;
	}

	// �t�@�C�������
	if( DXAStream->Archive->MemoryOpenFlag == FALSE )
	{
		ReadOnlyFileAccessClose( DXAStream->WinFilePointer ) ;
		DXAStream->WinFilePointer = 0 ;
	}

	// �[��������
	_MEMSET( DXAStream, 0, sizeof( DXARC_STREAM ) ) ;

	// �I��
	return 0 ;
}

// �t�@�C���̓��e��ǂݍ���
extern int DXA_STREAM_Read( DXARC_STREAM *DXAStream, void *Buffer, size_t ReadLength )
{
	size_t ReadSize ;
	ULONGLONG DataSize ;
	ULONGLONG DataStartAddress ;
	ULONGLONG DataAddress ;

	// �񓯊��ǂݍ��݂ŏ�Ԃ��܂��ҋ@��Ԃł͂Ȃ�������ҋ@��ԂɂȂ�܂ő҂�
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	if( DXAStream->Archive->V5Flag )
	{
		DataSize = DXAStream->FileHeadV5->DataSize ;
		DataAddress = DXAStream->FileHeadV5->DataAddress ;
		DataStartAddress = DXAStream->Archive->HeadV5.DataStartAddress ;
	}
	else
	{
		DataSize = DXAStream->FileHead->DataSize ;
		DataAddress = DXAStream->FileHead->DataAddress ;
		DataStartAddress = DXAStream->Archive->Head.DataStartAddress ;
	}

	// EOF �t���O�������Ă�����O��Ԃ�
	if( DXAStream->EOFFlag == TRUE ) return 0 ;

	// EOF ���o
	if( DataSize == DXAStream->FilePoint )
	{
		DXAStream->EOFFlag = TRUE ;
		return 0 ;
	}

	// �f�[�^��ǂݍ��ޗʂ�ݒ肷��
	ReadSize = ReadLength < DataSize - DXAStream->FilePoint ? ReadLength : ( size_t )( DataSize - DXAStream->FilePoint ) ;

	// �f�[�^�����k����Ă������ǂ����ŏ����𕪊�
	if( DXAStream->DecodeDataBuffer != NULL )
	{
		// �f�[�^���R�s�[����
		_MEMCPY( Buffer, (BYTE *)DXAStream->DecodeDataBuffer + DXAStream->FilePoint, ReadSize ) ;
	}
	else
	{
		// ��������Ƀf�[�^�����邩�ǂ����ŏ����𕪊�
		if( DXAStream->Archive->MemoryOpenFlag == TRUE )
		{
			// ��������ɂ���ꍇ

			// �f�[�^���R�s�[����
			_MEMCPY( Buffer, (BYTE *)DXAStream->Archive->MemoryImage + DataStartAddress + DataAddress + DXAStream->FilePoint, ReadSize ) ;

			if( DXAStream->Archive->MemoryImageReadOnlyFlag )
			{
				if( DXAStream->Archive->V5Flag )
				{
					if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( Buffer, ( int )ReadSize,                       DataSize + DXAStream->FilePoint, DXAStream->Archive->Key ) ;
					}
					else
					{
						DXA_KeyConv( Buffer, ( int )ReadSize, DataStartAddress + DataAddress + DXAStream->FilePoint, DXAStream->Archive->Key ) ;
					}
				}
				else
				{
					DXA_KeyConv( Buffer, ( int )ReadSize,                       DataSize + DXAStream->FilePoint, DXAStream->Archive->Key ) ;
				}
			}
		}
		else
		{
			// �t�@�C������ǂݍ���ł���ꍇ

			// �A�[�J�C�u�t�@�C���|�C���^�ƁA���z�t�@�C���|�C���^����v���Ă��邩���ׂ�
			// ��v���Ă��Ȃ�������A�[�J�C�u�t�@�C���|�C���^���ړ�����
			DXAStream->ASyncReadFileAddress = DataAddress + DataStartAddress + DXAStream->FilePoint ;
			if( ( ULONGLONG )ReadOnlyFileAccessTell( DXAStream->WinFilePointer ) != DXAStream->ASyncReadFileAddress )
			{
				ReadOnlyFileAccessSeek( DXAStream->WinFilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;
			}

			// �񓯊��ǂݍ��݂̏ꍇ�Ɠ����ǂݍ��݂̏ꍇ�ŏ����𕪊�
			if( DXAStream->UseASyncReadFlag )
			{
				// �t�@�C������ǂݍ���
				ReadOnlyFileAccessRead( Buffer, ReadSize, 1, DXAStream->WinFilePointer ) ;
				DXAStream->ReadBuffer = Buffer;
				DXAStream->ReadSize = ( int )ReadSize;
				DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_READ ;
			}
			else
			{
				// �f�[�^��ǂݍ���
				if( DXAStream->Archive->V5Flag )
				{
					if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConvFileRead( Buffer, ( int )ReadSize, DXAStream->WinFilePointer, DXAStream->Archive->Key, DataSize + DXAStream->FilePoint ) ;
					}
					else
					{
						DXA_KeyConvFileRead( Buffer, ( int )ReadSize, DXAStream->WinFilePointer, DXAStream->Archive->Key ) ;
					}
				}
				else
				{
					DXA_KeyConvFileRead( Buffer, ( int )ReadSize, DXAStream->WinFilePointer, DXAStream->Archive->Key, DataSize + DXAStream->FilePoint ) ;
				}
			}
		}
	}
	
	// EOF �t���O��|��
	DXAStream->EOFFlag = FALSE ;

	// �ǂݍ��񂾕������t�@�C���|�C���^���ړ�����
	DXAStream->FilePoint += ( int )ReadSize ;
	
	// �ǂݍ��񂾗e�ʂ�Ԃ�
	return ( int )ReadSize ;
}
	
// �t�@�C���|�C���^��ύX����
extern	int DXA_STREAM_Seek( DXARC_STREAM *DXAStream, LONGLONG SeekPoint, int SeekMode )
{
	ULONGLONG DataSize ;

	// �񓯊��ǂݍ��݂ŏ�Ԃ��܂��ҋ@��Ԃł͂Ȃ�������ҋ@��ԂɂȂ�܂ő҂�
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	if( DXAStream->Archive->V5Flag )
	{
		DataSize = DXAStream->FileHeadV5->DataSize ;
	}
	else
	{
		DataSize = DXAStream->FileHead->DataSize ;
	}

	// �V�[�N�^�C�v�ɂ���ď����𕪊�
	switch( SeekMode )
	{
	case SEEK_SET : break ;		
	case SEEK_CUR : SeekPoint += ( LONGLONG )( DXAStream->FilePoint ) ; break ;
	case SEEK_END :	SeekPoint  = ( LONGLONG )( DataSize + SeekPoint ) ; break ;
	}
	
	// �␳
	if( SeekPoint > ( LONGLONG )DataSize ) SeekPoint = ( LONGLONG )DataSize ;
	if( SeekPoint < 0 ) SeekPoint = 0 ;
	
	// �Z�b�g
	DXAStream->FilePoint = SeekPoint ;
	
	// EOF�t���O��|��
	DXAStream->EOFFlag = FALSE ;
	
	// �I��
	return 0 ;
}

// ���݂̃t�@�C���|�C���^�𓾂�
extern	LONGLONG DXA_STREAM_Tell( DXARC_STREAM *DXAStream )
{
	// �񓯊��ǂݍ��݂ŏ�Ԃ��܂��ҋ@��Ԃł͂Ȃ�������ҋ@��ԂɂȂ�܂ő҂�
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	return ( LONGLONG )DXAStream->FilePoint ;
}

// �t�@�C���̏I�[�ɗ��Ă��邩�A�̃t���O�𓾂�
extern	int DXA_STREAM_Eof( DXARC_STREAM *DXAStream )
{
	// �񓯊��ǂݍ��݂ŏ�Ԃ��܂��ҋ@��Ԃł͂Ȃ�������ҋ@��ԂɂȂ�܂ő҂�
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	return DXAStream->EOFFlag ? EOF : 0 ;
}

// �ǂݍ��ݏ������������Ă��邩�ǂ����𒲂ׂ�
extern	int	DXA_STREAM_IdleCheck( DXARC_STREAM *DXAStream )
{
	// �񓯊��ǂݍ��݂ł͂Ȃ��ꍇ�͉������� TRUE ��Ԃ�
	if( DXAStream->UseASyncReadFlag == FALSE ) return TRUE ;

	// ��Ԃɂ���ď����𕪊�
	switch( DXAStream->ASyncState )
	{
	case DXARC_STREAM_ASYNCSTATE_IDLE:			// �ҋ@���
		return TRUE;

	case DXARC_STREAM_ASYNCSTATE_PRESSREAD:		// ���k�f�[�^�ǂݍ��ݑ҂�

		// �ǂݍ��ݏI���҂�
		if( ReadOnlyFileAccessIdleCheck( DXAStream->WinFilePointer ) == FALSE ) return FALSE;

		// �ǂݍ��ݏI�������܂������O��
		if( DXAStream->Archive->V5Flag )
		{
			if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
			{
				DXA_KeyConv( DXAStream->DecodeTempBuffer, DXAStream->FileHeadV5->PressDataSize, DXAStream->FileHeadV5->DataSize, DXAStream->Archive->Key ) ;
			}
			else
			{
				DXA_KeyConv( DXAStream->DecodeTempBuffer, DXAStream->FileHeadV5->PressDataSize, DXAStream->ASyncReadFileAddress, DXAStream->Archive->Key ) ;
			}
		}
		else
		{
			DXA_KeyConv( DXAStream->DecodeTempBuffer, DXAStream->FileHead->PressDataSize, DXAStream->FileHead->DataSize, DXAStream->Archive->Key ) ;
		}

		// ��
		DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
	
		// �������̉��
		DXFREE( DXAStream->DecodeTempBuffer ) ;
		DXAStream->DecodeTempBuffer = NULL ;

		// ��Ԃ�ҋ@��Ԃɂ���
		DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_IDLE;
		return TRUE;

	case DXARC_STREAM_ASYNCSTATE_READ:			// �ǂݍ��ݑ҂�

		// �ǂݍ��ݏI���҂�
		if( ReadOnlyFileAccessIdleCheck( DXAStream->WinFilePointer ) == FALSE ) return FALSE;

		// �ǂݍ��ݏI������献���O��
		if( DXAStream->Archive->V5Flag )
		{
			if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
			{
				DXA_KeyConv( DXAStream->ReadBuffer, DXAStream->ReadSize, DXAStream->FileHeadV5->DataSize + ( DXAStream->ASyncReadFileAddress - ( DXAStream->FileHeadV5->DataAddress + DXAStream->Archive->HeadV5.DataStartAddress ) ), DXAStream->Archive->Key ) ;
			}
			else
			{
				DXA_KeyConv( DXAStream->ReadBuffer, DXAStream->ReadSize, DXAStream->ASyncReadFileAddress, DXAStream->Archive->Key ) ;
			}
		}
		else
		{
			DXA_KeyConv( DXAStream->ReadBuffer, DXAStream->ReadSize, DXAStream->FileHead->DataSize + ( DXAStream->ASyncReadFileAddress - ( DXAStream->FileHead->DataAddress + DXAStream->Archive->Head.DataStartAddress ) ), DXAStream->Archive->Key ) ;
		}

		// ��Ԃ�ҋ@��Ԃɂ���
		DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_IDLE;
		return TRUE;
	}

	return TRUE ;
}

// �t�@�C���̃T�C�Y���擾����
extern	LONGLONG DXA_STREAM_Size( DXARC_STREAM *DXAStream )
{
	if( DXAStream->Archive->V5Flag )
	{
		return DXAStream->FileHeadV5->DataSize ;
	}
	else
	{
		return ( LONGLONG )DXAStream->FileHead->DataSize ;
	}
}







// �t���p�X�ł͂Ȃ��p�X��������t���p�X�ɕϊ�����
static int DXA_DIR_ConvertFullPath( const TCHAR *Src, TCHAR *Dest )
{
	int i, j, k ;
	TCHAR iden[256], CurrentDir[MAX_PATH] ;

	// �J�����g�f�B���N�g���𓾂�
	GetCurrentDirectory( MAX_PATH, CurrentDir ) ;
	_TSTRUPR( CurrentDir ) ;

	if( Src == NULL )
	{
		lstrcpy( Dest, CurrentDir ) ;
		goto END ;
	}

	i = 0 ;
	j = 0 ;
	k = 0 ;

	// �ŏ��Ɂw\�x���́w/�x���Q��A���ő����Ă���ꍇ�̓l�b�g���[�N����Ă���Ɣ��f
	if( ( Src[0] == _T( '\\' ) && Src[1] == _T( '\\' ) ) ||
		( Src[0] == _T( '/' )  && Src[1] == _T( '/'  ) ) )
	{
		Dest[0] = _T( '\\' ) ;
		Dest[1] = _T( '\0' ) ;

		i += 2;
		j ++ ;
	}
	else
	// �ŏ����w\�x���́w/�x�̏ꍇ�̓J�����g�h���C�u�̃��[�g�f�B���N�g���܂ŗ�����
	if( Src[0] == _T( '\\' ) || Src[0] == _T( '/' ) )
	{
		Dest[0] = CurrentDir[0] ;
		Dest[1] = CurrentDir[1] ;
		Dest[2] = _T( '\0' ) ;

		i ++ ;
		j = 2 ;
	}
	else
	// �h���C�u����������Ă����炻�̃h���C�u��
	if( Src[1] == _T( ':' ) )
	{
		Dest[0] = CHARUP(Src[0]) ;
		Dest[1] = Src[1] ;
		Dest[2] = _T( '\0' ) ;

		i = 2 ;
		j = 2 ;

		if( Src[i] == _T( '\\' ) ) i ++ ;
	}
	else
	// ����ȊO�̏ꍇ�̓J�����g�f�B���N�g��
	{
		lstrcpy( Dest, CurrentDir ) ;
		j = lstrlen( Dest ) ;
		if( Dest[j-1] == _T( '\\' ) || Dest[j-1] == _T( '/' ) )
		{
			Dest[j-1] = _T( '\0' ) ;
			j -- ;
		}
	}

	for(;;)
	{
		switch( Src[i] )
		{
		case _T( '\0' ) :
			if( k != 0 )
			{
				Dest[j] = _T( '\\' ) ; j ++ ;
				lstrcpy( &Dest[j], iden ) ;
				j += k ;
				k = 0 ;
			}
			goto END ;

		case _T( '\\' ) :
		case _T( '/' ) :
			// �����񂪖���������X�L�b�v
			if( k == 0 )
			{
				i ++ ;
				break;
			}
			if( lstrcmp( iden, _T( "." ) ) == 0 )
			{
				// �Ȃɂ����Ȃ�
			}
			else
			if( lstrcmp( iden, _T( ".." ) ) == 0 )
			{
				// ����̃f�B���N�g����
				j -- ;
//				while( Dest[j] != _T( '\\' ) && Dest[j] != _T( '/' ) && Dest[j] != _T( ':' ) ) j -- ;
				for(;;)
				{
					if( Dest[j] == _T( '\\' ) || Dest[j] == _T( '/' ) || Dest[j] == _T( ':' ) ) break ;
					j -= CheckDoubleChar( Dest, j - 1, _GET_CHARSET() ) == 2 ? 2 : 1 ;
				}
				if( Dest[j] != _T( ':' ) ) Dest[j] = _T( '\0' ) ;
				else j ++ ;
			}
			else
			{
				Dest[j] = _T( '\\' ) ; j ++ ;
				lstrcpy( &Dest[j], iden ) ;
				j += k ;
			}

			k = 0 ;
			i ++ ;
			break ;
		
		default :
			if( _TMULT( Src[i], _GET_CHARSET() ) == FALSE  )
			{
				iden[k] = CHARUP(Src[i]) ;
				iden[k+1] = _T( '\0' ) ; 
				k ++ ;
				i ++ ;
			}
			else
			{
				iden[k]   = Src[i] ;
				iden[k+1] = Src[i+1] ;
				iden[k+2] = _T( '\0' ) ;
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

// �t�@�C�������ꏏ�ɂȂ��Ă���ƕ������Ă���p�X������t�@�C�����ƃf�B���N�g���p�X�𕪊�����
// �t���p�X�ł���K�v�͖����A�t�@�C���������ł��ǂ�
// DirPath �̏I�[�� �� �}�[�N�͕t���Ȃ�
static int DXA_DIR_AnalysisFileNameAndDirPath( DXARC *DXA, const char *Src, char *FileName, char *DirPath )
{
	int i, Last ;
	
	// �t�@�C�����𔲂��o��
	i = 0 ;
	Last = -1 ;
	while( Src[i] != '\0' )
	{
		if( CheckMultiByteChar( Src[i], DXA->CharSet ) == FALSE )
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
		else             _STRCPY( FileName, Src ) ;
	}
	
	// �f�B���N�g���p�X�𔲂��o��
	if( DirPath != NULL )
	{
		if( Last != -1 )
		{
			_MEMCPY( DirPath, Src, Last ) ;
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

// CmpStr �̏����� Src ���K�����邩�ǂ����𒲂ׂ�( 0:�K������  -1:�K�����Ȃ� )
static int DXA_DIR_FileNameCmp( DXARC *DXA, const char *Src, const char *CmpStr )
{
	const char *s, *c;

	s = Src;
	c = CmpStr;
	while( *c != '\0' || *s != '\0' )
	{
		if( CheckMultiByteChar( *c, DXA->CharSet ) == TRUE )
		{
			if( *((WORD *)s) != *((WORD *)c) ) return -1;
			c += 2;
			s += 2;
		}
		else
		{
			switch( *c )
			{
			case '?':
				c ++ ;
				s ++ ;
				break;

			case '*':
				while( *c == '*' ) c ++ ;
				if( *c == '\0' ) return 0;
				while( *s != '\0' && *s != *c ) s ++ ;
				if( *s == '\0' ) return -1;
				c ++ ;
				s ++ ;
				break;

			default:
				if( *c != *s ) return -1;
				c ++ ;
				s ++ ;
				break;
			}
		}
		if( ( *c == '\0' && *s != '\0' ) || ( *c != '\0' && *s == '\0' ) ) return -1;
	}

	return 0;
}

// �A�[�J�C�u�t�@�C�����t�H���_�Ɍ����Ăăt�@�C�����J�����̏��𓾂�( -1:�A�[�J�C�u�Ƃ��Ă͑��݂��Ȃ�����  0:���݂��� )
static int DXA_DIR_OpenTest( const TCHAR *FilePath, int *ArchiveIndex, char *ArchivePath, char *ArchiveFilePath )
{
	int i, len, arcindex ;
	TCHAR fullpath[256], path[256], temp[256], dir[256], *p ;

	// �t���p�X�𓾂�(���łɑS�Ă̕�����啶���ɂ���)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// �O��Ǝg�p����A�[�J�C�u�̃p�X�������ꍇ�͓����A�[�J�C�u���g�p����
	if( DXARCD.BackUseDirectoryPathLength != 0 && _TSTRNCMP( fullpath, DXARCD.BackUseDirectory, DXARCD.BackUseDirectoryPathLength ) == 0 &&
		( fullpath[ DXARCD.BackUseDirectoryPathLength ] == _T( '\\' ) || fullpath[ DXARCD.BackUseDirectoryPathLength ] == _T( '/' ) ) )
	{
		// �O��g�p�����c�w�`�t�@�C�����J��
		arcindex = DXA_DIR_OpenArchive( DXARCD.BackUseDirectory, NULL, -1, FALSE, FALSE, DXARCD.BackUseArchiveIndex ) ;
		if( arcindex == -1 ) return -1 ;

		// �c�w�`�t�@�C��������ꏊ�ȍ~�̃p�X���쐬����
		p = &fullpath[ DXARCD.BackUseDirectoryPathLength + 1 ] ;
	}
	else
	{
		// �O��Ƃ͈Ⴄ�p�X�̏ꍇ�͈ꂩ�璲�ׂ�

		// �f�B���N�g�������ǂ��čs��
		p = fullpath ;
		len = 0 ;
		for(;;)
		{
			// �l�b�g���[�N����Ă����ꍇ�̐�p����
			if( p - fullpath == 0 && fullpath[0] == _T( '\\' ) && fullpath[1] == _T( '\\' ) )
			{
				path[0] = _T( '\\' );
				path[1] = _T( '\\' );
				path[2] = _T( '\0' );
				len += 2;
				p += 2;
			}

			// �f�B���N�g��������
			for( i = 0 ; *p != _T( '\0' ) && *p !=  _T( '/' ) && *p != _T( '\\' ) ; )
			{
				if( _TMULT( *p, _GET_CHARSET() ) )
				{
					dir[i  ] = path[len+i  ] = p[ 0 ] ;
					dir[i+1] = path[len+i+1] = p[ 1 ] ;
					i += 2 ;
					p += 2 ;
				}
				else
				{
					dir[i] = path[len+i] = *p ;
					i ++ ;
					p ++ ;
				}
			}
			if( *p == _T( '\0' ) || i == 0 ) return -1;
			p ++ ;
			dir[i] = path[len+i] = _T( '\0' ) ;
			len += i ;

			// �t�H���_����DX�A�[�J�C�u�t�@�C�����ɂ���
			_MEMCPY( temp, path, len * 2 ) ;
			temp[len] = _T( '.' ) ;
			if( DXARCD.ArchiveExtensionLength == 0 )	_MEMCPY( &temp[len+1], _T( "DXA" ), 4 * sizeof( TCHAR ) ) ;
			else										_MEMCPY( &temp[len+1], DXARCD.ArchiveExtension, ( DXARCD.ArchiveExtensionLength + 1 ) * sizeof( TCHAR ) ) ;

			// �c�w�`�t�@�C���Ƃ��ĊJ���Ă݂�
			arcindex = DXA_DIR_OpenArchive( temp ) ;
			if( arcindex != -1 ) break ;

			// �J���Ȃ������玟�̊K�w��
			path[len] = _T( '\\' ) ;
			len ++ ;
		}

		// �J�����獡��̏���ۑ�����
		if( DXARCD.ArchiveExtensionLength == 0 )
			_MEMCPY( DXARCD.BackUseDirectory, temp, ( len + 3 + 2 ) * sizeof( TCHAR ) ) ;
		else
			_MEMCPY( DXARCD.BackUseDirectory, temp, ( len + DXARCD.ArchiveExtensionLength + 2 ) * sizeof( TCHAR ) ) ;
		DXARCD.BackUseDirectoryPathLength = len ;
		DXARCD.BackUseArchiveIndex        = arcindex ;
	}

	// �����Z�b�g����
	*ArchiveIndex = arcindex;
#ifdef UNICODE
	int CodePage ;
	CodePage = DXARCD.Archive[ arcindex ]->Archive.HeadV5.CodePage ;
	if( CodePage == 0 ) CodePage = _GET_CODEPAGE() ;
	if( ArchivePath )		WCharToMBChar( CodePage, ( DXWCHAR * )DXARCD.BackUseDirectory, ArchivePath,     512 ) ;
	if( ArchiveFilePath )	WCharToMBChar( CodePage, ( DXWCHAR * )p,                       ArchiveFilePath, 512 ) ;
#else
	if( ArchivePath     ) lstrcpy( ArchivePath,     DXARCD.BackUseDirectory );
	if( ArchiveFilePath ) lstrcpy( ArchiveFilePath, p                       );
#endif

	// �I��
	return 0;
}

// �A�[�J�C�u�t�@�C�����J��
static int DXA_DIR_OpenArchive( const TCHAR *FilePath, void *FileImage, int FileSize, int FileImageCopyFlag, int FileImageReadOnly, int ArchiveIndex, int OnMemory, int ASyncThread )
{
	int					i ;
	int					index ;
	int					newindex ;
	DXARC_DIR_ARCHIVE *	arc ;
	DXARC_DIR_ARCHIVE *	tarc ;
	DXARC				temparc ;

	// �A�[�J�C�u�̎w�肪����ꍇ�͂��̂܂܎g�p����
	if( ArchiveIndex != -1 )
	{
		tarc = DXARCD.Archive[ ArchiveIndex ] ;
		if( tarc != NULL )
		{
			if(	lstrcmp( FilePath, tarc->Path ) == 0 )
			{
				DXARCD.Archive[ArchiveIndex]->UseCounter ++ ;
				return ArchiveIndex ;
			}
		}
	}

	// ���ɊJ����Ă��邩���ׂ�
	newindex = -1 ;
	index    = 0 ;
	for( i = 0 ; i < DXARCD.ArchiveNum ; index ++ )
	{
		arc = DXARCD.Archive[index] ;
		if( arc == NULL )
		{
			newindex = index ;
			continue ;
		}
		
		i ++ ;

		if( lstrcmp( arc->Path, FilePath ) == 0 )
		{
			// ���ɊJ����Ă����ꍇ�͂��̃C���f�b�N�X��Ԃ�
			arc->UseCounter ++ ;
			return index ;
		}
	}
	
	// �Ȃ������ꍇ�͐V�K�Ƀf�[�^��ǉ�����

	// �n���h�������Ɉ�t�̏ꍇ�͎g�p����Ă��Ȃ��A�[�J�C�u�n���h�����������
	if( DXARCD.ArchiveNum == DXA_DIR_MAXARCHIVENUM )
	{
		// ���g�p�̃n���h�������
		DXA_DIR_CloseWaitArchive() ;
		
		// ����ł���t�ł���ꍇ�̓G���[
		if( DXARCD.ArchiveNum == DXA_DIR_MAXARCHIVENUM )
			return -1 ;
	} 
	if( newindex == -1 )
	{
		for( newindex = 0 ; DXARCD.Archive[newindex] != NULL ; newindex ++ ){}
	}

	// �A�[�J�C�u�t�@�C�������݂��Ă��邩�m�F���Ă珉��������
	DXA_Initialize( &temparc ) ;
	if( FileImage != NULL )
	{
		// ��������ɓW�J���ꂽ�t�@�C���C���[�W���g�p����ꍇ
		if( DXA_OpenArchiveFromMem( &temparc, FileImage, FileSize, FileImageCopyFlag, FileImageReadOnly, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL, FilePath ) < 0 )
			return -1 ;
	}
	else
	if( OnMemory == TRUE )
	{
		// �������ɓǂݍ��ޏꍇ
		if( DXA_OpenArchiveFromFileUseMem( &temparc, FilePath, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL, ASyncThread ) < 0 )
			return -1 ;
	}
	else
	{
		// �t�@�C������ǂݍ��ޏꍇ
		if( DXA_OpenArchiveFromFile( &temparc, FilePath, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL ) < 0 )
			return -1 ;
	}

	// �V�����A�[�J�C�u�f�[�^�p�̃��������m�ۂ���
	arc = DXARCD.Archive[ newindex ] = (DXARC_DIR_ARCHIVE *)DXALLOC( sizeof( DXARC_DIR_ARCHIVE ) ) ;
	if( DXARCD.Archive[ newindex ] == NULL )
	{
		DXA_CloseArchive( &temparc ) ;
		DXA_Terminate( &temparc ) ;
		return -1 ;
	}

	// ���Z�b�g
	_MEMCPY( &arc->Archive, &temparc, sizeof( DXARC ) ) ;
	arc->UseCounter = 1 ;
	lstrcpy( arc->Path, FilePath ) ;

	// �g�p���̃A�[�J�C�u�̐��𑝂₷
	DXARCD.ArchiveNum ++ ;

	// �C���f�b�N�X��Ԃ�
	return newindex ;
}

// ���ɊJ����Ă���A�[�J�C�u�̃n���h�����擾����( �߂�l: -1=�������� 0�ȏ�:�n���h�� )
static int DXA_DIR_GetArchive( const TCHAR *FilePath, void *FileImage )
{
	int i, index ;
	DXARC_DIR_ARCHIVE *arc ;

	index = 0 ;
	for( i = 0 ; i < DXARCD.ArchiveNum ; index ++ )
	{
		arc = DXARCD.Archive[index] ;
		if( arc == NULL ) continue ;

		i ++ ;

		if( FilePath )
		{
			if( lstrcmp( arc->Path, FilePath ) == 0 )
				return index ;
		}
		else
		{
			if( arc->Archive.MemoryImageCopyFlag )
			{
				if( arc->Archive.MemoryImageOriginal == FileImage )
					return index ;
			}
			else
			{
				if( arc->Archive.MemoryImage == FileImage )
					return index ;
			}
		}
	}

	return -1 ;
}

// �A�[�J�C�u�t�@�C�������
static int DXA_DIR_CloseArchive( int ArchiveHandle )
{
	DXARC_DIR_ARCHIVE *arc ;

	// �g�p����Ă��Ȃ������牽�������I��
	arc = DXARCD.Archive[ArchiveHandle] ;
	if( arc == NULL || arc->UseCounter == 0 ) return -1 ;

	// �Q�ƃJ�E���^�����炷
	arc->UseCounter -- ;

	// �I��
	return 0 ;
}

// �g�p�����̂�҂��Ă���A�[�J�C�u�t�@�C����S�ĕ���
static void DXA_DIR_CloseWaitArchive( void )
{
	int i, Num, index ;
	DXARC_DIR_ARCHIVE *arc ;
	
	Num = DXARCD.ArchiveNum ;
	for( i = 0, index = 0 ; i < Num ; index ++ )
	{
		if( DXARCD.Archive[index] == NULL ) continue ;
		i ++ ;

		arc = DXARCD.Archive[index] ;

		// �g���Ă����������Ȃ�
		if( arc->UseCounter > 0 ) continue ;

		// ��n��
		DXA_CloseArchive( &arc->Archive ) ;
		DXA_Terminate( &arc->Archive ) ;
		DXFREE( arc ) ;
		DXARCD.Archive[index] = NULL ;
		
		// �A�[�J�C�u�̐������炷
		DXARCD.ArchiveNum -- ;
	}
}

// �A�[�J�C�u���f�B���N�g���Ɍ����Ă鏈���̏�����
extern	int DXA_DIR_Initialize( void )
{
	// ���ɏ������ς݂̏ꍇ�͉������Ȃ�
	if( DXARCD.InitializeFlag )
		return -1 ;

	// �N���e�B�J���Z�N�V�����̏�����
	CriticalSection_Initialize( &DXARCD.CriticalSection ) ;

//	_MEMSET( &DXARCD, 0, sizeof( DXARC_DIR ) ) ;
//	DXA_DIR_Terminate() ;

	// �g�p����Ă��Ȃ��A�[�J�C�u�t�@�C�����������
	DXA_DIR_CloseWaitArchive() ;

	// �����������t���O�𗧂Ă�
	DXARCD.InitializeFlag = TRUE ;

	// �I��
	return 0 ;
}


// �A�[�J�C�u���f�B���N�g���Ɍ����Ă鏈���̌�n��
extern int DXA_DIR_Terminate( void )
{
	// ���Ɍ�n���ς݂̏ꍇ�͉������Ȃ�
	if( DXARCD.InitializeFlag == FALSE )
		return -1 ;

	// �g�p����Ă��Ȃ��A�[�J�C�u�t�@�C�����������
	DXA_DIR_CloseWaitArchive() ;

	// �N���e�B�J���Z�N�V�����̌�n��
	CriticalSection_Delete( &DXARCD.CriticalSection ) ;

	// �����������t���O��|��
	DXARCD.InitializeFlag = FALSE ;

	// �I��
	return 0 ;
}

// �A�[�J�C�u�t�@�C���̊g���q��ݒ肷��
extern int DXA_DIR_SetArchiveExtension( const TCHAR *Extension )
{
	int Length ;
	
	Length = lstrlen( Extension ) ;

	if( Length >= 64 || Extension == NULL || Extension[0] == _T( '\0' ) )
	{
		DXARCD.ArchiveExtension[0] = 0 ;
		DXARCD.ArchiveExtensionLength = 0 ;
	}
	else
	{
		DXARCD.ArchiveExtensionLength = Length ;
		lstrcpy( DXARCD.ArchiveExtension, Extension ) ;
	}

	// �I��
	return 0 ;
}

// �A�[�J�C�u�t�@�C���ƒʏ�̃t�H���_�̂ǂ�������݂����ꍇ�A�ǂ����D�悳���邩��ݒ肷��( 1:�t�H���_��D�� 0:�c�w�A�[�J�C�u�t�@�C����D��(�f�t�H���g) )
extern int DXA_DIR_SetDXArchivePriority( int Priority )
{
	DXARCD.DXAPriority = Priority ;

	// �I��
	return 0 ;
}

// �A�[�J�C�u�t�@�C���̌��������ݒ肷��
extern int DXA_DIR_SetKeyString( const char *KeyString )
{
	if( KeyString == NULL )
	{
		DXARCD.ValidKeyString = FALSE ;
	}
	else
	{
		DXARCD.ValidKeyString = TRUE ;
		if( lstrlenA( KeyString ) > DXA_KEYSTR_LENGTH )
		{
			_MEMCPY( DXARCD.KeyString, KeyString, DXA_KEYSTR_LENGTH ) ;
			DXARCD.KeyString[ DXA_KEYSTR_LENGTH ] = '\0' ;
		}
		else
		{
			_STRCPY( DXARCD.KeyString, KeyString ) ;
		}
	}

	// �I��
	return 0 ;
}

// �t�@�C�����ۂ��Ɠǂݍ��ފ֐�
extern LONGLONG DXA_DIR_LoadFile( const TCHAR *FilePath, void *Buffer, int BufferSize )
{
	LONGLONG siz ;
	DWORD_PTR handle ;

	handle = DXA_DIR_Open( FilePath ) ;
	if( handle == -1 ) return false ;

	DXA_DIR_Seek( handle, 0L, SEEK_END ) ;
	siz = DXA_DIR_Tell( handle ) ;
	DXA_DIR_Seek( handle, 0L, SEEK_SET ) ;

	if( siz <= BufferSize )
	{
		DXA_DIR_Read( Buffer, ( size_t )siz, 1, handle ) ;
	}

	DXA_DIR_Close( handle ) ;

	// �I��
	return siz ;
}

// DXA_DIR_Open �̊�{�֐�
extern DWORD_PTR DXA_DIR_Open( const TCHAR *FilePath, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	int index ;
	DXARC_DIR_FILE *file ;
	char DXAErrorStr[256], DxaInFilePath[256] ;

	// ����������Ă��Ȃ������珉��������
	if( DXARCD.InitializeFlag == FALSE )
	{
		DXA_DIR_Initialize() ;
	}

	// �N���e�B�J���Z�N�V�����̎擾
	CRITICALSECTION_LOCK( &DXARCD.CriticalSection ) ;

	UseCacheFlag  = UseCacheFlag ;
	BlockReadFlag = BlockReadFlag ;
	DXAErrorStr[0] = 0 ;

	// �󂫃f�[�^��T��
	if( DXARCD.FileNum == DXA_DIR_MAXFILENUM )
	{
		// �N���e�B�J���Z�N�V�����̉��
		CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

		DXST_ERRORLOG_ADD( _T( "�����ɊJ����t�@�C���̐������E�𒴂��܂���\n" ) ) ;
		return 0 ;
	}
	for( index = 0 ; DXARCD.File[index] != NULL ; index ++ ){}

	// �������̊m��
	DXARCD.File[index] = (DXARC_DIR_FILE *)DXALLOC( sizeof( DXARC_DIR_FILE ) ) ;
	if( DXARCD.File[index] == NULL )
	{
		// �N���e�B�J���Z�N�V�����̉��
		CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

		DXST_ERRORLOG_ADD( _T( "�t�@�C���̏����i�[���郁�����̊m�ۂɎ��s���܂���\n" ) ) ;
		return 0 ;
	}
	file = DXARCD.File[index] ;

	// �A�[�J�C�u�t�@�C���ƃt�H���_�̂ǂ����D�悷�邩�ŏ����𕪊�
	if( DXARCD.DXAPriority == 0 )
	{
		// �A�[�J�C�u��D�悷��ꍇ

		// �A�[�J�C�u�t�@�C�������������ׂ�
		if( DXA_DIR_OpenTest( FilePath, (int *)&file->UseArchiveIndex, NULL, DxaInFilePath ) == 0 )
		{
			// �A�[�J�C�u���g�p���Ă���t���O�𗧂Ă�
			file->UseArchiveFlag = 1 ;

			// �f�B���N�g�����Ɠ����̂c�w�`�t�@�C�����J�����炻�̒�����w��̃t�@�C����ǂݍ������Ƃ���
			if( DXA_STREAM_Initialize( &file->DXAStream, &DXARCD.Archive[ file->UseArchiveIndex ]->Archive, DxaInFilePath, UseASyncReadFlag ) < 0 )
			{
				_STRCPY( DXAErrorStr, DXSTRING( "�c�w�`�t�@�C���̒��Ɏw��̃t�@�C���͌�����܂���ł���\n" ) ) ;
				DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;
				goto ERR ;
			}
		}
		else
		{
			// �A�[�J�C�u�t�@�C�������������畁�ʂ̃t�@�C������ǂݍ��ސݒ���s��
			file->UseArchiveFlag = 0 ;

			// ���ʂ̃t�@�C�������������ׂ�
			file->WinFilePointer_ = ReadOnlyFileAccessOpen( FilePath, UseCacheFlag, TRUE, UseASyncReadFlag ) ;
			if( file->WinFilePointer_ == 0 ) goto ERR ;
		}
	}
	else
	{
		// ���ʂ̃t�@�C����D�悷��ꍇ

		// ���ʂ̃t�@�C�������������ׂ�
		if( ( file->WinFilePointer_ = ReadOnlyFileAccessOpen( FilePath, UseCacheFlag, TRUE, UseASyncReadFlag ) ) != 0 )
		{
			// �J�����畁�ʂ̃t�@�C������ǂݍ��ސݒ���s��
			file->UseArchiveFlag = 0 ;
		}
		else
		{
			// �A�[�J�C�u�t�@�C�������������ׂ�
			if( DXA_DIR_OpenTest( FilePath, (int *)&file->UseArchiveIndex, NULL, DxaInFilePath ) == 0 )
			{
				// �A�[�J�C�u���g�p���Ă���t���O�𗧂Ă�
				file->UseArchiveFlag = 1 ;

				// �f�B���N�g�����Ɠ����̂c�w�`�t�@�C�����J�����炻�̒�����w��̃t�@�C����ǂݍ������Ƃ���
				if( DXA_STREAM_Initialize( &file->DXAStream, &DXARCD.Archive[ file->UseArchiveIndex ]->Archive, DxaInFilePath, UseASyncReadFlag ) < 0 )
				{
					_STRCPY( DXAErrorStr, DXSTRING( "�c�w�`�t�@�C���̒��Ɏw��̃t�@�C���͌�����܂���ł���\n" ) ) ;
					DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;
					goto ERR ;
				}
			}
			else
			{
				// ����������G���[
				goto ERR;
			}
		}
	}

	// �n���h���̐��𑝂₷
	DXARCD.FileNum ++ ;

	// �N���e�B�J���Z�N�V�����̉��
	CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

	// �C���f�b�N�X��Ԃ�
	return index | 0xF0000000 ;

ERR:
	// �������̉��
	if( DXARCD.File[index] != NULL ) DXFREE( DXARCD.File[index] ) ;
	DXARCD.File[index] = NULL ;
	
	// �G���[������o��
//	DXST_ERRORLOGFMT_ADD(( "�t�@�C�� %s �̃I�[�v���Ɏ��s���܂���\n", FilePath )) ;
//	if( DXAErrorStr[0] != '\0' ) DXST_ERRORLOGFMT_ADD(( "�c�w�`�G���[�F%s", DXAErrorStr )) ;

	// �N���e�B�J���Z�N�V�����̉��
	CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

	// �G���[�I��
	return 0 ;
}

// �t�@�C�������
extern int DXA_DIR_Close( DWORD_PTR Handle )
{
	// �N���e�B�J���Z�N�V�����̎擾
	CRITICALSECTION_LOCK( &DXARCD.CriticalSection ) ;

	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;

	// �g�p����Ă��Ȃ������牽�������I��
	if( file == NULL )
	{
		// �N���e�B�J���Z�N�V�����̉��
		CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

		return -1 ;
	}
	
	// �A�[�J�C�u���g�p���Ă��邩�ǂ����ŕ���
	if( file->UseArchiveFlag == FALSE )
	{
		// �g�p���Ă��Ȃ��ꍇ�͕W�����o�͂̃t�@�C���|�C���^���������
		ReadOnlyFileAccessClose( file->WinFilePointer_ ) ;
		file->WinFilePointer_ = 0 ;
	}
	else
	{
		// �A�[�J�C�u���g�p���Ă����ꍇ�̓A�[�J�C�u�̎Q�Ɛ������炷
		
		// �A�[�J�C�u�t�@�C���̎Q�Ɛ������炷
		DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;

		// �A�[�J�C�u�t�@�C���̌�n��
		DXA_STREAM_Terminate( &file->DXAStream ) ;
	}

	// ���������������
	DXFREE( file ) ;
	DXARCD.File[Handle & 0x0FFFFFFF] = NULL ;
	
	// �������炷
	DXARCD.FileNum -- ;

	// �N���e�B�J���Z�N�V�����̉��
	CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

	// �I��
	return 0 ;
}

// �t�@�C���|�C���^�̈ʒu���擾����
extern	LONGLONG DXA_DIR_Tell( DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessTell( file->WinFilePointer_ ) ;
	else							return DXA_STREAM_Tell( &file->DXAStream ) ;
}

// �t�@�C���|�C���^�̈ʒu��ύX����
extern int DXA_DIR_Seek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessSeek( file->WinFilePointer_, SeekPoint, SeekType ) ;
	else							return DXA_STREAM_Seek( &file->DXAStream, SeekPoint, SeekType ) ;
}

// �t�@�C������f�[�^��ǂݍ���
extern size_t DXA_DIR_Read( void *Buffer, size_t BlockSize, size_t BlockNum, DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return ( size_t )-1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessRead( Buffer, BlockSize, BlockNum, file->WinFilePointer_ ) ;
	else							return DXA_STREAM_Read( &file->DXAStream, Buffer, BlockSize * BlockNum ) / BlockSize ;
}

// �t�@�C���̏I�[�𒲂ׂ�
extern int DXA_DIR_Eof( DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 ) return ReadOnlyFileAccessEof( file->WinFilePointer_ ) ;
	else							return DXA_STREAM_Eof( &file->DXAStream ) ;
}

extern	int DXA_DIR_ChDir( const TCHAR *Path )
{
	::SetCurrentDirectory( Path ) ;

	return 0 ;
}

extern	int DXA_DIR_GetDir( TCHAR *Buffer )
{
	::GetCurrentDirectory( 256, Buffer ) ;

	return 0 ;
}


extern	int DXA_DIR_IdleCheck( DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessIdleCheck( file->WinFilePointer_ ) ;
	else							return DXA_STREAM_IdleCheck( &file->DXAStream ) ;
}

// �߂�l: -1=�G���[  -1�ȊO=FindHandle
extern DWORD_PTR DXA_DIR_FindFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	DXA_DIR_FINDDATA *find;
	char nPath[256];

	// �������̊m��
	find = ( DXA_DIR_FINDDATA * )DXALLOC( sizeof( DXA_DIR_FINDDATA ) );
	if( find == NULL ) return ( DWORD_PTR )-1;
	_MEMSET( find, 0, sizeof( *find ) );

	// �w��̃I�u�W�F�N�g���A�[�J�C�u�t�@�C���������ׂ�
	if( DXA_DIR_OpenTest( FilePath, &find->UseArchiveIndex, NULL, nPath ) == -1 )
	{
		// �A�[�J�C�u�t�@�C�����ł͂Ȃ������ꍇ�̓t�@�C�����猟������
		find->UseArchiveFlag = 0;
		find->FindHandle = ReadOnlyFileAccessFindFirst( FilePath, Buffer );
	}
	else
	{
		// �A�[�J�C�u�t�@�C�����̏ꍇ�̓A�[�J�C�u�t�@�C�������猟������
		find->UseArchiveFlag = 1;
		find->FindHandle = DXA_FindFirst( &DXARCD.Archive[ find->UseArchiveIndex ]->Archive, nPath, Buffer );
	}

	// �����n���h�����擾�ł��Ȃ������ꍇ�̓G���[
	if( find->FindHandle == ( DWORD_PTR )-1 )
	{
		DXFREE( find );
		return ( DWORD_PTR )-1;
	}

	// �n���h����Ԃ�
	return (DWORD_PTR)find;
}

// �߂�l: -1=�G���[  0=����
extern int DXA_DIR_FindNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	DXA_DIR_FINDDATA *find;

	find = (DXA_DIR_FINDDATA *)FindHandle;
	if( find->UseArchiveFlag == 0 )
		return ReadOnlyFileAccessFindNext( find->FindHandle, Buffer );
	else
		return DXA_FindNext( find->FindHandle, Buffer );
}

// �߂�l: -1=�G���[  0=����
extern int DXA_DIR_FindClose( DWORD_PTR FindHandle )
{
	DXA_DIR_FINDDATA *find;

	find = (DXA_DIR_FINDDATA *)FindHandle;
	if( find->UseArchiveFlag == 0 )
	{
		ReadOnlyFileAccessFindClose( find->FindHandle );
	}
	else
	{
		DXA_FindClose( find->FindHandle );
		DXA_DIR_CloseArchive( find->UseArchiveIndex ) ;
	}

	DXFREE( find );

	return 0;
}




// �w��̂c�w�`�t�@�C�����ۂ��ƃ������ɓǂݍ���( �߂�l: -1=�G���[  0=���� )
extern int NS_DXArchivePreLoad( const TCHAR *FilePath , int ASyncThread )
{
	TCHAR fullpath[256];

	// �t���p�X�𓾂�(���łɑS�Ă̕�����啶���ɂ���)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	return DXA_DIR_OpenArchive( fullpath, NULL, -1, FALSE, FALSE, -1, TRUE, ASyncThread ) == -1 ? -1 : 0;
}

// �w��̂c�w�`�t�@�C���̎��O�ǂݍ��݂������������ǂ������擾����( �߂�l�F TRUE=�������� FALSE=�܂� )
extern int NS_DXArchiveCheckIdle( const TCHAR *FilePath )
{
	int handle;
	TCHAR fullpath[256];

	// �t���p�X�𓾂�(���łɑS�Ă̕�����啶���ɂ���)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// �t�@�C���p�X����n���h�����擾����
	handle = DXA_DIR_GetArchive( fullpath );
	if( handle == -1 ) return 0 ;

	// �����������������ǂ����𓾂�
	return DXA_CheckIdle( &DXARCD.Archive[handle]->Archive );
}

// �w��̂c�w�`�t�@�C��������������������
extern int NS_DXArchiveRelease( const TCHAR *FilePath )
{
	int handle;
	TCHAR fullpath[256];

	// �t���p�X�𓾂�(���łɑS�Ă̕�����啶���ɂ���)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// �t�@�C���p�X����n���h�����擾����
	handle = DXA_DIR_GetArchive( fullpath ) ;
	if( handle == -1 ) return 0 ;

	// ����
	DXA_DIR_CloseArchive( handle ) ;
	DXA_DIR_CloseWaitArchive() ;

	// �I��
	return 0 ;
}

// �c�w�`�t�@�C���̒��Ɏw��̃t�@�C�������݂��邩�ǂ����𒲂ׂ�ATargetFilePath �͂c�w�`�t�@�C�����J�����g�t�H���_�Ƃ����ꍇ�̃p�X( �߂�l:  -1=�G���[  0:����  1:���� )
extern int NS_DXArchiveCheckFile( const TCHAR *FilePath, const TCHAR *TargetFilePath )
{
	int index, ret ;
	DXARC_DIR_ARCHIVE *Archive ;
	TCHAR fullpath[256];

	// �t���p�X�𓾂�(���łɑS�Ă̕�����啶���ɂ���)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// �A�[�J�C�u�t�@�C�������邩�ǂ������ׂ�
	index = DXA_DIR_OpenArchive( FilePath ) ;
	if( index == -1 ) return -1 ;

	// �A�[�J�C�u�̒��Ɏw��̃t�@�C�������邩�ǂ����𒲂ׂ�
	Archive = DXARCD.Archive[ index ] ;

#ifdef UNICODE
	// �}���`�o�C�g������ɕϊ�
	char TargetFilePathA[1024];
	WCharToMBChar( Archive->Archive.HeadV5.CodePage == 0 ? _GET_CODEPAGE() : Archive->Archive.HeadV5.CodePage, ( DXWCHAR * )TargetFilePath, TargetFilePathA, 256 ) ;
	ret = DXA_GetFileInfo( &Archive->Archive, TargetFilePathA, NULL, NULL ) ;
#else
	ret = DXA_GetFileInfo( &Archive->Archive, TargetFilePath, NULL, NULL ) ;
#endif

	DXA_DIR_CloseArchive( index ) ;

	// ���ʂ�Ԃ�
	return ret == -1 ? 0 : 1 ;
}

// ��������ɓW�J���ꂽ�c�w�`�t�@�C�����w��̃t�@�C���p�X�ɂ��邱�Ƃɂ���
extern int NS_DXArchiveSetMemImage(		void *ArchiveImage, int ArchiveImageSize, const TCHAR *EmulateFilePath, int ArchiveImageCopyFlag, int ArchiveImageReadOnly )
{
	TCHAR fullpath[256];

	// �t���p�X�𓾂�(���łɑS�Ă̕�����啶���ɂ���)
	DXA_DIR_ConvertFullPath( EmulateFilePath, fullpath ) ;

	return DXA_DIR_OpenArchive( fullpath, ArchiveImage, ArchiveImageSize, ArchiveImageCopyFlag, ArchiveImageReadOnly, -1, FALSE, FALSE ) == -1 ? -1 : 0;
}

// DXArchiveSetMemImage �̐ݒ����������
extern int NS_DXArchiveReleaseMemImage(	void *ArchiveImage )
{
	int handle;

	// �t�@�C���p�X����n���h�����擾����
	handle = DXA_DIR_GetArchive( NULL, ArchiveImage ) ;
	if( handle == -1 ) return 0 ;

	// ����
	DXA_DIR_CloseArchive( handle ) ;
	DXA_DIR_CloseWaitArchive() ;

	// �I��
	return 0 ;
}

#endif


// �f�[�^���𓀂���( �߂�l:�𓀌�̃f�[�^�T�C�Y )
#define MIN_COMPRESS		(4)						// �Œሳ�k�o�C�g��
#define MAX_SEARCHLISTNUM	(64)					// �ő��v����T���ׂ̃��X�g��H��ő吔
#define MAX_SUBLISTNUM		(65536)					// ���k���ԒZ�k�̂��߂̃T�u���X�g�̍ő吔
#define MAX_COPYSIZE 		(0x1fff + MIN_COMPRESS)	// �Q�ƃA�h���X����R�s�[�o�؂�ő�T�C�Y( ���k�R�[�h���\���ł���R�s�[�T�C�Y�̍ő�l + �Œሳ�k�o�C�g�� )
#define MAX_ADDRESSLISTNUM	(1024 * 1024 * 1)		// �X���C�h�����̍ő�T�C�Y
#define MAX_POSITION		(1 << 24)				// �Q�Ɖ\�ȍő告�΃A�h���X( 16MB )

// ���k���ԒZ�k�p���X�g
typedef struct LZ_LIST
{
	LZ_LIST *next, *prev ;
	DWORD address ;
} LZ_LIST ;

// �f�[�^�����k����( �߂�l:���k��̃f�[�^�T�C�Y )
extern	int	DXA_Encode( void *Src, DWORD SrcSize, void *Dest )
{
	int dstsize ;
	int    bonus,    conbo,    conbosize,    address,    addresssize ;
	int maxbonus, maxconbo, maxconbosize, maxaddress, maxaddresssize ;
	BYTE keycode, *srcp, *destp, *dp, *sp, *sp2, *sp1 ;
	DWORD srcaddress, nextprintaddress, code ;
	int j ;
	DWORD i, m ;
	DWORD maxlistnum, maxlistnummask, listaddp ;
	DWORD sublistnum, sublistmaxnum ;
	LZ_LIST *listbuf, *listtemp, *list, *newlist ;
	BYTE *listfirsttable, *usesublistflagtable, *sublistbuf ;
	
	// �T�u���X�g�̃T�C�Y�����߂�
	{
			 if( SrcSize < 100 * 1024 )			sublistmaxnum = 1 ;
		else if( SrcSize < 3 * 1024 * 1024 )	sublistmaxnum = MAX_SUBLISTNUM / 3 ;
		else									sublistmaxnum = MAX_SUBLISTNUM ;
	}

	// ���X�g�̃T�C�Y�����߂�
	{
		maxlistnum = MAX_ADDRESSLISTNUM ;
		if( maxlistnum > SrcSize )
		{
			while( ( maxlistnum >> 1 ) > 0x100 && ( maxlistnum >> 1 ) > SrcSize )
				maxlistnum >>= 1 ;
		}
		maxlistnummask = maxlistnum - 1 ;
	}

	// �������̊m��
	usesublistflagtable   = (BYTE *)DXALLOC(
		sizeof( DWORD_PTR )	* 65536 +					// ���C�����X�g�̐擪�I�u�W�F�N�g�p�̈�
		sizeof( LZ_LIST   )	* maxlistnum +				// ���C�����X�g�p�̈�
		sizeof( BYTE      )	* 65536 +					// �T�u���X�g���g�p���Ă��邩�t���O�p�̈�
		sizeof( DWORD_PTR )	* 256 * sublistmaxnum ) ;	// �T�u���X�g�p�̈�
		
	// �A�h���X�̃Z�b�g
	listfirsttable =     usesublistflagtable + sizeof( BYTE      ) * 65536 ;
	sublistbuf     =          listfirsttable + sizeof( DWORD_PTR ) * 65536 ;
	listbuf        = (LZ_LIST *)( sublistbuf + sizeof( DWORD_PTR ) * 256 * sublistmaxnum ) ;
	
	// ������
	_MEMSET( usesublistflagtable, 0, sizeof( BYTE      ) * 65536               ) ;
	_MEMSET(          sublistbuf, 0, sizeof( DWORD_PTR ) * 256 * sublistmaxnum ) ;
	_MEMSET(      listfirsttable, 0, sizeof( DWORD_PTR ) * 65536               ) ;
	list = listbuf ;
	for( i = maxlistnum / 8 ; i ; i --, list += 8 )
	{
		list[0].address =
		list[1].address =
		list[2].address =
		list[3].address =
		list[4].address =
		list[5].address =
		list[6].address =
		list[7].address = 0xffffffff ;
	}

	srcp  = (BYTE *)Src ;
	destp = (BYTE *)Dest ;

	// ���k���f�[�^�̒��ň�ԏo���p�x���Ⴂ�o�C�g�R�[�h����������
	{
		DWORD qnum, table[256], mincode ;

		for( i = 0 ; i < 256 ; i ++ )
			table[i] = 0 ;
		
		sp   = srcp ;
		qnum = SrcSize / 8 ;
		i    = qnum * 8 ;
		for( ; qnum ; qnum --, sp += 8 )
		{
			table[sp[0]] ++ ;
			table[sp[1]] ++ ;
			table[sp[2]] ++ ;
			table[sp[3]] ++ ;
			table[sp[4]] ++ ;
			table[sp[5]] ++ ;
			table[sp[6]] ++ ;
			table[sp[7]] ++ ;
		}
		for( ; i < SrcSize ; i ++, sp ++ )
			table[*sp] ++ ;
			
		keycode = 0 ;
		mincode = table[0] ;
		for( i = 1 ; i < 256 ; i ++ )
		{
			if( mincode < table[i] ) continue ;
			mincode = table[i] ;
			keycode = (BYTE)i ;
		}
	}

	// ���k���̃T�C�Y���Z�b�g
	((DWORD *)destp)[0] = SrcSize ;

	// �L�[�R�[�h���Z�b�g
	destp[8] = keycode ;

	// ���k����
	dp               = destp + 9 ;
	sp               = srcp ;
	srcaddress       = 0 ;
	dstsize          = 0 ;
	listaddp         = 0 ;
	sublistnum       = 0 ;
	nextprintaddress = 1024 * 100 ;
	while( srcaddress < SrcSize )
	{
		// �c��T�C�Y���Œሳ�k�T�C�Y�ȉ��̏ꍇ�͈��k���������Ȃ�
		if( srcaddress + MIN_COMPRESS >= SrcSize ) goto NOENCODE ;

		// ���X�g���擾
		code = *((WORD *)sp) ;
		list = (LZ_LIST *)( listfirsttable + code * sizeof( DWORD_PTR ) ) ;
		if( usesublistflagtable[code] == 1 )
		{
			list = (LZ_LIST *)( (DWORD_PTR *)list->next + sp[2] ) ;
		}
		else
		{
			if( sublistnum < sublistmaxnum )
			{
				list->next = (LZ_LIST *)( sublistbuf + sizeof( DWORD_PTR ) * 256 * sublistnum ) ;
				list       = (LZ_LIST *)( (DWORD_PTR *)list->next + sp[2] ) ;
			
				usesublistflagtable[code] = 1 ;
				sublistnum ++ ;
			}
		}

		// ��Ԉ�v���̒����R�[�h��T��
		maxconbo   = -1 ;
		maxaddress = -1 ;
		maxbonus   = -1 ;
		for( m = 0, listtemp = list->next ; /*m < MAX_SEARCHLISTNUM &&*/ listtemp != NULL ; listtemp = listtemp->next, m ++ )
		{
			address = srcaddress - listtemp->address ;
			if( address >= MAX_POSITION )
			{
				if( listtemp->prev ) listtemp->prev->next = listtemp->next ;
				if( listtemp->next ) listtemp->next->prev = listtemp->prev ;
				listtemp->address = 0xffffffff ;
				continue ;
			}
			
			sp2 = &sp[-address] ;
			sp1 = sp ;
			if( srcaddress + MAX_COPYSIZE < SrcSize )
			{
				conbo = MAX_COPYSIZE / 4 ;
				while( conbo && *((DWORD *)sp2) == *((DWORD *)sp1) )
				{
					sp2 += 4 ;
					sp1 += 4 ;
					conbo -- ;
				}
				conbo = MAX_COPYSIZE - ( MAX_COPYSIZE / 4 - conbo ) * 4 ;

				while( conbo && *sp2 == *sp1 )
				{
					sp2 ++ ;
					sp1 ++ ;
					conbo -- ;
				}
				conbo = MAX_COPYSIZE - conbo ;
			}
			else
			{
				for( conbo = 0 ;
						conbo < MAX_COPYSIZE &&
						conbo + srcaddress < SrcSize &&
						sp[conbo - address] == sp[conbo] ;
							conbo ++ ){}
			}

			if( conbo >= 4 )
			{
				conbosize   = ( conbo - MIN_COMPRESS ) < 0x20 ? 0 : 1 ;
				addresssize = address < 0x100 ? 0 : ( address < 0x10000 ? 1 : 2 ) ;
				bonus       = conbo - ( 3 + conbosize + addresssize ) ;

				if( bonus > maxbonus )
				{
					maxconbo       = conbo ;
					maxaddress     = address ;
					maxaddresssize = addresssize ;
					maxconbosize   = conbosize ;
					maxbonus       = bonus ;
				}
			}
		}

		// ���X�g�ɓo�^
		newlist = &listbuf[listaddp] ;
		if( newlist->address != 0xffffffff )
		{
			if( newlist->prev ) newlist->prev->next = newlist->next ;
			if( newlist->next ) newlist->next->prev = newlist->prev ;
			newlist->address = 0xffffffff ;
		}
		newlist->address = srcaddress ;
		newlist->prev    = list ;
		newlist->next    = list->next ;
		if( list->next != NULL ) list->next->prev = newlist ;
		list->next       = newlist ;
		listaddp         = ( listaddp + 1 ) & maxlistnummask ;

		// ��v�R�[�h��������Ȃ�������񈳏k�R�[�h�Ƃ��ďo��
		if( maxconbo == -1 )
		{
NOENCODE:
			// �L�[�R�[�h�������ꍇ�͂Q��A���ŏo�͂���
			if( *sp == keycode )
			{
				if( destp != NULL )
				{
					dp[0]  =
					dp[1]  = keycode ;
					dp += 2 ;
				}
				dstsize += 2 ;
			}
			else
			{
				if( destp != NULL )
				{
					*dp = *sp ;
					dp ++ ;
				}
				dstsize ++ ;
			}
			sp ++ ;
			srcaddress ++ ;
		}
		else
		{
			// ���������ꍇ�͌������ʒu�ƒ������o�͂���
			
			// �L�[�R�[�h�ƌ������ʒu�ƒ������o��
			if( destp != NULL )
			{
				// �L�[�R�[�h�̏o��
				*dp++ = keycode ;

				// �o�͂���A�����͍Œ� MIN_COMPRESS ���邱�Ƃ��O��Ȃ̂� - MIN_COMPRESS �������̂��o�͂���
				maxconbo -= MIN_COMPRESS ;

				// �A�����O�`�S�r�b�g�ƘA�����A���΃A�h���X�̃r�b�g�����o��
				*dp = (BYTE)( ( ( maxconbo & 0x1f ) << 3 ) | ( maxconbosize << 2 ) | maxaddresssize ) ;

				// �L�[�R�[�h�̘A���̓L�[�R�[�h�ƒl�̓������񈳏k�R�[�h��
				// ���f���邽�߁A�L�[�R�[�h�̒l�ȏ�̏ꍇ�͒l���{�P����
				if( *dp >= keycode ) dp[0] += 1 ;
				dp ++ ;

				// �A�����T�`�P�Q�r�b�g���o��
				if( maxconbosize == 1 )
					*dp++ = (BYTE)( ( maxconbo >> 5 ) & 0xff ) ;

				// maxconbo �͂܂��g������ - MIN_COMPRESS ��������߂�
				maxconbo += MIN_COMPRESS ;

				// �o�͂��鑊�΃A�h���X�͂O��( ���݂̃A�h���X�|�P )��}���̂ŁA�|�P�������̂��o�͂���
				maxaddress -- ;

				// ���΃A�h���X���o��
				*dp++ = (BYTE)( maxaddress ) ;
				if( maxaddresssize > 0 )
				{
					*dp++ = (BYTE)( maxaddress >> 8 ) ;
					if( maxaddresssize == 2 )
						*dp++ = (BYTE)( maxaddress >> 16 ) ;
				}
			}
			
			// �o�̓T�C�Y�����Z
			dstsize += 3 + maxaddresssize + maxconbosize ;
			
			// ���X�g�ɏ���ǉ�
			if( srcaddress + maxconbo < SrcSize )
			{
				sp2 = &sp[1] ;
				for( j = 1 ; j < maxconbo && (DWORD_PTR)&sp2[2] - (DWORD_PTR)srcp < SrcSize ; j ++, sp2 ++ )
				{
					code = *((WORD *)sp2) ;
					list = (LZ_LIST *)( listfirsttable + code * sizeof( DWORD_PTR ) ) ;
					if( usesublistflagtable[code] == 1 )
					{
						list = (LZ_LIST *)( (DWORD_PTR *)list->next + sp2[2] ) ;
					}
					else
					{
						if( sublistnum < sublistmaxnum )
						{
							list->next = (LZ_LIST *)( sublistbuf + sizeof( DWORD_PTR ) * 256 * sublistnum ) ;
							list       = (LZ_LIST *)( (DWORD_PTR *)list->next + sp2[2] ) ;
						
							usesublistflagtable[code] = 1 ;
							sublistnum ++ ;
						}
					}

					newlist = &listbuf[listaddp] ;
					if( newlist->address != 0xffffffff )
					{
						if( newlist->prev ) newlist->prev->next = newlist->next ;
						if( newlist->next ) newlist->next->prev = newlist->prev ;
						newlist->address = 0xffffffff ;
					}
					newlist->address = srcaddress + j ;
					newlist->prev = list ;
					newlist->next = list->next ;
					if( list->next != NULL ) list->next->prev = newlist ;
					list->next = newlist ;
					listaddp = ( listaddp + 1 ) & maxlistnummask ;
				}
			}
			
			sp         += maxconbo ;
			srcaddress += maxconbo ;
		}
	}

	// ���k��̃f�[�^�T�C�Y��ۑ�����
	*((DWORD *)&destp[4]) = dstsize + 9 ;

	// �m�ۂ����������̉��
	DXFREE( usesublistflagtable ) ;

	// �f�[�^�̃T�C�Y��Ԃ�
	return dstsize + 9 ;
}

extern int DXA_Decode( void *Src, void *Dest )
{
	DWORD srcsize, destsize, code, indexsize, keycode, conbo, index = 0 ;
	BYTE *srcp, *destp, *dp, *sp ;

	destp = (BYTE *)Dest ;
	srcp  = (BYTE *)Src ;
	
	// �𓀌�̃f�[�^�T�C�Y�𓾂�
	destsize = *((DWORD *)&srcp[0]) ;

	// ���k�f�[�^�̃T�C�Y�𓾂�
	srcsize = *((DWORD *)&srcp[4]) - 9 ;

	// �L�[�R�[�h
	keycode = srcp[8] ;
	
	// �o�͐悪�Ȃ��ꍇ�̓T�C�Y�����Ԃ�
	if( Dest == NULL )
		return destsize ;
	
	// �W�J�J�n
	sp  = srcp + 9 ;
	dp  = destp ;
	while( srcsize )
	{
		// �L�[�R�[�h�������ŏ����𕪊�
		if( sp[0] != keycode )
		{
			// �񈳏k�R�[�h�̏ꍇ�͂��̂܂܏o��
			*dp = *sp ;
			dp      ++ ;
			sp      ++ ;
			srcsize -- ;
			continue ;
		}
	
		// �L�[�R�[�h���A�����Ă����ꍇ�̓L�[�R�[�h���̂��o��
		if( sp[1] == keycode )
		{
			*dp = (BYTE)keycode ;
			dp      ++ ;
			sp      += 2 ;
			srcsize -= 2 ;
			
			continue ;
		}

		// ���o�C�g�𓾂�
		code = sp[1] ;

		// �����L�[�R�[�h�����傫�Ȓl�������ꍇ�̓L�[�R�[�h
		// �Ƃ̃o�b�e�B���O�h�~�ׂ̈Ɂ{�P���Ă���̂Ł|�P����
		if( code > keycode ) code -- ;

		sp      += 2 ;
		srcsize -= 2 ;

		// �A�������擾����
		conbo = code >> 3 ;
		if( code & ( 0x1 << 2 ) )
		{
			conbo |= *sp << 5 ;
			sp      ++ ;
			srcsize -- ;
		}
		conbo += MIN_COMPRESS ;	// �ۑ����Ɍ��Z�����ŏ����k�o�C�g���𑫂�

		// �Q�Ƒ��΃A�h���X���擾����
		indexsize = code & 0x3 ;
		switch( indexsize )
		{
		case 0 :
			index = *sp ;
			sp      ++ ;
			srcsize -- ;
			break ;
			
		case 1 :
			index = *((WORD *)sp) ;
			sp      += 2 ;
			srcsize -= 2 ;
			break ;
			
		case 2 :
			index = *((WORD *)sp) | ( sp[2] << 16 ) ;
			sp      += 3 ;
			srcsize -= 3 ;
			break ;
		}
		index ++ ;		// �ۑ����Ɂ|�P���Ă���̂Ł{�P����

		// �W�J
		if( index < conbo )
		{
			DWORD num ;

			num  = index ;
			while( conbo > num )
			{
				_MEMCPY( dp, dp - num, num ) ;
				dp    += num ;
				conbo -= num ;
				num   += num ;
			}
			if( conbo != 0 )
			{
				_MEMCPY( dp, dp - num, conbo ) ;
				dp += conbo ;
			}
		}
		else
		{
			_MEMCPY( dp, dp - index, conbo ) ;
			dp += conbo ;
		}
	}

	// �𓀌�̃T�C�Y��Ԃ�
	return (int)destsize ;
}

}
