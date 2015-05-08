// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		�\�t�g�E�F�A�ň����摜�v���O����
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h ------------------------------------------------------------------
#include "DxSoftImage.h"

#ifndef DX_NON_SOFTIMAGE

#include "DxStatic.h"
#include "DxLib.h"
#include "DxASyncLoad.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxFile.h"
#include "DxSystem.h"
#include "DxLog.h"

namespace DxLib
{

// �}�N����` --------------------------------------------------------------------

// �\���̒�` --------------------------------------------------------------------

// �������ϐ��錾 --------------------------------------------------------------

SOFTIMAGEMANAGE SoftImageManage ;

// �֐��v���g�^�C�v�錾-----------------------------------------------------------

// �v���O���� --------------------------------------------------------------------

// �������A��n��

// ��{�C���[�W�Ǘ����̏�����
extern int InitializeSoftImageManage( void )
{
	// ���ɏ���������Ă����牽�����Ȃ�
	if( SoftImageManage.InitializeFlag == TRUE )
		return -1 ;

	// �\�t�g�C���[�W�n���h���Ǘ����̏�����
	InitializeHandleManage( DX_HANDLETYPE_SOFTIMAGE, sizeof( SOFTIMAGE ), MAX_SOFTIMAGE_NUM, InitializeSoftImageHandle, TerminateSoftImageHandle, DXSTRING( _T( "�\�t�g�C���[�W" ) ) ) ;

	// �������t���O�𗧂Ă�
	SoftImageManage.InitializeFlag = TRUE ;

	// �I��
	return 0 ;
}

// ��{�C���[�W�Ǘ����̌�n��
extern int TerminateSoftImageManage( void )
{
	// ���Ɍ�n������Ă����牽�����Ȃ�
	if( SoftImageManage.InitializeFlag == FALSE )
		return -1 ;

	// �������t���O��|��
	SoftImageManage.InitializeFlag = FALSE ;

	// �\�t�g�C���[�W�n���h���Ǘ����̌�n��
	TerminateHandleManage( DX_HANDLETYPE_SOFTIMAGE ) ;

	// �I��
	return 0 ;
}










// �\�t�g�E�G�A�C���[�W�n���h���̏�����
extern int InitializeSoftImageHandle( HANDLEINFO * )
{
	// ���ɉ������Ȃ�
	return 0 ;
}

// �\�t�g�E�G�A�C���[�W�n���h���̌�n��
extern int TerminateSoftImageHandle( HANDLEINFO *HandleInfo )
{
	SOFTIMAGE *SoftImg = ( SOFTIMAGE * )HandleInfo ;

	// �摜�f�[�^�̉��
	NS_ReleaseBaseImage( &SoftImg->BaseImage ) ;

	// �I��
	return 0 ;
}

// �\�t�g�E�G�A�ň����C���[�W��S�ĉ������
extern int NS_InitSoftImage( void )
{
	return AllHandleSub( DX_HANDLETYPE_SOFTIMAGE ) ;
}

// LoadSoftImage �̎������֐�
static int LoadSoftImage_Static(
	int SIHandle,
	const TCHAR *FileName,
	int ASyncThread
)
{
	SOFTIMAGE *SoftImg ;

	// �V�����\�t�g�C���[�W�n���h���̎擾
	if( ASyncThread )
	{
		if( SFTIMGCHK_ASYNC( SIHandle, SoftImg ) )
			return -1 ;
	}
	else
	{
		if( SFTIMGCHK( SIHandle, SoftImg ) )
			return -1 ;
	}

	// CreateBaseImageToFile �Ńt�@�C������ǂݍ���
	if( NS_CreateBaseImageToFile( FileName, &SoftImg->BaseImage, FALSE ) == -1 )
		return -1 ;

	// ����I��
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// LoadSoftImage �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void LoadSoftImage_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int SIHandle ;
	const TCHAR *FileName ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	SIHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = LoadSoftImage_Static( SIHandle, FileName, TRUE ) ;

	DecASyncLoadCount( SIHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteSoftImage( SIHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// LoadSoftImage �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int LoadSoftImage_UseGParam(
	const TCHAR *FileName,
	int ASyncLoadFlag
)
{
	int SIHandle ;

	CheckActiveState() ;

	// �n���h���̍쐬
	SIHandle = AddHandle( DX_HANDLETYPE_SOFTIMAGE ) ;
	if( SIHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		TCHAR FullPath[ 1024 ] ;
		int Addr ;

		ConvertFullPathT_( FileName, FullPath ) ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, SIHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = LoadSoftImage_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SIHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( SIHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadSoftImage_Static( SIHandle, FileName, FALSE ) < 0 )
			goto ERR ;
	}

	// �I��
	return SIHandle ;

ERR :
	NS_DeleteSoftImage( SIHandle ) ;

	return -1 ;
}

// �\�t�g�E�G�A�ň����C���[�W�̓ǂݍ���( -1:�G���[  -1�ȊO:�C���[�W�n���h�� )
extern	int		NS_LoadSoftImage( const TCHAR *FileName )
{
	return LoadSoftImage_UseGParam( FileName, GetASyncLoadFlag() ) ;
}

// LoadSoftImageToMem �̎������֐�
static int LoadSoftImageToMem_Static(
	int SIHandle,
	const void *FileImage,
	int FileImageSize,
	int ASyncThread
)
{
	SOFTIMAGE *SoftImg ;

	// �V�����\�t�g�C���[�W�n���h���̎擾
	if( ASyncThread )
	{
		if( SFTIMGCHK_ASYNC( SIHandle, SoftImg ) )
			return -1 ;
	}
	else
	{
		if( SFTIMGCHK( SIHandle, SoftImg ) )
			return -1 ;
	}

	// CreateBaseImageToMem �Ń���������ǂݍ���
	if( NS_CreateBaseImageToMem( FileImage, FileImageSize, &SoftImg->BaseImage, FALSE ) == -1 )
		return -1 ;

	// ����I��
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// LoadSoftImageToMem �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void LoadSoftImageToMem_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int SIHandle ;
	const void *FileImage ;
	int FileImageSize ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	SIHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileImage = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	FileImageSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = LoadSoftImageToMem_Static( SIHandle, FileImage, FileImageSize, TRUE ) ;

	DecASyncLoadCount( SIHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteSoftImage( SIHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD


// LoadSoftImageToMem �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int LoadSoftImageToMem_UseGParam(
	const void *FileImage,
	int FileImageSize,
	int ASyncLoadFlag
)
{
	int SIHandle ;

	CheckActiveState() ;

	// �n���h���̍쐬
	SIHandle = AddHandle( DX_HANDLETYPE_SOFTIMAGE ) ;
	if( SIHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, SIHandle ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, FileImage ) ;
		AddASyncLoadParamInt( NULL, &Addr, FileImageSize ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = LoadSoftImageToMem_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SIHandle ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, FileImage ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileImageSize ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( SIHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadSoftImageToMem_Static( SIHandle, FileImage, FileImageSize, FALSE ) < 0 )
			goto ERR ;
	}

	// �I��
	return SIHandle ;

ERR :
	NS_DeleteSoftImage( SIHandle ) ;

	return -1 ;
}

// �\�t�g�E�G�A�ň����C���[�W�̃���������̓ǂݍ���( -1:�G���[  -1�ȊO:�C���[�W�n���h�� )
extern	int		NS_LoadSoftImageToMem( const void *FileImage, int FileImageSize )
{
	return LoadSoftImageToMem_UseGParam( FileImage, FileImageSize, GetASyncLoadFlag() ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̍쐬( -1:�G���[  -1�ȊO:�C���[�W�n���h�� )
extern	int		NS_MakeSoftImage( int SizeX, int SizeY )
{
	SOFTIMAGE *SoftImg ;
	int NewHandle ;

	// �V�����\�t�g�C���[�W�n���h���̎擾
	NewHandle = AddHandle( DX_HANDLETYPE_SOFTIMAGE ) ;
	if( NewHandle == -1 ) return -1 ;
	if( SFTIMGCHK( NewHandle, SoftImg ) )
		return -1 ;

	// CreateARGB8ColorBaseImage ���g�p
	if( NS_CreateARGB8ColorBaseImage( SizeX, SizeY, &SoftImg->BaseImage ) == -1 )
	{
		SubHandle( NewHandle ) ;
		return -1 ;
	}

	// ����������n���h����Ԃ�
	return NewHandle ;
}

// �\�t�g�E�G�A�ň����C���[�W�̍쐬( RGBA8 �J���[ )
extern	int		NS_MakeARGB8ColorSoftImage( int SizeX, int SizeY )
{
	// NS_MakeSoftImage �Ɠ���
	return NS_MakeSoftImage( SizeX, SizeY ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̍쐬( XRGB8 �J���[ )
extern	int		NS_MakeXRGB8ColorSoftImage( int SizeX, int SizeY )
{
	SOFTIMAGE *SoftImg ;
	int NewHandle ;

	// �V�����\�t�g�C���[�W�n���h���̎擾
	NewHandle = AddHandle( DX_HANDLETYPE_SOFTIMAGE ) ;
	if( NewHandle == -1 ) return -1 ;
	if( SFTIMGCHK( NewHandle, SoftImg ) )
		return -1 ;

	// CreateXRGB8ColorBaseImage ���g�p
	if( NS_CreateXRGB8ColorBaseImage( SizeX, SizeY, &SoftImg->BaseImage ) == -1 )
	{
		SubHandle( NewHandle ) ;
		return -1 ;
	}

	// ����������n���h����Ԃ�
	return NewHandle ;
}

// �\�t�g�E�G�A�ň����C���[�W�̍쐬( ARGB4 �J���[ )
extern	int		NS_MakeARGB4ColorSoftImage( int SizeX, int SizeY )
{
	SOFTIMAGE *SoftImg ;
	int NewHandle ;

	// �V�����\�t�g�C���[�W�n���h���̎擾
	NewHandle = AddHandle( DX_HANDLETYPE_SOFTIMAGE ) ;
	if( NewHandle == -1 ) return -1 ;
	if( SFTIMGCHK( NewHandle, SoftImg ) )
		return -1 ;

	// CreateARGB4ColorBaseImage ���g�p
	if( NS_CreateARGB4ColorBaseImage( SizeX, SizeY, &SoftImg->BaseImage ) == -1 )
	{
		SubHandle( NewHandle ) ;
		return -1 ;
	}

	// ����������n���h����Ԃ�
	return NewHandle ;
}

// �\�t�g�E�G�A�ň����C���[�W�̍쐬( RGB8 �J���[ )
extern	int		NS_MakeRGB8ColorSoftImage( int SizeX, int SizeY )
{
	SOFTIMAGE *SoftImg ;
	int NewHandle ;

	// �V�����\�t�g�C���[�W�n���h���̎擾
	NewHandle = AddHandle( DX_HANDLETYPE_SOFTIMAGE ) ;
	if( NewHandle == -1 ) return -1 ;
	if( SFTIMGCHK( NewHandle, SoftImg ) )
		return -1 ;

	// CreateRGB8ColorBaseImage ���g�p
	if( NS_CreateRGB8ColorBaseImage( SizeX, SizeY, &SoftImg->BaseImage ) == -1 )
	{
		SubHandle( NewHandle ) ;
		return -1 ;
	}

	// ����������n���h����Ԃ�
	return NewHandle ;
}

// �\�t�g�E�G�A�ň����C���[�W�̍쐬( �p���b�g�Q�T�U�F �J���[ )
extern	int		NS_MakePAL8ColorSoftImage( int SizeX, int SizeY )
{
	SOFTIMAGE *SoftImg ;
	int NewHandle ;

	// �V�����\�t�g�C���[�W�n���h���̎擾
	NewHandle = AddHandle( DX_HANDLETYPE_SOFTIMAGE ) ;
	if( NewHandle == -1 ) return -1 ;
	if( SFTIMGCHK( NewHandle, SoftImg ) )
		return -1 ;

	// CreatePAL8ColorBaseImage ���g�p
	if( NS_CreatePAL8ColorBaseImage( SizeX, SizeY, &SoftImg->BaseImage ) == -1 )
	{
		SubHandle( NewHandle ) ;
		return -1 ;
	}

	// ����������n���h����Ԃ�
	return NewHandle ;
}

// �\�t�g�E�G�A�ň����C���[�W�̉��
extern	int		NS_DeleteSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;
	
	// �n���h���̉��
	SubHandle( SIHandle ) ; 

	// �I��
	return 0 ;
}

// �\�t�g�E�G�A�ň����C���[�W�̃T�C�Y���擾����
extern	int		NS_GetSoftImageSize( int SIHandle, int *Width, int *Height )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	if( Width  ) *Width  = SoftImg->BaseImage.Width ;
	if( Height ) *Height = SoftImg->BaseImage.Height ;

	// �I��
	return 0 ;
}

// �\�t�g�E�G�A�ň����C���[�W���p���b�g�摜���ǂ������擾����( TRUE:�p���b�g�摜  FALSE:�p���b�g�摜����Ȃ� )
extern	int		NS_CheckPaletteSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return SoftImg->BaseImage.ColorData.PixelByte == 1 ? TRUE : FALSE ;
}

// �\�t�g�E�G�A�ň����C���[�W�̃t�H�[�}�b�g�Ƀ��v�f�����邩�ǂ������擾����( TRUE:����  FALSE:�Ȃ� )
extern	int		NS_CheckAlphaSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return SoftImg->BaseImage.ColorData.AlphaMask != 0 && SoftImg->BaseImage.ColorData.PixelByte >= 2 ? TRUE : FALSE ;
}

// �`��Ώۂ̉�ʂ���w��̈�̃O���t�B�b�N���\�t�g�E�G�A�ň����C���[�W�ɓ]������
extern	int		NS_GetDrawScreenSoftImage( int x1, int y1, int x2, int y2, int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_GetDrawScreenBaseImage( x1, y1, x2, y2, &SoftImg->BaseImage ) ;
}

// �`��Ώۂ̉�ʂ���w��̈���\�t�g�E�G�A�C���[�W�n���h���ɓ]������( �]������W�w��� )
extern	int		NS_GetDrawScreenSoftImageDestPos( int x1, int y1, int x2, int y2, int SIHandle, int DestX, int DestY )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_GetDrawScreenBaseImageDestPos( x1, y1, x2, y2, &SoftImg->BaseImage, DestX, DestY ) ;
}

#ifdef __WINDOWS__

// �\�t�g�E�G�A�ň����C���[�W���g�p���� UpdateLayerdWindow ���s��
extern	int		NS_UpdateLayerdWindowForSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_UpdateLayerdWindowForBaseImage( &SoftImg->BaseImage ) ;
}

// �\�t�g�E�G�A�C���[�W�n���h���̎w��͈̔͂��g�p���� UpdateLayerdWindow ���s��
extern	int		NS_UpdateLayerdWindowForSoftImageRect( int SIHandle, int x1, int y1, int x2, int y2 )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_UpdateLayerdWindowForBaseImageRect( &SoftImg->BaseImage, x1, y1, x2, y2 ) ;
}

// ��Z�ς݃A���t�@�̃\�t�g�E�G�A�ň����C���[�W���g�p���� UpdateLayerdWindow ���s��
extern	int		NS_UpdateLayerdWindowForPremultipliedAlphaSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_UpdateLayerdWindowForPremultipliedAlphaBaseImage( &SoftImg->BaseImage ) ;
}

// ��Z�ς݃A���t�@�̃\�t�g�E�G�A�C���[�W�n���h���̎w��͈̔͂��g�p���� UpdateLayerdWindow ���s��
extern	int		NS_UpdateLayerdWindowForPremultipliedAlphaSoftImageRect( int SIHandle, int x1, int y1, int x2, int y2 )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_UpdateLayerdWindowForPremultipliedAlphaBaseImageRect( &SoftImg->BaseImage, x1, y1, x2, y2 ) ;
}

#endif // __WINDOWS__

// �\�t�g�E�G�A�ň����C���[�W���w��F�œh��Ԃ�
extern	int		NS_FillSoftImage( int SIHandle, int r, int g, int b, int a )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_FillBaseImage( &SoftImg->BaseImage, r, g, b, a ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w��̗̈���O�N���A����
extern	int		NS_ClearRectSoftImage( int SIHandle, int x, int y, int w, int h )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ClearRectBaseImage( &SoftImg->BaseImage, x, y, w, h ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̃p���b�g���擾����
extern	int		NS_GetPaletteSoftImage( int SIHandle, int PaletteNo, int *r, int *g, int *b, int *a )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_GetPaletteBaseImage( &SoftImg->BaseImage, PaletteNo, r, g, b, a ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̃p���b�g���Z�b�g����
extern	int		NS_SetPaletteSoftImage( int SIHandle, int PaletteNo, int  r, int  g, int  b, int  a )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_SetPaletteBaseImage( &SoftImg->BaseImage, PaletteNo, r, g, b, a ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�Ƀh�b�g��`�悷��
extern	int		NS_DrawPixelSoftImage( int SIHandle, int x, int y, int  r, int  g, int  b, int  a )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_SetPixelBaseImage( &SoftImg->BaseImage, x, y, r, g, b, a ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�Ƀh�b�g��`�悷��
extern	void	NS_DrawPixelSoftImage_Unsafe_XRGB8( int SIHandle, int x, int y, int  r, int  g, int  b )
{
	BASEIMAGE *BaseImage = &( ( SOFTIMAGE * )HandleManageArray[ DX_HANDLETYPE_SOFTIMAGE ].Handle[ SIHandle & DX_HANDLEINDEX_MASK ] )->BaseImage ;

	*((DWORD *)((BYTE *)BaseImage->GraphData + BaseImage->Pitch * y + x * 4)) = ( DWORD )( ( r << 16 ) | ( g << 8 ) | b ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�Ƀh�b�g��`�悷��
extern	void	NS_DrawPixelSoftImage_Unsafe_ARGB8( int SIHandle, int x, int y, int  r, int  g, int  b, int a )
{
	BASEIMAGE *BaseImage = &( ( SOFTIMAGE * )HandleManageArray[ DX_HANDLETYPE_SOFTIMAGE ].Handle[ SIHandle & DX_HANDLEINDEX_MASK ] )->BaseImage ;

	*((DWORD *)((BYTE *)BaseImage->GraphData + BaseImage->Pitch * y + x * 4)) = ( DWORD )( ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�Ƀh�b�g��`�悷��(�p���b�g�摜�p�A�L���l�͂O�`�Q�T�T)
extern	int		NS_DrawPixelPalCodeSoftImage( int SIHandle, int x, int y, int palNo )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_SetPixelPalCodeBaseImage( &SoftImg->BaseImage,  x,  y,  palNo ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�̐F�R�[�h���擾����(�p���b�g�摜�p�A�߂�l�͂O�`�Q�T�T)
extern	int		NS_GetPixelPalCodeSoftImage( int SIHandle, int x, int y )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_GetPixelPalCodeBaseImage( &SoftImg->BaseImage,  x,  y ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎��C���[�W���i�[����Ă��郁�����̈�̐擪�A�h���X���擾����
extern void *NS_GetImageAddressSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return NULL ;

	return SoftImg->BaseImage.GraphData ;
}

// �\�t�g�E�G�A�C���[�W�n���h���̃������Ɋi�[����Ă���摜�f�[�^��1���C���ӂ�̃o�C�g�����擾����
extern int NS_GetPitchSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return NULL ;

	return SoftImg->BaseImage.Pitch ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�̐F���擾����
extern	int		NS_GetPixelSoftImage(  int SIHandle, int x, int y, int *r, int *g, int *b, int *a )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_GetPixelBaseImage( &SoftImg->BaseImage, x, y, r, g, b, a ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�̐F���擾����
extern	void		NS_GetPixelSoftImage_Unsafe_XRGB8(  int SIHandle, int x, int y, int *r, int *g, int *b )
{
	BASEIMAGE *BaseImage = &( ( SOFTIMAGE * )HandleManageArray[ DX_HANDLETYPE_SOFTIMAGE ].Handle[ SIHandle & DX_HANDLEINDEX_MASK ] )->BaseImage ;
	DWORD Color ;

	Color = *((DWORD *)((BYTE *)BaseImage->GraphData + BaseImage->Pitch * y + x * 4))  ;
	*r = ( Color >> 16 ) & 0xff ;
	*g = ( Color >>  8 ) & 0xff ;
	*b =   Color         & 0xff ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�̐F���擾����
extern	void		NS_GetPixelSoftImage_Unsafe_ARGB8(  int SIHandle, int x, int y, int *r, int *g, int *b, int *a )
{
	BASEIMAGE *BaseImage = &( ( SOFTIMAGE * )HandleManageArray[ DX_HANDLETYPE_SOFTIMAGE ].Handle[ SIHandle & DX_HANDLEINDEX_MASK ] )->BaseImage ;
	DWORD Color ;

	Color = *((DWORD *)((BYTE *)BaseImage->GraphData + BaseImage->Pitch * y + x * 4))  ;
	*a = ( Color >> 24 ) & 0xff ;
	*r = ( Color >> 16 ) & 0xff ;
	*g = ( Color >>  8 ) & 0xff ;
	*b =   Color         & 0xff ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w����W�ɐ���`�悷��(�e�F�v�f�͂O�`�Q�T�T)
extern int NS_DrawLineSoftImage(  int SIHandle, int x1, int y1, int x2, int y2, int r, int g, int b, int a )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_DrawLineBaseImage( &SoftImg->BaseImage, x1, y1, x2, y2, r, g, b, a ) ;
}

// �\�t�g�E�G�A�ň����C���[�W��ʂ̃C���[�W��ɓ]������
extern	int		NS_BltSoftImage( int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int SrcSIHandle, int DestX, int DestY, int DestSIHandle )
{
	SOFTIMAGE *SrcSoftImg, *DestSoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SrcSIHandle,  SrcSoftImg ) ) 
		return -1 ;
	if( SFTIMGCHK( DestSIHandle, DestSoftImg ) )
		return -1 ;

	return NS_BltBaseImage( SrcX, SrcY, SrcSizeX, SrcSizeY, DestX, DestY, &SrcSoftImg->BaseImage, &DestSoftImg->BaseImage ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�𓧉ߐF�����t���œ]������
extern	int		NS_BltSoftImageWithTransColor( int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int SrcSIHandle, int DestX, int DestY, int DestSIHandle, int Tr, int Tg, int Tb, int Ta )
{
	SOFTIMAGE *SrcSoftImg, *DestSoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SrcSIHandle,  SrcSoftImg ) ) 
		return -1 ;
	if( SFTIMGCHK( DestSIHandle, DestSoftImg ) )
		return -1 ;

	return NS_BltBaseImageWithTransColor( SrcX, SrcY, SrcSizeX, SrcSizeY, DestX, DestY, &SrcSoftImg->BaseImage, &DestSoftImg->BaseImage, Tr, Tg, Tb, Ta ) ;
}

// �\�t�g�E�G�A�ň����C���[�W���A���t�@�l�̃u�����h���l��������œ]������( Opacity �͓����x : 0( ���S���� ) �` 255( ���S�s���� ) )( �o�͐悪 ARGB8 �`���ȊO�̏ꍇ�̓G���[�ɂȂ�܂� )
extern	int		NS_BltSoftImageWithAlphaBlend( int SrcX, int SrcY, int SrcSizeX, int SrcSizeY, int SrcSIHandle, int DestX, int DestY, int DestSIHandle, int Opacity )
{
	SOFTIMAGE *SrcSoftImg, *DestSoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SrcSIHandle,  SrcSoftImg ) ) 
		return -1 ;
	if( SFTIMGCHK( DestSIHandle, DestSoftImg ) )
		return -1 ;

	return NS_BltBaseImageWithAlphaBlend( SrcX, SrcY, SrcSizeX, SrcSizeY, DestX, DestY, &SrcSoftImg->BaseImage, &DestSoftImg->BaseImage, Opacity ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̍��E�𔽓]����
extern	int		NS_ReverseSoftImageH( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ReverseBaseImageH( &SoftImg->BaseImage ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̏㉺�𔽓]����
extern	int		NS_ReverseSoftImageV( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ReverseBaseImageV( &SoftImg->BaseImage ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̏㉺���E�𔽓]����
extern	int		NS_ReverseSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ReverseBaseImage( &SoftImg->BaseImage ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�Ɋ܂܂��s�N�Z���̃A���t�@�l���`�F�b�N����( �߂�l   0:�摜�ɃA���t�@����������  1:�摜�ɃA���t�@����������A���ׂčő�(255)�l  2:�摜�ɃA���t�@����������A���݂���A���t�@�l�͍ŏ�(0)�ƍő�(255)�̂݁@3:�摜�ɃA���t�@����������A�ŏ��ƍő�ȊO�̒��Ԃ̒l������ )  
extern	int		NS_CheckPixelAlphaSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_CheckPixelAlphaBaseImage( &SoftImg->BaseImage ) ;
}

#ifndef DX_NON_FONT

// �\�t�g�E�G�A�ň����C���[�W�ɕ�����C���[�W��]������
extern	int		NS_BltStringSoftImage( 
						int x, int y, const TCHAR *StrData,
						int DestSIHandle, int DestEdgeSIHandle,
						int VerticalFlag )
{
	SOFTIMAGE *SoftImg, *EdgeSoftImg = NULL ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( DestSIHandle, SoftImg ) )
		return -1 ;
	if( DestEdgeSIHandle != -1 )
	{
		if( SFTIMGCHK( DestEdgeSIHandle, EdgeSoftImg ) )
			return -1 ;
	}

	return NS_FontBaseImageBlt( x, y, StrData, &SoftImg->BaseImage, DestEdgeSIHandle != -1 ? &EdgeSoftImg->BaseImage : NULL, VerticalFlag ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�ɕ�����C���[�W��]������( �t�H���g�n���h���g�p�� )
extern	int		NS_BltStringSoftImageToHandle( 
						int x, int y, const TCHAR *StrData,
						int DestSIHandle, int DestEdgeSIHandle,
						int FontHandle, int VerticalFlag )
{
	SOFTIMAGE *SoftImg, *EdgeSoftImg = NULL ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( DestSIHandle, SoftImg ) )
		return -1 ;
	if( DestEdgeSIHandle != -1 )
	{
		if( SFTIMGCHK( DestEdgeSIHandle, EdgeSoftImg ) )
			return -1 ;
	}

	return NS_FontBaseImageBltToHandle( x, y, StrData, &SoftImg->BaseImage, DestEdgeSIHandle != -1 ? &EdgeSoftImg->BaseImage : NULL, FontHandle, VerticalFlag ) ;
}

#endif // DX_NON_FONT


// �\�t�g�E�G�A�ň����C���[�W����ʂɕ`�悷��
extern	int		NS_DrawSoftImage( int x, int y, int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_DrawBaseImage( x, y, &SoftImg->BaseImage ) ;
}

#ifndef DX_NON_SAVEFUNCTION

// �\�t�g�E�G�A�ň����C���[�W���a�l�o�摜�t�@�C���Ƃ��ĕۑ�����
extern	int		NS_SaveSoftImageToBmp( const TCHAR *FilePath, int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_SaveBaseImageToBmp( FilePath, &SoftImg->BaseImage ) ;
}

#ifndef DX_NON_PNGREAD
// �\�t�g�E�G�A�ň����C���[�W���o�m�f�摜�t�@�C���Ƃ��ĕۑ�����
extern	int		NS_SaveSoftImageToPng( const TCHAR *FilePath, int SIHandle, int CompressionLevel )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_SaveBaseImageToPng( FilePath, &SoftImg->BaseImage, CompressionLevel ) ;
}
#endif

#ifndef DX_NON_JPEGREAD
// �\�t�g�E�G�A�ň����C���[�W���i�o�d�f�摜�t�@�C���Ƃ��ĕۑ�����
extern	int		NS_SaveSoftImageToJpeg( const TCHAR *FilePath, int SIHandle, int Quality, int Sample2x1 )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_SaveBaseImageToJpeg( FilePath, &SoftImg->BaseImage, Quality, Sample2x1 ) ;
}
#endif

#endif // DX_NON_SAVEFUNCTION


}

#endif // DX_NON_SOFTIMAGE
