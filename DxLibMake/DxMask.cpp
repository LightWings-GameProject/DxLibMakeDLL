//-----------------------------------------------------------------------------
// 
// 		�c�w���C�u����		�}�X�N�f�[�^�Ǘ��v���O����
// 
//  	Ver 3.12a
// 
//-----------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

#include "DxCompileConfig.h"

#ifndef DX_NON_MASK

// �C���N���[�h----------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxMask.h"
#include "DxFont.h"
#include "DxUseCLib.h"
#include "DxFile.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxGraphicsBase.h"
#include "DxLog.h"
#include "Windows/DxWindow.h"
#include "DxGraphics.h"
#include "DxASyncLoad.h"

namespace DxLib
{

// �}�N����`------------------------------------------------------------------

// �}�X�N�n���h���̗L�����`�F�b�N
#define MASKHCHK( HAND, MPOINT )		HANDLECHK(       DX_HANDLETYPE_GMASK, HAND, *( ( HANDLEINFO ** )&MPOINT ) )
#define MASKHCHK_ASYNC( HAND, MPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_GMASK, HAND, *( ( HANDLEINFO ** )&MPOINT ) )

// FVF�̕ύX
#define SETFVF( type )	\
	if( GRH.SetFVF != ( type ) )\
	{\
		GraphicsDevice_SetFVF( ( type ) ) ;\
		GRH.SetFVF = ( type ) ;\
	}

// �^��`----------------------------------------------------------------------

// �f�[�^�錾------------------------------------------------------------------

MASKMANAGEDATA MaskManageData ;

// �֐��v���g�^�C�v�錾 -------------------------------------------------------

static int UpdateMaskImageTexture( RECT Rect ) ;						// �w��̈�̃}�X�N�C���[�W�e�N�X�`�����X�V����

// �v���O����------------------------------------------------------------------

// �}�X�N�����̏�����
extern int InitializeMask( void )
{
	if( MASKD.InitializeFlag == TRUE )
		return -1 ;

	// �}�X�N�n���h���Ǘ��f�[�^�̏�����
	InitializeHandleManage( DX_HANDLETYPE_GMASK, sizeof( MASKDATA ), MAX_MASK_NUM, InitializeMaskHandle, TerminateMaskHandle, DXSTRING( _T( "�}�X�N" ) ) ) ;

	// �������t���O�𗧂Ă�
	MASKD.InitializeFlag = TRUE ;

	// �I��
	return 0 ;
}

// �}�X�N�����̌�n��
extern int TerminateMask( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �}�X�N�X�N���[�����폜����
	NS_DeleteMaskScreen() ;

	// �쐬�����}�X�N�O���t�B�b�N���폜
	NS_InitMask() ;

	// �������t���O��|��
	MASKD.InitializeFlag = FALSE ;

	// �}�X�N�n���h���̌�n��
	TerminateHandleManage( DX_HANDLETYPE_GMASK ) ;

	// �I��
	return 0 ;
}

// �}�X�N�f�[�^������������
extern int NS_InitMask( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �n���h����������
	return AllHandleSub( DX_HANDLETYPE_GMASK ) ;
}

// �}�X�N�X�N���[�����쐬����
extern int NS_CreateMaskScreen( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// �}�X�N�ۑ��p�T�[�t�F�X�ƃ}�X�N�`��p�T�[�t�F�X�̍쐬
	CreateMaskScreenFunction( TRUE, GBASE.DrawSizeX, GBASE.DrawSizeY ) ; 

	// �}�X�N�T�[�t�F�X���쐬����Ă��邩�t���O�����Ă�
	MASKD.CreateMaskFlag = TRUE ;
	MASKD.MaskValidFlag = MASKD.MaskUseFlag && MASKD.CreateMaskFlag ;

//	// �����}�X�N�g�p�t���O�������Ă�����}�X�N��L���ɂ����Ƃ��s��
//	if( MASKD.MaskUseFlag )
		NS_SetUseMaskScreenFlag( TRUE ) ;

	// �I��
	return 0 ;
}

// �}�X�N�T�[�t�F�X�̍쐬
extern int CreateMaskSurface( BYTE **MaskBuffer, int *BufferPitch, int Width, int Height, int *TransModeP )
{
	// �s�b�`�̎Z�o
	*BufferPitch = ( Width + 3 ) / 4 * 4 ;

	// �o�b�t�@�̍쐬
	*MaskBuffer = ( BYTE * )DXALLOC( *BufferPitch * Height ) ;
	if( *MaskBuffer == NULL )
		return -1 ;

	// �o�b�t�@�̏�����
	_MEMSET( *MaskBuffer, 0, *BufferPitch * Height ) ;

	// ���ߐF���[�h�̃Z�b�g
	*TransModeP = DX_MASKTRANS_BLACK ;

	// �I��
	return 0 ;
}

// �}�X�N�n���h���̏�����
extern int InitializeMaskHandle( HANDLEINFO * )
{
	// ���ɉ��������I��
	return 0 ;
}

// �}�X�N�n���h���̌�n��
extern int TerminateMaskHandle( HANDLEINFO *HandleInfo )
{
	MASKDATA *Mask ;

	Mask = ( MASKDATA * )HandleInfo ;

	// �}�X�N�p�f�[�^�̈�̉��
	if( Mask->SrcData )
	{
		DXFREE( Mask->SrcData ) ;
		Mask->SrcData = NULL ;
	}

	// �I��
	return 0 ;
}

// MakeMask �̎������֐�
static int MakeMask_Static(
	int MaskHandle,
	int Width,
	int Height,
	int /*ASyncThread*/
)
{
	MASKDATA *Mask ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK_ASYNC( MaskHandle, Mask ) )
		return -1 ;

	// �}�X�N�ۑ��p�T�[�t�F�X�̍쐬
	if( CreateMaskSurface( &Mask->SrcData, &Mask->SrcDataPitch, Width, Height, &Mask->TransMode ) < 0 )
		return -1 ;

	// �f�[�^�̃Z�b�g
	Mask->MaskWidth = Width ;
	Mask->MaskHeight = Height ;

	// ����I��
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// MakeMask �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void MakeMask_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MaskHandle ;
	int Width ;
	int Height ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MaskHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Width = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Height = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = MakeMask_Static( MaskHandle, Width, Height, TRUE ) ;

	DecASyncLoadCount( MaskHandle ) ;
	if( Result < 0 )
	{
		SubHandle( MaskHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// �}�X�N�f�[�^�̒ǉ�
extern int MakeMask_UseGParam( int Width, int Height, int ASyncLoadFlag )
{
	int MaskHandle ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// �n���h���̍쐬
	MaskHandle = AddHandle( DX_HANDLETYPE_GMASK ) ;
	if( MaskHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, MaskHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, Width ) ;
		AddASyncLoadParamInt( NULL, &Addr, Height ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = MakeMask_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MaskHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Width ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Height ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( MaskHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MakeMask_Static( MaskHandle, Width, Height, FALSE ) < 0 )
			goto ERR ;
	}

	// �I��
	return MaskHandle ;

ERR :
	SubHandle( MaskHandle ) ;

	return -1 ;
}

// �}�X�N�f�[�^�̒ǉ�
extern int NS_MakeMask( int Width, int Height )
{
	return MakeMask_UseGParam( Width, Height, GetASyncLoadFlag() ) ;
}

// �}�X�N�f�[�^���폜
extern int NS_DeleteMask( int MaskHandle )
{
	return SubHandle( MaskHandle ) ;
}

// �}�X�N�f�[�^�T�[�t�F�X�ɂa�l�o�f�[�^���}�X�N�f�[�^�ƌ����Ăē]��
extern int NS_BmpBltToMask( HBITMAP Bmp, int BmpPointX, int BmpPointY, int MaskHandle )
{
	MASKDATA * MaskData ;
	BITMAP bm ;
	int ColorBitDepth ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK_ASYNC( MaskHandle, MaskData ) )
		return -1 ;

	// �r�b�g�}�b�v�f�[�^�̎擾
	GetObject( Bmp, sizeof( BITMAP ), ( void * )&bm ) ;
	bm.bmWidthBytes += bm.bmWidthBytes % 4 ? 4 - bm.bmWidthBytes % 4 : 0 ;

	// �]������
	ColorBitDepth = NS_GetScreenBitDepth() ;
	{
		BYTE *SrcPoint, *DestPoint ;
		int SrcAddIndex, DestAddIndex ;
		int DestHeight, DestWidth ;

		SrcPoint = ( BYTE *)bm.bmBits + BmpPointX * bm.bmBitsPixel / 8 + BmpPointY * bm.bmWidthBytes ;
		DestPoint = ( BYTE * )MaskData->SrcData ;

		SrcAddIndex = bm.bmWidthBytes - MaskData->MaskWidth * bm.bmBitsPixel / 8 ;
		DestAddIndex = MaskData->SrcDataPitch - MaskData->MaskWidth ;

		DestHeight = MaskData->MaskHeight ;
		DestWidth = MaskData->MaskWidth ;

#ifdef DX_NON_INLINE_ASM
		int i ;
		do
		{
			i = DestWidth ;
			do
			{
				*DestPoint = *SrcPoint ;
				DestPoint ++ ;
				SrcPoint += 3 ;
			}while( -- i != 0 ) ;
			DestPoint += DestAddIndex ;
			SrcPoint += SrcAddIndex ;
		}while( -- DestHeight != 0 ) ;
#else
		__asm{
			PUSH	EDI
			PUSH	ESI
			PUSHF
			CLD
			MOV		EDI, DestPoint
			MOV		ESI, SrcPoint
			MOV		EDX, DestHeight
			MOV		EBX, DestWidth
		LOOP81:
			MOV		ECX, EBX
		LOOP82:
			MOV		AL	, [ESI]
			MOV		[EDI], AL
			INC		EDI
			ADD		ESI, 3
			LOOP	LOOP82

			ADD		ESI, SrcAddIndex
			ADD		EDI, DestAddIndex
			DEC		EDX
			JNZ		LOOP81

			POPF
			POP		ESI
			POP		EDI
		}
#endif
	}

	// �I��
	return 0 ;
}

// �}�X�N�̑傫���𓾂� 
extern int NS_GetMaskSize( int *WidthBuf, int *HeightBuf, int MaskHandle )
{
	MASKDATA * Mask ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	*WidthBuf = Mask->MaskWidth ;
	*HeightBuf = Mask->MaskHeight ;

	// �I��
	return 0 ;
}

// LoadMask �̎������֐�
static int LoadMask_Static(
	int MaskHandle,
	const TCHAR *FileName,
	int /*ASyncThread*/
)
{
	HBITMAP Bmp ;
	BITMAP bm ;
	COLORDATA SrcColor;
	MASKDATA * Mask ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK_ASYNC( MaskHandle, Mask ) )
		return -1 ;

	// ���[�h�����݂�
	NS_CreateFullColorData( &SrcColor );
	if( ( Bmp = NS_CreateDIBGraph( FileName, FALSE, &SrcColor ) ) == NULL ) return -1 ;

	// �O���t�B�b�N�̃f�[�^���擾
	GetObject( Bmp, sizeof( BITMAP ), ( void * )&bm ) ;
	
	// �}�X�N�ۑ��p�T�[�t�F�X�̍쐬
	if( CreateMaskSurface( &Mask->SrcData, &Mask->SrcDataPitch, bm.bmWidth, bm.bmHeight, &Mask->TransMode ) < 0 )
	{
		DeleteObject( Bmp ) ;
		return -1 ;
	}

	// �f�[�^�̃Z�b�g
	Mask->MaskWidth = bm.bmWidth ;
	Mask->MaskHeight = bm.bmHeight ;

	// �}�X�N�f�[�^��]��
	NS_BmpBltToMask( Bmp, 0, 0, MaskHandle ) ;

	// �a�l�o�I�u�W�F�N�g���폜
	DeleteObject( Bmp ) ;

	// �I��
	return MaskHandle ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadMask �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void LoadMask_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MaskHandle ;
	const TCHAR *FileName ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MaskHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = LoadMask_Static( MaskHandle, FileName, TRUE ) ;

	DecASyncLoadCount( MaskHandle ) ;
	if( Result < 0 )
	{
		SubHandle( MaskHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// �}�X�N�f�[�^�����[�h���� 
extern int LoadMask_UseGParam( const TCHAR *FileName, int ASyncLoadFlag )
{
	int MaskHandle ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// �n���h���̍쐬
	MaskHandle = AddHandle( DX_HANDLETYPE_GMASK ) ;
	if( MaskHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FileName, FullPath ) ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, MaskHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = LoadMask_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MaskHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( MaskHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadMask_Static( MaskHandle, FileName, FALSE ) < 0 )
			goto ERR ;
	}

	// �I��
	return MaskHandle ;

ERR :
	SubHandle( MaskHandle ) ;

	return -1 ;
}

// �}�X�N�f�[�^�����[�h����
extern int NS_LoadMask( const TCHAR *FileName )
{
	return LoadMask_UseGParam( FileName, GetASyncLoadFlag() ) ;
}

// LoadDivMask �̎������֐�
static int LoadDivMask_Static(
	const TCHAR *FileName,
	int AllNum,
	int XNum,
	int YNum,
	int XSize,
	int YSize,
	int *HandleBuf,
	int /*ASyncThread*/
)
{
	MASKDATA * Mask ;
	HBITMAP Bmp ;
	BITMAP bm ;
	int i, j, k ;
	COLORDATA SrcColor;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// ���[�h�����݂�
	Bmp = NULL ;
	NS_CreateFullColorData( &SrcColor );
	if( ( Bmp = NS_CreateDIBGraph( FileName, FALSE, &SrcColor ) ) == NULL )
		return -1 ;

	// �O���t�B�b�N�̃f�[�^���擾
	GetObject( Bmp, sizeof( BITMAP ), ( void * )&bm ) ;

	// �T�C�Y�m�F
	if( XNum * XSize > bm.bmWidth || YNum * YSize > bm.bmHeight )
		goto ERR ;

	// �����]���J�n
	k = 0 ;
	for( i = 0 ; k != AllNum && i < YNum ; i ++ )
	{
		for( j = 0 ; k != AllNum && j < XNum ; j ++, k ++ )
		{
			if( MASKHCHK_ASYNC( HandleBuf[ k ], Mask ) )
				goto ERR ;

			// �}�X�N�̍쐬
			if( CreateMaskSurface( &Mask->SrcData, &Mask->SrcDataPitch, XSize, YSize, &Mask->TransMode ) < 0 )
				goto ERR ;

			// �f�[�^�̃Z�b�g
			Mask->MaskWidth = XSize ;
			Mask->MaskHeight = YSize ;

			// �}�X�N�f�[�^�̓]��
			NS_BmpBltToMask( Bmp, XSize * j, YSize * i, HandleBuf[ k ] ) ;
		}
	}

	// �a�l�o�̍폜
	DeleteObject( Bmp ) ;

	// �I��
	return 0 ;

	// �G���[�I��
ERR :
	if( Bmp != NULL )
		DeleteObject( Bmp ) ;
	
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadDivMask �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void LoadDivMask_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	const TCHAR *FileName ;
	int AllNum ;
	int XNum ;
	int YNum ;
	int XSize ;
	int YSize ;
	int *HandleBuf ;
	int Addr ;
	int i ;
	int Result ;

	Addr = 0 ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	AllNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	XNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	YNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	XSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	YSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	HandleBuf = ( int * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	Result = LoadDivMask_Static( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, TRUE ) ;

	for( i = 0 ; i < AllNum ; i ++ )
		DecASyncLoadCount( HandleBuf[ i ] ) ;

	if( Result < 0 )
	{
		for( i = 0 ; i < AllNum ; i ++ )
			NS_DeleteMask( HandleBuf[ i ] ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// �}�X�N���摜���番���ǂ݂���
extern int LoadDivMask_UseGParam(
	const TCHAR *FileName,
	int AllNum,
	int XNum,
	int YNum,
	int XSize,
	int YSize,
	int *HandleBuf,
	int ASyncLoadFlag
)
{
	int i ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	if( AllNum == 0 )
		return -1 ;

	// �O���t�B�b�N�n���h���̍쐬
	_MEMSET( HandleBuf, 0, sizeof( int ) * AllNum ) ;
	for( i = 0 ; i < AllNum ; i ++ )
	{
		HandleBuf[ i ] = AddHandle( DX_HANDLETYPE_GMASK ) ;
		if( HandleBuf[ i ] < 0 )
			goto ERR ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FileName, FullPath ) ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ; 
		AddASyncLoadParamInt( NULL, &Addr, AllNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, XNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, YNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, XSize ) ;
		AddASyncLoadParamInt( NULL, &Addr, YSize ) ;
		AddASyncLoadParamStruct( NULL, &Addr, HandleBuf, sizeof( int ) * AllNum ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = LoadDivMask_ASync ;
		Addr = 0 ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ; 
		AddASyncLoadParamInt( AParam->Data, &Addr, AllNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, XNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, YNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, XSize ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, YSize ) ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, HandleBuf, sizeof( int ) * AllNum ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		for( i = 0 ; i < AllNum ; i ++ )
			IncASyncLoadCount( HandleBuf[ i ], AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadDivMask_Static( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, FALSE ) < 0 )
			goto ERR ;
	}

	// ����I��
	return 0 ;

ERR :
	for( i = 0 ; i < AllNum ; i ++ )
	{
		NS_DeleteMask( HandleBuf[ i ] ) ;
	}

	return -1 ;
}

// �}�X�N���摜���番���ǂ݂���
extern int NS_LoadDivMask( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf )
{
	return LoadDivMask_UseGParam( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, GetASyncLoadFlag() ) ;
}

#ifndef DX_NON_FONT

// �����w�肠��̕�������}�X�N�X�N���[���ɕ`�悷��
extern int NS_DrawFormatStringMask( int x, int y, int Flag, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �ҏW��̕�������擾����
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawStringMaskToHandle( x, y, Flag, NS_GetDefaultFontHandle(), String ) ;
}

// �����w�肠��̕�������}�X�N�X�N���[���ɕ`�悷��
extern int NS_DrawFormatStringMaskToHandle( int x, int y, int Flag, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �ҏW��̕�������擾����
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawStringMaskToHandle( x, y, Flag, FontHandle, String ) ;
}

// ��������}�X�N�X�N���[���ɕ`�悷��(�t�H���g�n���h���w���)
extern int NS_DrawStringMaskToHandle( int x, int y, int Flag, int FontHandle, const TCHAR *String )
{
	BASEIMAGE Image ;
	int Color ;
	int Result ;
	SIZE DrawSize ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �}�X�N�X�N���[�����쐬����Ă��Ȃ�������G���[
	if( MASKD.MaskBuffer == NULL )
		return -1 ;

	RefreshDefaultFont() ;

	// BASEIMAGE�f�[�^�̍쐬
	NS_CreatePaletteColorData( &Image.ColorData ) ;
	NS_GetDrawScreenSize( &Image.Width, &Image.Height ) ;
	Image.Pitch = MASKD.MaskBufferPitch ;
	Image.GraphData = MASKD.MaskBuffer ;

	// �}�X�N�X�N���[����ɕ�����`��
	Color = Flag == 0 ? 0 : 0xff ;
	Result = NS_FontCacheStringDrawToHandle( x, y, String, Color, Color, 
											&Image, &GBASE.DrawArea, FontHandle,
											FALSE, &DrawSize ) ;

	RECT Rect ;

	if( GRA2.ValidHardWare )
	{
		// �X�V
		Rect.left = x ;
		Rect.top = y ;
		Rect.right = x + DrawSize.cx ;
		Rect.bottom = y + DrawSize.cy ;
		UpdateMaskImageTexture( Rect ) ;
	}

	return Result ;
}

// ��������}�X�N�X�N���[���ɕ`�悷��
extern int NS_DrawStringMask( int x, int y, int Flag, const TCHAR *String )
{
	return NS_DrawStringMaskToHandle( x, y, Flag, NS_GetDefaultFontHandle(), String ) ;
}

#endif // DX_NON_FONT

// �}�X�N�X�N���[�����g�p�����t���O�̎擾
extern int NS_GetMaskUseFlag( void )
{
	return MASKD.MaskUseFlag ;
}

// �}�X�N�̐��l�ɑ΂�����ʂ��t�]������
extern int NS_SetMaskReverseEffectFlag( int ReverseFlag )
{
	MASKD.MaskReverseEffectFlag = ReverseFlag ;

	// �I��
	return 0 ;
}

// �}�X�N�g�p���[�h�̎擾
extern int NS_GetUseMaskScreenFlag( void )
{
	return MASKD.MaskUseFlag ;
}

// �}�X�N�X�N���[���𕜋�������
extern int ReCreateMaskSurface( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// ���C���̃}�X�N�T�[�t�F�X���쐬����
	CreateMaskScreenFunction( MASKD.CreateMaskFlag, GBASE.DrawSizeX, GBASE.DrawSizeY ) ;

	// �����}�X�N�g�p�t���O�������Ă�����}�X�N��L���ɂ����Ƃ��s��
	if( MASKD.MaskUseFlag )
		NS_SetUseMaskScreenFlag( TRUE ) ;

	// �I��
	return 0 ;
}

// �}�X�N�̍쐬
extern	int CreateMaskOn2D( int DrawPointX, int DrawPointY ,int DestWidth, int DestHeight, void *DestBufP, int DestPitch, int DestBitDepth, 
					   int SrcWidth, int SrcHeight, void *SrcData )
{
	int SrcPitch ;
	BYTE *DestP, *SrcDataTemp ;
	BYTE *StartSrcPointY ;
	DWORD StartSrcPointX ;
	int TempHeight ;
	int PixelByte ;
	int StartSrcWidth, StartSrcHeight ;

	// �]������
	PixelByte = DestBitDepth / 8 ;
	SrcPitch = SrcWidth ;
	DestP = ( BYTE * )DestBufP + DrawPointX * PixelByte + DrawPointY * DestPitch ;
	SrcDataTemp = ( BYTE * )SrcData ;
	DestPitch -= DestBitDepth / 8 * DestWidth ;

	StartSrcPointY = ( BYTE * )SrcData + ( DrawPointY % SrcHeight ) * SrcWidth ; 
	StartSrcPointX = DrawPointX % SrcWidth ;
	StartSrcWidth = SrcWidth - DrawPointX % SrcWidth ;
	StartSrcHeight = SrcHeight - DrawPointY % SrcHeight ; 

	// �]������
#ifdef DX_NON_INLINE_ASM
	int i ;
	DWORD TempWidth ;
	BYTE *SrcPoint, *SrcPointX ;

	TempHeight = StartSrcHeight ;
	SrcPointX = StartSrcPointY ;
	switch( DestBitDepth )
	{
	case 8 :
		do
		{
			i = DestWidth ;
			TempWidth = StartSrcWidth ;
			SrcPoint = SrcPointX + StartSrcPointX ;
			do
			{
				*DestP = *SrcPoint ;
				DestP ++ ;
				SrcPoint ++ ;

				if( -- TempWidth == 0 )
				{
					SrcPoint = SrcPointX ;
					TempWidth = SrcWidth ;
				}
			}while( -- i != 0 ) ;

			DestP += DestPitch ;
			if( -- TempHeight == 0 )
			{
				SrcPointX = ( BYTE * )SrcData ;
				TempHeight = SrcHeight ;
			}
			else
			{
				SrcPointX += SrcPitch ;
			}
		}while( -- DestHeight != 0 ) ;
		break ;

	case 16 :
		do
		{
			i = DestWidth ;
			TempWidth = StartSrcWidth ;
			SrcPoint = SrcPointX + StartSrcPointX ;
			do
			{
				DestP[ 0 ] = *SrcPoint ;
				DestP[ 1 ] = *SrcPoint ;
				DestP += 2 ;
				SrcPoint ++ ;

				if( -- TempWidth == 0 )
				{
					SrcPoint = SrcPointX ;
					TempWidth = SrcWidth ;
				}
			}while( -- i != 0 ) ;

			DestP += DestPitch ;
			if( -- TempHeight == 0 )
			{
				SrcPointX = ( BYTE * )SrcData ;
				TempHeight = SrcHeight ;
			}
			else
			{
				SrcPointX += SrcPitch ;
			}
		}while( -- DestHeight != 0 ) ;
		break ;

	case 32 :
		do
		{
			i = DestWidth ;
			TempWidth = StartSrcWidth ;
			SrcPoint = SrcPointX + StartSrcPointX ;
			do
			{
				*( ( DWORD * )DestP ) = *SrcPoint | ( *SrcPoint << 8 ) | ( *SrcPoint << 16 ) ;
				DestP += 4 ;
				SrcPoint ++ ;

				if( -- TempWidth == 0 )
				{
					SrcPoint = SrcPointX ;
					TempWidth = SrcWidth ;
				}
			}while( -- i != 0 ) ;

			DestP += DestPitch ;
			if( -- TempHeight == 0 )
			{
				SrcPointX = ( BYTE * )SrcData ;
				TempHeight = SrcHeight ;
			}
			else
			{
				SrcPointX += SrcPitch ;
			}
		}while( -- DestHeight != 0 ) ;
		break ;
	}
#else
	switch( DestBitDepth ) 
	{
	case 8 :
		__asm{
			PUSHA
			PUSHF
			CLD
			MOV		EDI, DestP
			MOV		EAX, StartSrcHeight
			MOV		TempHeight, EAX
			MOV		ESI, StartSrcPointY
			ADD		ESI, StartSrcPointX
			MOV		EDX, StartSrcPointY
			JMP		LOOP84_a
		LOOP81_a:
			MOV		ESI, SrcData
			MOV		EDX, ESI
			ADD		ESI, StartSrcPointX
			MOV		EAX, SrcHeight
			MOV		TempHeight, EAX
		LOOP84_a:
			MOV		EBX, DestWidth
			MOV		ECX, StartSrcWidth
			JMP		LOOP83_a
		LOOP82_a:
			MOV		ESI, EDX
			MOV		ECX, SrcWidth
		LOOP83_a:
			MOVSB
			DEC		EBX
			JZ		R81_a
			LOOP	LOOP83_a
			JMP		LOOP82_a
		R81_a:
			ADD		EDX, SrcPitch
			MOV		ESI, EDX
			ADD		ESI, StartSrcPointX
			ADD		EDI, DestPitch
			DEC		DestHeight
			JZ		END8_a
			DEC		TempHeight
			JNZ		LOOP84_a
			JMP		LOOP81_a
		END8_a:
			POPF
			POPA
		}
		break ;

	case 16 :
		__asm{
			PUSHA
			PUSHF
			CLD
			MOV		EDI, DestP
			MOV		EAX, StartSrcHeight
			MOV		TempHeight, EAX
			MOV		ESI, StartSrcPointY
			ADD		ESI, StartSrcPointX
			MOV		EDX, StartSrcPointY
			JMP		LOOP164_a
		LOOP161_a:
			MOV		ESI, SrcData
			MOV		EDX, ESI
			ADD		ESI, StartSrcPointX
			MOV		EAX, SrcHeight
			MOV		TempHeight, EAX
		LOOP164_a:
			MOV		EBX, DestWidth
			MOV		ECX, StartSrcWidth
			JMP		LOOP163_a
		LOOP162_a:
			MOV		ESI, EDX
			MOV		ECX, SrcWidth
		LOOP163_a:
			MOV		AL, [ESI]
			MOV		AH, AL
			STOSW
			INC		ESI
			DEC		EBX
			JZ		R161_a
			LOOP	LOOP163_a
			JMP		LOOP162_a
		R161_a:
			ADD		EDX, SrcPitch
			MOV		ESI, EDX
			ADD		ESI, StartSrcPointX
			ADD		EDI, DestPitch
			DEC		DestHeight
			JZ		END16_a
			DEC		TempHeight
			JNZ		LOOP164_a
			JMP		LOOP161_a
		END16_a:
			POPF
			POPA
		}
		break ;

	case 32 :
		__asm{
			PUSHA
			PUSHF
			CLD
			MOV		EDI, DestP
			MOV		EAX, StartSrcHeight
			MOV		TempHeight, EAX
			MOV		ESI, StartSrcPointY
			ADD		ESI, StartSrcPointX
			MOV		EDX, StartSrcPointY
			JMP		LOOP324_a
		LOOP321_a:
			MOV		ESI, SrcData
			MOV		EDX, ESI
			ADD		ESI, StartSrcPointX
			MOV		EAX, SrcHeight
			MOV		TempHeight, EAX
		LOOP324_a:
			MOV		EBX, DestWidth
			MOV		ECX, StartSrcWidth
			JMP		LOOP323_a
		LOOP322_a:
			MOV		ESI, EDX
			MOV		ECX, SrcWidth
		LOOP323_a:
			XOR		EAX, EAX
			MOV		AL, [ESI]
			MOV		AH, AL
			SHL		EAX, 8
			MOV		AL, AH
			STOSD
			INC		ESI
			DEC		EBX
			JZ		R321_a
			LOOP	LOOP323_a
			JMP		LOOP322_a
		R321_a:
			ADD		EDX, SrcPitch
			MOV		ESI, EDX
			ADD		ESI, StartSrcPointX
			ADD		EDI, DestPitch
			DEC		DestHeight
			JZ		END32_a
			DEC		TempHeight
			JNZ		LOOP324_a
			JMP		LOOP321_a
		END32_a:
			POPF
			POPA
		}
		break ;
	}
#endif

	// �I��
	return 0 ;
}


// �}�X�N�p�^�[���̓W�J
extern int DrawMaskToDirectData_Base( int DrawPointX, int DrawPointY, void *DestBufP, int DestPitch, 
											int SrcWidth, int SrcHeight, const void *SrcData, int TransMode  )
{
	BYTE *DestP ,*SrcP ;
	BYTE TransColor ;
	int DestAddPoint ;
	int SrcAddPoint ;
	int BltWidth, BltHeight ;
	RECT Rect ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKD.MaskBuffer == NULL ) return 0 ;

	SETRECT( Rect, DrawPointX, DrawPointY, DrawPointX + SrcWidth, DrawPointY + SrcHeight ) ;
	RectClipping( &Rect, &GBASE.DrawArea ) ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;
	Rect.left -= DrawPointX ;
	Rect.right -= DrawPointX ;
	Rect.top -= DrawPointY ;
	Rect.bottom -= DrawPointY ;
	if( Rect.left > 0 ) DrawPointX += Rect.left ;
	if( Rect.top > 0 ) DrawPointY += Rect.top ;

	// �]������
	BltWidth = Rect.right - Rect.left ;
	BltHeight = Rect.bottom - Rect.top ;
	SrcP = ( BYTE * )SrcData + Rect.left + Rect.top * SrcWidth ;
	SrcAddPoint = SrcWidth - BltWidth ;

	DestP = ( BYTE * )DestBufP + DrawPointX + DrawPointY * DestPitch ;
	DestAddPoint = DestPitch - BltWidth ;

	TransColor = TransMode == DX_MASKTRANS_WHITE ? 0xff : 0 ;

	// ���ߐF�����邩�ǂ����ŏ����𕪊�
	if( TransMode == DX_MASKTRANS_NONE )
	{
#ifdef DX_NON_INLINE_ASM
		int i ;
		do
		{
			i = BltWidth ;

			do
			{
				*DestP = *SrcP ;
				DestP ++ ;
				SrcP ++ ;
			}while( -- i != 0 ) ;

			DestP += DestAddPoint ;
			SrcP += SrcAddPoint ;

		}while( -- BltHeight != 0 ) ;
#else
		// ���ߐF�Ȃ��o�[�W����
		__asm{
			CLD
			MOV		EDI, DestP
			MOV		ESI, SrcP
			MOV		EDX, BltWidth
			MOV		EBX, BltHeight
			MOV		EAX, SrcAddPoint
			MOV		ECX, DestAddPoint
			PUSH	EBP
			MOV		EBP, ECX
		LOOP811:
			MOV		ECX, EDX
			REP		MOVSB
			ADD		EDI, EBP
			ADD		ESI, EAX
			DEC		EBX
			JNZ		LOOP811
			POP		EBP
		}
#endif
	}
	else
	{
		// ���ߐF����o�[�W����
		
#ifdef DX_NON_INLINE_ASM
		int i ;
		TransColor &= 0xff ;
		do
		{
			i = BltWidth ;

			do
			{
				if( *SrcP != TransColor )
					*DestP = *SrcP ;
				DestP ++ ;
				SrcP ++ ;
			}while( -- i != 0 ) ;

			DestP += DestAddPoint ;
			SrcP += SrcAddPoint ;

		}while( -- BltHeight != 0 ) ;
#else
		__asm{
			MOV		EDI, DestP
			MOV		ESI, SrcP
			MOV		BL, TransColor
	LOOP821:
			MOV		ECX, BltWidth
	LOOP822:
			MOV		AL, [ESI]
			TEST	AL, BL
			JZ		R821
			MOV		[EDI], AL
	R821:
			INC		EDI
			INC		ESI
			LOOP	LOOP822

			ADD		EDI, DestAddPoint
			ADD		ESI, SrcAddPoint
			DEC		BltHeight
			JNZ		LOOP821
		}
#endif
	}

	// �I��
	return 0 ;
}

// �}�X�N�̃f�[�^�𒼐ڃ}�X�N��ʑS�̂ɕ`�悷��
extern int NS_DrawFillMaskToDirectData( int x1, int y1, int x2, int y2,  int Width, int Height, const void *MaskData )
{
	int MaskHandle ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ��̃}�X�N���쐬
	if( ( MaskHandle = NS_MakeMask( Width, Height ) ) == -1 ) return -1 ;

	// �}�X�N�Ƀf�[�^���Z�b�g
	NS_SetDataToMask( Width, Height, MaskData, MaskHandle ) ;

	// �}�X�N����ʑS�̂ɕ`��
	NS_DrawFillMask( x1, y1, x2, y2, MaskHandle ) ;

	// �}�X�N�n���h�����폜
	NS_DeleteMask( MaskHandle ) ;

	// �I��
	return 0 ;
}

// �}�X�N�̃f�[�^���}�X�N�ɓ]������
extern int NS_SetDataToMask( int Width, int Height, const void *MaskData, int MaskHandle )
{
	MASKDATA * Mask ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;
	
	if( Mask->MaskWidth != Width || Mask->MaskHeight != Height ) return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// �}�X�N�f�[�^���Ȃ������ꍇ�͏������I��
	if( MaskData == NULL ) return 0 ;

	// �}�X�N�O���t�B�b�N�Ƀ}�X�N�f�[�^��W�J
	DrawMaskToDirectData_Base( 0, 0, Mask->SrcData, Mask->SrcDataPitch,
							Width, Height, MaskData, DX_MASKTRANS_NONE ) ;

	// �I��
	return 0 ;
}



// �}�X�N��ʏ�̕`���Ԃ��擾����
extern int NS_GetMaskScreenData( int x1, int y1, int x2, int y2, int MaskHandle )
{
	MASKDATA * Mask ; 
	RECT Rect, Rect2 ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	if( MASKD.MaskBuffer == NULL )
		return 0 ;

	// �擾��`�̃Z�b�g
	SETRECT( Rect, x1, y1, x2, y2 ) ;
	NS_GetDrawScreenSize( ( int * )&Rect2.right, ( int * )&Rect2.bottom ) ;
	Rect2.left = 0 ; Rect2.top = 0 ;
	RectClipping( &Rect, &Rect2 ) ;

	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return -1 ;
	if( Rect.right - Rect.left != Mask->MaskWidth ||
		Rect.bottom - Rect.top != Mask->MaskHeight ) return -1 ;

	// �f�[�^�̓]��
	{
		BYTE *Dest, *Src ;
		int DestAddPitch, SrcAddPitch ;
		int DestWidth, DestHeight ;

		Rect.left -= x1 ;
		Rect.right -= x1 ;
		Rect.top -= y1 ;
		Rect.bottom -= y1 ;
		if( Rect.left > 0 ){ x1 += Rect.left ; }
		if( Rect.top  > 0 ){ y1 += Rect.top  ; }

		DestWidth = Rect.right - Rect.left ;
		DestHeight = Rect.bottom - Rect.top ;
		Dest = Mask->SrcData + Rect.left + ( Rect.top * Mask->SrcDataPitch ) ;
		Src = MASKD.MaskBuffer + x1 + ( y1 * MASKD.MaskBufferPitch ) ;

		SrcAddPitch = Mask->SrcDataPitch - DestWidth ;
		DestAddPitch = MASKD.MaskBufferPitch - DestWidth ;

#ifdef DX_NON_INLINE_ASM
		int i ;
		do
		{
			i = DestWidth ;
			do
			{
				*Dest = *Src ;
				Dest ++ ;
				Src ++ ;
			}while( -- i != 0 ) ;

			Dest += DestAddPitch ;
			Src += SrcAddPitch ;

		}while( -- DestHeight != 0 ) ;
#else
		__asm{
			CLD
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		EDX, DestWidth
			MOV		EBX, DestHeight
			MOV		EAX, SrcAddPitch
			MOV		ECX, DestAddPitch
			PUSH	EBP
			MOV		EBP, ECX
		LOOP1_b:
			MOV		ECX, EDX
			REP		MOVSB
			ADD		EDI, EBP
			ADD		ESI, EAX
			DEC		EBX
			JNZ		LOOP1_b
			POP		EBP
		}
#endif
	}

	// �I��
	return 0 ;
}

// �}�X�N�X�N���[�����폜����
extern int NS_DeleteMaskScreen( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

//	if( WinData.ActiveFlag == FALSE )
//		DxActiveWait() ;

	// �����}�X�N�g�p�t���O�������Ă�����}�X�N�𖳌��ɂ����Ƃ��s��
	if( MASKD.MaskUseFlag )
		NS_SetUseMaskScreenFlag( FALSE ) ;

	// �}�X�N�p�T�[�t�F�X���폜����
	ReleaseMaskSurface() ;

	// �}�X�N�T�[�t�F�X���쐬����Ă��邩�t���O��|��
	MASKD.CreateMaskFlag = FALSE ;
	MASKD.MaskValidFlag = MASKD.MaskUseFlag && MASKD.CreateMaskFlag ;

	// �I��
	return 0 ;
}














// DirectX �o�[�W�����ˑ����܂ފ֐�

// �}�X�N�X�N���[�����쐬����֐�
extern int CreateMaskScreenFunction( int MaskSurfaceFlag, int Width, int Height )
{
	bool OldEnable = false ;
	BYTE *MaskBufferOld ;
	int MaskBufferPitchOld ;
	int MaskBufferSizeXOld ;
	int MaskBufferSizeYOld ;
	D_IDirect3DTexture9 *MaskImageTextureOld ;
	D_IDirect3DTexture9 *MaskScreenTextureOld ;
	D_IDirect3DSurface9 *MaskScreenSurfaceOld ;
	MEMIMG MaskDrawMemImgOld ;

	if( MaskSurfaceFlag == FALSE ) return 0 ;

	// �T�C�Y�̒���
	if( MASKD.MaskBuffer != NULL )
	{
		// �T�C�Y�͑傫���ق��ɍ��킹��
		if( Width  < MASKD.MaskBufferSizeX ) Width  = MASKD.MaskBufferSizeX ;
		if( Height < MASKD.MaskBufferSizeY ) Height = MASKD.MaskBufferSizeY ;
	}

	// ���Ƀ}�X�N�o�b�t�@�����݂��āA�����܂ł����傫�ȃT�C�Y�̃o�b�t�@���w�肳�ꂽ�ꍇ�̓o�b�t�@��傫����蒼��
	if( MASKD.MaskBuffer != NULL && ( Width > MASKD.MaskBufferSizeX || Height > MASKD.MaskBufferSizeY ) )
	{
		OldEnable = true ;

		MaskBufferOld = MASKD.MaskBuffer ;
		MASKD.MaskBuffer = NULL ;
		MaskBufferPitchOld = MASKD.MaskBufferPitch ;

		MaskBufferSizeXOld = MASKD.MaskBufferSizeX ;
		MaskBufferSizeYOld = MASKD.MaskBufferSizeY ;

		MaskImageTextureOld = MASKD.MaskImageTexture ;
		MASKD.MaskImageTexture = NULL ;

		MaskScreenTextureOld = MASKD.MaskScreenTexture ;
		MASKD.MaskScreenTexture = NULL ;

		MaskScreenSurfaceOld = MASKD.MaskScreenSurface ;
		MASKD.MaskScreenSurface = NULL ;

		MaskDrawMemImgOld = MASKD.MaskDrawMemImg ;
		_MEMSET( &MASKD.MaskDrawMemImg, 0, sizeof( MEMIMG ) ) ;
	}

	MASKD.MaskBufferSizeX = Width ;
	MASKD.MaskBufferSizeY = Height ;

	// �}�X�N�ۑ��p�o�b�t�@�̍쐬
	if( MASKD.MaskBuffer == NULL )
	{
		// �}�X�N�o�b�t�@�̃s�b�`���Z�o( 4�̔{���ɂ��� )
		MASKD.MaskBufferPitch = ( Width + 3 ) / 4 * 4 ;

		// �������̊m��
		MASKD.MaskBuffer = ( BYTE * )DXALLOC( MASKD.MaskBufferPitch * Height ) ;

		// �}�X�N�̃N���A
		_MEMSET( MASKD.MaskBuffer, 0, MASKD.MaskBufferPitch * Height ) ;
	}

	int w, h ;

	// �n�[�h�E�G�A�̋@�\���g�p����ꍇ�̓e�N�X�`�����쐬����
	if( GRA2.ValidHardWare == TRUE )
	{
		// �J���[�o�b�t�@���A���t�@�o�b�t�@���쐬�s�\�ȏꍇ�͂��G���[
		if( GRH.MaskColorFormat == D_D3DFMT_UNKNOWN )
		{
			return DxLib_Error( DXSTRING( _T( "�}�X�N�`��p�J���[�o�b�t�@�Ŏg�p�ł���e�N�X�`���t�H�[�}�b�g������܂���ł���" ) ) ) ;
		}

		if( GRH.MaskAlphaFormat == D_D3DFMT_UNKNOWN )
		{
			return DxLib_Error( DXSTRING( _T( "�}�X�N�`��p�A���t�@�`�����l���o�b�t�@�Ŏg�p�ł���e�N�X�`���t�H�[�}�b�g������܂���ł���" ) ) ) ;
		}

		// ��ʃT�C�Y�����܂� 2 ��n��̒l������o��
		for( w = 1 ; w < Width  ; w <<= 1 ){}
		for( h = 1 ; h < Height ; h <<= 1 ){}
		MASKD.MaskTextureSizeX = w ;
		MASKD.MaskTextureSizeY = h ;

		// �}�X�N�p�C���[�W�e�N�X�`���̍쐬
		if( MASKD.MaskImageTexture == NULL )
		{
			if( GraphicsDevice_CreateTexture( w, h, 1, D_D3DUSAGE_DYNAMIC, GRH.MaskAlphaFormat, D_D3DPOOL_DEFAULT, &MASKD.MaskImageTexture, NULL ) != D_D3D_OK )
				return DxLib_Error( DXSTRING( _T( "�}�X�N�p�C���[�W�e�N�X�`���̍쐬�Ɏ��s���܂���" ) ) ) ;

			// �}�X�N�p�C���[�W�e�N�X�`���̏�����
			{
				D_D3DLOCKED_RECT LockRect ;
				int i, Width ;
				BYTE *Dest ;

				if( MASKD.MaskImageTexture->LockRect( 0, &LockRect, NULL, 0 ) == D_D3D_OK )
				{
					Dest = ( BYTE * )LockRect.pBits ;
					Width = GetD3DFormatColorData( GRH.MaskAlphaFormat )->PixelByte * GRA2.MainScreenSizeX ;
					for( i = 0 ; i < GRA2.MainScreenSizeY ; i ++, Dest += LockRect.Pitch )
						_MEMSET( Dest, 0, Width ) ;

					MASKD.MaskImageTexture->UnlockRect( 0 ) ;
				}
			}
		}

		// �}�X�N�p�X�N���[���e�N�X�`���̍쐬
		if( MASKD.MaskScreenTexture == NULL )
		{
			if( GraphicsDevice_CreateTexture( w, h, 1, D_D3DUSAGE_RENDERTARGET, GRH.ScreenFormat, D_D3DPOOL_DEFAULT, &MASKD.MaskScreenTexture, NULL ) != D_D3D_OK )
				return DxLib_Error( DXSTRING( _T( "�}�X�N�p�X�N���[���e�N�X�`���̍쐬�Ɏ��s���܂���" ) ) ) ;
		}

		// �}�X�N�p�X�N���[���T�[�t�F�X�̎擾
		if( MASKD.MaskScreenTexture && MASKD.MaskScreenSurface == NULL )
		{
			MASKD.MaskScreenTexture->GetSurfaceLevel( 0, &MASKD.MaskScreenSurface ) ;
		}
	}
	else
	{
		// �`���p�T�[�t�F�X�̍쐬
		if( CheckValidMemImg( &MASKD.MaskDrawMemImg ) == FALSE )
		{
			// MEMIMG ���g�p����ꍇ�͉�ʂƓ����傫���� MEMIMG ���쐬����
			if( MakeMemImgScreen( &MASKD.MaskDrawMemImg, Width, Height, -1 ) < 0 )
				return DxLib_Error( DXSTRING( _T( "�}�X�N�p MEMIMG �̍쐬�Ɏ��s���܂���" ) ) ) ;

			MASKD.ValidMaskDrawMemImg = TRUE ;
		}
	}

	// ��蒼�����ꍇ�͈ȑO�̓��e���R�s�[����
	if( OldEnable )
	{
		int i ;

		// �}�X�N�o�b�t�@�̓��e���R�s�[
		for( i = 0 ; i < MaskBufferSizeYOld ; i ++ )
			_MEMCPY( MASKD.MaskBuffer + MASKD.MaskBufferPitch * i, MaskBufferOld + MaskBufferPitchOld * i, MaskBufferSizeXOld ) ;
		DXFREE( MaskBufferOld ) ;
		MaskBufferOld = NULL ;

		// �n�[�h�E�G�A�̋@�\���g�p����ꍇ�̓e�N�X�`�����쐬����
		if( GRA2.ValidHardWare == TRUE )
		{
			RECT UpdateRect ;

			// �}�X�N�p�C���[�W�e�N�X�`���ɓ��e��]��
			UpdateRect.left = 0 ;
			UpdateRect.top = 0 ;
			UpdateRect.right = MaskBufferSizeXOld ;
			UpdateRect.bottom = MaskBufferSizeYOld ;
			UpdateMaskImageTexture( UpdateRect ) ;

			MaskImageTextureOld->Release() ;
			MaskImageTextureOld = NULL ;

			// �}�X�N�p�X�N���[���e�N�X�`���ɍ��܂ł̓��e��]��
			GraphicsDevice_StretchRect(
				MaskScreenSurfaceOld,    &UpdateRect,
				MASKD.MaskScreenSurface, &UpdateRect, D_D3DTEXF_NONE ) ;

			// �����_�[�^�[�Q�b�g�ɂ���Ă�����ύX����
			for( i = 0 ; i < DX_RENDERTARGET_COUNT ; i ++ )
			{
				if( GRH.TargetSurface[ i ] == MaskScreenSurfaceOld )
				{
					SetRenderTargetHardware( MASKD.MaskScreenSurface, i ) ;
				}
			}

			MaskScreenSurfaceOld->Release() ;
			MaskScreenSurfaceOld = NULL ;

			MaskScreenTextureOld->Release() ;
			MaskScreenTextureOld = NULL ;
		}
		else
		{
			// �Â��`���pMEMIMG �̓��e��V���� MEMIMG �ɃR�s�[����
			DrawMemImg( &MASKD.MaskDrawMemImg, &MaskDrawMemImgOld, 0, 0, FALSE, NULL ) ;
			TerminateMemImg( &MaskDrawMemImgOld ) ;
		}
	}

	// �I��
	return 0 ;
}

// �}�X�N�X�N���[�����ꎞ�폜����
extern int ReleaseMaskSurface( void )
{
	// �}�X�N�ۑ��p�o�b�t�@�̉��
	if( MASKD.MaskBuffer != NULL )
	{
		DXFREE( MASKD.MaskBuffer ) ;
		MASKD.MaskBuffer = NULL ;
	}

	// �}�X�N�`��p�� MEMIMG �����
	if( MASKD.ValidMaskDrawMemImg == TRUE )
	{
		TerminateMemImg( &MASKD.MaskDrawMemImg ) ;
		MASKD.ValidMaskDrawMemImg = FALSE ;
	}

	// �}�X�N�C���[�W�p�e�N�X�`���̉��
	if( MASKD.MaskImageTexture )
	{
		MASKD.MaskImageTexture->Release() ;
		MASKD.MaskImageTexture = NULL ;
	}

	// �}�X�N�X�N���[���T�[�t�F�X�̉��
	if( MASKD.MaskScreenSurface )
	{
		MASKD.MaskScreenSurface->Release() ;
		MASKD.MaskScreenSurface = NULL ;
	}

	// �}�X�N�X�N���[���e�N�X�`���̉��
	if( MASKD.MaskScreenTexture )
	{
		MASKD.MaskScreenTexture->Release() ;
		MASKD.MaskScreenTexture = NULL ;
	}

	// �I��
	return 0 ;
}

// �}�X�N�g�p���[�h��ύX
extern int NS_SetUseMaskScreenFlag( int ValidFlag )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �}�X�N�X�N���[�����Ȃ��ꍇ�͂����ŏ������I��
	if( MASKD.MaskBuffer == NULL ) return -1 ;

	// �t���X�N���[���}�X�N�������s���Ă���ꍇ�̓}�X�N�`��̌��ʂ𔽉f������
	MASK_FULLSCREEN_MASKUPDATE

	// ���܂łƃt���O�������ꍇ�͖���
//	if( ValidFlag == MASKD.MaskUseFlag ) return 0 ;

	// �}�X�N���g�p���邩�̃t���O���Z�b�g
	MASKD.MaskUseFlag = ValidFlag ;
	MASKD.MaskValidFlag = MASKD.MaskUseFlag && MASKD.CreateMaskFlag ;

	IMAGEDATA2 *Image2 ;

	// �`���̉摜�f�[�^�A�h���X���擾����
	Image2 = GetGraphData2( GBASE.TargetScreen[ 0 ] ) ;

	// �n�[�h�E�G�A�A�N�Z�����[�^���L�����ǂ����ŏ����𕪊�
	if( GRA2.ValidHardWare )
	{
		// �n�[�h�E�G�A�A�N�Z�����[�^���L���ȏꍇ

		// ���܂������_�f�[�^��`�悷��
		RenderVertexHardware() ;

		// �`����ύX����

		// �}�X�N�T�[�t�F�X�����݂��Ă��Ċ��L���ȏꍇ�̓}�X�N�T�[�t�F�X��`��Ώ̂ɂ���
		if( MASKD.MaskValidFlag && MASKD.MaskScreenSurface )
		{
			SetRenderTargetHardware( MASKD.MaskScreenSurface ) ;
		}
		else
		// �`��\�摜���L���ȏꍇ�͕`��\�摜��`��Ώۂɂ���
		if( Image2 )
		{
			if( Image2->Hard.Draw[ 0 ].Tex->RenderTargetSurface )
			{
				SetRenderTargetHardware( Image2->Hard.Draw[ 0 ].Tex->RenderTargetSurface ) ;
			}
			else
			{
				SetRenderTargetHardware( Image2->Hard.Draw[ 0 ].Tex->Surface[ GBASE.TargetScreenSurface[ 0 ] ] ) ;
			}
		}
		else
		{
			// ����ȊO�̏ꍇ�̓T�u�o�b�N�o�b�t�@���L���ȏꍇ�̓T�u�o�b�N�o�b�t�@��`��Ώ̂ɂ���
			SetRenderTargetHardware( GRH.SubBackBufferSurface ? GRH.SubBackBufferSurface : GRH.BackBufferSurface ) ;
		}

		// �g�p����y�o�b�t�@�̃Z�b�g�A�b�v
		SetupUseZBuffer() ;

		// �r���[�|�[�g�����ɖ߂�
		GRH.InitializeFlag = TRUE ;
		SetViewportHardwareEasy( GBASE.DrawArea.left, GBASE.DrawArea.top, GBASE.DrawArea.right, GBASE.DrawArea.bottom ) ;
		GRH.InitializeFlag = FALSE ;
	}
	else
	{
		// �\�t�g�E�G�A�����_�����O�̏ꍇ

		// �`���̌���
		if( MASKD.MaskValidFlag )
		{
			// �}�X�N���L���ȏꍇ�̓}�X�N��`���ɂ���
			GRS.TargetMemImg = &MASKD.MaskDrawMemImg ;
		}
		else
		{
			// ����ȊO�̏ꍇ�͕`��\�摜�����C���o�b�t�@
			GRS.TargetMemImg = Image2 ? &Image2->Soft.MemImg : &GRS.MainBufferMemImg ;
		}
	}

	// �I��
	return 0 ;
}

static 	DX_DIRECT3DSURFACE9 *UseBackupSurface, *DestTargetSurface ;

// �}�X�N���g�p�����`��̑O�ɌĂԊ֐�
// ( ���łɃT�u�o�b�t�@���g�p�����`��G���A�@�\���g�p���Ă���ꍇ�̏���������Ă��܂��Ă���� )
extern int MaskDrawBeginFunction( RECT Rect )
{
	// �t���X�N���[���}�X�N�������s���Ă���ꍇ�̓}�X�N�`��̌��ʂ𔽉f������
	MASK_FULLSCREEN_MASKUPDATE

	MASKD.MaskBeginFunctionCount ++ ;
	if( MASKD.MaskBeginFunctionCount == 1 )
	{
		IMAGEDATA2 *Image2 ;

		// �`���̉摜�f�[�^�A�h���X���擾����
		Image2 = GetGraphData2( GBASE.TargetScreen[ 0 ] ) ;

		// �`��G���A�ŃN���b�s���O
		RectClipping( &Rect, &GBASE.DrawArea ) ;

		// �n�[�h�E�G�A���g�p���邩�ǂ����ŏ����𕪊�
		if( GRA2.ValidHardWare )
		{
			// �n�[�h�E�G�A���g�p����ꍇ

			// �}�X�N���g�p���Ă���Ƃ��̂ݓ��ʂȏ���������
			if( MASKD.MaskValidFlag && MASKD.MaskScreenSurface )
			{
				// �}�X�N���g�p���Ă���ꍇ�͕`�����}�X�N�X�N���[���ɂ���
				UseBackupSurface = MASKD.MaskScreenSurface ;

				// �ŏI�o�͐�̌���
				
				// �`��\�摜���`���̏ꍇ�͂�����A
				// ����ȊO�̏ꍇ�̓T�u�o�b�N�o�b�t�@��
				// �g�p���Ă���ꍇ�̓T�u�o�b�N�o�b�t�@���A
				// �����ł͂Ȃ��ꍇ�̓o�b�N�o�b�t�@���o�͐�ɂ���
				if( Image2 )
				{
					DestTargetSurface = Image2->Orig->Hard.Tex[ 0 ].Surface[ GBASE.TargetScreenSurface[ 0 ] ] ;
				}
				else
				{
					DestTargetSurface = GRH.SubBackBufferSurface ? GRH.SubBackBufferSurface : GRH.BackBufferSurface ;
				}

				// �ŏI�o�͐悩��}�X�N�X�N���[���Ɍ��݂̕`��󋵂��R�s�[����
				GraphicsDevice_StretchRect(
					DestTargetSurface,       &Rect,
					MASKD.MaskScreenSurface, &Rect, D_D3DTEXF_NONE ) ; 
			}
		}
		else
		{
			// �n�[�h�E�G�A���g�p���Ȃ��ꍇ

			// �`��Ώ� MEMIMG ����}�X�N�����p MEMIMG �ɃC���[�W��]��
			if( MASKD.MaskValidFlag )
			{
				BltMemImg(
					&MASKD.MaskDrawMemImg, 
					Image2 ? &Image2->Soft.MemImg : &GRS.MainBufferMemImg,
					&Rect,
					Rect.left,
					Rect.top ) ;
			}
		}
	}

	// �I��
	return 0 ;
}

// �}�X�N���g�p�����`��̌�ɌĂԊ֐�
// ( ���łɃT�u�o�b�t�@���g�p�����`��G���A�@�\���g�p���Ă���ꍇ�̏���������Ă��܂��Ă���� )
extern int MaskDrawAfterFunction( RECT Rect )
{
	MASKD.MaskBeginFunctionCount -- ;
	if( MASKD.MaskBeginFunctionCount == 0 )
	{
		IMAGEDATA2 *Image2 ;

		// �`���̉摜�f�[�^�A�h���X���擾����
		Image2 = GetGraphData2( GBASE.TargetScreen[ 0 ] ) ;

		// �`��G���A�ŃN���b�s���O
		RectClipping( &Rect, &GBASE.DrawArea ) ;
		if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;

		// Direct3D ���g�p���Ă��邩�ǂ����ŏ����𕪊�
		// �n�[�h�E�G�A���g�p���邩�ǂ����ŏ����𕪊�
		if( GRA2.ValidHardWare )
		{
			// �n�[�h�E�G�A���g�p����ꍇ

			// �}�X�N���g�p���Ă���ꍇ�Ƃ��Ȃ��ꍇ�ŏ����𕪊�
			if( MASKD.MaskValidFlag && MASKD.MaskScreenSurface )
			{
#if 1
				int UseZBufferFlag ;
				VERTEX_2D Vert[ 4 ] ;
				DIRECT3DBLENDINFO BlendInfo ;

				// �`����ύX
				SetRenderTargetHardware( DestTargetSurface ) ;
				BeginScene() ;

				// �y�o�b�t�@���g�p���Ȃ��ݒ�ɂ���
				UseZBufferFlag = GRH.EnableZBufferFlag ;
				D_SetUseZBufferFlag( FALSE ) ;

				// �V�F�[�_�[���Z�b�g����Ă�����͂���
				if( GRH.SetVS )
				{
					GraphicsDevice_SetVertexShader( NULL ) ;
					GRH.SetVS = NULL ;
					GRH.SetVD = NULL ;
					GRH.SetFVF = 0 ;
				}
				if( GRH.SetPS )
				{
					GraphicsDevice_SetPixelShader( NULL ) ;
					GRH.SetPS = NULL ;
				}
				GRH.SetIB = NULL ;
				GRH.SetVB = NULL ;

				// �r���[�|�[�g�����ɖ߂�
				GRH.InitializeFlag = TRUE ;
				SetViewportHardwareEasy( GBASE.DrawArea.left, GBASE.DrawArea.top, GBASE.DrawArea.right, GBASE.DrawArea.bottom ) ;
				GRH.InitializeFlag = FALSE ;

				// �}�X�N�A���t�@�`�����l���ƃ}�X�N�J���[�o�b�t�@��Z�����ĕ`��
				BlendInfo.AlphaTestEnable = TRUE ;
				BlendInfo.AlphaRef = 0 ;
		//		BlendInfo.AlphaFunc = MASKD.MaskReverseEffectFlag ? D_D3DCMP_GREATER : D_D3DCMP_LESS ;
				BlendInfo.AlphaFunc = MASKD.MaskReverseEffectFlag ? D_D3DCMP_NOTEQUAL : D_D3DCMP_EQUAL ;
				BlendInfo.AlphaBlendEnable = FALSE ;
				BlendInfo.FactorColor = 0xffffffff ;
				BlendInfo.SrcBlend = -1 ;
				BlendInfo.DestBlend = -1 ;
				BlendInfo.UseTextureStageNum = 3 ;

				BlendInfo.SeparateAlphaBlendEnable = FALSE ;
				BlendInfo.SrcBlendAlpha = -1 ;
				BlendInfo.DestBlendAlpha = -1 ;
				BlendInfo.BlendOpAlpha = -1 ;

				BlendInfo.TextureStageInfo[ 0 ].ResultTempARG = -1 ;
				BlendInfo.TextureStageInfo[ 0 ].Texture = ( void * )MASKD.MaskScreenTexture ;
				BlendInfo.TextureStageInfo[ 0 ].TextureCoordIndex = 0 ;
				BlendInfo.TextureStageInfo[ 0 ].AlphaARG1 = D_D3DTA_CURRENT ;
				BlendInfo.TextureStageInfo[ 0 ].AlphaARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 0 ].AlphaOP = D_D3DTOP_SELECTARG1 ;
				BlendInfo.TextureStageInfo[ 0 ].ColorARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 0 ].ColorARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 0 ].ColorOP = D_D3DTOP_SELECTARG1 ;

				BlendInfo.TextureStageInfo[ 1 ].ResultTempARG = -1 ;
				BlendInfo.TextureStageInfo[ 1 ].Texture = ( void * )MASKD.MaskImageTexture ;
				BlendInfo.TextureStageInfo[ 1 ].TextureCoordIndex = 0 ;
				BlendInfo.TextureStageInfo[ 1 ].AlphaARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 1 ].AlphaARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 1 ].AlphaOP = D_D3DTOP_SELECTARG1 ;
				BlendInfo.TextureStageInfo[ 1 ].ColorARG1 = D_D3DTA_CURRENT ;
				BlendInfo.TextureStageInfo[ 1 ].ColorARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 1 ].ColorOP = D_D3DTOP_SELECTARG1 ;

				BlendInfo.TextureStageInfo[ 2 ].ResultTempARG = -1 ;
				BlendInfo.TextureStageInfo[ 2 ].Texture   = NULL ;
				BlendInfo.TextureStageInfo[ 2 ].TextureCoordIndex = 0 ;
				BlendInfo.TextureStageInfo[ 2 ].ColorARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 2 ].ColorARG2 = D_D3DTA_DIFFUSE ;
				BlendInfo.TextureStageInfo[ 2 ].ColorOP   = D_D3DTOP_DISABLE ;
				BlendInfo.TextureStageInfo[ 2 ].AlphaARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 2 ].AlphaARG2 = D_D3DTA_DIFFUSE ;
				BlendInfo.TextureStageInfo[ 2 ].AlphaOP   = D_D3DTOP_DISABLE ;

				// �u�����h���̕ύX
				D_SetUserBlendInfo( &BlendInfo, FALSE, FALSE ) ;

				// �V�F�[�_�[���g�p����ꍇ�͂����Ńs�N�Z���V�F�[�_�[��ݒ肷��
				if( GRH.UseShader && GRH.NormalDraw_NotUseShader == FALSE )
				{
					GRH.SetPS = GRH.MaskEffectPixelShader ;
					GraphicsDevice_SetPixelShader( GRH.SetPS ) ;
					GRH.NormalPS = FALSE ;
					GRH.DrawPrepAlwaysFlag = TRUE ;
				}

				// �`�揀��
				BeginScene() ;

				// ���_�̏���
				Vert[ 2 ].pos.x = Vert[ 0 ].pos.x = ( float )Rect.left   - 0.5f ;
				Vert[ 1 ].pos.y = Vert[ 0 ].pos.y = ( float )Rect.top    - 0.5f ;

				Vert[ 3 ].pos.x = Vert[ 1 ].pos.x = ( float )Rect.right  - 0.5f ;
				Vert[ 3 ].pos.y = Vert[ 2 ].pos.y = ( float )Rect.bottom - 0.5f ;

				Vert[ 2 ].u = Vert[ 0 ].u = ( float )Rect.left   / ( float )MASKD.MaskTextureSizeX ;
				Vert[ 1 ].v = Vert[ 0 ].v = ( float )Rect.top    / ( float )MASKD.MaskTextureSizeY ;
				Vert[ 3 ].u = Vert[ 1 ].u = ( float )Rect.right  / ( float )MASKD.MaskTextureSizeX ;
				Vert[ 3 ].v = Vert[ 2 ].v = ( float )Rect.bottom / ( float )MASKD.MaskTextureSizeY ;

				Vert[ 0 ].color =
				Vert[ 1 ].color =
				Vert[ 2 ].color =
				Vert[ 3 ].color = 0xffffffff ;

				Vert[ 0 ].pos.z = 
				Vert[ 1 ].pos.z = 
				Vert[ 2 ].pos.z = 
				Vert[ 3 ].pos.z = 0.0f ;

				Vert[ 0 ].rhw = 
				Vert[ 1 ].rhw = 
				Vert[ 2 ].rhw = 
				Vert[ 3 ].rhw = 1.0f ;

				// �`��
				SETFVF( VERTEXFVF_2D ) ;
				GraphicsDevice_DrawPrimitiveUP( D_D3DPT_TRIANGLESTRIP, 2, Vert, sizeof( VERTEX_2D ) ) ;
				EndScene() ;
				BeginScene() ;

				// �u�����h�������ɖ߂�
				SetUserBlendInfo( NULL ) ;

				// �V�F�[�_�[���g�p����ꍇ�͂����Ńs�N�Z���V�F�[�_�[�𖳌��ɂ���
				if( GRH.UseShader && GRH.NormalDraw_NotUseShader == FALSE )
				{
					GRH.SetPS = NULL ;
					GraphicsDevice_SetPixelShader( GRH.SetPS ) ;
				}

				// �y�o�b�t�@�̐ݒ�����ɖ߂�
				D_SetUseZBufferFlag( UseZBufferFlag ) ;

#else
				// �����o��
				RenderVertexHardware() ;
				EndScene() ;

				GraphicsDevice_StretchRect(
					UseBackupSurface,  &Rect,
					DestTargetSurface, &Rect, D_D3DTEXF_NONE ) ;
#endif

				// �`�����}�X�N�T�[�t�F�X�ɂ���
				SetRenderTargetHardware( MASKD.MaskScreenSurface ) ;

				// �r���[�|�[�g�����ɖ߂�
				GRH.InitializeFlag = TRUE ;
				SetViewportHardwareEasy( GBASE.DrawArea.left, GBASE.DrawArea.top, GBASE.DrawArea.right, GBASE.DrawArea.bottom ) ;
				GRH.InitializeFlag = FALSE ;
			}
		}
		else
		{
			// �n�[�h�E�G�A���g�p���Ȃ��ꍇ

			// �}�X�N���|�����Ă��Ȃ�������`���ɓ]������
			if( MASKD.MaskValidFlag )
			{
				BYTE *Dest, *Src, *Mask ;
				int DestAddPitch, SrcAddPitch, MaskAddPitch ;
				int DestWidth, DestHeight ;
				MEMIMG *TargetMemImg ;
				int PixelByte ;
				
				TargetMemImg = Image2 ? &Image2->Soft.MemImg : &GRS.MainBufferMemImg ;
				PixelByte = TargetMemImg->Base->ColorDataP->PixelByte ;

				DestWidth = Rect.right - Rect.left ;
				DestHeight = Rect.bottom - Rect.top ;

				MaskAddPitch = MASKD.MaskBufferPitch            - DestWidth ;
				SrcAddPitch  = MASKD.MaskDrawMemImg.Base->Pitch - PixelByte * DestWidth ;
				DestAddPitch = TargetMemImg->Base->Pitch        - PixelByte * DestWidth ;

				Mask = ( BYTE * )MASKD.MaskBuffer              + Rect.left             + Rect.top * MASKD.MaskBufferPitch ;
				Src  = ( BYTE * )MASKD.MaskDrawMemImg.UseImage + Rect.left * PixelByte + Rect.top * MASKD.MaskDrawMemImg.Base->Pitch ;
				Dest = ( BYTE * )TargetMemImg->UseImage        + Rect.left * PixelByte + Rect.top * TargetMemImg->Base->Pitch ;

				if( MASKD.MaskReverseEffectFlag == TRUE )
				{
#ifdef DX_NON_INLINE_ASM
					int i ;
					// ���ʔ��]��
					switch( PixelByte )
					{
					case 2 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask != 0 )
									*( ( WORD * )Dest ) = *( ( WORD * )Src ) ;

								Dest += 2 ;
								Src += 2 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;

					case 4 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask != 0 )
									*( ( DWORD * )Dest ) = *( ( DWORD * )Src ) ;

								Dest += 4 ;
								Src += 4 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;
					}
#else
					// ���ʔ��]��
					switch( PixelByte )
					{
					case 2 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_2_1:
							MOV		ECX, DestWidth
	LOOP_2_2:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JZ		LABEL_2_1
							MOV		BX, [ ESI ]
							MOV		[ EDI ], BX
	LABEL_2_1:
							ADD		ESI, 2
							ADD		EDI, 2
							INC		EDX
							DEC		ECX
							JNZ		LOOP_2_2

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_2_1
						}
						break ;

					case 4 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_4_1:
							MOV		ECX, DestWidth
	LOOP_4_2:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JZ		LABEL_4_1
							MOV		EBX, [ ESI ]
							MOV		[ EDI ], EBX
	LABEL_4_1:
							ADD		ESI, 4
							ADD		EDI, 4
							INC		EDX
							DEC		ECX
							JNZ		LOOP_4_2

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_4_1
						}
						break ;
					}
#endif
				}
				else
				{
#ifdef DX_NON_INLINE_ASM
					int i ;
					// ���ʔ��]���ĂȂ���
					switch( PixelByte )
					{
					case 2 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask == 0 )
									*( ( WORD * )Dest ) = *( ( WORD * )Src ) ;

								Dest += 2 ;
								Src += 2 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;

					case 4 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask == 0 )
									*( ( DWORD * )Dest ) = *( ( DWORD * )Src ) ;

								Dest += 4 ;
								Src += 4 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;
					}
#else
					// ���ʔ��]���ĂȂ���
					switch( PixelByte )
					{
					case 2 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_2_1R:
							MOV		ECX, DestWidth
	LOOP_2_2R:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JNZ		LABEL_2_1R
							MOV		BX, [ ESI ]
							MOV		[ EDI ], BX
	LABEL_2_1R:
							ADD		ESI, 2
							ADD		EDI, 2
							INC		EDX
							DEC		ECX
							JNZ		LOOP_2_2R

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_2_1R
						}
						break ;

					case 4 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_4_1R:
							MOV		ECX, DestWidth
	LOOP_4_2R:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JNZ		LABEL_4_1R
							MOV		EBX, [ ESI ]
							MOV		[ EDI ], EBX
	LABEL_4_1R:
							ADD		ESI, 4
							ADD		EDI, 4
							INC		EDX
							DEC		ECX
							JNZ		LOOP_4_2R

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_4_1R
						}
						break ;
					}
#endif
				}
			}
		}
	}

	// �I��
	return 0 ;
}

// �}�X�N���Z�b�g����
extern int NS_DrawMask( int x, int y, int MaskHandle, int TransMode )
{
	MASKDATA * Mask ; 
	RECT Rect, MovRect ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	if( MASKD.MaskBuffer == NULL )
		return 0 ;

	// �N���b�s���O����
	SETRECT( Rect, x, y, x + Mask->MaskWidth, y + Mask->MaskHeight ) ;
	RectClipping( &Rect, &GBASE.DrawArea ) ;
	MovRect = Rect ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;
	Rect.left -= x ;
	Rect.right -= x ;
	Rect.top -= y ;
	Rect.bottom -= y ;
	if( Rect.left > 0 ){ x += Rect.left ; }
	if( Rect.top  > 0 ){ y += Rect.top  ; }

	// �]������
	{
		BYTE *Dest, *Src ;
		int DestAddPitch, SrcAddPitch ;
		int DestWidth, DestHeight, DWordNum, BNum ;
#ifdef DX_NON_INLINE_ASM
		int i ;
#endif

		DestWidth = Rect.right - Rect.left ;
		DestHeight = Rect.bottom - Rect.top ;
		DWordNum = DestWidth / 4 ;
		BNum = DestWidth % 4 ;
		Src = Mask->SrcData + Rect.left + ( Rect.top * Mask->SrcDataPitch ) ;
		Dest = MASKD.MaskBuffer + x + ( y * MASKD.MaskBufferPitch ) ;

		SrcAddPitch = Mask->SrcDataPitch - DestWidth ;
		DestAddPitch = MASKD.MaskBufferPitch - DestWidth ;


		switch( TransMode )
		{
		case DX_MASKTRANS_NONE :
			// ���ߐF�Ȃ��]��
#ifdef DX_NON_INLINE_ASM
			do
			{
				i = DestWidth ;
				do
				{
					*Dest = *Src ;
					Dest ++ ;
					Src ++ ;
				}while( -- i != 0 ) ;
				Dest += DestAddPitch ;
				Src += SrcAddPitch ;
			}while( -- DestHeight != 0 ) ;
#else
			__asm{
				CLD
				MOV		EDI, Dest
				MOV		ESI, Src
				MOV		EDX, DestWidth
				MOV		EBX, DestHeight
				MOV		EAX, SrcAddPitch
				MOV		ECX, DestAddPitch
				PUSH	EBP
				MOV		EBP, ECX
			LOOP1_a:
				MOV		ECX, EDX
				REP		MOVSB
				ADD		EDI, EBP
				ADD		ESI, EAX
				DEC		EBX
				JNZ		LOOP1_a
				POP		EBP
			}
#endif
			break ;

		case DX_MASKTRANS_BLACK :
			// �����ߐF
#ifdef DX_NON_INLINE_ASM
			do
			{
				if( DWordNum != 0 )
				{
					i = DWordNum ;
					do
					{
						*( ( DWORD * )Dest ) |= *( ( DWORD * )Src ) ;
						Dest += 4 ;
						Src += 4 ;
					}while( -- i != 0 ) ;
				}

				if( BNum != 0 )
				{
					i = BNum ;
					do
					{
						*Dest |= *Src ;
						Dest ++ ;
						Src ++ ;
					}while( -- i != 0 ) ;
				}

				Dest += DestAddPitch ;
				Src += SrcAddPitch ;
			}while( -- DestHeight != 0 ) ;
#else
			__asm{
				MOV		EDI, Dest
				MOV		ESI, Src
				MOV		EDX, BNum
				SHL		EDX, 30
				OR		EDX, DWordNum
				MOV		EBX, DestHeight

			LOOP1B:
				MOV		ECX, EDX
				AND		ECX, 0x3fffffff // ((1 << 30) - 1)
				CMP		ECX, 0
				JE		R10B

			LOOP2B:
				MOV		EAX, [ESI]
				OR		[EDI], EAX

				ADD		EDI, 4
				ADD		ESI, 4

				DEC		ECX
				JNZ		LOOP2B

			R10B:

				MOV		ECX, EDX
				SHR		ECX, 30
				CMP		ECX, 0
				JE		R11B

			LOOP3B:
				MOV		AL, [ESI]
				OR		[EDI], AL
				INC		ESI
				INC		EDI
				DEC		ECX
				JNZ		LOOP3B

			R11B:

				ADD		EDI, DestAddPitch
				ADD		ESI, SrcAddPitch
				DEC		EBX
				JNZ		LOOP1B
			}
#endif
			break ;

		case DX_MASKTRANS_WHITE :
			// �����ߐF
#ifdef DX_NON_INLINE_ASM
			do
			{
				if( DWordNum != 0 )
				{
					i = DWordNum ;
					do
					{
						*( ( DWORD * )Dest ) &= *( ( DWORD * )Src ) ;
						Dest += 4 ;
						Src += 4 ;
					}while( -- i != 0 ) ;
				}

				if( BNum != 0 )
				{
					i = BNum ;
					do
					{
						*Dest &= *Src ;
						Dest ++ ;
						Src ++ ;
					}while( -- i != 0 ) ;
				}

				Dest += DestAddPitch ;
				Src += SrcAddPitch ;
			}while( -- DestHeight != 0 ) ;
#else
			__asm
			{
				MOV		EDI, Dest
				MOV		ESI, Src
				MOV		EDX, BNum
				SHL		EDX, 30
				OR		EDX, DWordNum
				MOV		EBX, DestHeight

			LOOP1W:
				MOV		ECX, EDX
				AND		ECX, 0x3fffffff // ((1 << 30) - 1)
				CMP		ECX, 0
				JE		R10W

			LOOP2W:
				MOV		EAX, [ESI]
				AND		[EDI], EAX
				ADD		EDI, 4
				ADD		ESI, 4

				DEC		ECX
				JNZ		LOOP2W

			R10W:
				MOV		ECX, EDX
				SHR		ECX, 30
				CMP		ECX, 0
				JE		R11W

			LOOP3W:
				MOV		AL, [ESI]
				AND		[EDI], AL
				INC		ESI
				INC		EDI
				DEC		ECX
				JNZ		LOOP3W

			R11W:
				ADD		EDI, DestAddPitch
				ADD		ESI, SrcAddPitch
				DEC		EBX
				JNZ		LOOP1W
			}
#endif
			break ;
		}
	}

	// �X�V
	if( GRA2.ValidHardWare )
	{
		UpdateMaskImageTexture( MovRect ) ;
	}

	// �I��
	return 0 ;
}

// �w��̃}�X�N����ʂ����ς��ɓW�J����
extern int NS_DrawFillMask( int x1, int y1, int x2, int y2, int MaskHandle )
{
	MASKDATA * Mask ;
	RECT Rect, MovRect ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;


	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	if( MASKD.MaskBuffer == NULL )
		return 0 ;

	if( x1 == x2 || y1 == y2 ) return 0 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// �N���b�s���O����
	SETRECT( Rect, x1, y1, x2, y2 ) ;
	RectClipping( &Rect, &GBASE.DrawArea ) ;
	MovRect = Rect ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;
	Rect.left -= x1 ;
	Rect.right -= x1 ;
	Rect.top -= y1 ;
	Rect.bottom -= y1 ;
	if( Rect.left > 0 ){ x1 += Rect.left ; }
	if( Rect.top  > 0 ){ y1 += Rect.top  ; }

	{
		BYTE *Dest, *Src, *Src2, *Src3 ;
		int DestWidth, DestHeight, SrcWidth, SrcHeight ;
		DWORD SrcAddPitch, DestAddPitch ;
		DWORD FirstX, FirstY ;
		DWORD CounterY ;

		SrcWidth = Mask->MaskWidth ;
		SrcHeight = Mask->MaskHeight ;

		DestWidth = Rect.right - Rect.left ;
		DestHeight = Rect.bottom - Rect.top ;

		FirstX = Mask->MaskWidth - Rect.left % Mask->MaskWidth ;
		FirstY = Mask->MaskHeight - Rect.top % Mask->MaskHeight ;

		Dest = MASKD.MaskBuffer + x1 + y1 * MASKD.MaskBufferPitch ;
		Src = Mask->SrcData + ( Rect.left % Mask->MaskWidth ) + ( Rect.top % Mask->MaskHeight ) * Mask->SrcDataPitch ;
		Src2 = Mask->SrcData ;
		Src3 = Mask->SrcData + ( Rect.left % Mask->MaskWidth ) ;

		SrcAddPitch = Mask->SrcDataPitch - ( ( DestWidth + Rect.left % Mask->MaskWidth ) % Mask->MaskWidth == 0 ? 0 : ( DestWidth + Rect.left % Mask->MaskWidth ) % Mask->MaskWidth ) + Rect.left % Mask->MaskWidth ;
		DestAddPitch = MASKD.MaskBufferPitch - DestWidth ;

		CounterY = FirstY ;

#ifdef DX_NON_INLINE_ASM
		int i, TempWidth, TempHeight ;

		TempHeight = FirstY ;
		do
		{
			TempWidth = FirstX ;
			i = DestWidth ;
			do
			{
				*Dest = *Src ;
				Dest ++ ;
				Src ++ ;
				if( -- TempWidth == 0 )
				{
					TempWidth = SrcWidth ;
					Src -= SrcWidth ;
				}
			}while( -- i != 0 ) ;

			Dest += DestAddPitch ;
			Src += SrcAddPitch ;

			if( -- TempHeight == 0 )
			{
				Src = Src3 ;
				TempHeight = SrcHeight ;
			}

		}while( -- DestHeight != 0 ) ;
#else
		__asm{
			CLD
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		EAX, SrcWidth
			MOV		EBX, FirstY
		LOOP1:
			MOV		ECX, FirstX
			MOV		EDX, DestWidth
		LOOP2:
			MOVSB
			DEC		ECX
			JZ		R2
		R1:
			DEC		EDX
			JZ		RE
			JMP		LOOP2

		R2:
			MOV		ECX, EAX
			SUB		ESI, ECX
			JMP		R1

		RE:
			ADD		ESI, SrcAddPitch
			ADD		EDI, DestAddPitch
			DEC		DestHeight
			JZ		RE2
			DEC		EBX
			JZ		R3
			JMP		LOOP1

		R3:
			MOV		ESI, Src3
			MOV		ECX, SrcHeight
			MOV		EBX, ECX
			JMP		LOOP1

		RE2:
		}
#endif
	}

	if( GRA2.ValidHardWare )
	{
		UpdateMaskImageTexture( MovRect ) ;
	}

	// �I��
	return 0 ;
}

// �}�X�N�̃Z�b�g
extern int NS_DrawMaskToDirectData( int x, int y, int Width, int Height, const void *MaskData, int TransMode )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �}�X�N�X�N���[�����Ȃ��ꍇ�͂����ŏ������I��
	if( MASKD.MaskBuffer == NULL ) return 0 ;

	// �}�X�N�f�[�^���Ȃ������ꍇ�͏������I��
	if( MaskData == NULL ) return 0 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// �}�X�N�X�N���[���Ƀ}�X�N��W�J
	DrawMaskToDirectData_Base( x, y, MASKD.MaskBuffer, MASKD.MaskBufferPitch,
							Width, Height, MaskData, TransMode ) ;

	RECT Rect ;

	if( GRA2.ValidHardWare )
	{
		// �X�V
		Rect.left = x ;
		Rect.top = y ;
		Rect.right = x + Width ;
		Rect.bottom = y + Height ;
		UpdateMaskImageTexture( Rect ) ;
	}

	// �I��
	return 0 ;
}

// �}�X�N�X�N���[�����w��̐F�œh��Ԃ�
extern int NS_FillMaskScreen( int Flag )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// �}�X�N�X�N���[�����Ȃ��ꍇ�͂����ŏ������I��
	if( MASKD.MaskBuffer == NULL ) return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// �}�X�N�X�N���[�����N���A����
	_MEMSET( MASKD.MaskBuffer, Flag ? 0xff : 0, MASKD.MaskBufferPitch * GBASE.DrawSizeY ) ;

	if( GRA2.ValidHardWare )
	{
		D_D3DLOCKED_RECT LockRect ;
		int i, Width = 0 ;
		BYTE *Dest ;

		if( MASKD.MaskImageTexture->LockRect( 0, &LockRect, NULL, 0 ) == D_D3D_OK )
		{
			Flag = Flag ? 0xff : 0 ;

			switch( GRH.MaskAlphaFormat )
			{
			case D_D3DFMT_A8R8G8B8 : Width = GRA2.MainScreenSizeX * 4 ; break ;
			case D_D3DFMT_A4R4G4B4 : Width = GRA2.MainScreenSizeX * 2 ; break ;
			case D_D3DFMT_A1R5G5B5 : Width = GRA2.MainScreenSizeX * 2 ; break ;
			}

			Dest = ( BYTE * )LockRect.pBits ;
			for( i = 0 ; i < GRA2.MainScreenSizeY ; i ++, Dest += LockRect.Pitch )
			{
				_MEMSET( Dest, ( unsigned char )Flag, Width ) ;
			}

			MASKD.MaskImageTexture->UnlockRect( 0 ) ;
		}
	}

	// �I��
	return 0 ;
}












// DirectX �o�[�W�����ˑ�������֐�

// DirectX9 �p

// �w��̈�̃}�X�N�C���[�W�e�N�X�`�����X�V����
static int UpdateMaskImageTexture( RECT Rect )
{
	D_D3DLOCKED_RECT LockRect ;
	int Width, Height ;
	BYTE *Dest, *Src ;
	DWORD DestAdd, SrcAdd ;
#ifdef DX_NON_INLINE_ASM
	int i ;
#endif

	RectClipping( &Rect, &GBASE.DrawArea ) ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;

	Width = Rect.right - Rect.left ;
	Height = Rect.bottom - Rect.top ;

	// �}�X�N�C���[�W�e�N�X�`�������b�N����
	if( MASKD.MaskImageTexture->LockRect( 0, &LockRect, &Rect, 0 ) != D_D3D_OK )
		return -1 ;

	// �]�����̏���
	Src = MASKD.MaskBuffer + MASKD.MaskBufferPitch * Rect.top + Rect.left ;
	SrcAdd = MASKD.MaskBufferPitch - Width ;

	// �w�蕔���C���[�W�̓]��
	Dest = ( BYTE * )LockRect.pBits ;
	switch( GRH.MaskAlphaFormat )
	{
	case D_D3DFMT_A8R8G8B8 :
		DestAdd = LockRect.Pitch - Width * 4 ;
#ifdef DX_NON_INLINE_ASM
		do
		{
			i = Width ;
			do
			{
				Dest[ 3 ] = *Src ;
				Dest += 4 ;
				Src ++ ;
			}while( -- i != 0 ) ;
			Dest += DestAdd ;
			Src += SrcAdd ;
		}while( -- Height != 0 ) ;
#else
		__asm
		{
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		ECX, Height
LOOP_A8R8G8B8_1:
			MOV		EDX, Width
LOOP_A8R8G8B8_2:
			MOV		AL, [ ESI ]
			MOV		[ EDI + 3 ], AL
			INC		ESI
			ADD		EDI, 4
			DEC		EDX
			JNZ		LOOP_A8R8G8B8_2

			ADD		EDI, DestAdd
			ADD		ESI, SrcAdd
			DEC		Height
			JNZ		LOOP_A8R8G8B8_1
		}
#endif
		break ;

	case D_D3DFMT_A1R5G5B5 :
	case D_D3DFMT_A4R4G4B4 :
		DestAdd = LockRect.Pitch - Width * 2 ;
#ifdef DX_NON_INLINE_ASM
		do
		{
			i = Width ;
			do
			{
				Dest[ 1 ] = *Src ;
				Dest += 2 ;
				Src ++ ;
			}while( -- i != 0 ) ;
			Dest += DestAdd ;
			Src += SrcAdd ;
		}while( -- Height != 0 ) ;
#else
		__asm
		{
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		ECX, Height
LOOP_A1R5G5B5_1:
			MOV		EDX, Width
LOOP_A1R5G5B5_2:
			MOV		AL, [ ESI ]
			MOV		[ EDI + 1 ], AL
			INC		ESI
			ADD		EDI, 2
			DEC		EDX
			JNZ		LOOP_A1R5G5B5_2

			ADD		EDI, DestAdd
			ADD		ESI, SrcAdd
			DEC		Height
			JNZ		LOOP_A1R5G5B5_1
		}
#endif
		break ;
	}

	// ���b�N����������
	MASKD.MaskImageTexture->UnlockRect( 0 ) ;

	// �I��
	return 0 ;
}

}

#endif


