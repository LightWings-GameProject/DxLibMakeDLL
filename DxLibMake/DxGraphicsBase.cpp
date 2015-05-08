// ----------------------------------------------------------------------------
// 
// 		�c�w���C�u����		���ʕ`��v���O����
// 
// 				Ver 3.12a
// 
// ----------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h----------------------------------------------------------------
#include "DxGraphicsBase.h"
#include "DxFile.h"

#include "DxMath.h"
#include "DxMask.h"
#include "DxBaseImage.h"
#include "DxSoftImage.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxMemory.h"
#include "DxUseCLib.h"
#include "DxGraphics.h"
#include "DxASyncLoad.h"
#include "DxLog.h"

#ifdef __WINDOWS__
#include "Windows/DxWindow.h"
#endif // __WINDOWS__

namespace DxLib
{

// �}�N����`------------------------------------------------------------------

// �N���b�s���O�t���O
#define CLIP_XP							(0x01)				// X+�����ɃN���b�v
#define CLIP_XM							(0x02)				// X-�����ɃN���b�v
#define CLIP_YP							(0x04)				// Y+�����ɃN���b�v
#define CLIP_YM							(0x08)				// Y-�����ɃN���b�v
#define CLIP_ZP							(0x10)				// Z+�����ɃN���b�v
#define CLIP_ZM							(0x20)				// Z-�����ɃN���b�v

// ���C�g�n���h���̗L�����`�F�b�N
#define LIGHTCHK( HAND, LPOINT, ERR )										\
	if( ( ( (HAND) & DX_HANDLEERROR_MASK ) != 0 ) ||											\
		( ( (HAND) & DX_HANDLETYPE_MASK ) != DX_HANDLETYPE_MASK_LIGHT ) ||							\
		( ( (HAND) & DX_HANDLEINDEX_MASK ) >= MAX_LIGHT_NUM ) ||								\
		( ( LPOINT = GBASE.Light.Data[ (HAND) & DX_HANDLEINDEX_MASK ] ) == NULL ) ||				\
		( (int)( (LPOINT)->ID << DX_HANDLECHECK_ADDRESS ) != ( (HAND) & DX_HANDLECHECK_MASK ) ) )\
	{																							\
		return (ERR) ;																			\
	}

// �ǂݍ��ݎ��̓��ߐF�ύX����
#define LUTRANS_START( Img )	\
	int _TransColor = GBASE.TransColor ; \
	if( GBASE.LeftUpColorIsTransColorFlag == TRUE )\
	{\
		GBASE.TransColor = NS_GetGraphImageFullColorCode( Img, 0, 0 ) & 0xffffff ;\
		BASEIM.TransColor = GBASE.TransColor ;\
	}

// �ǂݍ��ݎ��̓��ߐF�ύX����
#define LUTRANS_GPARAM( GParam, Img )	\
	if( GParam->LeftUpColorIsTransColorFlag == TRUE ) GParam->InitHandleGParam.TransColor = NS_GetGraphImageFullColorCode( Img, 0, 0 ) & 0xffffff ;

#define LUTRANS_RE_START( Img, GrH )		\
	LUTRANS_START( Img ) \

#define LUTRANS_RE_GPARAM( GParam, Img, GrH )		\
	LUTRANS_GPARAM( GParam, Img ) \

#define LUTRANS_RE_DIV_START( Img, GrH, Num )	\
	LUTRANS_START( Img ) \

#define LUTRANS_RE_DIV_GPARAM( GParam, Img, GrH, Num )	\
	LUTRANS_GPARAM( GParam, Img ) \

#define LUTRANS_END		\
	GBASE.TransColor = _TransColor ;	\
	BASEIM.TransColor = _TransColor ;

// �f�[�^�^�錾----------------------------------------------------------------

// �f�[�^�錾------------------------------------------------------------------

// �`�����̊�{�I�ȏ��
GRAPHICSBASEDATA GraphicsBaseData ;

// �֐��錾--------------------------------------------------------------------

// ���C�g�n���h���̒ǉ�
static int AddLightHandle( void ) ;

// �r���[�s�񂩂琅���A�����A�P��p�x���Z�o����
static void ViewMatrixToCameraRoll( void ) ;

// �v���O����------------------------------------------------------------------

// �O���t�B�b�N�`���{�f�[�^�̏�����
extern int InitializeGraphicsBase( void )
{
	void (*P)(void )                           = GBASE.GraphRestoreShred ;
	int NotDrawFlag                            = GBASE.NotDrawFlag ;
	int Emulation320x240Flag                   = GBASE.Emulation320x240Flag ;
	int NotUseBasicGraphDraw3DDeviceMethodFlag = GBASE.NotUseBasicGraphDraw3DDeviceMethodFlag ;
	int NotWaitVSyncFlag                       = GBASE.NotWaitVSyncFlag ;
	int PreSetWaitVSyncFlag                    = GBASE.PreSetWaitVSyncFlag ;
	int NotGraphBaseDataBackupFlag             = GBASE.NotGraphBaseDataBackupFlag ;
	int NotUseManagedTextureFlag               = GBASE.NotUseManagedTextureFlag ;
	LIGHTBASEDATA Light                        = GBASE.Light ;
	int i ;

	// �[��������
	_MEMSET( &GBASE, 0, sizeof( GBASE ) ) ;

	GBASE.Emulation320x240Flag                   = Emulation320x240Flag ;
	GBASE.BlendGraph                             = -1 ;
	GBASE.GraphRestoreShred                      = P ;
	GBASE.NotDrawFlag						     = NotDrawFlag ;
	GBASE.NotUseBasicGraphDraw3DDeviceMethodFlag = NotUseBasicGraphDraw3DDeviceMethodFlag ;
	GBASE.NotUseManagedTextureFlag               = NotUseManagedTextureFlag ;
	GBASE.NotWaitVSyncFlag                       = NotWaitVSyncFlag ;
	GBASE.PreSetWaitVSyncFlag                    = PreSetWaitVSyncFlag ;
	GBASE.NotGraphBaseDataBackupFlag             = NotGraphBaseDataBackupFlag ;
//	GRAPHICS.MainScreenSizeX                     = MainScreenSizeX ;
//	GRAPHICS.MainScreenSizeY                     = MainScreenSizeY ;
	GBASE.Light                                  = Light ;

	// �e�N�X�`���A�h���X���[�h�̏����ݒ�
	for( i = 0 ; i < USE_TEXTURESTAGE_NUM ; i ++ )
	{
		if( GBASE.TexAddressModeU[ i ] == 0 )
			GBASE.TexAddressModeU[ i ] = D_D3DTADDRESS_CLAMP ;
		if( GBASE.TexAddressModeV[ i ] == 0 )
			GBASE.TexAddressModeV[ i ] = D_D3DTADDRESS_CLAMP ;
		if( GBASE.TexAddressModeW[ i ] == 0 )
			GBASE.TexAddressModeW[ i ] = D_D3DTADDRESS_CLAMP ;
	}

	// �y�o�b�t�@�̏����ݒ�
	GBASE.EnableZBufferFlag2D = FALSE ;
	GBASE.WriteZBufferFlag2D  = FALSE ;
	GBASE.EnableZBufferFlag3D = FALSE ;
	GBASE.WriteZBufferFlag3D  = FALSE ;

	// ���C�g�̐ݒ�� Direct3D �ɔ��f����悤�Ƀt���O���Z�b�g
	GBASE.Light.D3DChange = TRUE ;

	// DrawPreparation ���ĂԂׂ��t���O�𗧂Ă�
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �`��y�l���Z�b�g
	GBASE.DrawZ = 0.2f;

	// �����y��r���[�h���Z�b�g
	GBASE.ZBufferCmpType2D = DX_CMP_LESSEQUAL;
	GBASE.ZBufferCmpType3D = DX_CMP_LESSEQUAL;

	// �����̓e�N�X�`���T�[�t�F�X���쐬���������
	GBASE.TextureImageCreateFlag = TRUE ;

	// ���ߐF�ɃA���t�@�e�X�g���g�p����
	GBASE.AlphaTestImageCreateFlag = TRUE ;
	BASEIM.AlphaTestImageCreateFlag = TRUE ;

	// �`�惂�[�h���l�A���X�g�l�C�o�[�ɃZ�b�g
	GBASE.DrawMode = DX_DRAWMODE_NEAREST ;

	// �ő�ٕ������P�ɃZ�b�g
	GBASE.MaxAnisotropy = 1 ;

	// �t�B�����[�h�̓|���S���`��
	GBASE.FillMode = D_D3DFILL_SOLID ;

	// �u�����h���[�h�����u�����h�ɃZ�b�g
	GBASE.BlendMode = DX_BLENDMODE_NOBLEND ;

	// �A���t�@�e�X�g���[�h�̓f�t�H���g����
	GBASE.AlphaTestMode = -1 ;

	// �`��P�x���Z�b�g
	NS_SetDrawBright( 255, 255, 255 ) ;
//	GBASE.bDrawBright = 0xffffff ;

	// �t�H�O�̏����ݒ�
	NS_SetFogMode( DX_FOGMODE_LINEAR ) ;
	NS_SetFogColor( 255, 255, 255 ) ;
	NS_SetFogStartEnd( 0.0f, 1500.0f ) ;

	// �g�p����s�N�Z���t�H�[�}�b�g���Z�b�g
//	SetCreateGraphColorBitDepth( COLORBITDEPTH ) ;
	NS_SetCreateGraphColorBitDepth( 0 ) ;

#ifndef DX_NON_MASK
	// �}�X�N�͎g�p����ɃZ�b�g
	MASKD.MaskUseFlag = TRUE ;
	MASKD.MaskValidFlag = FALSE ;
#endif

	// �`���X�N���[���̈���Z�b�g
	GBASE.TargetScreen[ 0 ] = DX_SCREEN_BACK ;
	GBASE.TargetScreenSurface[ 0 ] = 0 ;
	NS_SetDrawScreen( DX_SCREEN_FRONT ) ;

	// �`��\�̈��ݒ�
	{
		int Width, Height ;

		NS_GetGraphSize( GBASE.TargetScreen[ 0 ], &Width, &Height ) ;
		NS_SetDrawArea( 0, 0, Width, Height ) ;
	}

	// �W�������u�����h�ɂ���
//	NS_SetDrawBlendMode( DX_BLENDMODE_ALPHA, 255 ) ;
	
	// �`����\�ȏ�Ԃɂ���
	GBASE.NotDrawFlag = FALSE ;

	// �f�t�H���g�̃��C�g���쐬
	{
		VECTOR Dir ;

		Dir.x =  1.0f ;
		Dir.y = -1.0f ;
		Dir.z =  1.0f ;
		GBASE.Light.DefaultHandle = NS_CreateDirLightHandle( Dir ) ;
	}

	// �ˉe�s��̐ݒ��������
	GBASE.ProjectionMatrixMode = 0 ;
	GBASE.ProjNear = 10.0f ;
	GBASE.ProjFar = 10000.0f ;
	GBASE.ProjDotAspect = 1.0f ;
	GBASE.ProjFov = DEFAULT_FOV ;
	GBASE.ProjSize = 1.0f ;
	CreateIdentityMatrix( &GBASE.ProjMatrix ) ;

	// �}�e���A���̏����l���Z�b�g
	GBASE.Light.ChangeMaterial    = 1 ;
	GBASE.Light.Material.Diffuse  = GetColorF( 0.8f, 0.8f, 0.8f, 1.0f ) ;
	GBASE.Light.Material.Specular = GetColorF( 0.8f, 0.8f, 0.8f, 0.0f ) ;
	GBASE.Light.Material.Ambient  = GetColorF( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	GBASE.Light.Material.Emissive = GetColorF( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	GBASE.Light.Material.Power    = 20.0f ;

	// �R�c�`��X�P�[���l��������
	GBASE.Draw3DScale = 1.0f ;

	// �K�� DrawPreparation �����s����t���O�𗧂Ă�
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �O���t�B�b�N�n���h���쐬�n

// GraphImage �f�[�^����T�C�Y������o���A����ɍ������O���t�B�b�N�n���h�����쐬����
extern int NS_CreateDXGraph( const BASEIMAGE *RgbImage, const BASEIMAGE *AlphaImage, int TextureFlag )
{
	SETUP_GRAPHHANDLE_GPARAM GParam ;
	int NewGraph ;

	CheckActiveState() ;

	// �n���h���̍쐬
	NewGraph = AddGraphHandle() ;
	if( NewGraph == -1 ) return -1 ;

	InitSetupGraphHandleGParam( &GParam ) ;

	if( CreateDXGraph_UseGParam( &GParam, NewGraph, RgbImage, AlphaImage, TextureFlag ) == -1 )
	{
		NS_DeleteGraph( NewGraph ) ;
		return -1 ;
	}

	return NewGraph ;
}

// LoadBmpToGraph �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int LoadBmpToGraph_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const TCHAR *GraphName,
	int TextureFlag,
	int ReverseFlag,
	int SurfaceMode,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.FileName = GraphName ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = SurfaceMode ;

	return CreateGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// �摜��ǂ݂���
extern int NS_LoadBmpToGraph( const TCHAR *FileName, int TextureFlag, int ReverseFlag, int SurfaceMode )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return LoadBmpToGraph_UseGParam( &GParam, FALSE, -1, FileName, TextureFlag, ReverseFlag, SurfaceMode, GetASyncLoadFlag() ) ;
}

// �O���t�B�b�N�f�[�^�ւ̃O���t�B�b�N�̓ǂݍ���
extern int NS_ReloadGraph( const TCHAR *FileName, int GrHandle, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return LoadBmpToGraph_UseGParam( &GParam, TRUE, GrHandle, FileName, TRUE, ReverseFlag, DX_MOVIESURFACE_NORMAL, GetASyncLoadFlag() ) ;
}

// �a�l�o�t�@�C���̃������ւ̓ǂ݂��݁i�t���O��)
extern int NS_LoadGraph( const TCHAR *GraphName, int NotUse3DFlag )
{
	return NS_LoadBmpToGraph( GraphName, !NotUse3DFlag, FALSE ) ;
}

// �a�l�o�t�@�C���̃������ւ̔��]�ǂ݂��݁i�t���O��)
extern int NS_LoadReverseGraph( const TCHAR *GraphName, int NotUse3DFlag )
{
	return NS_LoadBmpToGraph( GraphName, !NotUse3DFlag, TRUE ) ;
}

// �a�l�o�̕����ǂ݂��݁i�t���O���j
extern int NS_LoadDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf, int NotUse3DFlag )
{
	return NS_LoadDivBmpToGraph( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, !NotUse3DFlag, FALSE ) ;
}

// LoadDivBmpToGraph �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int LoadDivBmpToGraph_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const TCHAR *FileName,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag	
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.FileName = FileName ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return CreateDivGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// �a�l�o�̕����ǂ݂���
extern int NS_LoadDivBmpToGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return LoadDivBmpToGraph_UseGParam( &GParam, FALSE, FileName, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// �O���t�B�b�N�f�[�^�ւ̃O���t�B�b�N�̕����ǂݍ���
extern int NS_ReloadDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, const int *HandleBuf, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return LoadDivBmpToGraph_UseGParam( &GParam, TRUE, FileName, AllNum, XNum, YNum, XSize, YSize, ( int * )HandleBuf, TRUE, ReverseFlag, GetASyncLoadFlag() ) ;
}

// �a�l�o�̔��]�����ǂ݂��݁i�t���O���j
extern int NS_LoadReverseDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf, int NotUse3DFlag )
{
	return NS_LoadDivBmpToGraph( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, !NotUse3DFlag, TRUE ) ;
}

// �摜�t�@�C������u�����h�p�摜��ǂݍ���
extern int NS_LoadBlendGraph( const TCHAR *FileName )
{
	int Result, UseBlendGraphFlag ;

	UseBlendGraphFlag = NS_GetUseBlendGraphCreateFlag() ;
	NS_SetUseBlendGraphCreateFlag( TRUE ) ;
	
	Result = NS_LoadGraph( FileName ) ;
	
	NS_SetUseBlendGraphCreateFlag( UseBlendGraphFlag ) ;
	
	return Result ;
}

#ifdef __WINDOWS__

// ���\�[�X����a�l�o�t�@�C����ǂݍ���
extern int NS_LoadGraphToResource( int ResourceID )
{
	BITMAPINFO *BmpInfo ;
	void *GraphData ;
	int NewHandle ;

	// ���\�[�X����摜�n���h���쐬�ɕK�v�ȃf�[�^���擾����
	if( GetBmpImageToResource( ResourceID, &BmpInfo, &GraphData ) == -1 )
		return -1 ;

	// �O���t�B�b�N�̍쐬
	NewHandle = NS_CreateGraphFromBmp( BmpInfo, GraphData ) ;

	// �������̉��
	DXFREE( BmpInfo ) ;
	DXFREE( GraphData ) ;

	// �n���h����Ԃ�
	return NewHandle ;
}

// ���\�[�X����摜�f�[�^�𕪊��ǂݍ��݂���
extern int NS_LoadDivGraphToResource( int ResourceID, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf )
{
	BITMAPINFO *BmpInfo ;
	void *GraphData ;

	// ���\�[�X����摜�n���h���쐬�ɕK�v�ȃf�[�^���擾����
	if( GetBmpImageToResource( ResourceID, &BmpInfo, &GraphData ) == -1 )
		return -1 ;

	// �O���t�B�b�N�̍쐬
	NS_CreateDivGraphFromBmp( BmpInfo, GraphData, AllNum, XNum, YNum, XSize, YSize, HandleBuf ) ;

	// �������̉��
	DXFREE( BmpInfo ) ;
	DXFREE( GraphData ) ;

	// ����
	return 0 ;
}


// ���\�[�X����O���t�B�b�N�f�[�^��ǂݍ���
extern int NS_LoadGraphToResource( const TCHAR *ResourceName, const TCHAR *ResourceType )
{
	void *Image ;
	int ImageSize ;
	int Result ;

	// ���\�[�X�̏����擾
	if( NS_GetResourceInfo( ResourceName, ResourceType, &Image, &ImageSize ) < 0 ) return -1 ;

	// �n���h���̍쐬
	Result = NS_CreateGraphFromMem( Image, ImageSize ) ;

	return Result ;
}

// ���\�[�X����O���t�B�b�N�f�[�^�𕪊��ǂݍ��݂���
extern int NS_LoadDivGraphToResource( const TCHAR *ResourceName, const TCHAR *ResourceType, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf )
{
	void *Image ;
	int ImageSize ;
	int Result ;

	// ���\�[�X�̏����擾
	if( NS_GetResourceInfo( ResourceName, ResourceType, &Image, &ImageSize ) < 0 ) return -1 ;

	// �n���h���̍쐬
	Result = NS_CreateDivGraphFromMem( Image, ImageSize, AllNum, XNum, YNum, XSize, YSize, HandleBuf ) ;

	return Result ;
}

#endif // __WINDOWS__



// �摜�ǂݍ��ݏ����ɕK�v�ȕϐ���Z�߂�����
struct CREATEGRAPH_LOADBASEIMAGE_PARAM
{
	int						UseTempBaseImage ;
	int						LoadHr ;
	BASEIMAGE				TempRgbBaseImage ;
	BASEIMAGE				TempAlphaBaseImage ;
	BASEIMAGE *				UseRgbBaseImage ;
	BASEIMAGE *				UseAlphaBaseImage ;
} ;

// CreateGraph �� CreateDivGraph �̋��ʂ��� BASEIMAGE �\�z�������֐�����������
static void CreateGraph_LoadBaseImage(
	 LOADGRAPH_PARAM *Param,
	 CREATEGRAPH_LOADBASEIMAGE_PARAM *LParam
 )
{
	LParam->UseTempBaseImage = FALSE ;
	LParam->LoadHr = -1 ;
	LParam->UseRgbBaseImage = NULL ;
	LParam->UseAlphaBaseImage = NULL ;
	LParam->TempRgbBaseImage.GraphData = NULL ;
	LParam->TempAlphaBaseImage.GraphData = NULL ;

	if( Param->FileName != NULL )
	{
		// �t�@�C������ǂݍ��ޏꍇ
		LParam->LoadHr = CreateGraphImage_plus_Alpha_UseGParam(
					&Param->GParam.LoadBaseImageGParam,
					Param->FileName,
					NULL,
					0,
					LOADIMAGE_TYPE_FILE,
					NULL,
					0,
					LOADIMAGE_TYPE_FILE,
					&LParam->TempRgbBaseImage,
					&LParam->TempAlphaBaseImage,
					FALSE
				) ;
		LParam->UseRgbBaseImage = &LParam->TempRgbBaseImage ;
		if( LParam->LoadHr == 0 ) LParam->UseAlphaBaseImage = &LParam->TempAlphaBaseImage ;
		LParam->UseTempBaseImage = TRUE ;
	}
	else
	if( Param->RgbMemImage != NULL )
	{
		// ��������̃t�@�C���C���[�W����ǂݍ��ޏꍇ
		LParam->LoadHr = CreateGraphImage_plus_Alpha_UseGParam(
					&Param->GParam.LoadBaseImageGParam,
					NULL,
					Param->RgbMemImage,
					Param->RgbMemImageSize,
					LOADIMAGE_TYPE_MEM,
					Param->AlphaMemImage,
					Param->AlphaMemImageSize,
					LOADIMAGE_TYPE_MEM,
					&LParam->TempRgbBaseImage,
					&LParam->TempAlphaBaseImage,
					FALSE
				) ;
		LParam->UseRgbBaseImage = &LParam->TempRgbBaseImage ;
		if( LParam->LoadHr == 0 ) LParam->UseAlphaBaseImage = &LParam->TempAlphaBaseImage ;
		LParam->UseTempBaseImage = TRUE ;
	}
#ifdef __WINDOWS__
	else
	if( Param->RgbBmpInfo != NULL )
	{
		int RgbCopyHr ;
		int AlphaCopyHr ;

		// BMP ����ǂݍ��ޏꍇ
		RgbCopyHr = NS_ConvBitmapToGraphImage( Param->RgbBmpInfo, Param->RgbBmpImage, &LParam->TempRgbBaseImage, TRUE ) ;
		if( RgbCopyHr == -1 )
			return ;

		if( Param->AlphaBmpInfo != NULL && Param->AlphaBmpImage != NULL )
		{
			AlphaCopyHr = NS_ConvBitmapToGraphImage( Param->AlphaBmpInfo, Param->AlphaBmpImage, &LParam->TempAlphaBaseImage, TRUE ) ;
			if( AlphaCopyHr == -1 )
			{
				if( RgbCopyHr == 1 ) NS_ReleaseGraphImage( &LParam->TempRgbBaseImage ) ;
				return ;
			}
			LParam->UseAlphaBaseImage = &LParam->TempAlphaBaseImage ;
		}
		else
		{
			_MEMSET( &LParam->TempAlphaBaseImage, 0, sizeof( LParam->TempAlphaBaseImage ) ) ;
		}
		LParam->UseRgbBaseImage = &LParam->TempRgbBaseImage ;
		LParam->UseTempBaseImage = TRUE ;

		LParam->LoadHr = 1 ;
	}
#endif // __WINDOWS__
	else
	if( Param->RgbBaseImage != NULL )
	{
		// ��{�C���[�W����ǂݍ��ޏꍇ
		LParam->UseRgbBaseImage = Param->RgbBaseImage ;
		LParam->UseAlphaBaseImage = Param->AlphaBaseImage ;

		LParam->LoadHr = 1 ;
	}

	if( LParam->LoadHr >= 0 )
	{
		// ���]�t���O�������Ă�����C���[�W�𔽓]����
		if( Param->ReverseFlag == TRUE )
		{
			NS_ReverseGraphImage( LParam->UseRgbBaseImage ) ;
			if( LParam->UseAlphaBaseImage != NULL ) NS_ReverseGraphImage( LParam->UseAlphaBaseImage ) ;
		}
	}
}

// CreateGraph �� CreateDivGraph �̋��ʂ��� BASEIMAGE ��n���������֐�����������
static void CreateGraph_TerminateBaseImage(
	LOADGRAPH_PARAM *Param,
	CREATEGRAPH_LOADBASEIMAGE_PARAM *LParam
)
{
	// �ǂݍ��񂾃O���t�B�b�N�̌�n��������
	if( LParam->UseTempBaseImage )
	{
		NS_ReleaseGraphImage( &LParam->TempRgbBaseImage ) ;
		NS_ReleaseGraphImage( &LParam->TempAlphaBaseImage ) ;
	}
	else
	{
		// ���]�t���O�������Ă�����C���[�W�����ɖ߂�
		if( Param->ReverseFlag == TRUE )
		{
			NS_ReverseGraphImage( LParam->UseRgbBaseImage ) ;
			if( LParam->UseAlphaBaseImage != NULL ) NS_ReverseGraphImage( LParam->UseAlphaBaseImage ) ;
		}
	}
}

// CreateDivGraph �̎������֐�
static int CreateDivGraph_Static(
	LOADGRAPH_PARAM *Param,
	int ASyncThread
)
{
	int Result ;
	int i ;
	CREATEGRAPH_LOADBASEIMAGE_PARAM LParam ;

	// �摜�f�[�^�̓ǂݍ���
	CreateGraph_LoadBaseImage( Param, &LParam ) ;
	if( LParam.LoadHr == -1 )
	{
		if( Param->FileName != NULL )
		{
			TCHAR FullPath[ 512 ] ;

			ConvertFullPathT_( Param->FileName, FullPath ) ;
			DXST_ERRORLOGFMT_ADD(( _T( "�摜�t�@�C�� %s �̃��[�h�Ɏ��s���܂���" ), FullPath )) ;
		}
		return -1 ;
	}

	// �����O���t�B�b�N�n���h�����쐬����
	Result = CreateDivGraphFromGraphImageBase_UseGParam(
				&Param->GParam.CreateGraphGParam,
				Param->ReCreateFlag,
				Param->BaseHandle,
				LParam.UseRgbBaseImage,
				LParam.UseAlphaBaseImage,
				Param->AllNum,
				Param->XNum,
				Param->YNum,
				Param->SizeX,
				Param->SizeY,
				Param->HandleBuf,
				Param->TextureFlag,
				Param->ReverseFlag,
				ASyncThread
			) ;

	// �t�@�C���p�X���Z�b�g����
	if( Result == 0 )
	{
		SetGraphBaseInfo_UseGParam(
			&Param->GParam.SetGraphBaseInfoGParam,
			Param->HandleBuf[ 0 ],
			Param->FileName,
			NULL,
			NULL,
			NULL,
			Param->RgbMemImage,
			Param->RgbMemImageSize,
			Param->AlphaMemImage,
			Param->AlphaMemImageSize,
			LParam.UseRgbBaseImage,
			LParam.UseAlphaBaseImage,
			Param->ReverseFlag,
			-1,
			ASyncThread
		) ;
		for( i = 1 ; i < Param->AllNum ; i ++ )
		{
			SetGraphBaseInfo_UseGParam(
				&Param->GParam.SetGraphBaseInfoGParam,
				Param->HandleBuf[ i ],
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				0,
				NULL,
				0,
				NULL,
				NULL,
				FALSE,
				Param->HandleBuf[ 0 ],
				ASyncThread
			) ;
		}
	}

	// �ǂݍ��񂾃O���t�B�b�N�̌�n��������
	CreateGraph_TerminateBaseImage( Param, &LParam ) ;

	// �I��
	return Result ;
}

#ifndef DX_NON_ASYNCLOAD

// CreateDivGraph �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void CreateDivGraph_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADGRAPH_PARAM *Param ;
	int Addr ;
	int i ;
	int Result ;

	Addr = 0 ;
	Param = ( LOADGRAPH_PARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	Param->HandleBuf = ( int * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	if( Param->FileName != NULL )
	{
		Param->FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	}

	if( Param->RgbBaseImage != NULL )
	{
		Param->RgbBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	if( Param->AlphaBaseImage != NULL )
	{
		Param->AlphaBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	Result = CreateDivGraph_Static( Param, TRUE ) ;

	DecASyncLoadCount( Param->BaseHandle ) ;
	for( i = 0 ; i < Param->AllNum ; i ++ )
		DecASyncLoadCount( Param->HandleBuf[ i ] ) ;

	if( Param->ReCreateFlag == FALSE )
	{
		NS_DeleteGraph( Param->BaseHandle ) ;
		if( Result < 0 )
		{
			for( i = 0 ; i < Param->AllNum ; i ++ )
				NS_DeleteGraph( Param->HandleBuf[ i ] ) ;
		}
	}
}
#endif // DX_NON_ASYNCLOAD

// �摜�𕪊����ăO���t�B�b�N�n���h�����쐬����֐�
extern int CreateDivGraph_UseGParam( 
	LOADGRAPH_PARAM *Param,
	int ASyncLoadFlag
)
{
	int i ;

	CheckActiveState() ;

	if( Param->AllNum == 0 )
		return -1 ;

	Param->BaseHandle = -1 ;
	if( Param->ReCreateFlag == FALSE )
	{
		// �O���t�B�b�N�n���h���̍쐬
		_MEMSET( Param->HandleBuf, 0, sizeof( int ) * Param->AllNum ) ;
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			Param->HandleBuf[ i ] = AddGraphHandle() ;
			if( Param->HandleBuf[ i ] < 0 )
				goto ERR ;
		}
		Param->BaseHandle = AddGraphHandle() ;
		if( Param->BaseHandle < 0 )
			goto ERR ;
	}
	else
	{
#ifndef DX_NON_ASYNCLOAD
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			if( NS_CheckHandleASyncLoad( Param->HandleBuf[ i ] ) > 0 )
				goto ERR ;
		}
#endif // DX_NON_ASYNCLOAD
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, Param, sizeof( *Param ) ) ;
		AddASyncLoadParamStruct( NULL, &Addr, Param->HandleBuf, sizeof( int ) * Param->AllNum ) ;
		if( Param->FileName != NULL )
		{
			AddASyncLoadParamString( NULL, &Addr, Param->FileName ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = CreateDivGraph_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, Param, sizeof( *Param ) ) ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, Param->HandleBuf, sizeof( int ) * Param->AllNum ) ;
		if( Param->FileName != NULL )
		{
			AddASyncLoadParamString( AParam->Data, &Addr, Param->FileName ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		for( i = 0 ; i < Param->AllNum ; i ++ )
			IncASyncLoadCount( Param->HandleBuf[ i ], AParam->Index ) ;
		IncASyncLoadCount( Param->BaseHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( CreateDivGraph_Static( Param, FALSE ) < 0 )
			goto ERR ;

		if( Param->ReCreateFlag == FALSE )
		{
			NS_DeleteGraph( Param->BaseHandle ) ;
		}
	}

	// ����I��
	return 0 ;

ERR :
	if( Param->ReCreateFlag == FALSE )
	{
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			NS_DeleteGraph( Param->HandleBuf[ i ] ) ;
		}
		NS_DeleteGraph( Param->BaseHandle ) ;
	}

	return -1 ;
}

// CreateGraph �̎������֐�
static int CreateGraph_Static(
	LOADGRAPH_PARAM *Param,
	int ASyncThread
)
{
	int Result = 0 ;
	CREATEGRAPH_LOADBASEIMAGE_PARAM LParam ;

	// �摜�f�[�^�̓ǂݍ���
	CreateGraph_LoadBaseImage( Param, &LParam ) ;

	if( LParam.LoadHr == -1 ) 
	{
		// �ēǂݍ��݂ł͂Ȃ��ꍇ�Ńt�@�C������̓ǂݍ��݂̏ꍇ�̓��[�r�[�̉\��������
		if( Param->ReCreateFlag == FALSE && Param->FileName != NULL )
		{
#ifndef DX_NON_MOVIE
			// �t�@�C�������邩�m�F
			{
				DWORD_PTR fp ;

				fp = FOPEN( Param->FileName ) ;
				if( fp == 0 )
				{
					TCHAR FullPath[ 512 ] ;

					ConvertFullPathT_( Param->FileName, FullPath ) ;
					DXST_ERRORLOGFMT_ADD(( _T( "�摜�t�@�C�� %s ������܂���" ), FullPath )) ;
					return -1 ;
				}
				FCLOSE( fp ) ;
			}

			// ����̃I�[�v�������݂�
			if( LoadBmpToGraph_OpenMovie_UseGParam( &Param->GParam, Param->GrHandle, Param->FileName, Param->TextureFlag, Param->SurfaceMode, ASyncThread ) < 0 )
				return -1 ;
#else	// DX_NON_MOVIE
			{
				DXST_ERRORLOGFMT_ADD(( _T( "�摜�t�@�C�� %s �̃I�[�v���Ɏ��s���܂���" ), Param->FileName )) ;
				return -1 ;
			}
#endif	// DX_NON_MOVIE

			// ����I��
			return 0 ;
		}
		else
		{
			return -1 ;
		}
	}

	// �O���t�B�b�N�n���h�����쐬����
	Result = CreateGraphFromGraphImageBase_UseGParam(
				&Param->GParam.CreateGraphGParam,
				Param->ReCreateFlag,
				Param->GrHandle,
				LParam.UseRgbBaseImage,
				LParam.UseAlphaBaseImage,
				Param->TextureFlag,
				ASyncThread
			) ;

	// �t�@�C���p�X���Z�b�g����
	if( Result == 0 )
	{
		SetGraphBaseInfo_UseGParam(
			&Param->GParam.SetGraphBaseInfoGParam,
			Param->GrHandle,
			Param->FileName,
			NULL,
			NULL,
			NULL,
			Param->RgbMemImage,
			Param->RgbMemImageSize,
			Param->AlphaMemImage,
			Param->AlphaMemImageSize,
			LParam.UseRgbBaseImage,
			LParam.UseAlphaBaseImage,
			Param->ReverseFlag,
			-1,
			ASyncThread
		) ;
	}

	// �ǂݍ��񂾃O���t�B�b�N�̌�n��������
	CreateGraph_TerminateBaseImage( Param, &LParam ) ;

	// �I��
	return Result ;
}

#ifndef DX_NON_ASYNCLOAD

// CreateGraph �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void CreateGraph_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADGRAPH_PARAM *Param ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	Param = ( LOADGRAPH_PARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	if( Param->FileName != NULL )
	{
		Param->FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	}

	if( Param->RgbBaseImage != NULL )
	{
		Param->RgbBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	if( Param->AlphaBaseImage != NULL )
	{
		Param->AlphaBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	Result = CreateGraph_Static( Param, TRUE ) ;
	DecASyncLoadCount( Param->GrHandle ) ;
	if( Param->ReCreateFlag == FALSE )
	{
		if( Result < 0 )
		{
			NS_DeleteGraph( Param->GrHandle ) ;
		}
	}
}
#endif // DX_NON_ASYNCLOAD

// CreateGraph �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateGraph_UseGParam( 
	LOADGRAPH_PARAM *Param,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	if( ASyncThread == FALSE )
	{
		CheckActiveState() ;
	}

	if( Param->ReCreateFlag == FALSE )
	{
		Param->GrHandle = AddGraphHandle() ;
		if( Param->GrHandle < 0 )
			return -1 ;
	}
	else
	{
#ifndef DX_NON_ASYNCLOAD
		if( NS_CheckHandleASyncLoad( Param->GrHandle ) > 0 )
			return -1 ;
#endif // DX_NON_ASYNCLOAD
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread == FALSE && ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, Param, sizeof( *Param ) ) ;
		if( Param->FileName != NULL )
		{
			ConvertFullPathT_( Param->FileName, FullPath ) ;
			AddASyncLoadParamString( NULL, &Addr, FullPath ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = CreateGraph_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, Param, sizeof( *Param ) ) ;
		if( Param->FileName != NULL )
		{
			AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( Param->GrHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( CreateGraph_Static( Param, ASyncThread ) < 0 )
			goto ERR ;
	}

	// ����I��
	return Param->ReCreateFlag == FALSE ? Param->GrHandle : 0 ;

ERR :
	if( Param->ReCreateFlag == FALSE )
	{
		NS_DeleteGraph( Param->GrHandle ) ;
		Param->GrHandle = -1 ;
	}

	return -1 ;
}

// CreateGraphFromMem �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateGraphFromMem_UseGParam( 
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const void *MemImage,
	int MemImageSize,
	const void *AlphaImage,
	int AlphaImageSize,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.RgbMemImage = MemImage ;
	Param.RgbMemImageSize = MemImageSize ;
	Param.AlphaMemImage = AlphaImage ;
	Param.AlphaMemImageSize = AlphaImageSize ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = DX_MOVIESURFACE_NORMAL ;

	return CreateGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// ��������̃O���t�B�b�N�C���[�W����O���t�B�b�N�n���h�����쐬����
extern int NS_CreateGraphFromMem( const void *MemImage, int MemImageSize, const void *AlphaImage, int AlphaImageSize, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateGraphFromMem_UseGParam( &GParam, FALSE, -1, MemImage, MemImageSize, AlphaImage, AlphaImageSize, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// ��������̃O���t�B�b�N�C���[�W��������̃O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateGraphFromMem( const void *MemImage, int MemImageSize, int GrHandle, const void *AlphaImage, int AlphaImageSize, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateGraphFromMem_UseGParam( &GParam, TRUE, GrHandle, MemImage, MemImageSize, AlphaImage, AlphaImageSize, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// CreateDivGraphFromMem �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateDivGraphFromMem_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const void *MemImage,
	int MemImageSize,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	const void *AlphaImage,
	int AlphaImageSize,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.RgbMemImage = MemImage ;
	Param.RgbMemImageSize = MemImageSize ;
	Param.AlphaMemImage = AlphaImage ;
	Param.AlphaMemImageSize = AlphaImageSize ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return CreateDivGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// ��������̃O���t�B�b�N�C���[�W���番���O���t�B�b�N�n���h�����쐬����
extern int NS_CreateDivGraphFromMem( const void *MemImage, int MemImageSize, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag, const void *AlphaImage, int AlphaImageSize )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateDivGraphFromMem_UseGParam( &GParam, FALSE, MemImage, MemImageSize, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, AlphaImage, AlphaImageSize, GetASyncLoadFlag() ) ;
}

// ��������̃O���t�B�b�N�C���[�W��������̕����O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateDivGraphFromMem( const void *MemImage, int MemImageSize, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag, const void *AlphaImage, int AlphaImageSize )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateDivGraphFromMem_UseGParam( &GParam, TRUE, MemImage, MemImageSize, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, AlphaImage, AlphaImageSize, GetASyncLoadFlag() ) ;
}

// CreateGraphFromBmp �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateGraphFromBmp_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const BITMAPINFO *BmpInfo,
	const void *GraphData,
	const BITMAPINFO *AlphaInfo,
	const void *AlphaData,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.RgbBmpInfo = BmpInfo ;
	Param.RgbBmpImage = ( void * )GraphData ;
	Param.AlphaBmpInfo = AlphaInfo ;
	Param.AlphaBmpImage = ( void * )AlphaData ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = DX_MOVIESURFACE_NORMAL ;

	return CreateGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// �r�b�g�}�b�v�f�[�^����O���t�B�b�N�n���h�����쐬����
extern int NS_CreateGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, const BITMAPINFO *AlphaInfo, const void *AlphaData, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateGraphFromBmp_UseGParam( &GParam, FALSE, -1, BmpInfo, GraphData, AlphaInfo, AlphaData, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// �r�b�g�}�b�v�f�[�^��������̃O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, int GrHandle, const BITMAPINFO *AlphaInfo, const void *AlphaData, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateGraphFromBmp_UseGParam( &GParam, TRUE, GrHandle, BmpInfo, GraphData, AlphaInfo, AlphaData, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// CreateDivGraphFromBmp �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateDivGraphFromBmp_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const BITMAPINFO *BmpInfo,
	const void *GraphData,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	const BITMAPINFO *AlphaInfo,
	const void *AlphaData,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.RgbBmpInfo = BmpInfo ;
	Param.RgbBmpImage = ( void * )GraphData ;
	Param.AlphaBmpInfo = AlphaInfo ;
	Param.AlphaBmpImage = ( void * )AlphaData ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return CreateDivGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// �r�b�g�}�b�v�f�[�^���番���O���t�B�b�N�n���h�����쐬����
extern int NS_CreateDivGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag, const BITMAPINFO *AlphaInfo, const void *AlphaData )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateDivGraphFromBmp_UseGParam( &GParam, FALSE, BmpInfo, GraphData, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, AlphaInfo, AlphaData, GetASyncLoadFlag() ) ;
}

// �r�b�g�}�b�v�f�[�^��������̕����O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateDivGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag, const BITMAPINFO *AlphaInfo, const void *AlphaData )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateDivGraphFromBmp_UseGParam( &GParam, TRUE, BmpInfo, GraphData, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, AlphaInfo, AlphaData, GetASyncLoadFlag() ) ;
}

// GraphImage�f�[�^����O���t�B�b�N�n���h�����쐬����
extern int NS_CreateGraphFromGraphImage( const BASEIMAGE *Image, int TextureFlag, int ReverseFlag )
{
	return NS_CreateGraphFromGraphImage( Image, NULL, TextureFlag, ReverseFlag ) ;
}

// CreateGraphFromGraphImageBase �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateGraphFromGraphImageBase_UseGParam(
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	BASEIMAGE *Image,
	const BASEIMAGE *AlphaImage,
	int TextureFlag,
	int ASyncThread
)
{
	int SizeX, SizeY ;
	int UseTempImage = FALSE ;
	BASEIMAGE TempImage ;
	int Result ;

	// �č쐬�ł͂Ȃ��ꍇ�͉摜�̓��ߐF�̏������s��
	if( ReCreateFlag == FALSE )
	{
		// ���ߐF�J���[�g�p��O��Ƃ����摜�ɂ���ꍇ�́A�����œ��ߐF�J���[�����O��摜���쐬����
		if( GParam->UseTransColorGraphCreateFlag && GParam->NotUseTransColor == FALSE )
		{
			int w, h, i, j, r, g, b, a, r2, g2, b2, a2, tr, tg, tb, useAlpha;

			w = Image->Width;
			h = Image->Height;
			tr = ( GParam->InitHandleGParam.TransColor >> 16 ) & 0xff;
			tg = ( GParam->InitHandleGParam.TransColor >>  8 ) & 0xff;
			tb = ( GParam->InitHandleGParam.TransColor >>  0 ) & 0xff;
			useAlpha = Image->ColorData.AlphaWidth != 0;
			if( NS_CreateARGB8ColorBaseImage( w, h, &TempImage ) == 0 )
			{
				// ���ߐF�̃`�F�b�N���s���Ȃ���摜��]��
				for( i = 0; i < h; i++ )
				{
					for( j = 0; j < w; j++ )
					{
						NS_GetPixelBaseImage(      Image, j, i, &r, &g, &b, &a );
						NS_SetPixelBaseImage( &TempImage, j, i,  r,  g,  b, r == tr && g == tg && b == tb ? 0 : ( useAlpha ? a : 255 ) );
					}
				}

				// ���ߐF�ł͂Ȃ������̐F�𓧉ߐF�̕����ɑ������
				for( i = 0; i < h; i++ )
				{
					for( j = 0; j < w; j++ )
					{
						NS_GetPixelBaseImage( Image, j, i, &r, &g, &b, &a );
						if( r != tr || g != tg || b != tb )
						{
							if( j >    0 ){ NS_GetPixelBaseImage( Image, j-1, i  ,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j-1, i  ,r,g,b,0); }
							if( i >    0 ){ NS_GetPixelBaseImage( Image, j  , i-1,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j  , i-1,r,g,b,0); }
							if( j < w -1 ){ NS_GetPixelBaseImage( Image, j+1, i  ,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j+1, i  ,r,g,b,0); }
							if( i < h -1 ){ NS_GetPixelBaseImage( Image, j  , i+1,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j  , i+1,r,g,b,0); }
						}
					}
				}

				// �V�����C���[�W�̏����Z�b�g
				Image = &TempImage;

				// ���摜���g�p���Ă���t���O�𗧂Ă�
				UseTempImage = TRUE ;
			}
		}
	}

	// �Ȃ�̋@�\������t�����Ă��Ȃ��r�b�g�̃}�X�N���Z�b�g����
	// (NoneMask �͓r��������ꂽ�����o�ϐ��Ȃ̂ŁA���ʌ݊������������邽�߂ɁE�E�E)
	NS_SetColorDataNoneMask( ( COLORDATA * )&Image->ColorData ) ;
	if( AlphaImage != NULL ) NS_SetColorDataNoneMask( ( COLORDATA * )&AlphaImage->ColorData ) ;

	// �č쐬�̏ꍇ�̓T�C�Y�����ꂩ�`�F�b�N����
	if( ReCreateFlag )
	{
		// �T�C�Y���������G���[
		if( NS_GetGraphSize( GrHandle, &SizeX, &SizeY ) == -1 ) return -1 ;
		if( SizeX != Image->Width || SizeY != Image->Height ) return -1 ;
		if( AlphaImage != NULL && ( SizeX != AlphaImage->Width || SizeY != AlphaImage->Height ) ) return -1 ;

		LUTRANS_RE_GPARAM( GParam, Image, GrHandle )

		Result = 0 ;
	}
	else
	{
		LUTRANS_GPARAM( GParam, Image ) ;

		// �O���t�B�b�N�n���h�����쐬����
		Result = CreateDXGraph_UseGParam( &GParam->InitHandleGParam, GrHandle, Image, AlphaImage, TextureFlag, ASyncThread ) ;
	}

	if( Result != -1 )
	{
		BltBmpOrGraphImageToGraphBase(
			NULL,
			NULL,
			NULL,
			FALSE,
			Image,
			AlphaImage,
			0,
			0,
			GrHandle,
			GParam->NotUseTransColor ? FALSE : TRUE,
			ASyncThread
		) ;
	}

	// ���摜���g�p���Ă����ꍇ�͉������
	if( UseTempImage )
	{
		NS_ReleaseBaseImage( &TempImage );
	}

	// ���������s����Ԃ�
	return Result ;
}

// CreateGraphFromGraphImage �̓����֐�
extern int CreateGraphFromGraphImageBase( BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int TextureFlag )
{
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;
	int GrHandle ;

	CheckActiveState() ;

	GrHandle = AddGraphHandle() ;
	if( GrHandle == -1 ) return -1 ;

	InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	if( CreateGraphFromGraphImageBase_UseGParam( &GParam, FALSE, GrHandle, Image, AlphaImage, TextureFlag, FALSE ) == -1 )
	{
		NS_DeleteGraph( GrHandle ) ;
		return -1 ;
	}

	return GrHandle ;
}

// CreateGraphFromGraphImage �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateGraphFromGraphImage_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const BASEIMAGE *Image,
	const BASEIMAGE *AlphaImage,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.RgbBaseImage = ( BASEIMAGE * )Image ;
	Param.AlphaBaseImage = ( BASEIMAGE * )AlphaImage ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = DX_MOVIESURFACE_NORMAL ;

	return CreateGraph_UseGParam( &Param, ASyncLoadFlag, ASyncThread ) ;
}

// GraphImage�f�[�^����O���t�B�b�N�n���h�����쐬����
extern int NS_CreateGraphFromGraphImage( const BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateGraphFromGraphImage_UseGParam( &GParam, FALSE, -1, Image, AlphaImage, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ; 
}

// GraphImage�f�[�^��������̃O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateGraphFromGraphImage( const BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int GrHandle, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateGraphFromGraphImage_UseGParam( &GParam, TRUE, GrHandle, Image, AlphaImage, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// GraphImage�f�[�^��������̃O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateGraphFromGraphImage( const BASEIMAGE *Image, int GrHandle, int TextureFlag, int ReverseFlag )
{
	return NS_ReCreateGraphFromGraphImage( Image, NULL, GrHandle, TextureFlag, ReverseFlag ) ;
}

// ReCreateGraphFromGraphImage �̓����֐�
extern int ReCreateGraphFromGraphImageBase( BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int GrHandle, int TextureFlag )
{
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;
	int Result ;

	CheckActiveState() ;

	InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	Result = CreateGraphFromGraphImageBase_UseGParam( &GParam, TRUE, GrHandle, Image, AlphaImage, TextureFlag, FALSE ) ;

	return Result ;
}

// GraphImage�f�[�^���番���O���t�B�b�N�n���h�����쐬����
extern int NS_CreateDivGraphFromGraphImage( BASEIMAGE *Image, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	return NS_CreateDivGraphFromGraphImage( Image, NULL, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag ) ;
}

// CreateDivGraphFromGraphImageBase �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateDivGraphFromGraphImageBase_UseGParam( CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam, int ReCreateFlag, int BaseHandle, BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag, int ASyncThread )
{
	int Result ;
	int GrSizeX, GrSizeY ;

	if( ReCreateFlag == FALSE )
	{
		int i, j, k, x, y, basex, addx ;

		// ���ʂɃO���t�B�b�N���쐬����
		if( CreateGraphFromGraphImageBase_UseGParam( GParam, FALSE, BaseHandle, Image, AlphaImage, TextureFlag, ASyncThread ) < 0 )
			return -1 ;

		if( ReverseFlag == TRUE )
		{
			basex = ( XNum - 1 ) * SizeX ;
			addx = -SizeX ;
		}
		else
		{
			basex = 0 ;
			addx = SizeX ;
		}
		
		// �w��̖����ɕ�������
		k = 0 ;
		y = 0 ;
		for( i = 0 ; k < AllNum && i < YNum ; i ++, y += SizeY )
		{
			x = basex ;
			for( j = 0 ; k < AllNum && j < XNum ; j ++, k ++, x += addx )
			{
				if( DerivationGraphBase( HandleBuf[ k ], x, y, SizeX, SizeY, BaseHandle, ASyncThread ) < 0 )
				{
					DXST_ERRORLOG_ADD( _T( "�O���t�B�b�N�̕����Ɏ��s���܂��� in CreateDivGraphFromGraphImage\n" ) ) ;
					return -1 ;
				}
			}
		}
		Result = 0 ;
	}
	else
	{
		// �Ȃ�̋@�\������t�����Ă��Ȃ��r�b�g�̃}�X�N���Z�b�g����
		// (NoneMask �͓r��������ꂽ�����o�ϐ��Ȃ̂ŁA���ʌ݊������������邽�߂ɁE�E�E)
		NS_SetColorDataNoneMask( ( COLORDATA * )&Image->ColorData ) ;
		if( AlphaImage != NULL ) NS_SetColorDataNoneMask( ( COLORDATA * )&AlphaImage->ColorData ) ;

		// �T�C�Y���������G���[
		if( NS_GetGraphSize( HandleBuf[ 0 ], &GrSizeX, &GrSizeY ) == -1 ) return -1 ;
		if( Image->Width < XNum * SizeX || Image->Height < YNum * SizeY ) return -1 ;
		if( AlphaImage != NULL && ( AlphaImage->Width != Image->Width || AlphaImage->Height != Image->Height ) ) return -1 ;

		LUTRANS_RE_DIV_GPARAM( GParam, Image, HandleBuf, AllNum )

		// �����O���t�B�b�N���O���t�B�b�N�n���h���ɓ]������
		Result = BltBmpOrGraphImageToDivGraphBase(
				NULL,
				NULL,
				NULL,
				FALSE,
				Image,
				AlphaImage,
				AllNum,
				XNum,
				YNum,
				SizeX,
				SizeY,
				HandleBuf,
				ReverseFlag,
				GParam->NotUseTransColor ? FALSE : TRUE,
				ASyncThread
		) ; 
	}

	// �I��
	return Result ;
}

// CreateDivGraphFromGraphImage �̓����֐�
extern int CreateDivGraphFromGraphImageBase( BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag )
{
	int BaseHandle, i ;
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;

	// ��{�O���t�B�b�N�n���h���̍쐬
	BaseHandle = AddGraphHandle() ;
	if( BaseHandle == -1 )
		return -1 ;

	// �����O���t�B�b�N�n���h���̍쐬
	_MEMSET( HandleBuf, 0, AllNum * sizeof( int ) ) ;
	for( i = 0 ; i < AllNum ; i ++ )
	{
		HandleBuf[ i ] = AddGraphHandle() ;
		if( HandleBuf[ i ] == -1 )
			goto ERR ;
	}

	// �������p�f�[�^�̏���
	InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	// �����O���t�B�b�N�n���h���̍쐬
	if( CreateDivGraphFromGraphImageBase_UseGParam( &GParam, FALSE, BaseHandle, Image, AlphaImage, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, FALSE ) < 0 )
		goto ERR ;
	
	// ���ƂȂ����n���h�������
	NS_DeleteGraph( BaseHandle ) ;

	// �I��
	return 0 ;
	
ERR :
	for( i = 0 ; i < AllNum ; i ++ )
	{
		NS_DeleteGraph( HandleBuf[ i ] ) ;
	}
	NS_DeleteGraph( BaseHandle ) ;

	// �G���[�I��	
	return -1 ;
}

// CreateDivGraphFromGraphImage �̃O���[�o���ϐ��ɃA�N�Z�X���Ȃ��o�[�W����
extern int CreateDivGraphFromGraphImage_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const BASEIMAGE *Image,
	const BASEIMAGE *AlphaImage,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.RgbBaseImage = ( BASEIMAGE * )Image ;
	Param.AlphaBaseImage = ( BASEIMAGE * )AlphaImage ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return CreateDivGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// GraphImage�f�[�^���番���O���t�B�b�N�n���h�����쐬����
extern int NS_CreateDivGraphFromGraphImage( BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateDivGraphFromGraphImage_UseGParam( &GParam, FALSE, Image, AlphaImage, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// GraphImage�f�[�^��������̕����O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateDivGraphFromGraphImage( BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	InitLoadGraphGParam( &GParam ) ;

	return CreateDivGraphFromGraphImage_UseGParam( &GParam, TRUE, Image, AlphaImage, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// GraphImage�f�[�^��������̕����O���t�B�b�N�n���h���Ƀf�[�^��]������
extern int NS_ReCreateDivGraphFromGraphImage( BASEIMAGE *Image, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	return NS_ReCreateDivGraphFromGraphImage( Image, NULL, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag ) ;
}

// ReCreateDivGraphFromGraphImage �̓����֐�
extern int ReCreateDivGraphFromGraphImageBase( BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf, int TextureFlag, int ReverseFlag )
{
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;
	int Result ;

	CheckActiveState() ;

	InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	Result = CreateDivGraphFromGraphImageBase_UseGParam( &GParam, TRUE, -1, Image, AlphaImage, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, FALSE ) ;

	// �I��
	return Result ;
}


























// CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM �̃f�[�^���Z�b�g����
extern void InitCreateGraphHandleAndBltGraphImageGParam( CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam )
{
	InitSetupGraphHandleGParam( &GParam->InitHandleGParam ) ;

	GParam->UseTransColorGraphCreateFlag	= GBASE.UseTransColorGraphCreateFlag ;
	GParam->NotUseTransColor				= GBASE.NotUseTransColor ;
	GParam->LeftUpColorIsTransColorFlag		= GBASE.LeftUpColorIsTransColorFlag ;
}

// SETUP_GRAPHHANDLE_GPARAM �̃f�[�^���Z�b�g����
extern void InitSetupGraphHandleGParam( SETUP_GRAPHHANDLE_GPARAM *GParam )
{
	GParam->TransColor							= GBASE.TransColor ;

	GParam->CreateImageColorBitDepth			= GBASE.CreateImageColorBitDepth ;
	GParam->CreateImageChannelBitDepth			= GBASE.CreateImageChannelBitDepth ;
	GParam->AlphaTestImageCreateFlag			= GBASE.AlphaTestImageCreateFlag ;
	GParam->AlphaChannelImageCreateFlag			= GBASE.AlphaChannelImageCreateFlag ;
	GParam->CubeMapTextureCreateFlag			= GBASE.CubeMapTextureCreateFlag ;
	GParam->BlendImageCreateFlag				= GBASE.BlendImageCreateFlag ;
	GParam->UseManagedTextureFlag				= GBASE.NotUseManagedTextureFlag ? FALSE : TRUE ;

	GParam->DrawValidImageCreateFlag			= GBASE.DrawValidImageCreateFlag ;
	GParam->DrawValidAlphaImageCreateFlag		= GBASE.DrawValidAlphaImageCreateFlag ;
	GParam->DrawValidFloatTypeGraphCreateFlag	= GBASE.DrawValidFloatTypeGraphCreateFlag ;
	GParam->DrawValidGraphCreateZBufferFlag		= GBASE.NotDrawValidGraphCreateZBufferFlag ? FALSE : TRUE ;
	GParam->CreateDrawValidGraphZBufferBitDepth	= GBASE.CreateDrawValidGraphZBufferBitDepth ;
	GParam->CreateDrawValidGraphChannelNum		= GBASE.CreateDrawValidGraphChannelNum ;
	GParam->DrawValidMSSamples					= GBASE.DrawValidMSSamples ;
	GParam->DrawValidMSQuality					= GBASE.DrawValidMSQuality ;

	GParam->MipMapCount							= GRH.MipMapCount ;
	GParam->UserMaxTextureSize					= GRH.UserMaxTextureSize ;
	GParam->NotUseDivFlag						= GRH.NotUseDivFlag ;
}

extern void InitSetupGraphHandleGParam_Normal_NonDrawValid( SETUP_GRAPHHANDLE_GPARAM *GParam, int BitDepth, int AlphaChannel, int AlphaTest )
{
	GParam->TransColor							= 0x00ff00ff ;

	GParam->CreateImageColorBitDepth			= BitDepth ;
	GParam->CreateImageChannelBitDepth			= 0 ;
	GParam->AlphaTestImageCreateFlag			= AlphaTest ;
	GParam->AlphaChannelImageCreateFlag			= AlphaChannel ;
	GParam->CubeMapTextureCreateFlag			= FALSE ;
	GParam->BlendImageCreateFlag				= FALSE ;
	GParam->UseManagedTextureFlag				= TRUE ;

	GParam->DrawValidImageCreateFlag			= FALSE ;
	GParam->DrawValidAlphaImageCreateFlag		= FALSE ;
	GParam->DrawValidFloatTypeGraphCreateFlag	= FALSE ;
	GParam->DrawValidGraphCreateZBufferFlag		= FALSE ;
	GParam->CreateDrawValidGraphZBufferBitDepth	= 0 ;
	GParam->CreateDrawValidGraphChannelNum		= 0 ;
	GParam->DrawValidMSSamples					= 0 ;
	GParam->DrawValidMSQuality					= 0 ;

	GParam->MipMapCount							= 1 ;
	GParam->UserMaxTextureSize					= 0 ;
	GParam->NotUseDivFlag						= TRUE ;
}

extern void InitSetupGraphHandleGParam_Normal_DrawValid_NoneZBuffer( SETUP_GRAPHHANDLE_GPARAM *GParam, int BitDepth, int AlphaChannel )
{
	GParam->TransColor							= 0x00ff00ff ;

	GParam->CreateImageColorBitDepth			= BitDepth ;
	GParam->CreateImageChannelBitDepth			= 0 ;
	GParam->AlphaTestImageCreateFlag			= FALSE ;
	GParam->AlphaChannelImageCreateFlag			= AlphaChannel ;
	GParam->CubeMapTextureCreateFlag			= FALSE ;
	GParam->BlendImageCreateFlag				= FALSE ;
	GParam->UseManagedTextureFlag				= FALSE ;

	GParam->DrawValidImageCreateFlag			= TRUE ;
	GParam->DrawValidAlphaImageCreateFlag		= AlphaChannel ;
	GParam->DrawValidFloatTypeGraphCreateFlag	= FALSE ;
	GParam->DrawValidGraphCreateZBufferFlag		= FALSE ;
	GParam->CreateDrawValidGraphZBufferBitDepth	= 0 ;
	GParam->CreateDrawValidGraphChannelNum		= 0 ;
	GParam->DrawValidMSSamples					= 0 ;
	GParam->DrawValidMSQuality					= 0 ;

	GParam->MipMapCount							= 1 ;
	GParam->UserMaxTextureSize					= 0 ;
	GParam->NotUseDivFlag						= TRUE ;
}

// SETGRAPHBASEINFO_GPARAM �̃f�[�^���Z�b�g����
extern void InitSetGraphBaseInfoGParam( SETGRAPHBASEINFO_GPARAM *GParam )
{
	GParam->NotGraphBaseDataBackupFlag = GBASE.NotGraphBaseDataBackupFlag ;
}

// LOADGRAPH_GPARAM �̃f�[�^���Z�b�g����
extern void InitLoadGraphGParam( LOADGRAPH_GPARAM *GParam )
{
	InitLoadBaseImageGParam( &GParam->LoadBaseImageGParam ) ;
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_MOVIE
	InitOpenMovieGParam( &GParam->OpenMovieGParam ) ;
#endif
#endif
	InitCreateGraphHandleAndBltGraphImageGParam( &GParam->CreateGraphGParam ) ;
	InitSetGraphBaseInfoGParam( &GParam->SetGraphBaseInfoGParam ) ;
}

// ��������̃O���t�B�b�N�f�[�^����O���t�B�b�N�n���h�����쐬����
extern int NS_CreateGraph( int Width, int Height, int Pitch, const void *GraphData, const void *AlphaData, int GrHandle )
{
	BITMAPINFO	BmpInfo ;
	BYTE *DData, *AData, *SData, *DestData ;
	int i ;
	int DPitch, SPitch ;
	int NewGraph ;

	// �s�b�`�쐬
	{
		int Byte ; 
		
		Byte = Width * 3 ;

		SPitch = ( Pitch == 0 ) ? Width * 3 : Pitch ;
		DPitch = ( ( Byte + 3 ) / 4 ) * 4 ;
	}

	// �����s�b�`�������Ȃ�O���t�B�b�N�f�[�^���R�s�[����K�v�Ȃ�
	AData = NULL ;
	if( SPitch != DPitch )
	{
		// RGB�f�[�^�̍쐬
		{
			// �L���X�g
			SData = ( BYTE * )GraphData ;

			// �O���t�B�b�N�f�[�^�𕡐�
			if( ( DestData = DData = ( BYTE *)DXCALLOC( Height * DPitch ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, DData += DPitch )
				memcpy( DData, SData, SPitch ) ;
			DData = DestData ;
		}

		// �A���t�@�f�[�^�̍쐬
		if( AlphaData != NULL )
		{
			// �L���X�g
			SData = ( BYTE * )AlphaData ;

			// �O���t�B�b�N�f�[�^�𕡐�
			if( ( DestData = AData = ( BYTE *)DXCALLOC( Height * DPitch ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, AData += DPitch )
				memcpy( AData, SData, SPitch ) ;
			AData = DestData ;
		}
	}
	else
	{
		DData = ( BYTE * )GraphData ;
		AData = ( BYTE * )AlphaData ;
	}

	// �V�O���t�B�b�N���쐬
	_MEMSET( &BmpInfo, 0, sizeof( BmpInfo ) ) ;
	BmpInfo.bmiHeader.biSize	= sizeof( BITMAPINFOHEADER ) ;
	BmpInfo.bmiHeader.biWidth	= Width ;
	BmpInfo.bmiHeader.biHeight	= -Height ;
	BmpInfo.bmiHeader.biPlanes	= 1 ;
	BmpInfo.bmiHeader.biBitCount= 24 ;

	if( GrHandle != -1 )
	{
		NS_ReCreateGraphFromBmp( &BmpInfo, DData, GrHandle, AData != NULL ? &BmpInfo : NULL, AData ) ;
		NewGraph = 0 ;
	}
	else
	{
		NewGraph = NS_CreateGraphFromBmp( &BmpInfo, DData, AData != NULL ? &BmpInfo : NULL, AData ) ;
	}

	// �����������
	if( SPitch != DPitch )
	{
		DXFREE( DData ) ;
		DXFREE( AData ) ;
	}

	// �I��
	return NewGraph ;
}

// ��������̃O���t�B�b�N�f�[�^���番���O���t�B�b�N�n���h�����쐬����
extern int NS_CreateDivGraph( int Width, int Height, int Pitch, const void *GraphData, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, const void *AlphaData )
{
	BITMAPINFO	BmpInfo ;
	BYTE *DData, *AData, *SData, *DestData ;
	int i, Result ;
	int DPitch, SPitch ;

	// �s�b�`�쐬
	{
		int Byte ; 
		
		Byte = Width * 3 ;

		SPitch = ( Pitch == 0 ) ? Width * 3 : Pitch ;
		DPitch = ( ( Byte + 3 ) / 4 ) * 4 ;
	}

	// �����s�b�`�������Ȃ�O���t�B�b�N�f�[�^���R�s�[����K�v�Ȃ�
	AData = NULL ;
	if( SPitch != DPitch )
	{
		// RGB�f�[�^�̍쐬
		{
			// �L���X�g
			SData = ( BYTE * )GraphData ;

			// �O���t�B�b�N�f�[�^�𕡐�
			if( ( DestData = DData = ( BYTE *)DXCALLOC( Height * DPitch ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, DData += DPitch )
				memcpy( DData, SData, SPitch ) ;
			DData = DestData ;
		}

		// �A���t�@�f�[�^�̍쐬
		if( AlphaData != NULL )
		{
			// �L���X�g
			SData = ( BYTE * )AlphaData ;

			// �O���t�B�b�N�f�[�^�𕡐�
			if( ( DestData = AData = ( BYTE *)DXCALLOC( Height * DPitch ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, AData += DPitch )
				memcpy( AData, SData, SPitch ) ;
			AData = DestData ;
		}
	}
	else
	{
		DData = ( BYTE * )GraphData ;
		AData = ( BYTE * )AlphaData ;
	}

	// �V�O���t�B�b�N���쐬
	_MEMSET( &BmpInfo, 0, sizeof( BmpInfo ) ) ;
	BmpInfo.bmiHeader.biSize	= sizeof( BITMAPINFOHEADER ) ;
	BmpInfo.bmiHeader.biWidth	= Width ;
	BmpInfo.bmiHeader.biHeight	= -Height ;
	BmpInfo.bmiHeader.biPlanes	= 1 ;
	BmpInfo.bmiHeader.biBitCount= 24 ;

	Result = NS_CreateDivGraphFromBmp( &BmpInfo, DData, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TRUE, FALSE, &BmpInfo, AData ) ;

	// �����������
	if( SPitch != DPitch )
	{
		DXFREE( DData ) ;
		DXFREE( AData ) ;
	}

	// �I��
	return Result ;
}

// ��������̃O���t�B�b�N�f�[�^����O���t�B�b�N�n���h�����č쐬����
extern int NS_ReCreateGraph( int Width, int Height, int Pitch, const void *GraphData, int GrHandle, const void *AlphaData )
{
	return NS_CreateGraph( Width, Height, Pitch, GraphData, AlphaData, GrHandle ) ;
}

#ifndef DX_NON_SOFTIMAGE

// �\�t�g�E�G�A�ň����C���[�W����u�����h�p�摜�O���t�B�b�N�n���h�����쐬����
extern	int		NS_CreateBlendGraphFromSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;
	LOADGRAPH_GPARAM GParam ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	InitLoadGraphGParam( &GParam ) ;
	GParam.CreateGraphGParam.InitHandleGParam.BlendImageCreateFlag = TRUE ;

	return CreateGraphFromGraphImage_UseGParam( &GParam, FALSE, -1, &SoftImg->BaseImage, NULL, TRUE, FALSE, GetASyncLoadFlag() ) ;
}

// �\�t�g�E�G�A�ň����C���[�W����O���t�B�b�N�n���h�����쐬����
extern	int		NS_CreateGraphFromSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_CreateGraphFromBaseImage( &SoftImg->BaseImage ) ;
}

// �\�t�g�E�G�A�ň����C���[�W�̎w��̗̈���g���ăO���t�B�b�N�n���h�����쐬����( -1:�G���[  -1�ȊO:�O���t�B�b�N�n���h�� )
extern	int		NS_CreateGraphFromRectSoftImage( int SIHandle, int x, int y, int SizeX, int SizeY )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_CreateGraphFromRectBaseImage( &SoftImg->BaseImage, x, y, SizeX, SizeY ) ;
}

// �\�t�g�E�G�A�ň����C���[�W��������̃O���t�B�b�N�n���h���ɉ摜�f�[�^��]������
extern	int		NS_ReCreateGraphFromSoftImage( int SIHandle, int GrHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ReCreateGraphFromBaseImage( &SoftImg->BaseImage, GrHandle ) ;
}

// �\�t�g�E�G�A�ň����C���[�W��������̃O���t�B�b�N�n���h���ɉ摜�f�[�^��]������
extern	int		NS_ReCreateGraphFromRectSoftImage( int SIHandle, int x, int y, int SizeX, int SizeY, int GrHandle )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ReCreateGraphFromRectBaseImage( &SoftImg->BaseImage, x, y, SizeX, SizeY, GrHandle ) ;
}

// �\�t�g�E�G�A�ň����C���[�W���番���O���t�B�b�N�n���h�����쐬����
extern	int		NS_CreateDivGraphFromSoftImage( int SIHandle, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf )
{
	SOFTIMAGE *SoftImg ;

	// �A�h���X�̎擾
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_CreateDivGraphFromBaseImage( &SoftImg->BaseImage, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf ) ;
}

#endif // DX_NON_SOFTIMAGE

// ��{�C���[�W�f�[�^����O���t�B�b�N�n���h�����쐬����
extern	int		NS_CreateGraphFromBaseImage( const BASEIMAGE *BaseImage )
{
	return NS_CreateGraphFromGraphImage( BaseImage ) ;
}

// ��{�C���[�W�̎w��̗̈���g���ăO���t�B�b�N�n���h�����쐬����
extern	int		NS_CreateGraphFromRectBaseImage( const BASEIMAGE *BaseImage, int x, int y, int SizeX, int SizeY )
{
	BASEIMAGE DummyImage ;

	// �ʒu��T�C�Y���s����������G���[
	if( SizeX <= 0 || SizeY <= 0 ||
		x < 0 || x + SizeX >  BaseImage->Width ||
		y < 0 || y + SizeY >  BaseImage->Height ) 
		return -1 ;

	// �_�~�[�̊�{�C���[�W���ł����グ�ăO���t�B�b�N�n���h�����쐬����
	DummyImage.ColorData = BaseImage->ColorData ;
	DummyImage.Width = SizeX ;
	DummyImage.Height = SizeY ;
	DummyImage.Pitch = BaseImage->Pitch ;
	DummyImage.MipMapCount = 0 ;
	DummyImage.GraphDataCount = 0 ;
	DummyImage.GraphData = ( BYTE * )BaseImage->GraphData + x * BaseImage->ColorData.PixelByte + y * BaseImage->Pitch ;
	return NS_CreateGraphFromBaseImage( &DummyImage ) ;
}

// ��{�C���[�W�f�[�^��������̃O���t�B�b�N�n���h���ɉ摜�f�[�^��]������
extern	int		NS_ReCreateGraphFromBaseImage( const BASEIMAGE *BaseImage, int GrHandle )
{
	return NS_ReCreateGraphFromGraphImage( BaseImage, GrHandle ) ;
}

// ��{�C���[�W�̎w��̗̈���g���Ċ����̃O���t�B�b�N�n���h���ɉ摜�f�[�^��]������
extern	int		NS_ReCreateGraphFromRectBaseImage( const BASEIMAGE *BaseImage, int x, int y, int SizeX, int SizeY, int GrHandle )
{
	BASEIMAGE DummyImage ;

	// �ʒu��T�C�Y���s����������G���[
	if( SizeX <= 0 || SizeY <= 0 ||
		x < 0 || x + SizeX >  BaseImage->Width ||
		y < 0 || y + SizeY >  BaseImage->Height ) 
		return -1 ;

	// �_�~�[�̊�{�C���[�W���ł����グ�ăO���t�B�b�N�n���h�����쐬����
	DummyImage.ColorData = BaseImage->ColorData ;
	DummyImage.Width = SizeX ;
	DummyImage.Height = SizeY ;
	DummyImage.Pitch = BaseImage->Pitch ;
	DummyImage.MipMapCount = 0 ;
	DummyImage.GraphDataCount = 0 ;
	DummyImage.GraphData = ( BYTE * )BaseImage->GraphData + x * BaseImage->ColorData.PixelByte + y * BaseImage->Pitch ;
	return NS_ReCreateGraphFromGraphImage( &DummyImage, GrHandle ) ;
}


// ��{�C���[�W�f�[�^���番���O���t�B�b�N�n���h�����쐬����
extern int NS_CreateDivGraphFromBaseImage( BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf )
{
	return NS_CreateDivGraphFromGraphImage( BaseImage,  AllNum,  XNum,  YNum,  SizeX,  SizeY, HandleBuf ) ;
}

// �O���t�B�b�N�f�[�^�ւ̃O���t�B�b�N�̔��]�ǂݍ���
extern int NS_ReloadReverseGraph( const TCHAR *FileName, int GrHandle )
{
	return NS_ReloadGraph( FileName, GrHandle, TRUE ) ;
}

// �O���t�B�b�N�f�[�^�ւ̃O���t�B�b�N�̔��]�����ǂݍ���
extern int NS_ReloadReverseDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, const int *HandleBuf )
{
	return NS_ReloadDivGraph( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, TRUE ) ;
}

// MakeGraph �̎������֐�
static int MakeGraph_Static(
	SETUP_GRAPHHANDLE_GPARAM *GParam,
	int GrHandle,
	int SizeX,
	int SizeY,
	int NotUse3DFlag,
	int AlphaValidFlag,
	int ASyncThread
)
{
	// �n���h���̏�����
	if( SetupGraphHandle_UseGParam(
			GParam,
			GrHandle,
			SizeX,
			SizeY,
			!NotUse3DFlag,
			AlphaValidFlag,
			FALSE,
			DX_BASEIMAGE_FORMAT_NORMAL,
			-1,
			ASyncThread
		) == -1 )
		return -1 ;

	// �I��
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// MakeGraph �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void MakeGraph_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	SETUP_GRAPHHANDLE_GPARAM *GParam ;
	int GrHandle ;
	int SizeX ;
	int SizeY ;
	int NotUse3DFlag ;
	int AlphaValidFlag ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( SETUP_GRAPHHANDLE_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	GrHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeX = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeY = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	NotUse3DFlag = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	AlphaValidFlag = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = MakeGraph_Static( GParam, GrHandle, SizeX, SizeY, NotUse3DFlag, AlphaValidFlag, TRUE ) ;

	DecASyncLoadCount( GrHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteGraph( GrHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// ��̃O���t�B�b�N�n���h�����쐬����֐�
extern int MakeGraph_UseGParam(
	SETUP_GRAPHHANDLE_GPARAM *GParam,
	int SizeX,
	int SizeY,
	int NotUse3DFlag,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	int GrHandle ;

	CheckActiveState() ;

	// �n���h���̍쐬
	GrHandle = AddGraphHandle() ;
	if( GrHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, GrHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeX ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeY ) ;
		AddASyncLoadParamInt( NULL, &Addr, NotUse3DFlag ) ;
		AddASyncLoadParamInt( NULL, &Addr, GParam->AlphaChannelImageCreateFlag ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = MakeGraph_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, GrHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeX ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeY ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, NotUse3DFlag ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, GParam->AlphaChannelImageCreateFlag ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( GrHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MakeGraph_Static( GParam, GrHandle, SizeX, SizeY, NotUse3DFlag, GParam->AlphaChannelImageCreateFlag, ASyncThread ) < 0 )
			goto ERR ;
	}

	// �I��
	return GrHandle ;

ERR :
	NS_DeleteGraph( GrHandle ) ;

	return -1 ;
}

// ��̃O���t�B�b�N���쐬
extern	int NS_MakeGraph( int SizeX, int SizeY, int NotUse3DFlag )
{
	SETUP_GRAPHHANDLE_GPARAM GParam ;

	InitSetupGraphHandleGParam( &GParam ) ;

	return MakeGraph_UseGParam( &GParam, SizeX, SizeY, NotUse3DFlag, GetASyncLoadFlag() ) ;
}

// �`��\�ȉ�ʂ��쐬
extern int NS_MakeScreen( int SizeX, int SizeY, int UseAlphaChannel )
{
	SETUP_GRAPHHANDLE_GPARAM GParam ;

	// �\�t�g�E�G�A�����_�����O���[�h�ł̓A���t�@�`�����l���t���̕`��\�摜�͍��Ȃ�
	if( UseAlphaChannel && NS_GetScreenMemToSystemMemFlag() == TRUE )
		return -1 ;

	InitSetupGraphHandleGParam( &GParam ) ;

	GParam.DrawValidImageCreateFlag = TRUE ;
	GParam.DrawValidAlphaImageCreateFlag = UseAlphaChannel ;
	return MakeGraph_UseGParam( &GParam, SizeX, SizeY, FALSE, GetASyncLoadFlag() ) ;
}




// MakeShadowMap �̎������֐�
static int MakeShadowMap_Static(
	SETUP_SHADOWMAPHANDLE_GPARAM *GParam,
	int SmHandle,
	int SizeX,
	int SizeY,
	int ASyncThread
)
{
	// �n���h���̏�����
	if( SetupShadowMapHandle_UseGParam(
			GParam,
			SmHandle,
			SizeX,
			SizeY,
			TRUE,
			16,
			ASyncThread
		) == -1 )
		return -1 ;

	// �I��
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// MakeShadowMap �̔񓯊��ǂݍ��݃X���b�h����Ă΂��֐�
static void MakeShadowMap_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	SETUP_SHADOWMAPHANDLE_GPARAM *GParam ;
	int SmHandle ;
	int SizeX ;
	int SizeY ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( SETUP_SHADOWMAPHANDLE_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	SmHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeX = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeY = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = MakeShadowMap_Static( GParam, SmHandle, SizeX, SizeY, TRUE ) ;

	DecASyncLoadCount( SmHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteGraph( SmHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// �V���h�E�}�b�v�n���h�����쐬����֐�
extern int MakeShadowMap_UseGParam(
	SETUP_SHADOWMAPHANDLE_GPARAM *GParam,
	int SizeX,
	int SizeY,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	int SmHandle ;

	CheckActiveState() ;

	// �n���h���̍쐬
	SmHandle = AddShadowMapHandle() ;
	if( SmHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// �p�����[�^�ɕK�v�ȃ������̃T�C�Y���Z�o
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, SmHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeX ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeY ) ;

		// �������̊m��
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// �����ɕK�v�ȏ����Z�b�g
		AParam->ProcessFunction = MakeShadowMap_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SmHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeX ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeY ) ;

		// �f�[�^��ǉ�
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// �񓯊��ǂݍ��݃J�E���g���C���N�������g
		IncASyncLoadCount( SmHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MakeShadowMap_Static( GParam, SmHandle, SizeX, SizeY, ASyncThread ) < 0 )
			goto ERR ;
	}

	// �I��
	return SmHandle ;

ERR :
	NS_DeleteGraph( SmHandle ) ;

	return -1 ;
}

// �V���h�E�}�b�v�n���h�����쐬����
extern int NS_MakeShadowMap( int SizeX, int SizeY )
{
	SETUP_SHADOWMAPHANDLE_GPARAM GParam ;

	// Direct3DDevice ��������������G���[
	if( GraphicsDevice_IsValid() == 0 )
		return -1 ;

	GParam.Dummy = 0 ;

	return MakeShadowMap_UseGParam( &GParam, SizeX, SizeY, GetASyncLoadFlag() ) ;
}

// �V���h�E�}�b�v�n���h�����폜����
extern int NS_DeleteShadowMap( int SmHandle )
{
	return SubHandle( SmHandle ) ;
}






// ��{�C���[�W�`��

// ��{�C���[�W�f�[�^��`�悷��
extern	int		NS_DrawBaseImage( int x, int y, BASEIMAGE *BaseImage )
{
	int TempHandle ;

/*
	if( NS_GetActiveGraph() == DX_SCREEN_BACK )
	{
		DWORD Pitch;
		int Width, Height;
		void *Image;
		COLORDATA *ColorData;
		BASEIMAGE DestImage;

		NS_GetDrawScreenSize( &Width, &Height ) ;

		NS_GraphLock( DX_SCREEN_BACK, (int *)&Pitch, &Image, &ColorData ) ;
		DestImage.ColorData = *ColorData;
		DestImage.Width     = Width;
		DestImage.Height    = Height;
		DestImage.Pitch     = Pitch;
		DestImage.GraphData = Image;

		NS_BltBaseImage( 0, 0, BaseImage->Width, BaseImage->Height, x, y, BaseImage, &DestImage ) ;

		NS_GraphUnLock( DX_SCREEN_BACK ) ;
	}
	else
*/
	{
		TempHandle = CreateGraphFromGraphImageBase( BaseImage, NULL, TRUE ) ;
		NS_DrawGraph( x, y, TempHandle, BaseImage->ColorData.AlphaWidth ? TRUE : FALSE ) ;
		NS_DeleteGraph( TempHandle ) ;
	}

	return 0 ;
}








// �J�����֌W�֐�

// �r���[�s�񂩂琅���A�����A�P��p�x���Z�o����
static void ViewMatrixToCameraRoll( void )
{
	VECTOR XVec, YVec, ZVec ;
	float Sin, Cos, f ;

	XVec.x = GBASE.ViewMatrix.m[ 0 ][ 0 ] ;
	XVec.y = GBASE.ViewMatrix.m[ 1 ][ 0 ] ;
	XVec.z = GBASE.ViewMatrix.m[ 2 ][ 0 ] ;

	YVec.x = GBASE.ViewMatrix.m[ 0 ][ 1 ] ;
	YVec.y = GBASE.ViewMatrix.m[ 1 ][ 1 ] ;
	YVec.z = GBASE.ViewMatrix.m[ 2 ][ 1 ] ;

	ZVec.x = GBASE.ViewMatrix.m[ 0 ][ 2 ] ;
	ZVec.y = GBASE.ViewMatrix.m[ 1 ][ 2 ] ;
	ZVec.z = GBASE.ViewMatrix.m[ 2 ][ 2 ] ;

	if( ZVec.x < 0.00000001f && ZVec.x > -0.00000001f &&
		ZVec.z < 0.00000001f && ZVec.z > -0.00000001f )
	{
		GBASE.CameraHRotate = 0.0f ;
		if( ZVec.y < 0.0f )
		{
			GBASE.CameraVRotate = ( float )DX_PI / 2.0f ;
			GBASE.CameraTRotate = _ATAN2( -YVec.x, YVec.z ) ;
		}
		else
		{
			GBASE.CameraVRotate = ( float )-DX_PI / 2.0f ;
			GBASE.CameraTRotate = _ATAN2( -YVec.x, -YVec.z ) ;
		}
	}
	else
	{
		if( YVec.y < 0.0f )
		{
			GBASE.CameraHRotate = _ATAN2( -ZVec.x, -ZVec.z ) ;

			_SINCOS( -GBASE.CameraHRotate, &Sin, &Cos ) ;
			f      = XVec.z * Cos - XVec.x * Sin ;
			XVec.x = XVec.z * Sin + XVec.x * Cos ;
			XVec.z = f ;

			f      = ZVec.z * Cos - ZVec.x * Sin ;
			ZVec.x = ZVec.z * Sin + ZVec.x * Cos ;
			ZVec.z = f ;

			if( ZVec.y > 0.0f )
			{
				GBASE.CameraVRotate = -_ATAN2( -ZVec.z, ZVec.y ) - ( float )DX_PI / 2.0f ;
			}
			else
			{
				GBASE.CameraVRotate =  _ATAN2( -ZVec.z, -ZVec.y ) + ( float )DX_PI / 2.0f ;
			}

			_SINCOS( -GBASE.CameraVRotate, &Sin, &Cos ) ;
			f      = XVec.y * Cos - XVec.z * Sin ;
			XVec.z = XVec.y * Sin + XVec.z * Cos ;
			XVec.y = f ;

			f      = ZVec.y * Cos - ZVec.z * Sin ;
			ZVec.z = ZVec.y * Sin + ZVec.z * Cos ;
			ZVec.y = f ;

			GBASE.CameraTRotate = _ATAN2( XVec.y, XVec.x ) ;
		}
		else
		{
			GBASE.CameraHRotate = _ATAN2( ZVec.x, ZVec.z ) ;

			_SINCOS( -GBASE.CameraHRotate, &Sin, &Cos ) ;
			f      = XVec.z * Cos - XVec.x * Sin ;
			XVec.x = XVec.z * Sin + XVec.x * Cos ;
			XVec.z = f ;

			f      = ZVec.z * Cos - ZVec.x * Sin ;
			ZVec.x = ZVec.z * Sin + ZVec.x * Cos ;
			ZVec.z = f ;

			GBASE.CameraVRotate = _ATAN2( -ZVec.y, ZVec.z ) ;

			_SINCOS( -GBASE.CameraVRotate, &Sin, &Cos ) ;
			f      = XVec.y * Cos - XVec.z * Sin ;
			XVec.z = XVec.y * Sin + XVec.z * Cos ;
			XVec.y = f ;

			f      = ZVec.y * Cos - ZVec.z * Sin ;
			ZVec.z = ZVec.y * Sin + ZVec.z * Cos ;
			ZVec.y = f ;

			GBASE.CameraTRotate = _ATAN2( XVec.y, XVec.x ) ;
		}
	}

//	NS_DrawFormatString( 0, 20, NS_GetColor( 255,255,255 ), "H:%.3f V:%.3f T:%.3f", GBASE.CameraHRotate, GBASE.CameraVRotate, GBASE.CameraTRotate ) ;

}

// �J������ Near�N���b�v�ʂ� Far�N���b�v�ʂ̋�����ݒ肷��
extern int NS_SetCameraNearFar( float Near, float Far )
{
	// �p�����[�^�̕ۑ�
	GBASE.ProjNear = Near ;
	GBASE.ProjFar = Far ;

	// �ˉe�s��̍X�V
	RefreshProjectionMatrix() ;

	// �I��
	return 0 ;
}

// �J�����̐ݒ������A���_�A�����_�A�A�b�v�x�N�g���͂x����
extern int NS_SetCameraPositionAndTarget_UpVecY( VECTOR Position, VECTOR Target )
{
	VECTOR Up = { 0.0f, 1.0f, 0.0f }, Side, Dir ;
	MATRIX LookAt ;

	// ���_�ƒ����_��ۑ�
	GBASE.CameraPosition = Position ;
	GBASE.CameraTarget = Target ;

	// �A�b�v�x�N�g�����Z�o
	VectorSub( &Dir, &GBASE.CameraTarget, &GBASE.CameraPosition ) ;
	VectorOuterProduct( &Side, &Dir, &Up ) ;
	VectorOuterProduct( &GBASE.CameraUp, &Side, &Dir ) ;
	VectorNormalize( &GBASE.CameraUp, &GBASE.CameraUp ) ;

	// �s��̍쐬
	CreateLookAtMatrix( &LookAt, &GBASE.CameraPosition, &GBASE.CameraTarget, &GBASE.CameraUp ) ;

	// �r���[�s��ɃZ�b�g
	NS_SetTransformToView( &LookAt ) ;
	GBASE.CameraMatrix = LookAt ;

	// ��]�p�x���Z�o
	ViewMatrixToCameraRoll() ;

	// �I��
	return 0 ;
}

// �J�����̐ݒ������A���_�A�����_�A�A�b�v�x�N�g���w���
extern int NS_SetCameraPositionAndTargetAndUpVec( VECTOR Position, VECTOR Target, VECTOR Up )
{
	VECTOR Side, Dir ;
	MATRIX LookAt ;

	// ���_�ƒ����_��ۑ�
	GBASE.CameraPosition = Position ;
	GBASE.CameraTarget = Target ;

	// �A�b�v�x�N�g�����Z�o
	VectorSub( &Dir, &GBASE.CameraTarget, &GBASE.CameraPosition ) ;
	VectorOuterProduct( &Side, &Dir, &Up ) ;
	VectorOuterProduct( &GBASE.CameraUp, &Side, &Dir ) ;
	VectorNormalize( &GBASE.CameraUp, &GBASE.CameraUp ) ;

	// �s��̍쐬
	CreateLookAtMatrix( &LookAt, &GBASE.CameraPosition, &GBASE.CameraTarget, &GBASE.CameraUp ) ;

	// �r���[�s��ɃZ�b�g
	NS_SetTransformToView( &LookAt ) ;
	GBASE.CameraMatrix = LookAt ;

	// ��]�p�x���Z�o
	ViewMatrixToCameraRoll() ;

	// �I��
	return 0 ;
}

// �J�����̐ݒ������A������]�p�x�A������]�p�x�A�P���]�p�x�A���_�w���
extern int NS_SetCameraPositionAndAngle( VECTOR Position, float VRotate, float HRotate, float TRotate )
{
	VECTOR ZVec, YVec, XVec, TVec, Target ;
	MATRIX LookAt ;
	float Sin, Cos ;

	// ������]�p�x�A������]�p�x�A�P���]�p�x�A���_��ۑ�����
	GBASE.CameraHRotate = HRotate ;
	GBASE.CameraVRotate = VRotate ;
	GBASE.CameraTRotate = TRotate ;
	GBASE.CameraPosition = Position ;

	// �s��̍쐬
	_SINCOS( VRotate, &Sin, &Cos ) ;
	ZVec.x  =  0.0f ;
	ZVec.y  = -Sin ;
	ZVec.z  =  Cos ;

	YVec.z  =  Sin ;
	YVec.y  =  Cos ;
	YVec.x  = 0.0f ;

	_SINCOS( HRotate, &Sin, &Cos ) ;
	ZVec.x  =  Sin * ZVec.z ;
	ZVec.z *=  Cos ;

	YVec.x  =  Sin * YVec.z ;
	YVec.z *=  Cos ;

	XVec.x  =  Cos ;
	XVec.y  = 0.0f ;
	XVec.z  = -Sin ;

	_SINCOS( TRotate, &Sin, &Cos ) ;
	TVec.x = -Sin * XVec.x + Cos * YVec.x ;
	TVec.y = -Sin * XVec.y + Cos * YVec.y ;
	TVec.z = -Sin * XVec.z + Cos * YVec.z ;

	VectorAdd( &Target, &Position, &ZVec ) ;
	CreateLookAtMatrix( &LookAt, &Position, &Target, &TVec ) ;

	// �r���[�s��ɃZ�b�g
	NS_SetTransformToView( &LookAt ) ;
	GBASE.CameraMatrix = LookAt ;

	// ���_�ƒ����_��ۑ�
	GBASE.CameraPosition = Position ;
	GBASE.CameraTarget = Target ;
	GBASE.CameraUp = TVec ;

//	ViewMatrixToCameraRoll() ;

	// �I��
	return 0 ;
}

// �r���[�s��𒼐ڐݒ肷��
extern int NS_SetCameraViewMatrix( MATRIX ViewMatrix )
{
	// �p�����[�^��ۑ�
	GBASE.CameraMatrix = ViewMatrix ;

	// �r���[�s��ɃZ�b�g
	NS_SetTransformToView( &ViewMatrix ) ;

	// �I��
	return 0 ;
}

// ��ʏ�ɂ�����J���������Ă���f���̒��S�̍��W��ݒ肷��
extern int NS_SetCameraScreenCenter( float x, float y )
{
	MATRIX ViewportMatrix ;

	// �l��ۑ�
	GBASE.CameraScreenCenterX = x ;
	GBASE.CameraScreenCenterY = y ;

	// �r���[�|�[�g�s��̍쐬
	CreateViewportMatrix(
		&ViewportMatrix,
		x, y, 
		(float)( GBASE.DrawSizeX * GBASE.Draw3DScale ),
		(float)( GBASE.DrawSizeY * GBASE.Draw3DScale ) ) ;

	// �r���[�|�[�s��Ƃ��ăZ�b�g
	NS_SetTransformToViewport( &ViewportMatrix ) ;

	// �I��
	return 0 ;
}

// �R�c�`��̃X�P�[�����Z�b�g����
extern int NS_SetDraw3DScale( float Scale )
{
	// �X�P�[���l��␳
	if( Scale < 0.00001f && Scale > -0.00001f )
		Scale = 1.0f ;

	// �X�P�[���l��ۑ�
	GBASE.Draw3DScale = Scale ;

	// �����_�̍Čv�Z
	NS_SetCameraScreenCenter( GBASE.CameraScreenCenterX, GBASE.CameraScreenCenterY ) ;

	// �I��
	return 0 ;
}





// ���ߖ@�J�������Z�b�g�A�b�v����
extern int NS_SetupCamera_Perspective( float Fov )
{
	// �p�����[�^�̕ۑ�
	GBASE.ProjectionMatrixMode = 0 ;
	GBASE.ProjFov = Fov ;

	// �ˉe�s��̍X�V
	RefreshProjectionMatrix() ;

	// �I��
	return 0 ;
}

// ���ˉe�J�������Z�b�g�A�b�v����
extern int NS_SetupCamera_Ortho( float Size )
{
	// �p�����[�^�̕ۑ�
	GBASE.ProjectionMatrixMode = 1 ;
	GBASE.ProjSize = Size ;

	// �ˉe�s��̍X�V
	RefreshProjectionMatrix() ;

	// �I��
	return 0 ;
}

// �ˉe�s���ݒ肷��
extern int NS_SetupCamera_ProjectionMatrix( MATRIX ProjectionMatrix )
{
	// �p�����[�^�̕ۑ�
	GBASE.ProjectionMatrixMode = 2 ;
	GBASE.ProjMatrix = ProjectionMatrix ;

	// �ˉe�s��̍X�V
	RefreshProjectionMatrix() ;

	// �I��
	return 0 ;
}

extern int NS_SetTransformToProjection( const MATRIX *Matrix )
{
	return NS_SetupCamera_ProjectionMatrix( *Matrix ) ;
}

// �J�����̃h�b�g�A�X�y�N�g���ݒ肷��
extern int NS_SetCameraDotAspect( float DotAspect )
{
	// �p�����[�^�̕ۑ�
	GBASE.ProjDotAspect = DotAspect ;

	// �ˉe�s��̍X�V
	RefreshProjectionMatrix() ;

	// �I��
	return 0 ;
}





// �w��̍��W���J�����̎��E�ɓ����Ă��邩�ǂ����𔻒肷��( �߂�l TRUE:���E�ɓ����Ă��Ȃ�  FALSE:���E�ɓ����Ă��� )
extern int NS_CheckCameraViewClip( VECTOR CheckPos )
{
	if( GBASE.ClippingPlane[ 0 ].x * CheckPos.x + GBASE.ClippingPlane[ 0 ].y * CheckPos.y + GBASE.ClippingPlane[ 0 ].z * CheckPos.z + GBASE.ClippingPlane[ 0 ].w < 0.0f ||
		GBASE.ClippingPlane[ 1 ].x * CheckPos.x + GBASE.ClippingPlane[ 1 ].y * CheckPos.y + GBASE.ClippingPlane[ 1 ].z * CheckPos.z + GBASE.ClippingPlane[ 1 ].w < 0.0f ||
		GBASE.ClippingPlane[ 2 ].x * CheckPos.x + GBASE.ClippingPlane[ 2 ].y * CheckPos.y + GBASE.ClippingPlane[ 2 ].z * CheckPos.z + GBASE.ClippingPlane[ 2 ].w < 0.0f ||
		GBASE.ClippingPlane[ 3 ].x * CheckPos.x + GBASE.ClippingPlane[ 3 ].y * CheckPos.y + GBASE.ClippingPlane[ 3 ].z * CheckPos.z + GBASE.ClippingPlane[ 3 ].w < 0.0f ||
		GBASE.ClippingPlane[ 4 ].x * CheckPos.x + GBASE.ClippingPlane[ 4 ].y * CheckPos.y + GBASE.ClippingPlane[ 4 ].z * CheckPos.z + GBASE.ClippingPlane[ 4 ].w < 0.0f ||
		GBASE.ClippingPlane[ 5 ].x * CheckPos.x + GBASE.ClippingPlane[ 5 ].y * CheckPos.y + GBASE.ClippingPlane[ 5 ].z * CheckPos.z + GBASE.ClippingPlane[ 5 ].w < 0.0f )
		return TRUE ;

	return FALSE ;
}

// �w��̍��W���J�����̎��E�ɓ����Ă��邩�ǂ����𔻒肷��A�߂�l�ŊO��Ă���������m�邱�Ƃ��ł���( �߂�l 0:���E�ɓ����Ă���  0�ȊO:���E�ɓ����Ă��Ȃ�( DX_CAMERACLIP_LEFT �� DX_CAMERACLIP_RIGHT �� or ���Z�ō������ꂽ���́Aand ���Z�ŕ������m�F�ł��� ) )
extern int NS_CheckCameraViewClip_Dir( VECTOR CheckPos )
{
	int clip ;

	clip = 0 ;
	if( GBASE.ClippingPlane[ 0 ].x * CheckPos.x + GBASE.ClippingPlane[ 0 ].y * CheckPos.y + GBASE.ClippingPlane[ 0 ].z * CheckPos.z + GBASE.ClippingPlane[ 0 ].w < 0.0f ) clip |= DX_CAMERACLIP_LEFT ;
	if( GBASE.ClippingPlane[ 1 ].x * CheckPos.x + GBASE.ClippingPlane[ 1 ].y * CheckPos.y + GBASE.ClippingPlane[ 1 ].z * CheckPos.z + GBASE.ClippingPlane[ 1 ].w < 0.0f ) clip |= DX_CAMERACLIP_RIGHT ;
	if( GBASE.ClippingPlane[ 2 ].x * CheckPos.x + GBASE.ClippingPlane[ 2 ].y * CheckPos.y + GBASE.ClippingPlane[ 2 ].z * CheckPos.z + GBASE.ClippingPlane[ 2 ].w < 0.0f ) clip |= DX_CAMERACLIP_BOTTOM ;
	if( GBASE.ClippingPlane[ 3 ].x * CheckPos.x + GBASE.ClippingPlane[ 3 ].y * CheckPos.y + GBASE.ClippingPlane[ 3 ].z * CheckPos.z + GBASE.ClippingPlane[ 3 ].w < 0.0f ) clip |= DX_CAMERACLIP_TOP ;
	if( GBASE.ClippingPlane[ 4 ].x * CheckPos.x + GBASE.ClippingPlane[ 4 ].y * CheckPos.y + GBASE.ClippingPlane[ 4 ].z * CheckPos.z + GBASE.ClippingPlane[ 4 ].w < 0.0f ) clip |= DX_CAMERACLIP_BACK ;
	if( GBASE.ClippingPlane[ 5 ].x * CheckPos.x + GBASE.ClippingPlane[ 5 ].y * CheckPos.y + GBASE.ClippingPlane[ 5 ].z * CheckPos.z + GBASE.ClippingPlane[ 5 ].w < 0.0f ) clip |= DX_CAMERACLIP_FRONT ;

	return clip ;
}


// ��̍��W�ŕ\�����{�b�N�X���J�����̎��E�ɓ����Ă��邩�ǂ����𔻒肷��( �߂�l TRUE:���E�ɓ����Ă��Ȃ�  FALSE:���E�ɓ����Ă��� )
extern int NS_CheckCameraViewClip_Box( VECTOR BoxPos1, VECTOR BoxPos2 )
{
	VECTOR CheckBoxPos[ 8 ] ;
	VECTOR Tmp ;

	Tmp   = BoxPos2   ; CheckBoxPos[ 0 ] = Tmp ;
	Tmp.x = BoxPos1.x ; CheckBoxPos[ 1 ] = Tmp ;
	Tmp.y = BoxPos1.y ; CheckBoxPos[ 2 ] = Tmp ;
	Tmp.x = BoxPos2.x ; CheckBoxPos[ 3 ] = Tmp ;
	Tmp.z = BoxPos1.z ; CheckBoxPos[ 4 ] = Tmp ;
	Tmp.y = BoxPos2.y ; CheckBoxPos[ 5 ] = Tmp ;
	Tmp.x = BoxPos1.x ; CheckBoxPos[ 6 ] = Tmp ;
	Tmp.y = BoxPos1.y ; CheckBoxPos[ 7 ] = Tmp ;

	return CheckCameraViewClip_Box_PosDim( CheckBoxPos ) ;
}


// �W���W�Ō`���{�b�N�X���J�����̎��E�ɓ����Ă��邩�ǂ����𔻒肷��( �߂�l TRUE:���E�ɓ����Ă��Ȃ�  FALSE:���E�ɓ����Ă��� )( CheckPos�� VECTOR 8���̔z��̐擪�A�h���X�A�z��̊e�v�f�ԍ��̓��e 0:+x +y +z   1:-x +y +z   2:-x -y +z   3:+x -y +z   4:+x -y -z   5:+x +y -z   6:-x +y -z   7:-x -y -z )
extern int CheckCameraViewClip_Box_PosDim( VECTOR *CheckBoxPos )
{
	BYTE clip[ 8 ] ;
	VECTOR *pos ;
	int i ;

	// �e���_�����̈�ɓ����Ă��邩�ǂ����𒲂ׂ�
	pos = CheckBoxPos ;
	for( i = 0 ; i < 8 ; i ++, pos ++ )
	{
		clip[ i ] = 0 ;
		if( GBASE.ClippingPlane[ 0 ].x * pos->x + GBASE.ClippingPlane[ 0 ].y * pos->y + GBASE.ClippingPlane[ 0 ].z * pos->z + GBASE.ClippingPlane[ 0 ].w < 0.0f ) clip[ i ] |= CLIP_XM ;
		if( GBASE.ClippingPlane[ 1 ].x * pos->x + GBASE.ClippingPlane[ 1 ].y * pos->y + GBASE.ClippingPlane[ 1 ].z * pos->z + GBASE.ClippingPlane[ 1 ].w < 0.0f ) clip[ i ] |= CLIP_XP ;
		if( GBASE.ClippingPlane[ 2 ].x * pos->x + GBASE.ClippingPlane[ 2 ].y * pos->y + GBASE.ClippingPlane[ 2 ].z * pos->z + GBASE.ClippingPlane[ 2 ].w < 0.0f ) clip[ i ] |= CLIP_YM ;
		if( GBASE.ClippingPlane[ 3 ].x * pos->x + GBASE.ClippingPlane[ 3 ].y * pos->y + GBASE.ClippingPlane[ 3 ].z * pos->z + GBASE.ClippingPlane[ 3 ].w < 0.0f ) clip[ i ] |= CLIP_YP ;
		if( GBASE.ClippingPlane[ 4 ].x * pos->x + GBASE.ClippingPlane[ 4 ].y * pos->y + GBASE.ClippingPlane[ 4 ].z * pos->z + GBASE.ClippingPlane[ 4 ].w < 0.0f ) clip[ i ] |= CLIP_ZM ;
		if( GBASE.ClippingPlane[ 5 ].x * pos->x + GBASE.ClippingPlane[ 5 ].y * pos->y + GBASE.ClippingPlane[ 5 ].z * pos->z + GBASE.ClippingPlane[ 5 ].w < 0.0f ) clip[ i ] |= CLIP_ZP ;
		if( clip[ i ] == 0 ) return FALSE ;
	}

	// �ǂꂩ�̑��ɑS�Ă̒��_������Ă���ꍇ�͂P�O�O�����̈�ɓ����Ă��Ȃ�
	if( ( ( clip[0] & CLIP_XP ) && ( clip[1] & CLIP_XP ) &&
		  ( clip[2] & CLIP_XP ) && ( clip[3] & CLIP_XP ) &&
		  ( clip[4] & CLIP_XP ) && ( clip[5] & CLIP_XP ) &&
		  ( clip[6] & CLIP_XP ) && ( clip[7] & CLIP_XP ) ) ||
		( ( clip[0] & CLIP_XM ) && ( clip[1] & CLIP_XM ) &&
		  ( clip[2] & CLIP_XM ) && ( clip[3] & CLIP_XM ) &&
		  ( clip[4] & CLIP_XM ) && ( clip[5] & CLIP_XM ) &&
		  ( clip[6] & CLIP_XM ) && ( clip[7] & CLIP_XM ) ) ||
		( ( clip[0] & CLIP_YP ) && ( clip[1] & CLIP_YP ) &&
		  ( clip[2] & CLIP_YP ) && ( clip[3] & CLIP_YP ) &&
		  ( clip[4] & CLIP_YP ) && ( clip[5] & CLIP_YP ) &&
		  ( clip[6] & CLIP_YP ) && ( clip[7] & CLIP_YP ) ) ||
		( ( clip[0] & CLIP_YM ) && ( clip[1] & CLIP_YM ) &&
		  ( clip[2] & CLIP_YM ) && ( clip[3] & CLIP_YM ) &&
		  ( clip[4] & CLIP_YM ) && ( clip[5] & CLIP_YM ) &&
		  ( clip[6] & CLIP_YM ) && ( clip[7] & CLIP_YM ) ) ||
		( ( clip[0] & CLIP_ZP ) && ( clip[1] & CLIP_ZP ) &&
		  ( clip[2] & CLIP_ZP ) && ( clip[3] & CLIP_ZP ) &&
		  ( clip[4] & CLIP_ZP ) && ( clip[5] & CLIP_ZP ) &&
		  ( clip[6] & CLIP_ZP ) && ( clip[7] & CLIP_ZP ) ) ||
		( ( clip[0] & CLIP_ZM ) && ( clip[1] & CLIP_ZM ) &&
		  ( clip[2] & CLIP_ZM ) && ( clip[3] & CLIP_ZM ) &&
		  ( clip[4] & CLIP_ZM ) && ( clip[5] & CLIP_ZM ) &&
		  ( clip[6] & CLIP_ZM ) && ( clip[7] & CLIP_ZM ) ) )
		  return TRUE ;

	// �����ɂ����猩���Ă���Ƃ�������
	return FALSE ;
}
#if 0
{
	int i ;
	float x[ 8 ], y[ 8 ], z[ 8 ], w[ 8 ], revw ;
	VECTOR *pos ;
	BYTE clip[ 8 ] ;

	// �e���_�����̈�ɓ����Ă��邩�ǂ����𒲂ׂ�
	pos = CheckBoxPos ;
	for( i = 0 ; i < 8 ; i ++, pos ++ )
	{
		x[ i ] = GBASE.ViewProjectionViewportMatrix.m[ 0 ][ 0 ] * pos->x + GBASE.ViewProjectionViewportMatrix.m[ 1 ][ 0 ] * pos->y + GBASE.ViewProjectionViewportMatrix.m[ 2 ][ 0 ] * pos->z + GBASE.ViewProjectionViewportMatrix.m[ 3 ][ 0 ] ;
		y[ i ] = GBASE.ViewProjectionViewportMatrix.m[ 0 ][ 1 ] * pos->x + GBASE.ViewProjectionViewportMatrix.m[ 1 ][ 1 ] * pos->y + GBASE.ViewProjectionViewportMatrix.m[ 2 ][ 1 ] * pos->z + GBASE.ViewProjectionViewportMatrix.m[ 3 ][ 1 ] ;
		z[ i ] = GBASE.ViewProjectionViewportMatrix.m[ 0 ][ 2 ] * pos->x + GBASE.ViewProjectionViewportMatrix.m[ 1 ][ 2 ] * pos->y + GBASE.ViewProjectionViewportMatrix.m[ 2 ][ 2 ] * pos->z + GBASE.ViewProjectionViewportMatrix.m[ 3 ][ 2 ] ;
		w[ i ] = GBASE.ViewProjectionViewportMatrix.m[ 0 ][ 3 ] * pos->x + GBASE.ViewProjectionViewportMatrix.m[ 1 ][ 3 ] * pos->y + GBASE.ViewProjectionViewportMatrix.m[ 2 ][ 3 ] * pos->z + GBASE.ViewProjectionViewportMatrix.m[ 3 ][ 3 ] ;

		revw = 1.0f / w[ i ] ;
		x[ i ] *= revw ;
		y[ i ] *= revw ;
		z[ i ] *= revw ;

		// �����}�C�i�X�̏ꍇ�͍��W�𔽓]
		if( w[ i ] < 0.0f )
		{
			x[ i ] = -x[ i ] ;
			y[ i ] = -y[ i ] ;
			z[ i ] = -z[ i ] ;
		}

		// ������
		clip[ i ] = 0 ;
		     if( x[ i ] < GBASE.DrawAreaF.left   ) clip[ i ] |= CLIP_XM ;
		else if( x[ i ] > GBASE.DrawAreaF.right  ) clip[ i ] |= CLIP_XP ;
		     if( y[ i ] < GBASE.DrawAreaF.top    ) clip[ i ] |= CLIP_YM ;
		else if( y[ i ] > GBASE.DrawAreaF.bottom ) clip[ i ] |= CLIP_YP ;
		     if( z[ i ] <  0.0f ) clip[ i ] |= CLIP_ZM ;
		else if( z[ i ] >  1.0f ) clip[ i ] |= CLIP_ZP ;
		if( clip[ i ] == 0 ) return FALSE ;
	}

/*
	// �f�o�b�O�p�R�[�h
	for( int j = 0 ; j < 8 ; j ++ )
	{
		NS_DrawFormatString( 0, 32 + j * 16, NS_GetColor( 255,255,255 ), "%2d:%d%d%d%d%d%d    %.2f %.2f %.2f %.2f    %.2f %.2f %.2f",
			j,
			( clip[ j ] & CLIP_ZM ) != 0 ? 1 : 0,
			( clip[ j ] & CLIP_ZP ) != 0 ? 1 : 0,
			( clip[ j ] & CLIP_YM ) != 0 ? 1 : 0,
			( clip[ j ] & CLIP_YP ) != 0 ? 1 : 0,
			( clip[ j ] & CLIP_XM ) != 0 ? 1 : 0,
			( clip[ j ] & CLIP_XP ) != 0 ? 1 : 0,
			x[ j ], y[ j ], z[ j ], w[ j ], CheckBoxPos[ j ].x, CheckBoxPos[ j ].y, CheckBoxPos[ j ].z ) ;
	}
*/

	// ���ׂĂ̒��_�����̈�ɓ����Ă��Ȃ��Ă��{�b�N�X���͉̂��̈�ɓ����Ă��邱�Ƃ�����̂ł��̔���

	// �ǂꂩ�̑��ɑS�Ă̒��_������Ă���ꍇ�͂P�O�O�����̈�ɓ����Ă��Ȃ�
	if( ( ( clip[0] & CLIP_XP ) && ( clip[1] & CLIP_XP ) &&
		  ( clip[2] & CLIP_XP ) && ( clip[3] & CLIP_XP ) &&
		  ( clip[4] & CLIP_XP ) && ( clip[5] & CLIP_XP ) &&
		  ( clip[6] & CLIP_XP ) && ( clip[7] & CLIP_XP ) ) ||
		( ( clip[0] & CLIP_XM ) && ( clip[1] & CLIP_XM ) &&
		  ( clip[2] & CLIP_XM ) && ( clip[3] & CLIP_XM ) &&
		  ( clip[4] & CLIP_XM ) && ( clip[5] & CLIP_XM ) &&
		  ( clip[6] & CLIP_XM ) && ( clip[7] & CLIP_XM ) ) ||
		( ( clip[0] & CLIP_YP ) && ( clip[1] & CLIP_YP ) &&
		  ( clip[2] & CLIP_YP ) && ( clip[3] & CLIP_YP ) &&
		  ( clip[4] & CLIP_YP ) && ( clip[5] & CLIP_YP ) &&
		  ( clip[6] & CLIP_YP ) && ( clip[7] & CLIP_YP ) ) ||
		( ( clip[0] & CLIP_YM ) && ( clip[1] & CLIP_YM ) &&
		  ( clip[2] & CLIP_YM ) && ( clip[3] & CLIP_YM ) &&
		  ( clip[4] & CLIP_YM ) && ( clip[5] & CLIP_YM ) &&
		  ( clip[6] & CLIP_YM ) && ( clip[7] & CLIP_YM ) ) ||
		( ( clip[0] & CLIP_ZP ) && ( clip[1] & CLIP_ZP ) &&
		  ( clip[2] & CLIP_ZP ) && ( clip[3] & CLIP_ZP ) &&
		  ( clip[4] & CLIP_ZP ) && ( clip[5] & CLIP_ZP ) &&
		  ( clip[6] & CLIP_ZP ) && ( clip[7] & CLIP_ZP ) ) ||
		( ( clip[0] & CLIP_ZM ) && ( clip[1] & CLIP_ZM ) &&
		  ( clip[2] & CLIP_ZM ) && ( clip[3] & CLIP_ZM ) &&
		  ( clip[4] & CLIP_ZM ) && ( clip[5] & CLIP_ZM ) &&
		  ( clip[6] & CLIP_ZM ) && ( clip[7] & CLIP_ZM ) ) )
		  return TRUE ;

	// �����ɂ����猩���Ă���Ƃ�������
	return FALSE ;
}
#endif







// �J������ Near �N���b�v�ʂ̋������擾����
extern float NS_GetCameraNear( void )
{
	return GBASE.ProjNear ;
}

// �J������ Far �N���b�v�ʂ̋������擾����
extern float NS_GetCameraFar( void )
{
	return GBASE.ProjFar ;
}









// �J�����̈ʒu���擾����
extern	VECTOR	NS_GetCameraPosition( void )
{
	return GBASE.CameraPosition ;
}

// �J�����̒����_���擾����
extern	VECTOR	NS_GetCameraTarget( void )
{
	return GBASE.CameraTarget ;
}

// �J�����̎����ɐ����ȏ�����̃x�N�g���𓾂�
extern	VECTOR	NS_GetCameraUpVector( void )
{
	return GBASE.CameraUp ;
}

// �J�����̐��������̌������擾����
extern	float	NS_GetCameraAngleHRotate( void )
{
	return GBASE.CameraHRotate ;
}

// �J�����̐��������̌������擾����
extern	float	NS_GetCameraAngleVRotate( void )
{
	return GBASE.CameraVRotate ;
}

// �J�����̌����̔P��p�x���擾����
extern	float	NS_GetCameraAngleTRotate( void )
{
	return GBASE.CameraTRotate ;
}




// �r���[�s����擾����
extern MATRIX NS_GetCameraViewMatrix( void )
{
//	return GBASE.CameraMatrix ;
	return GBASE.ViewMatrix ;
}

// �r���{�[�h�s����擾����
extern MATRIX NS_GetCameraBillboardMatrix( void )
{
	return GBASE.BillboardMatrix ;
}

// �J�����̎���p���擾����
extern float NS_GetCameraFov( void )
{
	return GBASE.ProjFov ;
}

// �J�����̎���T�C�Y���擾����
extern float NS_GetCameraSize( void )
{
	return GBASE.ProjSize ;
}

// �ˉe�s����擾����
extern MATRIX NS_GetCameraProjectionMatrix( void )
{
//	return GBASE.ProjMatrix ;
	return GBASE.ProjectionMatrix ;
}

// �J�����̃h�b�g�A�X�y�N�g��𓾂�
extern float NS_GetCameraDotAspect( void )
{
	return GBASE.ProjDotAspect ;
}

// �r���[�|�[�g�s����擾����
extern MATRIX NS_GetCameraViewportMatrix( void )
{
	return GBASE.ViewportMatrix ;
}

// Direct3D�Ŏ����K�p�����r���[�|�[�g�s����擾����
extern MATRIX NS_GetCameraAPIViewportMatrix( void )
{
	return GBASE.Direct3DViewportMatrix ;
}










// ���C�g�֌W�֐�

// ���C�e�B���O�v�Z�@�\���g�p���邩�ǂ�����ݒ肷��
extern int NS_SetUseLighting( int Flag )
{
//	if( GBASE.Light.ProcessDisable == !Flag )
//		return 0 ;
	GBASE.Light.ProcessDisable = Flag ? FALSE : TRUE ;

	// �Z�b�g
	D_SetLightEnable( Flag ) ;

	// ���C�g�̐ݒ肪�ύX���ꂽ�t���O�𗧂Ă�
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�e�B���O�v�Z�ɒ��_�J���[�̃f�B�t���[�Y�J���[���g�p���邩�ǂ�����ݒ肷��
extern int NS_SetMaterialUseVertDifColor( int UseFlag )
{
	GBASE.Light.MaterialNotUseVertexDiffuseColor = UseFlag ? FALSE : TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�e�B���O�v�Z�ɒ��_�J���[�̃X�y�L�����J���[���g�p���邩�ǂ�����ݒ肷��
extern int NS_SetMaterialUseVertSpcColor( int UseFlag )
{
	GBASE.Light.MaterialNotUseVertexSpecularColor = UseFlag ? FALSE : TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�e�B���O�v�Z�Ɏg�p����}�e���A���p�����[�^��ݒ肷��
extern int NS_SetMaterialParam( MATERIALPARAM Material )
{
	GBASE.Light.ChangeMaterial = 1 ;
	GBASE.Light.Material = Material ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �R�c�}�`�`��ɃX�y�L�������g�p���邩�ǂ�����ݒ肷��
extern int NS_SetUseSpecular( int UseFlag )
{
	GBASE.NotUseSpecular = UseFlag ? FALSE : TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �O���[�o���A���r�G���g���C�g�J���[��ݒ肷��
extern int NS_SetGlobalAmbientLight( COLOR_F Color )
{
	     if( Color.r < 0.0f ) Color.r = 0.0f ;
	else if( Color.r > 1.0f ) Color.r = 1.0f ;
	     if( Color.g < 0.0f ) Color.g = 0.0f ;
	else if( Color.g > 1.0f ) Color.g = 1.0f ;
	     if( Color.b < 0.0f ) Color.b = 0.0f ;
	else if( Color.b > 1.0f ) Color.b = 1.0f ;

	// �n�[�h�E�G�A�ɐݒ肷��
	D_SetAmbientLight( _DTOL( Color.r * 255.0f ), _DTOL( Color.g * 255.0f ), _DTOL( Color.b * 255.0f ) ) ;

	// �I��
	return 0 ;
}

// ���C�g�̃^�C�v���f�B���N�V���i�����C�g�ɂ���
extern int NS_ChangeLightTypeDir( VECTOR Direction )
{
	LIGHT_HANDLE *pLH ;
	float Length ;

	LIGHTCHK( GBASE.Light.DefaultHandle, pLH, -1 ) ;

	// �^�C�v��ύX
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_DIRECTIONAL ;

	// �������Z�b�g
	Length = _SQRT( VectorInnerProduct( &Direction, &Direction ) ) ;
	Direction.x /= Length ;
	Direction.y /= Length ;
	Direction.z /= Length ;
	pLH->Param.Direction = Direction ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�̃^�C�v���X�|�b�g���C�g�ɂ���
extern int NS_ChangeLightTypeSpot( VECTOR Position, VECTOR Direction, float OutAngle, float InAngle/*, float Falloff*/, float Range, float Atten0, float Atten1, float Atten2 )
{
	float Falloff = 1.0f ;

	LIGHT_HANDLE *pLH ;
	LIGHTCHK( GBASE.Light.DefaultHandle, pLH, -1 ) ;

	// �^�C�v���Z�b�g
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_SPOT ;

	// �p�����[�^���Z�b�g
	pLH->Param.Position = Position ;
	pLH->Param.Direction = Direction ;
	pLH->Param.Phi = OutAngle ;
	pLH->Param.Theta = InAngle ;
	pLH->Param.Falloff = Falloff ;
	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�̃^�C�v���|�C���g���C�g�ɂ���
extern int NS_ChangeLightTypePoint( VECTOR Position, float Range, float Atten0, float Atten1, float Atten2 )
{
	LIGHT_HANDLE *pLH ;
	LIGHTCHK( GBASE.Light.DefaultHandle, pLH, -1 ) ;

	// �^�C�v���Z�b�g
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_POINT ;

	// �p�����[�^���Z�b�g
	pLH->Param.Range = Range ;
	pLH->Param.Position = Position ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�̗L���A�������Z�b�g����
extern int NS_SetLightEnable( int EnableFlag )
{
	return NS_SetLightEnableHandle( GBASE.Light.DefaultHandle, EnableFlag ) ;
}

// ���C�g�̃f�B�t���[�Y�J���[��ݒ肷��
extern int NS_SetLightDifColor( COLOR_F Color )
{
	return NS_SetLightDifColorHandle( GBASE.Light.DefaultHandle, Color ) ;
}

// ���C�g�̃X�y�L�����J���[��ݒ肷��
extern int NS_SetLightSpcColor( COLOR_F Color )
{
	return NS_SetLightSpcColorHandle( GBASE.Light.DefaultHandle, Color ) ;
}

// ���C�g�̃A���r�G���g�J���[��ݒ肷��
extern int NS_SetLightAmbColor( COLOR_F Color )
{
	return NS_SetLightAmbColorHandle( GBASE.Light.DefaultHandle, Color ) ;
}

// ���C�g�̕�����ݒ肷��
extern int NS_SetLightDirection( VECTOR Direction )
{
	return NS_SetLightDirectionHandle( GBASE.Light.DefaultHandle, Direction ) ;
}

// ���C�g�̈ʒu��ݒ肷��
extern int NS_SetLightPosition( VECTOR Position )
{
	return NS_SetLightPositionHandle( GBASE.Light.DefaultHandle, Position ) ;
}

// ���C�g�̋��������p�����[�^��ݒ肷��( �L�������A���������W���O�A�P�A�Q )
extern int NS_SetLightRangeAtten( float Range, float Atten0, float Atten1, float Atten2 )
{
	return NS_SetLightRangeAttenHandle( GBASE.Light.DefaultHandle, Range,  Atten0,  Atten1,  Atten2 ) ;
}

// ���C�g�̃X�|�b�g���C�g�̃p�����[�^��ݒ肷��( �O���R�[���p�x�A�����R�[���p�x�A�t�H�[���I�t( 1.0f �𐄏� ) )
extern int NS_SetLightAngle( float OutAngle, float InAngle /*, float Falloff */ )
{
	return NS_SetLightAngleHandle( GBASE.Light.DefaultHandle,  OutAngle,  InAngle /*,  Falloff */ ) ;
}

// �f�t�H���g���C�g�� SetUseShadowMap �Ŏw�肵���V���h�E�}�b�v��K�p���邩�ǂ�����ݒ肷��( SmSlotIndex:�V���h�E�}�b�v�X���b�g( SetUseShadowMap �̑������ɐݒ肷��l ) UseFlag:�K�p�ɂ��邩�ǂ����̃t���O( TRUE:�K�p����( �f�t�H���g )  FALSE:�K�p���Ȃ� ) )
extern int NS_SetLightUseShadowMap( int SmSlotIndex, int UseFlag )
{
	return NS_SetLightUseShadowMapHandle( GBASE.Light.DefaultHandle, SmSlotIndex, UseFlag ) ;
}

// ���C�g�n���h���̃��C�g�̃^�C�v���擾����( �߂�l�� DX_LIGHTTYPE_DIRECTIONAL �� )
extern	int NS_GetLightType( void )
{
	return NS_GetLightTypeHandle( GBASE.Light.DefaultHandle ) ;
}

// ���C�g�n���h���̃��C�g���ʂ̗L���A������ݒ肷��( TRUE:�L��  FALSE:���� )
extern	int NS_GetLightEnable( void )
{
	return NS_GetLightEnableHandle( GBASE.Light.DefaultHandle ) ;
}

// ���C�g�n���h���̃��C�g�̃f�B�t���[�Y�J���[��ݒ肷��
extern COLOR_F NS_GetLightDifColor( void )
{
	return NS_GetLightDifColorHandle( GBASE.Light.DefaultHandle ) ;
}

// ���C�g�n���h���̃��C�g�̃X�y�L�����J���[��ݒ肷��
extern COLOR_F NS_GetLightSpcColor(  void )
{
	return NS_GetLightSpcColorHandle( GBASE.Light.DefaultHandle ) ;
}

// ���C�g�n���h���̃��C�g�̃A���r�G���g�J���[��ݒ肷��
extern COLOR_F NS_GetLightAmbColor(  void )
{
	return NS_GetLightAmbColorHandle( GBASE.Light.DefaultHandle ) ;
}

// ���C�g�n���h���̃��C�g�̕�����ݒ肷��
extern	VECTOR NS_GetLightDirection( void )
{
	return NS_GetLightDirectionHandle( GBASE.Light.DefaultHandle ) ;
}

// ���C�g�n���h���̃��C�g�̈ʒu��ݒ肷��
extern	VECTOR NS_GetLightPosition( void )
{
	return NS_GetLightPositionHandle( GBASE.Light.DefaultHandle ) ;
}

// ���C�g�̋��������p�����[�^���擾����( �L�������A���������W���O�A�P�A�Q )
extern int NS_GetLightRangeAtten( float *Range, float *Atten0, float *Atten1, float *Atten2 )
{
	return NS_GetLightRangeAttenHandle( GBASE.Light.DefaultHandle, Range, Atten0, Atten1, Atten2 ) ;
}

// ���C�g�̃X�|�b�g���C�g�̃p�����[�^���擾����( �O���R�[���p�x�A�����R�[���p�x�A�t�H�[���I�t )
extern int NS_GetLightAngle( float *OutAngle, float *InAngle /*, float *Falloff */ )
{
	return NS_GetLightAngleHandle( GBASE.Light.DefaultHandle, OutAngle, InAngle /*, Falloff */ ) ;
}

// ���C�g�n���h���̒ǉ�
static int AddLightHandle( void )
{
	LIGHT_HANDLE *pLH, **ppLH ;
	int i ;

	// �g�p����Ă��Ȃ����C�g�n���h����T��
	if( GBASE.Light.Area != MAX_LIGHT_NUM )
	{
		ppLH = &GBASE.Light.Data[ GBASE.Light.Area ] ;
		i = GBASE.Light.Area ;
	}
	else
	{
		ppLH = GBASE.Light.Data ;
		for( i = 0 ; *ppLH != NULL ; i ++, ppLH ++ ){}
	}

	// �f�[�^�̈���m�ۂ���
	if( ( pLH = *ppLH = ( LIGHT_HANDLE * )DXCALLOC( sizeof( LIGHT_HANDLE ) ) ) == NULL )
		return DxLib_Error( DXSTRING( _T( "���C�g�p�̃������̊m�ۂɎ��s���܂���" ) ) ) ;

	// �[��������
	_MEMSET( pLH, 0, sizeof( LIGHT_HANDLE ) ) ;

	// �����l���Z�b�g
	pLH->Param.Ambient.r = 0.33f ;
	pLH->Param.Ambient.g = 0.33f ;
	pLH->Param.Ambient.b = 0.33f ;
	pLH->Param.Ambient.a = 0.33f ;
	pLH->Param.Diffuse.r = 1.00f ;
	pLH->Param.Diffuse.g = 1.00f ;
	pLH->Param.Diffuse.b = 1.00f ;
	pLH->Param.Diffuse.a = 1.00f ;
	pLH->Param.Specular.r = 1.00f ;
	pLH->Param.Specular.g = 1.00f ;
	pLH->Param.Specular.b = 1.00f ;
	pLH->Param.Specular.a = 1.00f ;
	pLH->Param.Attenuation0 = 0.1f ;
	pLH->Param.Attenuation1 = 0.0f ;
	pLH->Param.Attenuation2 = 0.0f ;
	pLH->Param.Falloff = 1.0f ;
	pLH->Param.Theta = 2.0f ;
	pLH->Param.Phi = 2.0f ;
	pLH->Param.Range = 2000.0f ;
	pLH->EnableFlag = FALSE ;
	pLH->SetD3DIndex = -1 ;
	pLH->ChangeFlag = FALSE ;

	// �G���[�`�F�b�N�p�h�c�̐ݒ�
	pLH->ID = GBASE.Light.HandleID ;
	GBASE.Light.HandleID ++ ;
	if( GBASE.Light.HandleID >= ( DX_HANDLECHECK_MASK >> DX_HANDLECHECK_ADDRESS ) )
		GBASE.Light.HandleID = 0 ;

	// �n���h���l���Z�b�g
	pLH->Handle = i | DX_HANDLETYPE_MASK_LIGHT | ( pLH->ID << DX_HANDLECHECK_ADDRESS ) ;

	// ���C�g�̐��𑝂₷
	GBASE.Light.Num ++ ;

	// �g�p����Ă��郉�C�g�n���h�������݂���͈͂��X�V����
	if( GBASE.Light.Area == i ) GBASE.Light.Area ++ ;

	// �L���ɂ���
	NS_SetLightEnableHandle( pLH->Handle, TRUE ) ;

	// �n���h����Ԃ�
	return pLH->Handle ;
}

// �f�B���N�V���i�����C�g�n���h�����쐬����
extern int NS_CreateDirLightHandle( VECTOR Direction )
{
	int LHandle ;
	LIGHT_HANDLE *pLH ;
	float Length ;

	// �V�����n���h���̒ǉ�
	LHandle = AddLightHandle() ;
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// �^�C�v���Z�b�g
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_DIRECTIONAL ;

	// �������Z�b�g
	Length = _SQRT( VectorInnerProduct( &Direction, &Direction ) ) ;
	Direction.x /= Length ;
	Direction.y /= Length ;
	Direction.z /= Length ;
	pLH->Param.Direction = Direction ;

	// �n���h����Ԃ�
	return LHandle ;
}

// �X�|�b�g���C�g�n���h�����쐬����
extern int NS_CreateSpotLightHandle( VECTOR Position, VECTOR Direction, float OutAngle, float InAngle/*, float Falloff*/, float Range, float Atten0, float Atten1, float Atten2 )
{
	float Falloff = 1.0f ;
	int LHandle ;
	LIGHT_HANDLE *pLH ;

	// �V�����n���h���̒ǉ�
	LHandle = AddLightHandle() ;
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// �^�C�v���Z�b�g
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_SPOT ;

	// �p�����[�^���Z�b�g
	pLH->Param.Position = Position ;
	pLH->Param.Direction = Direction ;
	pLH->Param.Phi = OutAngle ;
	pLH->Param.Theta = InAngle ;
	pLH->Param.Falloff = Falloff ;
	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// �n���h����Ԃ�
	return LHandle ;
}

// �|�C���g���C�g�n���h�����쐬����
extern int NS_CreatePointLightHandle( VECTOR Position, float Range, float Atten0, float Atten1, float Atten2 )
{
	int LHandle ;
	LIGHT_HANDLE *pLH ;

	// �V�����n���h���̒ǉ�
	LHandle = AddLightHandle() ;
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// �^�C�v���Z�b�g
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_POINT ;

	// �p�����[�^���Z�b�g
	pLH->Param.Position = Position ;
	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// �n���h����Ԃ�
	return LHandle ;
}

// ���C�g�n���h�����폜����
extern int NS_DeleteLightHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// ���C�g�n���h���̑��������炷
	GBASE.Light.Num -- ;

	// �L���ȃ��C�g�n���h�������݂���͈͂̍X�V
	if( ( LHandle & DX_HANDLEINDEX_MASK ) == GBASE.Light.Area - 1 )
	{
		if( GBASE.Light.Num == 0 )
		{
			GBASE.Light.Area = 0 ;
		}
		else
		{
			while( GBASE.Light.Data[ GBASE.Light.Area - 1 ] == NULL )
				GBASE.Light.Area -- ; 
		}
	}

	// �f�[�^�̈���������
	DXFREE( pLH ) ;

	// �e�[�u���� NULL ���Z�b�g����
	GBASE.Light.Data[ LHandle & DX_HANDLEINDEX_MASK ] = NULL ;

	// �ύX�t���O�𗧂Ă�
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h����S�č폜����
extern int NS_DeleteLightHandleAll( void )
{
	int i ;
	LIGHT_HANDLE **ppLH ;

	// �W�����C�g�ȊO�̂��ׂẴ��C�g�f�[�^���폜����
	ppLH = GBASE.Light.Data ;
	for( i = 0 ; i < GBASE.Light.Area ; i ++, ppLH ++ )
	{
		if( *ppLH && ( *ppLH )->Handle != GBASE.Light.DefaultHandle )
			NS_DeleteLightHandle( ( *ppLH )->Handle ) ;
	}

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̃^�C�v��ύX����( DX_LIGHTTYPE_DIRECTIONAL �� )
extern int NS_SetLightTypeHandle( int LHandle, int LightType )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// ���C�g�^�C�v���s���������ꍇ�̓G���[
	if( LightType != DX_LIGHTTYPE_DIRECTIONAL &&
		LightType != DX_LIGHTTYPE_SPOT &&
		LightType != DX_LIGHTTYPE_POINT )
		return -1 ;

	// �^�C�v���Z�b�g
	pLH->Param.LightType = LightType ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̗L���A������ݒ肷��( TRUE:�L��  FALSE:���� )
extern int NS_SetLightEnableHandle( int LHandle, int EnableFlag )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// ���܂łƓ����ꍇ�͉��������ɏI��
	if( pLH->EnableFlag == EnableFlag )
		return 0 ;

	// �t���O��ۑ�
	pLH->EnableFlag = EnableFlag ;
	if( EnableFlag == FALSE )
	{
		pLH->SetD3DIndex = -1 ;
	}

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̃f�B�t���[�Y�J���[��ݒ肷��
extern int NS_SetLightDifColorHandle( int LHandle, COLOR_F Color )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	Color.a = 0.0f ;
	pLH->Param.Diffuse = Color ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̃X�y�L�����J���[��ݒ肷��
extern int NS_SetLightSpcColorHandle( int LHandle, COLOR_F Color )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	Color.a = 0.0f ;
	pLH->Param.Specular = Color ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̃A���r�G���g�J���[��ݒ肷��
extern int NS_SetLightAmbColorHandle( int LHandle, COLOR_F Color )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	Color.a = 0.0f ;
	pLH->Param.Ambient = Color ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̕�����ݒ肷��
extern int NS_SetLightDirectionHandle( int LHandle, VECTOR Direction )
{
	LIGHT_HANDLE *pLH ;
	float Length ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// ������ۑ�����
	Length = _SQRT( VectorInnerProduct( &Direction, &Direction ) ) ;
	Direction.x /= Length ;
	Direction.y /= Length ;
	Direction.z /= Length ;
	pLH->Param.Direction = Direction ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̈ʒu��ݒ肷��
extern int NS_SetLightPositionHandle( int LHandle, VECTOR Position )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	pLH->Param.Position = Position ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̋��������p�����[�^��ݒ肷��( �L�������A���������W���O�A�P�A�Q )
extern int NS_SetLightRangeAttenHandle( int LHandle, float Range, float Atten0, float Atten1, float Atten2 )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̃X�|�b�g���C�g�̃p�����[�^��ݒ肷��( �O���R�[���p�x�A�����R�[���p�x�A�t�H�[���I�t( 1.0f �𐄏� ) )
extern int NS_SetLightAngleHandle( int LHandle, float OutAngle, float InAngle /*, float Falloff */ )
{
	LIGHT_HANDLE *pLH ;
	float Falloff = 1.0f ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	pLH->Param.Phi = OutAngle ;
	pLH->Param.Theta = InAngle ;
	pLH->Param.Falloff = Falloff ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�� SetUseShadowMap �Ŏw�肵���V���h�E�}�b�v��K�p���邩�ǂ�����ݒ肷��
// ( SmSlotIndex:�V���h�E�}�b�v�X���b�g( SetUseShadowMap �̑������ɐݒ肷��l )
//   UseFlag:�K�p�ɂ��邩�ǂ����̃t���O( TRUE:�K�p����( �f�t�H���g )  FALSE:�K�p���Ȃ� ) )
extern int NS_SetLightUseShadowMapHandle( int LHandle, int SmSlotIndex, int UseFlag )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// �G���[�`�F�b�N
	if( SmSlotIndex < 0 || SmSlotIndex >= MAX_USE_SHADOWMAP_NUM )
	{
		return -1 ;
	}

	// �t���O�Z�b�g
	pLH->ShadowMapSlotDisableFlag[ SmSlotIndex ] = UseFlag == FALSE ? TRUE : FALSE ;

	// �ύX�t���O�𗧂Ă�
	pLH->ChangeFlag = TRUE ;
	GBASE.Light.D3DChange = TRUE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̃^�C�v���擾����( �߂�l�� DX_LIGHTTYPE_DIRECTIONAL �� )
extern int NS_GetLightTypeHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	return pLH->Param.LightType ;
}

// ���C�g�n���h���̃��C�g���ʂ̗L���A�������擾����( TRUE:�L��  FALSE:���� )
extern int NS_GetLightEnableHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	return pLH->EnableFlag ;
}

// ���C�g�n���h���̃��C�g�̃f�B�t���[�Y�J���[���擾����
extern COLOR_F NS_GetLightDifColorHandle( int LHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	// �F��Ԃ�
	return pLH->Param.Diffuse ;
}

// ���C�g�n���h���̃��C�g�̃X�y�L�����J���[���擾����
extern COLOR_F NS_GetLightSpcColorHandle( int LHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	// �F��Ԃ�
	return pLH->Param.Specular ;
}

// ���C�g�n���h���̃��C�g�̃A���r�G���g�J���[���擾����
extern COLOR_F NS_GetLightAmbColorHandle( int LHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	// �F��Ԃ�
	return pLH->Param.Ambient ;
}

// ���C�g�n���h���̃��C�g�̕������擾����
extern VECTOR NS_GetLightDirectionHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;
	VECTOR ErrorResult = { -1.0f, -1.0f, -1.0f } ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	return pLH->Param.Direction ;
}

// ���C�g�n���h���̃��C�g�̈ʒu���擾����
extern VECTOR NS_GetLightPositionHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;
	VECTOR ErrorResult = { -1.0f, -1.0f, -1.0f } ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	return pLH->Param.Position ;
}

// ���C�g�n���h���̃��C�g�̋��������p�����[�^���擾����( �L�������A���������W���O�A�P�A�Q )
extern int NS_GetLightRangeAttenHandle( int LHandle, float *Range, float *Atten0, float *Atten1, float *Atten2 )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	if( Range ) *Range = pLH->Param.Range ;
	if( Atten0 ) *Atten0 = pLH->Param.Attenuation0 ;
	if( Atten1 ) *Atten1 = pLH->Param.Attenuation1 ;
	if( Atten2 ) *Atten2 = pLH->Param.Attenuation2 ;

	// �I��
	return 0 ;
}

// ���C�g�n���h���̃��C�g�̃X�|�b�g���C�g�̃p�����[�^���擾����( �O���R�[���p�x�A�����R�[���p�x�A�t�H�[���I�t )
extern int NS_GetLightAngleHandle( int LHandle, float *OutAngle, float *InAngle /*, float *Falloff */ )
{
	LIGHT_HANDLE *pLH ;

	// �G���[����
	LIGHTCHK( LHandle, pLH, -1 ) ;

	if( OutAngle ) *OutAngle = pLH->Param.Phi ;
	if( InAngle  ) *InAngle  = pLH->Param.Theta ;
//	if( Falloff  ) *Falloff  = pLH->Param.Falloff ;

	// �I��
	return 0 ;
}

// �L���ɂȂ��Ă��郉�C�g�n���h���̐����擾����
extern int NS_GetEnableLightHandleNum( void )
{
	// �X�V�t���O�������Ă���������X�V����
	if( GBASE.Light.D3DChange )
	{
		RefreshLightState() ;
	}

	// �L���ɂȂ��Ă��郉�C�g�n���h���̐���Ԃ�
	return GBASE.Light.EnableNum ;
}

// �L���ɂȂ��Ă��郉�C�g�n���h�����擾����
extern int NS_GetEnableLightHandle( int Index )
{
	// �C���f�b�N�X�`�F�b�N
	if( Index < 0 || Index >= MAX_D3DENABLELIGHTINDEX_NUM )
		return -1 ;

	// �X�V�t���O�������Ă���������X�V����
	if( GBASE.Light.D3DChange )
	{
		RefreshLightState() ;
	}

	// �͈͊O�̏ꍇ�̓G���[
	if( Index >= GBASE.Light.EnableNum )
		return -1 ;

	// �n���h���l��Ԃ�
	return GBASE.Light.Data[ GBASE.Light.EnableD3DIndex[ Index ] ]->Handle ;
}


// �O���t�B�b�N�����֐��̗L�����擾	
extern	int NS_GetValidRestoreShredPoint( void )
{
	return GBASE.GraphRestoreShred != NULL ;
}

// �r���[�s����擾����
extern	int NS_GetTransformToViewMatrix( MATRIX *MatBuf )
{
	*MatBuf = GBASE.ViewMatrix ;

	// �I��
	return 0 ;
}

// ���[���h�s����擾����
extern	int NS_GetTransformToWorldMatrix( MATRIX *MatBuf )
{
	*MatBuf = GBASE.WorldMatrix ;

	// �I��
	return 0 ;
}

// �ˉe�s����擾����
extern	int NS_GetTransformToProjectionMatrix( MATRIX *MatBuf )
{
	*MatBuf = GBASE.ProjectionMatrix ;

	// �I��
	return 0 ;
}

// �r���[�|�[�g�s����擾����
extern	int NS_GetTransformToViewportMatrix( MATRIX *MatBuf )
{
	*MatBuf = GBASE.ViewportMatrix ;

	// �I��
	return 0 ;
}

// Direct3D�Ŏ����K�p�����r���[�|�[�g�s����擾����
extern	int NS_GetTransformToAPIViewportMatrix( MATRIX *MatBuf )
{
	*MatBuf = GBASE.Direct3DViewportMatrix ;

	// �I��
	return 0 ;
}


// ���[�J�����W�����ʍ��W���擾����
extern	int NS_GetTransformPosition( VECTOR *LocalPos, float *x, float *y )
{
	float w ;
	VECTOR ScreenPos ;

	if( LocalPos == NULL ) return -1 ;

	// ���W�ϊ�
	w = 1.0F ;
	VectorTransform4( &ScreenPos, &w,
						 LocalPos,   &w, &GBASE.BlendMatrix ) ;
	w = 1.0F / w ;

	// �y�l���O�ȉ��̏ꍇ�͐���Ȃw�x�l�͓����Ȃ��̂ŃG���[
	if( ScreenPos.z * w < 0.0F ) return -1 ;

	if( x != NULL ) *x = ScreenPos.x * w ;
	if( y != NULL ) *y = ScreenPos.y * w ;

	// ����I��
	return 0 ;
}

// ���[���h��ԏ�̃r���{�[�h�̃T�C�Y����X�N���[���ɓ��e�����ꍇ�̃s�N�Z���P�ʂ̃T�C�Y���擾����
extern float NS_GetBillboardPixelSize( VECTOR WorldPos, float WorldSize )
{
	VECTOR ScreenPos, ScreenLeft, ScreenRightDirection ;

	ScreenPos = NS_ConvWorldPosToScreenPos( WorldPos ) ;
	ScreenRightDirection = VNorm( VGet( GBASE.ViewMatrix.m[ 0 ][ 0 ], GBASE.ViewMatrix.m[ 1 ][ 0 ], GBASE.ViewMatrix.m[ 2 ][ 0 ] ) ) ;
	ScreenLeft = NS_ConvWorldPosToScreenPos( VSub( WorldPos, VScale( ScreenRightDirection, WorldSize / 2.0f ) ) ) ;
	return VSub( ScreenPos, ScreenLeft ).x * 2.0f ;
}

// ���[���h���W���J�������W�ɕϊ�����
extern VECTOR NS_ConvWorldPosToViewPos( VECTOR WorldPos )
{
	VECTOR ViewPos ;

	VectorTransform( &ViewPos, &WorldPos, &GBASE.ViewMatrix ) ;

	return ViewPos ;
}

// ���[���h���W���X�N���[�����W�ɕϊ�����
extern VECTOR NS_ConvWorldPosToScreenPos( VECTOR WorldPos )
{
	float w ;
	VECTOR ScreenPos ;

	// ���W�ϊ�
	w = 1.0F ;
	VectorTransform4( &ScreenPos, &w, &WorldPos, &w, &GBASE.BlendMatrix ) ;

	// ��ʍ��W��Ԃ�
	w = 1.0F / w ;
	ScreenPos.x *= w ;
	ScreenPos.y *= w ;
	ScreenPos.z *= w ;

	return ScreenPos ;
}

// ���[���h���W���X�N���[�����W�ɕϊ�����A�Ō�̂w�x�y���W���v�Ŋ���O�̒l�𓾂�
extern FLOAT4 NS_ConvWorldPosToScreenPosPlusW( VECTOR WorldPos )
{
	VECTOR ScreenPos ;
	float w ;
	FLOAT4 Result ;

	// ���W�ϊ�
	w = 1.0f ;
	VectorTransform4( &ScreenPos, &w, &WorldPos, &w, &GBASE.BlendMatrix ) ;

	Result.x = ScreenPos.x ;
	Result.y = ScreenPos.y ;
	Result.z = ScreenPos.z ;
	Result.w = w ;

	return Result ;
}

// �X�N���[�����W�����[���h���W�ɕϊ�����
extern VECTOR NS_ConvScreenPosToWorldPos( VECTOR ScreenPos )
{
	float w ;
	VECTOR WorldPos ;

	// �u�����h�s��̋t�s�񂪗L���ł͂Ȃ�������t�s����\�z����
	if( GBASE.ValidInverseBlendMatrix == FALSE )
	{
		CreateInverseMatrix( &GBASE.InverseBlendMatrix, &GBASE.BlendMatrix ) ;
		GBASE.ValidInverseBlendMatrix = TRUE ;
	}

	// ���W�ϊ�
	w = 1.0F ;
	VectorTransform4( &WorldPos, &w, &ScreenPos, &w, &GBASE.InverseBlendMatrix ) ;

	// ���[���h���W��Ԃ�
	w = 1.0F / w ;
	WorldPos.x *= w ;
	WorldPos.y *= w ;
	WorldPos.z *= w ;
	return WorldPos ;
}

// �X�N���[�����W�����[���h���W�ɕϊ�����( Z���W�����` )
extern VECTOR NS_ConvScreenPosToWorldPos_ZLinear( VECTOR ScreenPos )
{
	float w, inz ;
	VECTOR WorldPos ;

	// �u�����h�s��̋t�s�񂪗L���ł͂Ȃ�������t�s����\�z����
	if( GBASE.ValidInverseBlendMatrix == FALSE )
	{
		CreateInverseMatrix( &GBASE.InverseBlendMatrix, &GBASE.BlendMatrix ) ;
		GBASE.ValidInverseBlendMatrix = TRUE ;
	}

	// �X�N���[�����W�����[���h���W�ɕϊ�����ۂɎg�p����s�񂪗L���ł͂Ȃ�������\�z����
	if( GBASE.ValidConvScreenPosToWorldPosMatrix == FALSE )
	{
		MATRIX TempMatrix ;

		CreateMultiplyMatrix( &TempMatrix, &GBASE.ProjectionMatrix, &GBASE.ViewportMatrix ) ;
		CreateInverseMatrix( &GBASE.ConvScreenPosToWorldPosMatrix, &TempMatrix ) ;
		GBASE.ValidConvScreenPosToWorldPosMatrix = TRUE ;
	}

	// VectorTransform4 �ɓn���ׂ� z �l�����߂�
	{
		float k1, k2, resz, testz, minz, maxz, sa, target, sikii_plus, sikii_minus ;
		int num ;

		k1 = ScreenPos.x * GBASE.ConvScreenPosToWorldPosMatrix.m[0][2] + ScreenPos.y * GBASE.ConvScreenPosToWorldPosMatrix.m[1][2] + GBASE.ConvScreenPosToWorldPosMatrix.m[3][2] ;
		k2 = ScreenPos.x * GBASE.ConvScreenPosToWorldPosMatrix.m[0][3] + ScreenPos.y * GBASE.ConvScreenPosToWorldPosMatrix.m[1][3] + GBASE.ConvScreenPosToWorldPosMatrix.m[3][3] ;
		target = ScreenPos.z * ( GBASE.ProjFar - GBASE.ProjNear ) + GBASE.ProjNear ;
		testz = 0.5f ;
		sikii_plus  =  ( GBASE.ProjFar - GBASE.ProjNear ) / 100000.0f ;
		sikii_minus = -sikii_plus ;
		minz = 0.0f ;
		maxz = 1.0f ;
		for( num = 0 ; num < 20 ; num ++ )
		{
			resz = ( k1 + testz * GBASE.ConvScreenPosToWorldPosMatrix.m[2][2] ) / ( k2 + testz * GBASE.ConvScreenPosToWorldPosMatrix.m[2][3] ) ;
			if( resz > target )
			{
				maxz = testz ;
			}
			else
			{
				minz = testz ;
			}
			sa = resz - target ;
			if( sa < 0.0f )
			{
				if( sa > sikii_minus ) break ;
			}
			else
			{
				if( sa < sikii_plus  ) break ;
			}
			testz = ( maxz + minz ) * 0.5f ;
		}
		inz = testz ;
	}

	// ���W�ϊ�
	w = 1.0F ;
	ScreenPos.z = inz ;
	VectorTransform4( &WorldPos, &w, &ScreenPos, &w, &GBASE.InverseBlendMatrix ) ;

	// ���[���h���W��Ԃ�
	w = 1.0F / w ;
	WorldPos.x *= w ;
	WorldPos.y *= w ;
	WorldPos.z *= w ;
	return WorldPos ;
}





// �ݒ�n�֐�

// GetCreateGraphColorBitDepth �̋�����
extern int NS_GetGraphColorBitDepth( void )
{
	return NS_GetCreateGraphColorBitDepth() ;
}

// SetCreateGraphColorBitDepth �̋�����
extern int NS_SetGraphColorBitDepth( int ColorBitDepth )
{
	return NS_SetCreateGraphColorBitDepth( ColorBitDepth ) ;
}

// �쐬����O���t�B�b�N�̐F�[�x��ݒ肷��
extern int NS_SetCreateGraphColorBitDepth( int BitDepth )
{
	// �l��ۑ�����
	GBASE.CreateImageColorBitDepth = BitDepth ;
	
	// �I��
	return 0 ;
}

// �쐬����O���t�B�b�N�̐F�[�x���擾����
extern int NS_GetCreateGraphColorBitDepth( void )
{
	return GBASE.CreateImageColorBitDepth ;
}

// �쐬����O���t�B�b�N�̂P�`�����l���ӂ�̃r�b�g�[�x��ݒ肷��
extern int NS_SetCreateGraphChannelBitDepth( int BitDepth )
{
	// �l��ۑ�����
	GBASE.CreateImageChannelBitDepth = BitDepth ;
	
	// �I��
	return 0 ;
}

// �쐬����O���t�B�b�N�̂P�`�����l���ӂ�̃r�b�g�[�x���擾����
extern int NS_GetCreateGraphChannelBitDepth( void )
{
	return GBASE.CreateImageChannelBitDepth ;
}

// �`��\�ȃO���t�B�b�N���쐬���邩�ǂ����̃t���O���Z�b�g����( TRUE:�쐬����  FALSE:�쐬���Ȃ� )
extern int NS_SetDrawValidGraphCreateFlag( int Flag )
{
	// �t���O��ۑ�����
	GBASE.DrawValidImageCreateFlag = Flag ;
	
	// �I��
	return 0 ;
}

// �`��\�ȃO���t�B�b�N���쐬���邩�ǂ����̃t���O���擾����
extern int NS_GetDrawValidGraphCreateFlag( void )
{
	return GBASE.DrawValidImageCreateFlag ;
}

// �`��\�ȃ��`�����l���t���摜���쐬���邩�ǂ����̃t���O���Z�b�g����,SetDrawValidGraphCreateFlag �֐��ŕ`��\�摜���쐬����悤�ɐݒ肳��Ă��Ȃ��ƌ��ʂ��Ȃ�( TRUE:���`�����l���t�� FALSE:���`�����l���Ȃ� )
extern int NS_SetDrawValidAlphaChannelGraphCreateFlag( int Flag )
{
	// �t���O��ۑ�����
	GBASE.DrawValidAlphaImageCreateFlag = Flag ;
	
	// �I��
	return 0 ;
}

// �`��\�ȃ��`�����l���t���摜���쐬���邩�ǂ����̃t���O���擾����
extern int NS_GetDrawValidAlphaChannelGraphCreateFlag( void )
{
	// �t���O��Ԃ�
	return GBASE.DrawValidAlphaImageCreateFlag ;
}

// �`��\�ȕ��������_�^�̉摜���쐬���邩�ǂ����̃t���O���Z�b�g����,SetDrawValidGraphCreateFlag �֐��ŕ`��\�摜���쐬����悤�ɐݒ肳��Ă��Ȃ��ƌ��ʂ��Ȃ��A�f�o�t�� Float�^�̉摜�ɑΉ����Ă��Ȃ��ꍇ�͎��s����( TRUE:Float�^ FALSE:Int�^ )
extern int NS_SetDrawValidFloatTypeGraphCreateFlag( int Flag )
{
	// �t���O��ۑ�����
	GBASE.DrawValidFloatTypeGraphCreateFlag = Flag ;

	// �I��
	return 0 ;
}

// �`��\�ȕ��������_�^�̉摜���쐬���邩�ǂ����̃t���O���擾����
extern int NS_GetDrawValidFloatTypeGraphCreateFlag( void )
{
	// �t���O��Ԃ�
	return GBASE.DrawValidFloatTypeGraphCreateFlag ;
}

// �`��\�摜���쐬����ۂɐ�p�̂y�o�b�t�@���쐬���邩�ǂ�����ݒ肷��( �f�t�H���g�ł� TRUE( �쐬���� ) )
extern int NS_SetDrawValidGraphCreateZBufferFlag( int Flag )
{
	// �t���O��ۑ�����
	GBASE.NotDrawValidGraphCreateZBufferFlag = Flag ? FALSE : TRUE ;

	// �I��
	return 0 ;
}

// �`��\�摜���쐬����ۂɐ�p�̂y�o�b�t�@���쐬���邩�ǂ������擾����
extern int NS_GetDrawValidGraphCreateZBufferFlag( void )
{
	// �t���O��Ԃ�
	return GBASE.NotDrawValidGraphCreateZBufferFlag ? FALSE : TRUE ;
}

// SetDrawScreen �̈����Ƃ��ēn����( �`��ΏۂƂ��Ďg�p�ł��� )�O���t�B�b�N�n���h���ɓK�p����y�o�b�t�@�̃r�b�g�[�x��ݒ肷��( BitDepth:�r�b�g�[�x( �w��\�Ȓl�� 16, 24, 32 �̉��ꂩ( SetDrawValidGraphCreateFlag �֐��ŕ`��ΏۂƂ��Ďg�p�ł���O���t�B�b�N�n���h�����쐬����悤�ɐݒ肳��Ă��Ȃ��ƌ��ʂ���܂��� )
extern int NS_SetCreateDrawValidGraphZBufferBitDepth( int BitDepth )
{
	// �r�b�g�[�x��ۑ�����
	GBASE.CreateDrawValidGraphZBufferBitDepth = BitDepth ;

	// �I��
	return 0 ;
}

// SetDrawScreen �̈����Ƃ��ēn����( �`��ΏۂƂ��Ďg�p�ł��� )�O���t�B�b�N�n���h���ɓK�p����y�o�b�t�@�̃r�b�g�[�x���擾����
extern int NS_GetCreateDrawValidGraphZBufferBitDepth( void )
{
	// �r�b�g�[�x��Ԃ�
	return GBASE.CreateDrawValidGraphZBufferBitDepth == 0 ? 16 : GBASE.CreateDrawValidGraphZBufferBitDepth ;
}

// �쐬����`��\�ȉ摜�̃`�����l������ݒ肷��,SetDrawValidGraphCreateFlag �֐��ŕ`��\�摜���쐬����悤�ɐݒ肳��Ă��Ȃ��ƌ��ʂ��Ȃ�
extern int NS_SetCreateDrawValidGraphChannelNum( int ChannelNum )
{
	// �`�����l������ۑ�����
	GBASE.CreateDrawValidGraphChannelNum = ChannelNum ;

	// �I��
	return 0 ;
}

// �쐬����`��\�ȉ摜�̃`�����l�������擾����
extern int NS_GetCreateDrawValidGraphChannelNum( void )
{
	// �`�����l������Ԃ�
	return GBASE.CreateDrawValidGraphChannelNum ;
}

// �`��\�ȉ摜�̃}���`�T���v�����O�ݒ���s��
extern int NS_SetDrawValidMultiSample( int Samples, int Quality )
{
	if( Samples > 16 )
		Samples = 16 ;

	// �p�����[�^��ۑ�����
	GBASE.DrawValidMSSamples = Samples ;
	GBASE.DrawValidMSQuality = Quality ;

	// �I��
	return 0 ;
}

// �y�o�b�t�@��L���ɂ��邩�A�t���O���Z�b�g����
extern	int NS_SetUseZBufferFlag( int Flag )
{
	if( GBASE.EnableZBufferFlag2D == Flag &&
		GBASE.EnableZBufferFlag3D == Flag ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.EnableZBufferFlag2D = Flag ;
	GBASE.EnableZBufferFlag3D = Flag ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɏ������݂��s�����A�t���O���Z�b�g����
extern	int NS_SetWriteZBufferFlag( int Flag )
{
	if( GBASE.WriteZBufferFlag2D == Flag &&
		GBASE.WriteZBufferFlag3D == Flag ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.WriteZBufferFlag2D = Flag ;
	GBASE.WriteZBufferFlag3D = Flag ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �y�l�̔�r���[�h���Z�b�g����
extern	int NS_SetZBufferCmpType( int CmpType /* DX_CMP_NEVER �� */ )
{
	if( GBASE.ZBufferCmpType2D == CmpType &&
		GBASE.ZBufferCmpType3D == CmpType ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.ZBufferCmpType2D = CmpType ;
	GBASE.ZBufferCmpType3D = CmpType ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �y�o�C�A�X���Z�b�g����
extern	int NS_SetZBias( int Bias )
{
	if( GBASE.ZBias2D == Bias &&
		GBASE.ZBias3D == Bias ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.ZBias2D = Bias ;
	GBASE.ZBias3D = Bias ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �y�o�b�t�@��L���ɂ��邩�A�t���O���Z�b�g����( �R�c�}�`�`��݂̂ɉe�� )
extern int NS_SetUseZBuffer3D( int Flag )
{
	if( GBASE.EnableZBufferFlag3D == Flag ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.EnableZBufferFlag3D = Flag ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɏ������݂��s�����A�t���O���Z�b�g����( �R�c�}�`�`��݂̂ɉe�� )
extern int NS_SetWriteZBuffer3D( int Flag )
{
	if( GBASE.WriteZBufferFlag3D == Flag ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.WriteZBufferFlag3D = Flag ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �y�l�̔�r���[�h���Z�b�g����( �R�c�}�`�`��݂̂ɉe�� )
extern int NS_SetZBufferCmpType3D( int CmpType /* DX_CMP_NEVER �� */ )
{
	if( GBASE.ZBufferCmpType3D == CmpType ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.ZBufferCmpType3D = CmpType ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �y�o�C�A�X���Z�b�g����( �R�c�}�`�`��݂̂ɉe�� )
extern int NS_SetZBias3D( int Bias )
{
	if( GBASE.ZBias3D == Bias ) return 0 ;

	// ���_��`��
	NS_RenderVertex() ;

	GBASE.ZBias3D = Bias ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �A���t�@�e�X�g���g�p����O���t�B�b�N���쐬���邩�ǂ����̃t���O���擾����
extern int NS_GetUseAlphaTestGraphCreateFlag( void )
{
	return GBASE.AlphaTestImageCreateFlag ;
}

// SetUseAlphaTestGraphCreateFlag �̋�����
extern int NS_SetUseAlphaTestFlag( int Flag )
{
	return NS_SetUseAlphaTestGraphCreateFlag( Flag ) ;
}

// �L���[�u�}�b�v�e�N�X�`�����쐬���邩�ǂ����̃t���O��ݒ肷��
extern int NS_SetCubeMapTextureCreateFlag( int Flag )
{
	GBASE.CubeMapTextureCreateFlag = Flag == FALSE ? FALSE : TRUE ;

	// ����I��
	return 0 ;
}

// �L���[�u�}�b�v�e�N�X�`�����쐬���邩�ǂ����̃t���O���擾����
extern int NS_GetCubeMapTextureCreateFlag( void )
{
	return 	GBASE.CubeMapTextureCreateFlag ;
}

// GetUseAlphaTestGraphCreateFlag �̋�����
extern int NS_GetUseAlphaTestFlag( void )
{
	return NS_GetUseAlphaTestGraphCreateFlag() ;
}

// SetDrawValidGraphCreateFlag �̋�����
extern int NS_SetDrawValidFlagOf3DGraph( int Flag )
{
	return NS_SetDrawValidGraphCreateFlag( Flag ) ;
}

// SetDrawBlendMode �֐��̑������� DX_BLENDMODE_NOBLEND ���������ۂɁA�f�t�H���g�ł͑������͓����łQ�T�T���w�肵�����ƂɂȂ邪�A���̎����Q�T�T�������Ȃ����ǂ�����ݒ肷��( TRUE:���Ȃ�(�������̒l���g�p�����)   FALSE:����(�������̒l�͖�������� 255 ����Ɏg�p�����)(�f�t�H���g) )���`�����l���t���摜�ɑ΂��ĕ`����s���ꍇ�݈̂Ӗ�������֐�
extern int NS_SetUseNoBlendModeParam( int Flag )
{
	GBASE.UseNoBlendModeParam = Flag ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �I��
	return 0 ;
}

// �A���t�@�e�X�g���g�p����O���t�B�b�N���쐬���邩�ǂ����̃t���O���Z�b�g����
extern int NS_SetUseAlphaTestGraphCreateFlag( int Flag )
{
	// �t���O���Z�b�g����
	GBASE.AlphaTestImageCreateFlag = Flag ;
	BASEIM.AlphaTestImageCreateFlag = Flag ;
	
	// �I��
	return 0 ;
}

// �u�����h�����p�摜���쐬���邩�ǂ����̃t���O���Z�b�g����
extern int NS_SetUseBlendGraphCreateFlag( int Flag )
{
	// �t���O��ۑ�����
	GBASE.BlendImageCreateFlag = Flag ;
	
	// �I��
	return 0 ;
}

// �V�X�e����������ɃO���t�B�b�N���쐬���邩�ǂ����̃t���O���擾����( TRUE:�V�X�e����������ɍ쐬  FALSE:�u�q�`�l��ɍ쐬 )
extern int NS_GetUseSystemMemGraphCreateFlag( void )
{
	return GBASE.SystemMemImageCreateFlag ;
}

// �u�����h�����p�摜���쐬���邩�ǂ����̃t���O���擾����
extern int NS_GetUseBlendGraphCreateFlag( void )
{
	return GBASE.BlendImageCreateFlag ;
}

// �摜����̐F�𓧉ߐF�ɂ��邩�ǂ����̃t���O���Z�b�g����
extern int NS_SetLeftUpColorIsTransColorFlag( int Flag )
{
	// �t���O���Z�b�g����
	GBASE.LeftUpColorIsTransColorFlag = Flag ;

	// �I��
	return 0 ;
}

// �Q�c�`�掞�ɂy�o�b�t�@�ɏ������ނy�l��ύX����
extern int NS_SetDrawZ( float Z )
{
	GBASE.DrawZ = Z;

	// �I��
	return 0;
}

// ���ߐF�@�\���g�p���邩�ǂ�����ݒ肷��
extern int NS_SetUseTransColor( int Flag )
{
	GBASE.NotUseTransColor = !Flag;

	// �I��
	return 0;
}

// ���ߐF�@�\���g�p���邱�Ƃ�O��Ƃ����摜�̓ǂݍ��ݏ������s�����ǂ�����ݒ肷��( TRUE �ɂ���� SetDrawMode( DX_DRAWMODE_BILINEAR ); ��������Ԃ� DrawGraphF ���̕��������_�^���W���󂯎��֐��ŏ����_�ȉ��̒l���w�肵���ꍇ�ɔ�������`�挋�ʂ̕s���R���ɘa������ʂ����� ( �f�t�H���g�� FALSE ) )
extern int NS_SetUseTransColorGraphCreateFlag( int Flag )
{
	GBASE.UseTransColorGraphCreateFlag = Flag;

	return 0;
}

// ���`�����l���t���O���t�B�b�N���쐬���邩�ǂ����̃t���O���Z�b�g����( TRUE:���`�����l���t��   FALSE:���`�����l������ )
extern int NS_SetUseAlphaChannelGraphCreateFlag( int Flag )
{
	// �t���O��ۑ�����
	GBASE.AlphaChannelImageCreateFlag = Flag ;
	
	// �I��
	return 0 ;
}

// SetUseAlphaChannelGraphCreateFlag �̋�����
extern int NS_SetUseGraphAlphaChannel( int Flag )
{
	return NS_SetUseAlphaChannelGraphCreateFlag( Flag ) ;
}

// ���`�����l���t���O���t�B�b�N���쐬���邩�ǂ����̃t���O���擾����( TRUE:���`�����l���t��   FALSE:���`�����l������ )
extern int NS_GetUseAlphaChannelGraphCreateFlag( void )
{
	return GBASE.AlphaChannelImageCreateFlag ;
}

// GetUseAlphaChannelGraphCreateFlag �̋�����
extern int NS_GetUseGraphAlphaChannel( void )
{
	return NS_GetUseAlphaChannelGraphCreateFlag() ;
}



// ��Ǘ��e�N�X�`�����g�p���邩�A�̃t���O���Z�b�g( TRUE:�g�p����  FALSE:�g�p���Ȃ� )
extern int NS_SetUseNotManageTextureFlag( int Flag )
{
	GBASE.NotUseManagedTextureFlag = Flag ;

	// �I��
	return 0 ;
}

// �O���t�B�b�N�ɐݒ肷�铧�ߐF���Z�b�g����
extern int NS_SetTransColor( int Red, int Green, int Blue )
{
	if( NS_GetColorBitDepth() == 8 ) return 0 ;
	GBASE.TransColor = ( BYTE )Red << 16 | ( BYTE )Green << 8 | ( BYTE )Blue ;
	BASEIM.TransColor = GBASE.TransColor ;

	// �I��
	return 0 ;
}

// ���ߐF�𓾂�
extern int NS_GetTransColor( int *Red, int *Green, int *Blue )
{
	int TransColor ;

	if( NS_GetColorBitDepth() == 8 ) return -1 ;

	TransColor = GBASE.TransColor ;

	*Red   = ( TransColor >> 16 ) & 0xff ;
	*Green = ( TransColor >> 8  ) & 0xff ;
	*Blue  =   TransColor         & 0xff ;
 
	// �I��
	return 0 ;
}

// �A�N�e�B�u�ɂȂ��Ă���O���t�B�b�N�̃n���h���𓾂�
extern int NS_GetActiveGraph( void )
{
	return NS_GetDrawScreen() ;
}

// �`����ʂ̎擾
extern int NS_GetDrawScreen( void )
{
	return GBASE.TargetScreen[ 0 ] ;
}


// �e�N�X�`���t�H�[�}�b�g�̃C���f�b�N�X�𓾂�
extern int NS_GetTexFormatIndex( const IMAGEFORMATDESC *Format )
{
	int Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB32 ;

	switch( Format->BaseFormat )
	{
	case DX_BASEIMAGE_FORMAT_DXT1 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT1 ;
	case DX_BASEIMAGE_FORMAT_DXT2 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT2 ;
	case DX_BASEIMAGE_FORMAT_DXT3 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT3 ;
	case DX_BASEIMAGE_FORMAT_DXT4 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT4 ;
	case DX_BASEIMAGE_FORMAT_DXT5 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT5 ;

	default:
		switch( Format->ChannelNum )
		{
		case 1 :
			if( Format->FloatTypeFlag )
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F32 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F32 ;
					}
				}
			}
			else
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 16 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I16 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 16 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I16 ;
					}
				}
			}
			break ;

		case 2 :
			if( Format->FloatTypeFlag )
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F32 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 64 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F32 ;
					}
				}
			}
			else
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 16 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I16 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I16 ;
					}
				}
			}
			break ;

		case 4 :
		default :
			if( Format->FloatTypeFlag )
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F32 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 128 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F32 ;
					}
				}
			}
			else
			{
				if( Format->ChannelBitDepth >= 9 )
				{
					return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_I16 ;
				}
				else
				{
					int ColorBitDepthIndex = Format->ColorBitDepth == 16 ? 0 : 1 ;

					if( Format->DrawValidFlag )
					{
						if( Format->AlphaChFlag == TRUE || Format->AlphaTestFlag == TRUE )
						{
							Result = DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ALPHA_RGB32 ;
						}
						else
						{
							Result = DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_RGB16 + ColorBitDepthIndex ;
						}
					}
					else if( Format->AlphaChFlag   ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB16     + ColorBitDepthIndex ;
					else if( Format->AlphaTestFlag ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_RGB16 + ColorBitDepthIndex ;
					else                             Result = DX_GRAPHICSIMAGE_FORMAT_3D_RGB16           + ColorBitDepthIndex ;
				}
			}
			break ;
		}
		break ;
	}

	return Result ;
}

// �u�r�x�m�b�҂������邩�̃t���O�Z�b�g
extern int NS_SetWaitVSyncFlag( int Flag )
{
	GBASE.NotWaitVSyncFlag = !Flag ;

	if( DxSysData.DxLib_InitializeFlag == FALSE )
	{
		GBASE.PreSetWaitVSyncFlag = Flag ;
	}

	return 0 ;
}

// �u�r�x�m�b�҂�������ݒ�ɂȂ��Ă��邩�ǂ������擾����
extern int NS_GetWaitVSyncFlag( void )
{
	return !GBASE.NotWaitVSyncFlag ;
}

// �F�����
extern int NS_ColorKaiseki( const void *PixelData, LPCOLORDATA ColorData )
{
	D_DDPIXELFORMAT *PData = ( D_DDPIXELFORMAT * )PixelData ;
	DWORD ColorMask ;
	int ColorWidth, ColorLoc ;
	int i, j ;

	unsigned int  ColorMaskBuf[ 5 ] ;
	unsigned char *ColorWidthBuf[ 5 ] ;
	unsigned char *ColorLocBuf[ 5 ] ;

	// �r�b�g�f�v�X��ۑ�
	ColorData->ColorBitDepth = ( char )PData->dwRGBBitCount ;

	// �s�N�Z��������̃o�C�g����ۑ�
	ColorData->PixelByte = ColorData->ColorBitDepth / 8 ; 

	// �F������͂���
	{
		// �J���[�}�X�N���𓾂鏀��������
		ColorData->RedMask		= PData->dwRBitMask;
		ColorMaskBuf[ 0 ]		= ColorData->RedMask ;
		ColorWidthBuf[ 0 ]		= &ColorData->RedWidth ;
		ColorLocBuf[ 0 ]		= &ColorData->RedLoc ;

		ColorData->GreenMask	= PData->dwGBitMask;
		ColorMaskBuf[ 1 ]		= ColorData->GreenMask ;
		ColorWidthBuf[ 1 ]		= &ColorData->GreenWidth ;
		ColorLocBuf[ 1 ]		= &ColorData->GreenLoc ;

		ColorData->BlueMask		= PData->dwBBitMask;
		ColorMaskBuf[ 2 ]		= ColorData->BlueMask ;
		ColorWidthBuf[ 2 ]		= &ColorData->BlueWidth ;
		ColorLocBuf[ 2 ]		= &ColorData->BlueLoc ;

		ColorData->AlphaMask	= PData->dwRGBAlphaBitMask;
		ColorMaskBuf[ 3 ]		= ColorData->AlphaMask ;
		ColorWidthBuf[ 3 ]		= &ColorData->AlphaWidth ;
		ColorLocBuf[ 3 ]		= &ColorData->AlphaLoc ;

		if( ColorData->ColorBitDepth == 32 ) ColorData->NoneMask = 0xffffffff;
		else                            ColorData->NoneMask = ( 1 << ColorData->ColorBitDepth ) - 1;
		ColorData->NoneMask		&= ~( ColorData->RedMask | ColorData->GreenMask | ColorData->BlueMask | ColorData->AlphaMask );
		ColorMaskBuf[ 4 ]		= ColorData->NoneMask ;
		ColorWidthBuf[ 4 ]		= &ColorData->NoneWidth ;
		ColorLocBuf[ 4 ]		= &ColorData->NoneLoc ;
	}

	// �F�̉�͊J�n
 	for( j = 0 ; j < 5 ; j ++ )
	{
		// �J���[�}�X�N���擾
		ColorMask = ColorMaskBuf[ j ] ;

		if( ColorMask == 0 )
		{
			*ColorLocBuf[ j ] = 0 ;
			*ColorWidthBuf[ j ] = 0 ;
			continue ;
		}

		// �J���[���̐擪�r�b�g�܂ł̃r�b�g���ƃr�b�g���̏�����
		ColorLoc = 0 ;
		ColorWidth = 0 ;

		// �J���[���̂���擪�r�b�g�܂ł̃r�b�g�������
		for( i = 0 ; i < 32 ; i ++ )
		{
			// �擪�r�b�g�ɓ��B���Ȃ��Ԃ͌J��Ԃ�
			if( ColorMask & 1 )
			{	
				ColorLoc = i ;
				break ;
			}
			ColorMask >>= 1 ;
		}
		// �����J���[��񂪂Ȃ���Έُ�I��
		if( i == 32 ) return -1 ;

		// �J���[���̃r�b�g���𒲂ׂ�
		for( i = 0 ; i < 32 ; i ++ )
		{
			// �r�b�g���r�؂�Ȃ��ԌJ��Ԃ�
			if( !( ColorMask & 1 ) ) 
			{
				ColorWidth = i ;
				break ;
			}
			ColorMask >>= 1 ;
		}

		// ��͌��ʂ�ۑ�����
		*ColorLocBuf[ j ] = ( unsigned char )ColorLoc ;
		*ColorWidthBuf[ j ] = ( unsigned char )ColorWidth ;
	}

	// �Ȃ�̋@�\������t�����Ă��Ȃ��r�b�g�̃}�X�N���Z�b�g����
//	NS_SetColorDataNoneMask( ColorData ) ;

	// �I��
	return 0 ;
}

// DDPIXELFORMAT�f�[�^���쐬����
extern int NS_CreatePixelFormat( D_DDPIXELFORMAT *PixelFormatBuf, int ColorBitDepth,
								 DWORD RedMask, DWORD GreenMask, DWORD BlueMask, DWORD AlphaMask )
{
	_MEMSET( PixelFormatBuf, 0, sizeof( D_DDPIXELFORMAT ) ) ;
	PixelFormatBuf->dwFlags				= D_DDPF_RGB ;
	PixelFormatBuf->dwSize				= sizeof( D_DDPIXELFORMAT ) ;
	PixelFormatBuf->dwRGBBitCount		= ColorBitDepth ;
	PixelFormatBuf->dwRBitMask			= RedMask ;
	PixelFormatBuf->dwGBitMask			= GreenMask ;
	PixelFormatBuf->dwBBitMask			= BlueMask ;
	PixelFormatBuf->dwRGBAlphaBitMask = AlphaMask ;

	// �I��
	return 0 ;
}

// �f�t�H���g�̕ϊ��s����Z�b�g����
#define FOV				(60.0 * DX_PI / 180.0)
#define NEARZ			(0.0)
#define FARZ			(1000.0)
#define TAN_FOV_HALF	(0.52359877559829887307710723054658) // /*(0.57735026918962573)*/ tan( FOV * 0.5 )
extern	int NS_SetDefTransformMatrix( void )
{
	MATRIX mat ;
	float D ;
	
	// ���[���h�ϊ��s��͒P�ʍs��
	CreateIdentityMatrix( &mat ) ;
	NS_SetTransformToWorld( &mat ) ;

	// �r���[�|�[�g�s��̃Z�b�g
	CreateViewportMatrix( &mat, (float)GBASE.DrawSizeX / 2, (float)GBASE.DrawSizeY / 2,
								(float)( GBASE.DrawSizeX * GBASE.Draw3DScale ), (float)( GBASE.DrawSizeY * GBASE.Draw3DScale ) ) ;
	NS_SetTransformToViewport( &mat ) ;

	// �r���[�s��� z = 0.0 �̎��ɒ��x�X�N���[���S�̂��ʂ�ʒu�ƕ��������J������
	{
		VECTOR up, at, eye ;
		int w, h ;

		w = GBASE.DrawSizeX ;
		h = GBASE.DrawSizeY ;
		D = (float)( ( h / 2 ) / TAN_FOV_HALF ) ;
		eye.x = (float)w / 2 ;	eye.y = (float)h / 2 ;	eye.z = -D ;
		at.x = eye.x ;			at.y = eye.y ;			at.z = eye.z + 1.0F ;
		up.x = 0.0f ;			up.y = 1.0F ;			up.z = 0.0f ;
		CreateLookAtMatrix( &mat, &eye, &at, &up ) ;
		NS_SetTransformToView( &mat ) ;
	}

	// �v���W�F�N�V�����s��͕��ʂ�
	CreatePerspectiveFovMatrix( &mat, (float)FOV, (float)( D * 0.1F ), (float)( D + FARZ ) ) ;
	NS_SetTransformToProjection( &mat ) ;

/*
	// ���C�g���Z�b�g
	{
		LIGHT l ;

		// Initialize the structure.
		_MEMSET(&l, 0, sizeof(l));

		// Set up a white point light.
		l.Type = DX_LIGHTTYPE_D3DLIGHT_DIRECTIONAL;
		l.Diffuse.r  = 1.0f;
		l.Diffuse.g  = 1.0f;
		l.Diffuse.b  = 1.0f;
		l.Ambient.r  = 1.0f;
		l.Ambient.g  = 1.0f;
		l.Ambient.b  = 1.0f;
		l.Specular.r = 1.0f;
		l.Specular.g = 1.0f;
		l.Specular.b = 1.0f;

		// Position it high in the scene and behind the user.
		// Remember, these coordinates are in world space, so
		// the user could be anywhere in world space, too. 
		// For the purposes of this example, assume the user
		// is at the origin of world space.
		l.Position.x = 0.0f;
		l.Position.y = 100.0f;
		l.Position.z = -100.0f;
		NS_VectorNormalize( &l.Position, &l.Position ) ;

		// Don't attenuate.
		l.Attenuation0 = 100.0f; 
		l.Range        = 50.0f;

//		SetLight( 0, &l ) ;
//		SetLightEnable( 0, TRUE ) ;
	}

	// �}�e���A�����Z�b�g
	{
		MATERIAL m ;

		_MEMSET( &m, 0, sizeof( m ) ) ;

		// �f�B�t���[�Y���˂�\�� RGBA ��ݒ肷��B 
		m.Diffuse.r = 1.0f; 
		m.Diffuse.g = 1.0f; 
		m.Diffuse.b = 1.0f; 
		m.Diffuse.a = 1.0f; 

		// �A���r�G���g���˂�\�� RGBA ��ݒ肷��B 
		m.Ambient.r = 1.0f; 
		m.Ambient.g = 1.0f; 
		m.Ambient.b = 1.0f; 
		m.Ambient.a = 1.0f; 

		// �X�y�L���� �n�C���C�g�̐F�ƃV���[�v����ݒ肷��B 
		m.Specular.r = 1.0f; 
		m.Specular.g = 1.0f; 
		m.Specular.b = 1.0f; 
		m.Specular.a = 1.0f; 
		m.Power = 50.0f; 

		// RGBA �G�~�b�V�����F��ݒ肷��B
		m.Emissive.r = 0.0f;
		m.Emissive.g = 0.0f;
		m.Emissive.b = 0.0f;
		m.Emissive.a = 0.0f;

		NS_SetMaterial( &m ) ;
	}

	// �A���r�G���g�F���Z�b�g
	NS_SetAmbientColor( 0x00808080 ) ;
	NS_SetNormalizeNormals( TRUE ) ;
	NS_SetTextureAddress( DX_TEXADDRESS_WRAP ) ;
*/
	// �I��
	return 0 ;
}


// �U�S�O���S�W�O�̉�ʂłR�Q�O���Q�S�O�̉�ʉ𑜓x�ɂ��邩�ǂ����̃t���O���Z�b�g����A�U�S�O���S�W�O�ȊO�̉𑜓x�ł͖���( TRUE:�L��  FALSE:���� )
extern int NS_SetEmulation320x240( int Flag )
{
	GBASE.Emulation320x240Flag = Flag;

	// �I��
	return 0;
}

// �摜���쐬����ۂɎg�p�����摜�f�[�^�̃o�b�N�A�b�v�����ăf�o�C�X���X�g���Ɏg�p���邩�ǂ����̃t���O���Z�b�g����( TRUE:�o�b�N�A�b�v������( �f�t�H���g )  FALSE:�o�b�N�A�b�v�����Ȃ� )
extern int NS_SetUseGraphBaseDataBackup( int Flag )
{
	GBASE.NotGraphBaseDataBackupFlag = Flag ? FALSE : TRUE ;

	// �I��
	return 0 ;
}

// �摜���쐬����ۂɎg�p�����摜�f�[�^�̃o�b�N�A�b�v�����ăf�o�C�X���X�g���Ɏg�p���邩�ǂ����̃t���O���擾����
extern int NS_GetUseGraphBaseDataBackup( void )
{
	return GBASE.NotGraphBaseDataBackupFlag ? FALSE : TRUE ;
}

// �`��\�̈�𓾂�
extern int NS_GetDrawArea( RECT *Rect )
{
	// �Z�b�g
	*Rect = GBASE.DrawArea ;

	// �I��
	return 0 ;
}

// ��Ǘ��e�N�X�`�����g�p���邩�A�̃t���O���擾����( TRUE:�g�p����  FALSE:�g�p���Ȃ� )
extern int NS_GetUseNotManageTextureFlag( void )
{
	return GBASE.NotUseManagedTextureFlag ;
}




// �P���}�`�̕`��ɂR�c�f�o�C�X�̋@�\���g�p���邩�ǂ����̃t���O���Z�b�g����
extern int NS_SetUseBasicGraphDraw3DDeviceMethodFlag( int Flag )
{
	GBASE.NotUseBasicGraphDraw3DDeviceMethodFlag = !Flag ;

	return 0 ;
}

// �ʏ�g�p���Ȃ�
extern int NS_SetWindowDrawRect( const RECT *DrawRect )
{
	GBASE.WindowDrawRect = *DrawRect ;

	return 0 ;
}

// �`��\�̈��`��Ώۉ�ʑS�̂ɂ���
extern int NS_SetDrawAreaFull( void )
{
	int Width, Height ;

	NS_GetGraphSize( GBASE.TargetScreen[ 0 ], &Width, &Height ) ;
	return NS_SetDrawArea( 0, 0, Width, Height ) ;
}

// �`��u�����h���[�h���擾����
extern int NS_GetDrawBlendMode( int *BlendMode, int *BlendParam )
{
	if( BlendMode	!= NULL ) *BlendMode =	GBASE.BlendMode ;
	if( BlendParam	!= NULL ) *BlendParam =	GBASE.BlendParam ;

	// �I��
	return 0 ;
}

// �`�惂�[�h���擾����
extern int NS_GetDrawMode( void )
{
	return GBASE.DrawMode ;
}

// �`��P�x���擾����
extern int NS_GetDrawBright( int *Red, int *Green, int *Blue )
{
	if( Red		!= NULL ) *Red =	GBASE.DrawBright.Red ;
	if( Green	!= NULL ) *Green =	GBASE.DrawBright.Green ;
	if( Blue	!= NULL ) *Blue =	GBASE.DrawBright.Blue ;

	// �I��
	return 0 ;
}

// �t���[���J�E���^�[������������
extern int ResetFrameCounter( void )
{
	GBASE.FrameCounter = 0 ;
	
	return 0 ;
}

// �t���[���J�E���^�[���C���N�������g����
extern int IncFrameCounter( void )
{
	GBASE.FrameCounter ++ ;
	
	return 0 ;
}

// �t���[���J�E���^�[�̒l���擾����
extern LONGLONG GetFrameCounter( void )
{
	return GBASE.FrameCounter ;
}

// ���C�g�̕ύX�𔽉f����
extern int RefreshLightState( void )
{
	int i, j, k, OldEnableNum, OldMaxIndex ;
	LIGHT_HANDLE *pLH ;

	// ���C�g�̐ݒ�𔽉f����
	D_SetLightEnable( GBASE.Light.ProcessDisable ? FALSE : TRUE ) ;

	// �ݒ肪�ύX����Ă��Ȃ������牽�����Ȃ�
	if( GBASE.Light.D3DChange == FALSE && GBASE.Light.D3DRefresh == FALSE )
		return 0 ;

	// �L���ȃ��C�g�����Ԃɔ��f����
	OldEnableNum = GBASE.Light.EnableNum ;
	OldMaxIndex = GBASE.Light.MaxD3DEnableIndex ;
	GBASE.Light.EnableNum = 0 ;
	GBASE.Light.MaxD3DEnableIndex = -1 ;
	for( k = 3 ; k >= 1 ; k -- )
	{
		for( i = 0, j = 0 ; i < GBASE.Light.Num ; j ++ )
		{
			if( GBASE.Light.Data[ j ] == NULL )
				continue ;
			i ++ ;

			pLH = GBASE.Light.Data[ j ] ;
			if( pLH->EnableFlag == FALSE || pLH->Param.LightType != k )
				continue ;

			if( MAX_D3DENABLELIGHTINDEX_NUM > GBASE.Light.EnableNum )
			{
				GBASE.Light.EnableD3DIndex[ GBASE.Light.EnableNum ] = j ;
			}

			if( pLH->SetD3DIndex != GBASE.Light.EnableNum || pLH->ChangeFlag == TRUE || GBASE.Light.D3DRefresh == TRUE )
			{
				pLH->SetD3DIndex = GBASE.Light.EnableNum ;
				pLH->ChangeFlag = FALSE ;
				D_SetLightParam( pLH->SetD3DIndex, pLH->EnableFlag, &pLH->Param ) ;
			}
			GBASE.Light.MaxD3DEnableIndex = GBASE.Light.EnableNum ;
			GBASE.Light.EnableNum ++ ;
		}
	}

	// �O����L���ȃ��C�g�̐������Ȃ��ꍇ�͗L���ȃ��C�g��OFF�ɂ���
	for( i = GBASE.Light.MaxD3DEnableIndex + 1 ; i <= OldMaxIndex ; i ++ )
	{
		D_SetLightParam( i, FALSE, NULL ) ;
	}

	// �ݒ肪�ύX���ꂽ�t���O��|��
	GBASE.Light.D3DChange = FALSE ;
	GRH.DrawPrepAlwaysFlag = TRUE ;

	// �ݒ�̍۔��f���K�v���ǂ����̃t���O��|��
	GBASE.Light.D3DRefresh = FALSE ;

	// �s�N�Z���V�F�[�_�[�ɐݒ肷��V���h�E�}�b�v�̏����X�V����
	RefreshShadowMapPSParam() ;

	// �I��
	return 0 ;
}

// �ˉe�s��p�����[�^�Ɋ�Â��Ďˉe�s����\�z����
extern void RefreshProjectionMatrix( void )
{
	float Aspect = 1.0f ;

	if( GBASE.ProjectionMatrixMode != 2 )
	{
		Aspect = ( float )GBASE.DrawSizeY / ( float )GBASE.DrawSizeX * GBASE.ProjDotAspect ;
	}

	switch( GBASE.ProjectionMatrixMode )
	{
	case 0 :	// ���ߖ@
		CreatePerspectiveFovMatrix( &GBASE.ProjMatrix, GBASE.ProjFov, GBASE.ProjNear, GBASE.ProjFar, Aspect ) ;
		break ;

	case 1 :	// ���ˉe
		CreateOrthoMatrix( &GBASE.ProjMatrix, GBASE.ProjSize, GBASE.ProjNear, GBASE.ProjFar, Aspect ) ;
		break ;

	case 2 :	// �s�񒼎w��
		break ;
	}

	// �n�[�h�E�G�A�ɔ��f����
	_SetTransformToProjection( &GBASE.ProjMatrix ) ;
}

// ���_�ϊ��s����|�����킹���s����X�V����
extern void RefreshBlendTransformMatrix( void )
{
	if( GBASE.ValidBlendMatrix == TRUE ) return ;

	CreateMultiplyMatrix( &GBASE.ViewProjectionViewportMatrix, &GBASE.ViewMatrix, &GBASE.ProjectionMatrix ) ;
	CreateMultiplyMatrix( &GBASE.ViewProjectionViewportMatrix, &GBASE.ViewProjectionViewportMatrix, &GBASE.ViewportMatrix ) ;
	CreateMultiplyMatrix( &GBASE.BlendMatrix, &GBASE.WorldMatrix, &GBASE.ViewProjectionViewportMatrix ) ;

	GBASE.BillboardMatrix.m[0][0] = GBASE.ViewMatrix.m[0][0] ;
	GBASE.BillboardMatrix.m[0][1] = GBASE.ViewMatrix.m[1][0] ;
	GBASE.BillboardMatrix.m[0][2] = GBASE.ViewMatrix.m[2][0] ;
	GBASE.BillboardMatrix.m[0][3] = 0.0F ;

	GBASE.BillboardMatrix.m[1][0] = GBASE.ViewMatrix.m[0][1] ;
	GBASE.BillboardMatrix.m[1][1] = GBASE.ViewMatrix.m[1][1] ;
	GBASE.BillboardMatrix.m[1][2] = GBASE.ViewMatrix.m[2][1] ;
	GBASE.BillboardMatrix.m[1][3] = 0.0F ;

	GBASE.BillboardMatrix.m[2][0] = GBASE.ViewMatrix.m[0][2] ;
	GBASE.BillboardMatrix.m[2][1] = GBASE.ViewMatrix.m[1][2] ;
	GBASE.BillboardMatrix.m[2][2] = GBASE.ViewMatrix.m[2][2] ;
	GBASE.BillboardMatrix.m[2][3] = 0.0F ;

	GBASE.BillboardMatrix.m[3][0] = 
	GBASE.BillboardMatrix.m[3][1] = 
	GBASE.BillboardMatrix.m[3][2] = 
	GBASE.BillboardMatrix.m[3][3] = 0.0F ;

	GBASE.ValidInverseBlendMatrix = FALSE ;
	GBASE.ValidBlendMatrix = TRUE ;
	GBASE.ValidConvScreenPosToWorldPosMatrix = FALSE ;

	GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.left,  GBASE.DrawAreaF.top,    0.0f ) ) ;
	GBASE.ViewClipPos[ 0 ][ 0 ][ 1 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.right, GBASE.DrawAreaF.top,    0.0f ) ) ;
	GBASE.ViewClipPos[ 0 ][ 1 ][ 0 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.left,  GBASE.DrawAreaF.bottom, 0.0f ) ) ;
	GBASE.ViewClipPos[ 0 ][ 1 ][ 1 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.right, GBASE.DrawAreaF.bottom, 0.0f ) ) ;
	GBASE.ViewClipPos[ 1 ][ 0 ][ 0 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.left,  GBASE.DrawAreaF.top,    1.0f ) ) ;
	GBASE.ViewClipPos[ 1 ][ 0 ][ 1 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.right, GBASE.DrawAreaF.top,    1.0f ) ) ;
	GBASE.ViewClipPos[ 1 ][ 1 ][ 0 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.left,  GBASE.DrawAreaF.bottom, 1.0f ) ) ;
	GBASE.ViewClipPos[ 1 ][ 1 ][ 1 ] = NS_ConvScreenPosToWorldPos( VGet( GBASE.DrawAreaF.right, GBASE.DrawAreaF.bottom, 1.0f ) ) ;

	VECTOR Normal ;

	// �w�y����
	Normal = VCross( VSub( GBASE.ViewClipPos[ 1 ][ 0 ][ 0 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ] ), VSub( GBASE.ViewClipPos[ 0 ][ 1 ][ 0 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ] ) ) ;
	CreateNormalizePlane( &GBASE.ClippingPlane[ 0 ], &GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ], &Normal ) ;

	Normal = VCross( VSub( GBASE.ViewClipPos[ 0 ][ 1 ][ 1 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 1 ] ), VSub( GBASE.ViewClipPos[ 1 ][ 0 ][ 1 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 1 ] ) ) ;
	CreateNormalizePlane( &GBASE.ClippingPlane[ 1 ], &GBASE.ViewClipPos[ 0 ][ 0 ][ 1 ], &Normal ) ;

	// �x�y����
	Normal = VCross( VSub( GBASE.ViewClipPos[ 1 ][ 1 ][ 0 ], GBASE.ViewClipPos[ 0 ][ 1 ][ 0 ] ), VSub( GBASE.ViewClipPos[ 0 ][ 1 ][ 1 ], GBASE.ViewClipPos[ 0 ][ 1 ][ 0 ] ) ) ;
	CreateNormalizePlane( &GBASE.ClippingPlane[ 2 ], &GBASE.ViewClipPos[ 0 ][ 1 ][ 0 ], &Normal ) ;

	Normal = VCross( VSub( GBASE.ViewClipPos[ 0 ][ 0 ][ 1 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ] ), VSub( GBASE.ViewClipPos[ 1 ][ 0 ][ 0 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ] ) ) ;
	CreateNormalizePlane( &GBASE.ClippingPlane[ 3 ], &GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ], &Normal ) ;

	// �w�x����
	Normal = VCross( VSub( GBASE.ViewClipPos[ 0 ][ 1 ][ 0 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ] ), VSub( GBASE.ViewClipPos[ 0 ][ 0 ][ 1 ], GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ] ) ) ;
	CreateNormalizePlane( &GBASE.ClippingPlane[ 4 ], &GBASE.ViewClipPos[ 0 ][ 0 ][ 0 ], &Normal ) ;

	Normal = VCross( VSub( GBASE.ViewClipPos[ 1 ][ 0 ][ 1 ], GBASE.ViewClipPos[ 1 ][ 0 ][ 0 ] ), VSub( GBASE.ViewClipPos[ 1 ][ 1 ][ 0 ], GBASE.ViewClipPos[ 1 ][ 0 ][ 0 ] ) ) ;
	CreateNormalizePlane( &GBASE.ClippingPlane[ 5 ], &GBASE.ViewClipPos[ 1 ][ 0 ][ 0 ], &Normal ) ;
}


// �R�c�̗����̂�`�悷��
extern int NS_DrawCube3D( VECTOR Pos1, VECTOR Pos2, int DifColor, int SpcColor, int FillFlag )
{
	if( GBASE.Light.ProcessDisable == FALSE && FillFlag == TRUE )
	{
		static WORD FillIndex[ 36 ] =
		{
			 0,  1,  2,   3,  2,  1,
			 4,  5,  6,   7,  6,  5,
			 8,  9, 10,  11, 10,  9,
			12, 13, 14,  15, 14, 13,
			16, 17, 18,  19, 18, 17,
			20, 21, 22,  23, 22, 21,
		} ;
		VERTEX3D Vertex[ 6 ][ 4 ] ;
		int i, dr, dg, db, a, sr, sg, sb ;

		// ���_�f�[�^�̍쐬
		NS_GetColor2( DifColor, &dr, &dg, &db ) ;
		NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
		a = GBASE.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GBASE.BlendParam ;
		Vertex[ 0 ][ 0 ].pos.x = Pos1.x ; Vertex[ 0 ][ 0 ].pos.y = Pos2.y ; Vertex[ 0 ][ 0 ].pos.z = Pos1.z ;
		Vertex[ 0 ][ 1 ].pos.x = Pos2.x ; Vertex[ 0 ][ 1 ].pos.y = Pos2.y ; Vertex[ 0 ][ 1 ].pos.z = Pos1.z ;
		Vertex[ 0 ][ 2 ].pos.x = Pos1.x ; Vertex[ 0 ][ 2 ].pos.y = Pos1.y ; Vertex[ 0 ][ 2 ].pos.z = Pos1.z ;
		Vertex[ 0 ][ 3 ].pos.x = Pos2.x ; Vertex[ 0 ][ 3 ].pos.y = Pos1.y ; Vertex[ 0 ][ 3 ].pos.z = Pos1.z ;
		Vertex[ 0 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 0 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 0 ][ 0 ].norm.z = -1.0f ;
		Vertex[ 0 ][ 1 ].norm = Vertex[ 0 ][ 0 ].norm ;
		Vertex[ 0 ][ 2 ].norm = Vertex[ 0 ][ 0 ].norm ;
		Vertex[ 0 ][ 3 ].norm = Vertex[ 0 ][ 0 ].norm ;

		Vertex[ 1 ][ 0 ].pos.x = Pos2.x ; Vertex[ 1 ][ 0 ].pos.y = Pos2.y ; Vertex[ 1 ][ 0 ].pos.z = Pos2.z ;
		Vertex[ 1 ][ 1 ].pos.x = Pos1.x ; Vertex[ 1 ][ 1 ].pos.y = Pos2.y ; Vertex[ 1 ][ 1 ].pos.z = Pos2.z ;
		Vertex[ 1 ][ 2 ].pos.x = Pos2.x ; Vertex[ 1 ][ 2 ].pos.y = Pos1.y ; Vertex[ 1 ][ 2 ].pos.z = Pos2.z ;
		Vertex[ 1 ][ 3 ].pos.x = Pos1.x ; Vertex[ 1 ][ 3 ].pos.y = Pos1.y ; Vertex[ 1 ][ 3 ].pos.z = Pos2.z ;
		Vertex[ 1 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 1 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 1 ][ 0 ].norm.z = 1.0f ;
		Vertex[ 1 ][ 1 ].norm = Vertex[ 1 ][ 0 ].norm ;
		Vertex[ 1 ][ 2 ].norm = Vertex[ 1 ][ 0 ].norm ;
		Vertex[ 1 ][ 3 ].norm = Vertex[ 1 ][ 0 ].norm ;

		Vertex[ 2 ][ 0 ].pos.x = Pos1.x ; Vertex[ 2 ][ 0 ].pos.y = Pos2.y ; Vertex[ 2 ][ 0 ].pos.z = Pos2.z ;
		Vertex[ 2 ][ 1 ].pos.x = Pos1.x ; Vertex[ 2 ][ 1 ].pos.y = Pos2.y ; Vertex[ 2 ][ 1 ].pos.z = Pos1.z ;
		Vertex[ 2 ][ 2 ].pos.x = Pos1.x ; Vertex[ 2 ][ 2 ].pos.y = Pos1.y ; Vertex[ 2 ][ 2 ].pos.z = Pos2.z ;
		Vertex[ 2 ][ 3 ].pos.x = Pos1.x ; Vertex[ 2 ][ 3 ].pos.y = Pos1.y ; Vertex[ 2 ][ 3 ].pos.z = Pos1.z ;
		Vertex[ 2 ][ 0 ].norm.x = -1.0f ;
		Vertex[ 2 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 2 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 2 ][ 1 ].norm = Vertex[ 2 ][ 0 ].norm ;
		Vertex[ 2 ][ 2 ].norm = Vertex[ 2 ][ 0 ].norm ;
		Vertex[ 2 ][ 3 ].norm = Vertex[ 2 ][ 0 ].norm ;

		Vertex[ 3 ][ 0 ].pos.x = Pos2.x ; Vertex[ 3 ][ 0 ].pos.y = Pos2.y ; Vertex[ 3 ][ 0 ].pos.z = Pos1.z ;
		Vertex[ 3 ][ 1 ].pos.x = Pos2.x ; Vertex[ 3 ][ 1 ].pos.y = Pos2.y ; Vertex[ 3 ][ 1 ].pos.z = Pos2.z ;
		Vertex[ 3 ][ 2 ].pos.x = Pos2.x ; Vertex[ 3 ][ 2 ].pos.y = Pos1.y ; Vertex[ 3 ][ 2 ].pos.z = Pos1.z ;
		Vertex[ 3 ][ 3 ].pos.x = Pos2.x ; Vertex[ 3 ][ 3 ].pos.y = Pos1.y ; Vertex[ 3 ][ 3 ].pos.z = Pos2.z ;
		Vertex[ 3 ][ 0 ].norm.x = 1.0f ;
		Vertex[ 3 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 3 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 3 ][ 1 ].norm = Vertex[ 3 ][ 0 ].norm ;
		Vertex[ 3 ][ 2 ].norm = Vertex[ 3 ][ 0 ].norm ;
		Vertex[ 3 ][ 3 ].norm = Vertex[ 3 ][ 0 ].norm ;

		Vertex[ 4 ][ 0 ].pos.x = Pos1.x ; Vertex[ 4 ][ 0 ].pos.y = Pos2.y ; Vertex[ 4 ][ 0 ].pos.z = Pos2.z ;
		Vertex[ 4 ][ 1 ].pos.x = Pos2.x ; Vertex[ 4 ][ 1 ].pos.y = Pos2.y ; Vertex[ 4 ][ 1 ].pos.z = Pos2.z ;
		Vertex[ 4 ][ 2 ].pos.x = Pos1.x ; Vertex[ 4 ][ 2 ].pos.y = Pos2.y ; Vertex[ 4 ][ 2 ].pos.z = Pos1.z ;
		Vertex[ 4 ][ 3 ].pos.x = Pos2.x ; Vertex[ 4 ][ 3 ].pos.y = Pos2.y ; Vertex[ 4 ][ 3 ].pos.z = Pos1.z ;
		Vertex[ 4 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 4 ][ 0 ].norm.y = 1.0f ;
		Vertex[ 4 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 4 ][ 1 ].norm = Vertex[ 4 ][ 0 ].norm ;
		Vertex[ 4 ][ 2 ].norm = Vertex[ 4 ][ 0 ].norm ;
		Vertex[ 4 ][ 3 ].norm = Vertex[ 4 ][ 0 ].norm ;

		Vertex[ 5 ][ 0 ].pos.x = Pos1.x ; Vertex[ 5 ][ 0 ].pos.y = Pos1.y ; Vertex[ 5 ][ 0 ].pos.z = Pos1.z ;
		Vertex[ 5 ][ 1 ].pos.x = Pos2.x ; Vertex[ 5 ][ 1 ].pos.y = Pos1.y ; Vertex[ 5 ][ 1 ].pos.z = Pos1.z ;
		Vertex[ 5 ][ 2 ].pos.x = Pos1.x ; Vertex[ 5 ][ 2 ].pos.y = Pos1.y ; Vertex[ 5 ][ 2 ].pos.z = Pos2.z ;
		Vertex[ 5 ][ 3 ].pos.x = Pos2.x ; Vertex[ 5 ][ 3 ].pos.y = Pos1.y ; Vertex[ 5 ][ 3 ].pos.z = Pos2.z ;
		Vertex[ 5 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 5 ][ 0 ].norm.y = -1.0f ;
		Vertex[ 5 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 5 ][ 1 ].norm = Vertex[ 5 ][ 0 ].norm ;
		Vertex[ 5 ][ 2 ].norm = Vertex[ 5 ][ 0 ].norm ;
		Vertex[ 5 ][ 3 ].norm = Vertex[ 5 ][ 0 ].norm ;
		for( i = 0 ; i < 4 * 6 ; i ++ )
		{
			( ( VERTEX3D * )Vertex )[ i ].dif.b = ( BYTE )db ;
			( ( VERTEX3D * )Vertex )[ i ].dif.g = ( BYTE )dg ;
			( ( VERTEX3D * )Vertex )[ i ].dif.r = ( BYTE )dr ;
			( ( VERTEX3D * )Vertex )[ i ].dif.a = ( BYTE )a ;
			( ( VERTEX3D * )Vertex )[ i ].spc.b = ( BYTE )sb ;
			( ( VERTEX3D * )Vertex )[ i ].spc.g = ( BYTE )sg ;
			( ( VERTEX3D * )Vertex )[ i ].spc.r = ( BYTE )sr ;
			( ( VERTEX3D * )Vertex )[ i ].spc.a = 0 ;
			( ( VERTEX3D * )Vertex )[ i ].u = 0.0f ;
			( ( VERTEX3D * )Vertex )[ i ].v = 0.0f ;
			( ( VERTEX3D * )Vertex )[ i ].su = 0.0f ;
			( ( VERTEX3D * )Vertex )[ i ].sv = 0.0f ;
		}

		NS_DrawPrimitiveIndexed3D( ( VERTEX3D * )Vertex, 6 * 4, FillIndex, 36, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		static WORD FillIndex[ 36 ] =
		{
			0, 1, 3,  3, 1, 2,  3, 2, 7,
			7, 2, 6,  2, 1, 6,  6, 1, 5,
			1, 0, 5,  5, 0, 4,  0, 3, 4,
			4, 3, 7,  7, 6, 4,  4, 6, 5,
		} ;
		static WORD LineIndex[ 24 ] =
		{
			0, 1,   1, 2,   2, 3,   3, 0,
			0, 4,   1, 5,   2, 6,   3, 7,
			4, 5,   5, 6,   6, 7,   7, 4,
		} ;
		VERTEX3D Vertex[ 8 ] ;
		int i, dr, dg, db, a, sr, sg, sb ;

		// ���_�f�[�^�̍쐬
		NS_GetColor2( DifColor, &dr, &dg, &db ) ;
		NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
		a = GBASE.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GBASE.BlendParam ;
		Vertex[ 0 ].pos.x = Pos1.x ; Vertex[ 0 ].pos.y = Pos2.y ; Vertex[ 0 ].pos.z = Pos2.z ;
		Vertex[ 1 ].pos.x = Pos2.x ; Vertex[ 1 ].pos.y = Pos2.y ; Vertex[ 1 ].pos.z = Pos2.z ;
		Vertex[ 2 ].pos.x = Pos2.x ; Vertex[ 2 ].pos.y = Pos2.y ; Vertex[ 2 ].pos.z = Pos1.z ;
		Vertex[ 3 ].pos.x = Pos1.x ; Vertex[ 3 ].pos.y = Pos2.y ; Vertex[ 3 ].pos.z = Pos1.z ;
		Vertex[ 4 ].pos.x = Pos1.x ; Vertex[ 4 ].pos.y = Pos1.y ; Vertex[ 4 ].pos.z = Pos2.z ;
		Vertex[ 5 ].pos.x = Pos2.x ; Vertex[ 5 ].pos.y = Pos1.y ; Vertex[ 5 ].pos.z = Pos2.z ;
		Vertex[ 6 ].pos.x = Pos2.x ; Vertex[ 6 ].pos.y = Pos1.y ; Vertex[ 6 ].pos.z = Pos1.z ;
		Vertex[ 7 ].pos.x = Pos1.x ; Vertex[ 7 ].pos.y = Pos1.y ; Vertex[ 7 ].pos.z = Pos1.z ;
		for( i = 0 ; i < 8 ; i ++ )
		{
			Vertex[ i ].dif.b = ( BYTE )db ;
			Vertex[ i ].dif.g = ( BYTE )dg ;
			Vertex[ i ].dif.r = ( BYTE )dr ;
			Vertex[ i ].dif.a = ( BYTE )a ;
			Vertex[ i ].spc.b = ( BYTE )sb ;
			Vertex[ i ].spc.g = ( BYTE )sg ;
			Vertex[ i ].spc.r = ( BYTE )sr ;
			Vertex[ i ].spc.a = 0 ;
			Vertex[ i ].u = 0.0f ;
			Vertex[ i ].v = 0.0f ;
			Vertex[ i ].su = 0.0f ;
			Vertex[ i ].sv = 0.0f ;
		}

		if( FillFlag == TRUE ) 
		{
			NS_DrawPrimitiveIndexed3D( Vertex, 8, FillIndex, 36, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
		}
		else
		{
			NS_DrawPrimitiveIndexed3D( Vertex, 8, LineIndex, 24, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
		}
	}

	// �I��
	return 0 ;
}

// �R�c�̋��̂�`�悷��
extern int NS_DrawSphere3D( VECTOR CenterPos, float r, int DivNum, int DifColor, int SpcColor, int FillFlag )
{
	VERTEX3D *Vertex, *vert ;
	WORD *Index, *ind ;
	float *SinCosTable1, *SinCosTable2, *t1, *t2 ;
	int CirVertNumH, CirVertNumV, i, j, k, l, indexnum, vertnum ;
	int dr, dg, db, a, sr, sg, sb ;

	NS_GetColor2( DifColor, &dr, &dg, &db ) ;
	NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
	a = GBASE.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GBASE.BlendParam ;

	CirVertNumH = DivNum + 4 ;
	CirVertNumV = DivNum + 1 ;
	indexnum = FillFlag ? ( CirVertNumV - 1 ) * CirVertNumH * 6 + CirVertNumH * 3 * 2 : CirVertNumH * ( CirVertNumV + 1 ) * 2 + CirVertNumV * CirVertNumH * 2 ;
	vertnum = CirVertNumV * CirVertNumH + 2 ;
	Vertex = ( VERTEX3D * )DXALLOC( sizeof( WORD ) * indexnum + sizeof( VERTEX3D ) * vertnum + sizeof( float ) * ( CirVertNumV + CirVertNumH ) * 2 ) ;
	if( Vertex == NULL ) return 0 ;
	SinCosTable1 = ( float * )( Vertex + vertnum ) ;
	SinCosTable2 = SinCosTable1 + CirVertNumV * 2 ;
	Index = ( WORD * )( SinCosTable2 + CirVertNumH * 2 ) ;

	// ���_�f�[�^�̍쐬
	t1 = SinCosTable1 ;
	t2 = SinCosTable2 ;
	for( i = 0 ; i < CirVertNumH ; i ++, t2 += 2 )
	{
		_SINCOS( 2 * ( float )DX_PI / CirVertNumH * i, t2, t2 + 1 ) ;
	}
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		_SINCOS( ( float )DX_PI / 2.0f - ( float )DX_PI / ( CirVertNumV + 1 ) * ( i + 1 ), t1, t1 + 1 ) ;
	}
	vert = Vertex ;
	t1 = SinCosTable1 ;
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		t2 = SinCosTable2 ;
		for( j = 0 ; j < CirVertNumH ; j ++, vert ++, t2 += 2 )
		{
			vert->pos.x = t2[ 1 ] * t1[ 1 ] * r + CenterPos.x ;
			vert->pos.y = t1[ 0 ] * r + CenterPos.y ;
			vert->pos.z = t2[ 0 ] * t1[ 1 ] * r + CenterPos.z ;
			vert->norm.x = t2[ 1 ] * t1[ 1 ] ;
			vert->norm.y = t1[ 0 ] ;
			vert->norm.z = t2[ 0 ] * t1[ 1 ] ;

			vert->dif.b = ( BYTE )db ;
			vert->dif.g = ( BYTE )dg ;
			vert->dif.r = ( BYTE )dr ;
			vert->dif.a = ( BYTE )a ;
			vert->spc.b = ( BYTE )sb ;
			vert->spc.g = ( BYTE )sg ;
			vert->spc.r = ( BYTE )sr ;
			vert->spc.a = 0 ;
			vert->u = 0.0f ;
			vert->v = 0.0f ;
			vert->su = 0.0f ;
			vert->sv = 0.0f ;
		}
	}
	vert->pos.x = CenterPos.x ;
	vert->pos.y = r + CenterPos.y ;
	vert->pos.z = CenterPos.z ;
	vert->norm.x = 0.0f ;
	vert->norm.y = 1.0f ;
	vert->norm.z = 0.0f ;

	vert->dif.b = ( BYTE )db ;
	vert->dif.g = ( BYTE )dg ;
	vert->dif.r = ( BYTE )dr ;
	vert->dif.a = ( BYTE )a ;
	vert->spc.b = ( BYTE )sb ;
	vert->spc.g = ( BYTE )sg ;
	vert->spc.r = ( BYTE )sr ;
	vert->spc.a = 0 ;
	vert->u = 0.0f ;
	vert->v = 0.0f ;
	vert->su = 0.0f ;
	vert->sv = 0.0f ;

	vert ++ ;

	vert->pos.x = CenterPos.x ;
	vert->pos.y = -r + CenterPos.y;
	vert->pos.z = CenterPos.z ;
	vert->norm.x = 0.0f ;
	vert->norm.y = -1.0f ;
	vert->norm.z = 0.0f ;

	vert->dif.b = ( BYTE )db ;
	vert->dif.g = ( BYTE )dg ;
	vert->dif.r = ( BYTE )dr ;
	vert->dif.a = ( BYTE )a ;
	vert->spc.b = ( BYTE )sb ;
	vert->spc.g = ( BYTE )sg ;
	vert->spc.r = ( BYTE )sr ;
	vert->spc.a = 0 ;
	vert->u = 0.0f ;
	vert->v = 0.0f ;
	vert->su = 0.0f ;
	vert->sv = 0.0f ;

	vert ++ ;

	// �h��Ԃ����ǂ����ŏ����𕪊�
	if( FillFlag )
	{
		ind = Index ;
		l = CirVertNumV * CirVertNumH ;
		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )l ;
			ind[ 2 ] = ( WORD )( j + 1 ) ;
		}
		ind[ 0 ] = ( WORD )j ;
		ind[ 1 ] = ( WORD )l ;
		ind[ 2 ] = 0 ;

		ind += 3 ;

		k = 0 ;
		for( i = 0 ; i < CirVertNumV - 1 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 6 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ind[ 0 ] + 1 ;
				ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
				
				ind[ 3 ] = ind[ 2 ] ;
				ind[ 4 ] = ind[ 1 ] ;
				ind[ 5 ] = ind[ 3 ] + 1 ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
			
			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ( WORD )( k + CirVertNumH ) ;

			ind += 6 ;
		}

		l = CirVertNumV * CirVertNumH + 1 ;
		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )( j + 1 + k ) ;
			ind[ 1 ] = ( WORD )l ;
			ind[ 2 ] = ( WORD )( j + k ) ;
		}
		ind[ 0 ] = ( WORD )k ;
		ind[ 1 ] = ( WORD )l ;
		ind[ 2 ] = ( WORD )( j + k ) ;

		ind += 3 ;
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNumH ; i ++ )
		{
			ind[ 0 ] = ( WORD )( CirVertNumV * CirVertNumH ) ;
			ind[ 1 ] = ( WORD )i ;
			ind += 2 ;

			k = i ;
			for( j = 0 ; j < CirVertNumV - 1 ; j ++, k += CirVertNumH, ind += 2 )
			{
				ind[ 0 ] = ( WORD )k ;
				ind[ 1 ] = ( WORD )( k + CirVertNumH ) ;
			}
			
			ind[ 0 ] = ( WORD )k ;
			ind[ 1 ] = ( WORD )( CirVertNumV * CirVertNumH + 1 ) ;
			ind += 2 ;
		}

		k = 0 ;
		for( i = 0 ; i < CirVertNumV ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 2 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ( WORD )( j + k + 1 ) ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind += 2 ;
		}

		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
	}

	// �������̉��
	DXFREE( Vertex ) ;

	// �I��
	return 0 ;
}

// �R�c�̃J�v�Z����`��
extern int NS_DrawCapsule3D( VECTOR Pos1, VECTOR Pos2, float r, int DivNum, int DifColor, int SpcColor, int FillFlag )
{
	VERTEX3D *Vertex, *vert1, *vert2 ;
	WORD *Index, *ind ;
	VECTOR SubV, xv, yv, zv ;
	float *SinCosTable1, *SinCosTable2, *t1, *t2, Length, x, y, z ;
	int CirVertNumH, CirVertNumV, i, j, k, topind, botind, ettopind, etbotind, indexnum ;
	int dr, dg, db, a, sr, sg, sb, vertnum ;

	NS_GetColor2( DifColor, &dr, &dg, &db ) ;
	NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
	a = GBASE.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GBASE.BlendParam ;

	SubV = VSub( Pos2, Pos1 ) ;
	Length = VectorInnerProduct( &SubV, &SubV ) ;
	if( Length < 0.00000001f )
	{
		xv = VGet( 1.0f, 0.0f, 0.0f ) ;
		yv = VGet( 0.0f, 1.0f, 0.0f ) ;
		zv = VGet( 0.0f, 0.0f, 1.0f ) ;
	}
	else
	{
		Length = _SQRT( Length ) ;
		yv.x = SubV.x / Length ;
		yv.y = SubV.y / Length ;
		yv.z = SubV.z / Length ;

		xv = VGet( 1.0f, 0.0f, 0.0f ) ;
		VectorOuterProduct( &zv, &xv, &yv ) ;
		if( VSquareSize( zv ) < 0.0000001f )
		{
			xv = VGet( 0.0f, 0.0f, 1.0f ) ;
			VectorOuterProduct( &zv, &xv, &yv ) ;
		}
		VectorOuterProduct( &xv, &yv, &zv ) ;
		VectorNormalize( &xv, &xv ) ;
		VectorNormalize( &zv, &zv ) ;
	}

	CirVertNumH = DivNum + 4 ;
	CirVertNumV = DivNum + 2 ;
	indexnum = FillFlag ? ( CirVertNumH * 3 + CirVertNumH * 6 * ( CirVertNumV - 1 ) ) * 2 + CirVertNumH * 6 : CirVertNumH * ( CirVertNumV * 2 + 1 ) * 2 + CirVertNumH * CirVertNumV * 2 * 2 ;
	vertnum = ( CirVertNumV * CirVertNumH + 1 ) * 2 ;
	Vertex = ( VERTEX3D * )DXALLOC( sizeof( WORD ) * indexnum + sizeof( VERTEX3D ) * vertnum + sizeof( float ) * ( CirVertNumH + CirVertNumV ) * 2 ) ;
	if( Vertex == NULL ) return 0 ;
	SinCosTable1 = ( float * )( Vertex + vertnum ) ;
	SinCosTable2 = SinCosTable1 + CirVertNumV * 2 ;
	Index = ( WORD * )( SinCosTable2 + CirVertNumH * 2 ) ;
	topind = CirVertNumV * CirVertNumH * 2 ;
	botind = CirVertNumV * CirVertNumH * 2 + 1 ;
	ettopind = ( CirVertNumV - 1 ) * CirVertNumH ;
	etbotind = CirVertNumV * CirVertNumH ;

	// ���_�f�[�^�̍쐬
	t1 = SinCosTable1 ;
	t2 = SinCosTable2 ;
	for( i = 0 ; i < CirVertNumH ; i ++, t2 += 2 )
	{
		_SINCOS( 2 * ( float )DX_PI / CirVertNumH * i, t2, t2 + 1 ) ;
	}
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		_SINCOS( ( float )DX_PI / 2.0f - ( float )( DX_PI / 2.0f ) / CirVertNumV * ( i + 1 ), t1, t1 + 1 ) ;
	}
	vert1 = Vertex ;
	t1 = SinCosTable1 ;
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		t2 = SinCosTable2 ;
		vert2 = Vertex + etbotind + ( CirVertNumV - 1 - i ) * CirVertNumH ;
		for( j = 0 ; j < CirVertNumH ; j ++, vert1 ++, vert2 ++, t2 += 2 )
		{
			vert1->pos.x = t2[ 1 ] * t1[ 1 ] * r ;
			vert1->pos.y = t1[ 0 ] * r + Length ;
			vert1->pos.z = t2[ 0 ] * t1[ 1 ] * r ;
			vert1->norm.x = t2[ 1 ] * t1[ 1 ] ;
			vert1->norm.y = t1[ 0 ] ;
			vert1->norm.z = t2[ 0 ] * t1[ 1 ] ;

			vert1->dif.b = ( BYTE )db ;
			vert1->dif.g = ( BYTE )dg ;
			vert1->dif.r = ( BYTE )dr ;
			vert1->dif.a = ( BYTE )a ;
			vert1->spc.b = ( BYTE )sb ;
			vert1->spc.g = ( BYTE )sg ;
			vert1->spc.r = ( BYTE )sr ;
			vert1->spc.a = 0 ;
			vert1->u = 0.0f ;
			vert1->v = 0.0f ;
			vert1->su = 0.0f ;
			vert1->sv = 0.0f ;

			vert2->pos.x =    vert1->pos.x ;
			vert2->pos.y = -( vert1->pos.y - Length ) ;
			vert2->pos.z =    vert1->pos.z ;
			vert2->norm.x =  vert1->norm.x ;
			vert2->norm.y = -vert1->norm.y ;
			vert2->norm.z =  vert1->norm.z ;

			vert2->dif.b = ( BYTE )db ;
			vert2->dif.g = ( BYTE )dg ;
			vert2->dif.r = ( BYTE )dr ;
			vert2->dif.a = ( BYTE )a ;
			vert2->spc.b = ( BYTE )sb ;
			vert2->spc.g = ( BYTE )sg ;
			vert2->spc.r = ( BYTE )sr ;
			vert2->spc.a = 0 ;
			vert2->u = 0.0f ;
			vert2->v = 0.0f ;
			vert2->su = 0.0f ;
			vert2->sv = 0.0f ;
		}
	}

	vert1 = Vertex + topind ;
	vert1->pos.x = 0.0f ;
	vert1->pos.y = r + Length ;
	vert1->pos.z = 0.0f ;
	vert1->norm.x = 0.0f ;
	vert1->norm.y = 1.0f ;
	vert1->norm.z = 0.0f ;

	vert1->dif.b = ( BYTE )db ;
	vert1->dif.g = ( BYTE )dg ;
	vert1->dif.r = ( BYTE )dr ;
	vert1->dif.a = ( BYTE )a ;
	vert1->spc.b = ( BYTE )sb ;
	vert1->spc.g = ( BYTE )sg ;
	vert1->spc.r = ( BYTE )sr ;
	vert1->spc.a = 0 ;
	vert1->u = 0.0f ;
	vert1->v = 0.0f ;
	vert1->su = 0.0f ;
	vert1->sv = 0.0f ;

	vert1 ++ ;

	vert1->pos.x = 0.0f ;
	vert1->pos.y = -r ;
	vert1->pos.z = 0.0f ;
	vert1->norm.x = 0.0f ;
	vert1->norm.y = -1.0f ;
	vert1->norm.z = 0.0f ;

	vert1->dif.b = ( BYTE )db ;
	vert1->dif.g = ( BYTE )dg ;
	vert1->dif.r = ( BYTE )dr ;
	vert1->dif.a = ( BYTE )a ;
	vert1->spc.b = ( BYTE )sb ;
	vert1->spc.g = ( BYTE )sg ;
	vert1->spc.r = ( BYTE )sr ;
	vert1->spc.a = 0 ;
	vert1->u = 0.0f ;
	vert1->v = 0.0f ;
	vert1->su = 0.0f ;
	vert1->sv = 0.0f ;

	vert1 ++ ;

	vert1 = Vertex ;
	for( i = 0 ; i < vertnum ; i ++, vert1 ++ )
	{
		x = vert1->pos.x ;
		y = vert1->pos.y ;
		z = vert1->pos.z ;
		vert1->pos.x = x * xv.x + y * yv.x + z * zv.x + Pos1.x ;
		vert1->pos.y = x * xv.y + y * yv.y + z * zv.y + Pos1.y ;
		vert1->pos.z = x * xv.z + y * yv.z + z * zv.z + Pos1.z ;
		x = vert1->norm.x ;
		y = vert1->norm.y ;
		z = vert1->norm.z ;
		vert1->norm.x = x * xv.x + y * yv.x + z * zv.x ;
		vert1->norm.y = x * xv.y + y * yv.y + z * zv.y ;
		vert1->norm.z = x * xv.z + y * yv.z + z * zv.z ;
	}

	// �h��Ԃ����ǂ����ŏ����𕪊�
	if( FillFlag )
	{
		ind = Index ;
		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )topind ;
			ind[ 2 ] = ( WORD )( j + 1 ) ;
		}
		ind[ 0 ] = ( WORD )j ;
		ind[ 1 ] = ( WORD )topind ;
		ind[ 2 ] = 0 ;

		ind += 3 ;

		// �㕔�̋������̃C���f�b�N�X���X�g���쐬
		k = 0 ;
		for( i = 0 ; i < CirVertNumV - 1 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 6 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ind[ 0 ] + 1 ;
				ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
				
				ind[ 3 ] = ind[ 2 ] ;
				ind[ 4 ] = ind[ 1 ] ;
				ind[ 5 ] = ind[ 3 ] + 1 ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
			
			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ( WORD )( k + CirVertNumH ) ;

			ind += 6 ;
		}

		// ���S�̉~�������̃C���f�b�N�X���X�g���쐬
		for( i = 0 ; i < CirVertNumH - 1 ; i ++, ind += 6 )
		{
			ind[ 0 ] = ( WORD )( ettopind + i ) ;
			ind[ 1 ] = ind[ 0 ] + 1 ;
			ind[ 2 ] = ( WORD )( etbotind + i ) ;

			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ind[ 3 ] + 1 ;
		}
		ind[ 0 ] = ( WORD )( ettopind + i ) ;
		ind[ 1 ] = ( WORD )ettopind ;
		ind[ 2 ] = ( WORD )( etbotind + i ) ;

		ind[ 3 ] = ind[ 2 ] ;
		ind[ 4 ] = ind[ 1 ] ;
		ind[ 5 ] = ( WORD )etbotind ;

		ind += 6 ;
		k += CirVertNumH ;

		// �����̋������̃C���f�b�N�X���X�g���쐬
		for( i = 0 ; i < CirVertNumV - 1 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 6 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ind[ 0 ] + 1 ;
				ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
				
				ind[ 3 ] = ind[ 2 ] ;
				ind[ 4 ] = ind[ 1 ] ;
				ind[ 5 ] = ind[ 3 ] + 1 ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
			
			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ( WORD )( k + CirVertNumH ) ;

			ind += 6 ;
		}

		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )( j + 1 + k ) ;
			ind[ 1 ] = ( WORD )botind ;
			ind[ 2 ] = ( WORD )( j + k ) ;
		}
		ind[ 0 ] = ( WORD )k ;
		ind[ 1 ] = ( WORD )botind ;
		ind[ 2 ] = ( WORD )( j + k ) ;

		ind += 3 ;
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNumH ; i ++ )
		{
			ind[ 0 ] = ( WORD )topind ;
			ind[ 1 ] = ( WORD )i ;
			ind += 2 ;

			k = i ;
			for( j = 0 ; j < CirVertNumV * 2 - 1 ; j ++, k += CirVertNumH, ind += 2 )
			{
				ind[ 0 ] = ( WORD )k ;
				ind[ 1 ] = ( WORD )( k + CirVertNumH ) ;
			}
			
			ind[ 0 ] = ( WORD )k ;
			ind[ 1 ] = ( WORD )botind ;
			ind += 2 ;
		}

		k = 0 ;
		for( i = 0 ; i < CirVertNumV * 2 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 2 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ( WORD )( j + k + 1 ) ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind += 2 ;
		}

		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
	}

	// �������̉��
	DXFREE( Vertex ) ;

	// �I��
	return 0 ;
}

// �R�c�̉~����`�悷��
extern int NS_DrawCone3D( VECTOR TopPos, VECTOR BottomPos, float r, int DivNum, int DifColor, int SpcColor, int FillFlag )
{
	VERTEX3D *Vertex, *vert1, *vert2, *vert3 ;
	MATRIX Axis ;
	WORD *Index, *ind ;
	VECTOR SubV, xv, yv, zv, norm, bottomvec ;
	float *SinCosTable, *t, Length ;
	int CirVertNum, vertnum, indexnum ;
	int dr, dg, db, a, sr, sg, sb, i, j, k, topind, bottomind, num ;

	NS_GetColor2( DifColor, &dr, &dg, &db ) ;
	NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
	a = GBASE.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GBASE.BlendParam ;

	SubV = VSub( TopPos, BottomPos ) ;
	Length = VSize( SubV ) ;
	zv = VScale( SubV, 1.0f / Length ) ;
	xv = VGet( 1.0f, 0.0f, 0.0f ) ;
	VectorOuterProduct( &yv, &xv, &zv ) ;
	if( VSquareSize( yv ) < 0.0000001f )
	{
		xv = VGet( 0.0f, 0.0f, 1.0f ) ;
		VectorOuterProduct( &yv, &xv, &zv ) ;
	}
	xv = VNorm( VCross( yv, zv ) ) ;
	yv = VNorm( yv ) ;
	Axis = MGetAxis1( xv, yv, zv, BottomPos ) ;

	norm = VNorm( VCross( VGet( 0.0f, 0.0f, 1.0f ), VSub( VGet( r, 0.0f, 0.0f ), VGet( 0.0f, Length, 0.0f ) ) ) ) ;

	CirVertNum = DivNum + 4 ;
	vertnum = CirVertNum * 3 ;
	topind = CirVertNum ;
	bottomind = CirVertNum * 2 ;
	indexnum = FillFlag ? CirVertNum * 3 + ( CirVertNum - 2 ) * 3 : CirVertNum * 2 + CirVertNum * 2 ;
	Vertex = ( VERTEX3D * )DXALLOC( sizeof( WORD ) * indexnum + sizeof( VERTEX3D ) * vertnum + sizeof( float ) * CirVertNum * 2 ) ;
	if( Vertex == NULL ) return 0 ;
	SinCosTable = ( float * )( Vertex + vertnum ) ;
	Index = ( WORD * )( SinCosTable + CirVertNum * 2 ) ;

	// ���_�f�[�^�̍쐬
	t = SinCosTable ;
	for( i = 0 ; i < CirVertNum ; i ++, t += 2 )
	{
		_SINCOS( 2 * ( float )DX_PI / CirVertNum * i, t, t + 1 ) ;
	}
	vert1 = Vertex ;
	vert2 = Vertex + CirVertNum ;
	vert3 = Vertex + CirVertNum * 2 ;
	t = SinCosTable ;
	bottomvec = VTransformSR( VGet( 0.0f, 0.0f, -1.0f ), Axis ) ;
	for( i = 0 ; i < CirVertNum ; i ++, vert1 ++, vert2 ++, vert3 ++, t += 2 )
	{
		vert1->pos = VTransform( VGet( t[ 0 ] * r, t[ 1 ] * r, 0.0f ), Axis ) ;
		vert2->pos = TopPos ;
		vert3->pos = vert1->pos ;
		vert1->norm = VTransformSR( VGet( t[ 0 ] * norm.x, t[ 1 ] * norm.x, norm.y ), Axis ) ;
		vert2->norm = vert1->norm ;
		vert3->norm = bottomvec ;

		vert1->dif.b = ( BYTE )db ; vert1->dif.g = ( BYTE )dg ; vert1->dif.r = ( BYTE )dr ; vert1->dif.a = ( BYTE )a ;
		vert2->dif.b = ( BYTE )db ; vert2->dif.g = ( BYTE )dg ; vert2->dif.r = ( BYTE )dr ; vert2->dif.a = ( BYTE )a ;
		vert3->dif.b = ( BYTE )db ; vert3->dif.g = ( BYTE )dg ; vert3->dif.r = ( BYTE )dr ; vert3->dif.a = ( BYTE )a ;
		vert1->spc.b = ( BYTE )sb ; vert1->spc.g = ( BYTE )sg ; vert1->spc.r = ( BYTE )sr ; vert1->spc.a = 0 ;
		vert2->spc.b = ( BYTE )sb ; vert2->spc.g = ( BYTE )sg ; vert2->spc.r = ( BYTE )sr ; vert2->spc.a = 0 ;
		vert3->spc.b = ( BYTE )sb ; vert3->spc.g = ( BYTE )sg ; vert3->spc.r = ( BYTE )sr ; vert3->spc.a = 0 ;
		vert1->u = 0.0f ; vert1->v = 0.0f ;
		vert2->u = 0.0f ; vert2->v = 0.0f ;
		vert3->u = 0.0f ; vert3->v = 0.0f ;
		vert1->su = 0.0f ; vert1->sv = 0.0f ;
		vert2->su = 0.0f ; vert2->sv = 0.0f ;
		vert3->su = 0.0f ; vert3->sv = 0.0f ;
	}

	// �h��Ԃ����ǂ����ŏ����𕪊�
	if( FillFlag )
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNum - 1 ; i ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )i ;
			ind[ 1 ] = ( WORD )( topind + i ) ;
			ind[ 2 ] = ( WORD )( i + 1 ) ;
		}
		ind[ 0 ] = ( WORD )i ;
		ind[ 1 ] = ( WORD )( topind + i ) ;
		ind[ 2 ] = 0 ;
		ind += 3 ;

		j = bottomind + CirVertNum - 1 ;
		i = bottomind ;
		ind[ 0 ] = ( WORD )i ;
		ind[ 1 ] = ( WORD )( i + 1 ) ;
		ind[ 2 ] = ( WORD )j ;
		ind += 3 ;

		num = CirVertNum - 2 ;
		for( k = 1 ; k < num ; )
		{
			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )( i + 1 ) ;
			ind[ 2 ] = ( WORD )( i + 2 ) ;
			ind += 3 ;
			k ++ ;
			i ++ ;
			if( k >= num ) break ;

			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )( i + 1 ) ;
			ind[ 2 ] = ( WORD )( j - 1 ) ;
			ind += 3 ;

			j -- ;
			k ++ ;
			if( k >= num ) break ;
		}
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNum ; i ++, ind += 2 )
		{
			ind[ 0 ] = ( WORD )i ;
			ind[ 1 ] = ( WORD )( topind + i ) ;
		}
		for( i = 0 ; i < CirVertNum - 1 ; i ++, ind += 2 )
		{
			ind[ 0 ] = ( WORD )( bottomind + i ) ;
			ind[ 1 ] = ( WORD )( bottomind + i + 1 ) ;
		}
		ind[ 0 ] = ( WORD )( bottomind + i ) ;
		ind[ 1 ] = ( WORD )bottomind ;
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
	}

	// �������̉��
	DXFREE( Vertex ) ;

	// �I��
	return 0 ;
}

extern	int NS_DrawRectRotaGraph( int X, int Y, int SrcX, int SrcY, int Width, int Height, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag ) 
{
	int TempHandle ;

	// �T�C�Y����
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// ��]�`��
	NS_DrawRotaGraph( X, Y, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

extern int NS_DrawRectRotaGraph2( int x, int y, int SrcX, int SrcY, int Width, int Height, int cx, int cy, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// �T�C�Y����
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// ��]�`��
	NS_DrawRotaGraph2( x, y, cx, cy, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

extern int NS_DrawRectRotaGraph3(  int x,   int y,   int SrcX, int SrcY, int Width, int Height, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// �T�C�Y����
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// ��]�`��
	NS_DrawRotaGraph3( x, y, cx, cy, ExtRateX, ExtRateY, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

// �O���t�B�b�N�̎w���`�����݂̂�`��
extern int NS_DrawRectGraphF( float DestX, float DestY, int SrcX, int SrcY, int Width, int Height, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// �T�C�Y����
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// �`��
	if( TurnFlag )
	{
		NS_DrawTurnGraphF( DestX, DestY, TempHandle, TransFlag ) ;
	}
	else
	{
		NS_DrawGraphF( DestX, DestY, TempHandle, TransFlag ) ;
	}

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

// �O���t�B�b�N�̎w���`�����݂̂��g��`��
extern int NS_DrawRectExtendGraphF( float DestX1, float DestY1, float DestX2, float DestY2, int SrcX, int SrcY, int SrcWidth, int SrcHeight, int GraphHandle, int TransFlag )
{
	int TempHandle ;

	// �T�C�Y����
	if( SrcWidth <= 0 || SrcHeight <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, SrcWidth, SrcHeight, GraphHandle ) ;

	// �g��`��
	NS_DrawExtendGraphF( DestX1, DestY1, DestX2, DestY2, TempHandle, TransFlag ) ;

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

extern int NS_DrawRectRotaGraphF( float X, float Y, int SrcX, int SrcY, int Width, int Height, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// �T�C�Y����
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// ��]�`��
	NS_DrawRotaGraphF( X, Y, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

extern int NS_DrawRectRotaGraph2F( float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// �T�C�Y����
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// ��]�`��
	NS_DrawRotaGraph2F( x, y, cxf, cyf, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

extern int NS_DrawRectRotaGraph3F( float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, double ExtRateX, double ExtRateY, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// �T�C�Y����
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// �؂������O���t�B�b�N���쐬
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// ��]�`��
	NS_DrawRotaGraph3F( x, y, cxf, cyf, ExtRateX, ExtRateY, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// �폜
	NS_DeleteGraph( TempHandle ) ;

	// �I��
	return 0 ;
}

// DrawCircleGauge �֐��̕⏕�֐�
void DrawCircleGaugePolygon(
	int GraphHandle,
	float CenterX,
	float CenterY,
	VECTOR *pVector,
	float SizeR,
	float UScale,
	float VScale,
	float UStart,
	float VStart
)
{
	int i ;
	VERTEX vert[ 3 ] ;
	
	for( i = 0; i < 3; i ++ )
	{
		vert[i].x = pVector[i].x + CenterX + 0.5f ;
		vert[i].y = pVector[i].y + CenterY + 0.5f ;
		vert[i].u = ( pVector[i].x * SizeR + 0.5f ) * UScale + UStart ;
		vert[i].v = ( pVector[i].y * SizeR + 0.5f ) * VScale + VStart ;
		vert[i].b = 255 ;
		vert[i].g = 255 ;
		vert[i].r = 255 ;
		vert[i].a = 255 ;
	}
	NS_DrawPolygon( vert, 1, GraphHandle, TRUE );
}

// �~�O���t�I�ȕ`����s��
int NS_DrawCircleGauge( int CenterX, int CenterY, double Percent, int GrHandle, double StartPercent )
{
	VECTOR vertex[3] ;
	VECTOR position[4] ;
	VECTOR positionR ;
	VECTOR positionR2 ;
	float radian ;
	float radian2 ;
	float length ;
	float Sin ;
	float Cos ;
	float CenterXF ;
	float CenterYF ;
	float SizeR ;
	int SizeX ;
	int SizeY ;
	int i ;
	float StartRad ;
	float EndRad ;
	IMAGEDATA2 *Image2 ;
	float UScale ;
	float VScale ;
	float UStart ;
	float VStart ;
	RECT DrawRect ;
	RECT GaugeDrawRect ;

	Image2 = GetGraphData2( GrHandle ) ;
	if( Image2 == NULL )
	{
		return -1 ;
	}

	if( Image2->Orig->FormatDesc.TextureFlag )
	{
		UStart = ( float )Image2->Hard.Draw[ 0 ].UsePosX / Image2->Hard.Draw[ 0 ].Tex->TexWidth ;
		VStart = ( float )Image2->Hard.Draw[ 0 ].UsePosY / Image2->Hard.Draw[ 0 ].Tex->TexHeight ;
		UScale = ( float )Image2->Width                  / Image2->Hard.Draw[ 0 ].Tex->TexWidth ;
		VScale = ( float )Image2->Height                 / Image2->Hard.Draw[ 0 ].Tex->TexHeight ;
	}
	else
	{
		UScale = 1.0f ;
		VScale = 1.0f ;
		UStart = 0.0f ;
		VStart = 0.0f ;
	}

	if( StartPercent > 200.0 )
	{
		StartPercent = 200.0 ;
	}
	else
	if( StartPercent < -100.0 )
	{
		StartPercent = -100.0 ;
	}

	if( Percent > 200.0 )
	{
		Percent = 200.0 ;
	}
	else
	if( Percent < -100.0 )
	{
		Percent = -100.0 ;
	}

	if( Percent - StartPercent > 100.0 )
	{
		StartPercent = 0.0 ;
		Percent = 100.0 ;
	}

	if( StartPercent == Percent ||
		StartPercent > Percent )
	{
		return 0 ;
	}

	NS_GetGraphSize( GrHandle, &SizeX, &SizeY );

	radian   = ( float )( Percent      * 2 * DX_PI / 100.0 );
	radian2  = ( float )( StartPercent * 2 * DX_PI / 100.0 );
	length   = SizeX * 0.75f;
	CenterXF = ( float )CenterX;
	CenterYF = ( float )CenterY;
	SizeR    = 1.0f / SizeX;

	vertex[0].x = 0.0f;
	vertex[0].y = 0.0f;
	vertex[1] = vertex[0];
	vertex[2] = vertex[0];

	position[0].x = 0.0f;
	position[0].y = -length;

	position[1].x = length;
	position[1].y = 0.0f;

	position[2].x = 0.0f;
	position[2].y = length;

	position[3].x = -length;
	position[3].y = 0.0f;

	_SINCOS( radian2 - DX_PI_F * 0.5f, &Sin, &Cos ) ;
	positionR2.x = Cos * length;
	positionR2.y = Sin * length;

	_SINCOS( radian - DX_PI_F * 0.5f, &Sin, &Cos ) ;
	positionR.x = Cos * length;
	positionR.y = Sin * length;

	DrawRect = GBASE.DrawArea ;
	GaugeDrawRect.left   = CenterX - SizeX / 2 ;
	GaugeDrawRect.top    = CenterY - SizeY / 2 ;
	GaugeDrawRect.right  = CenterX + SizeX / 2 ;
	GaugeDrawRect.bottom = CenterY + SizeY / 2 ;
	RectClipping( &GaugeDrawRect, &DrawRect ) ;
	NS_SetDrawArea( GaugeDrawRect.left, GaugeDrawRect.top, GaugeDrawRect.right, GaugeDrawRect.bottom ) ; 

	// 90�x�����`��
	for( i = 0 ; i < 12 ; i ++ )
	{
		StartRad = DX_PI_F * 0.5f * ( i - 4 ) ;
		EndRad   = DX_PI_F * 0.5f * ( i - 4 + 1 ) ;

		if( ( radian2 >= StartRad && radian2 <= EndRad ) || ( StartRad >= radian2 && StartRad <= radian ) )
		{
			vertex[1] = radian2 <= StartRad ? position[ i         % 4 ] : positionR2;
			vertex[2] = radian  >= EndRad   ? position[ ( i + 1 ) % 4 ] : positionR;
			DrawCircleGaugePolygon( GrHandle, CenterXF, CenterYF, vertex, SizeR, UScale, VScale, UStart, VStart );
		}
	}

	// �`��\�͈͂����ɖ߂�
	NS_SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;

	// �I��
	return 0;
}









// �u�����h�摜�ƍ������ĉ摜��`�悷��
extern	int NS_DrawBlendGraph( int x, int y, int GrHandle, int TransFlag, int BlendGraph, int BorderParam, int BorderRange )
{
	int B_BlendGraph, B_BorderParam, B_BorderRange ;

	B_BlendGraph = GBASE.BlendGraph ;
	B_BorderParam = GBASE.BlendGraphBorderParam ;
	B_BorderRange = GBASE.BlendGraphBorderRange ;

	NS_SetBlendGraph( BlendGraph, BorderParam, BorderRange ) ;
	NS_DrawGraph( x, y, GrHandle, TransFlag ) ;
	NS_SetBlendGraph( B_BlendGraph, B_BorderParam, B_BorderRange ) ;

	// �I��
	return 0 ;
}

// �u�����h�摜�ƍ������ĉ摜��`�悷��( �u�����h�摜�̋N�_���W���w�肷��� )
extern	int NS_DrawBlendGraphPos( int x, int y, int GrHandle, int TransFlag, int bx, int by, int BlendGraph, int BorderParam, int BorderRange )
{
	int B_BlendGraph, B_BorderParam, B_BorderRange, B_PosX, B_PosY ;

	B_BlendGraph = GBASE.BlendGraph ;
	B_BorderParam = GBASE.BlendGraphBorderParam ;
	B_BorderRange = GBASE.BlendGraphBorderRange ;
	B_PosX = GBASE.BlendGraphX ;
	B_PosY = GBASE.BlendGraphY ;

	NS_SetBlendGraph( BlendGraph, BorderParam, BorderRange ) ;
	NS_SetBlendGraphPosition( bx, by ) ;
	NS_DrawGraph( x, y, GrHandle, TransFlag ) ;
	NS_SetBlendGraphPosition( B_PosX, B_PosY ) ;
	NS_SetBlendGraph( B_BlendGraph, B_BorderParam, B_BorderRange ) ;

	// �I��
	return 0 ;
}

static struct
{
	float DrawZ;
	int BlendMode, BlendParam;
	int ZBufferCmpType2D, WriteZBufferFlag2D;
	int ZBufferCmpType3D, WriteZBufferFlag3D;
} DrawZBuffer_Static;

static void FASTCALL DrawZBuffer_Pre( int WriteZMode )
{
	DrawZBuffer_Static.DrawZ              = GBASE.DrawZ;
	DrawZBuffer_Static.BlendMode          = GBASE.BlendMode;
	DrawZBuffer_Static.BlendParam         = GBASE.BlendParam;
	DrawZBuffer_Static.WriteZBufferFlag2D = GBASE.WriteZBufferFlag2D;
	DrawZBuffer_Static.WriteZBufferFlag3D = GBASE.WriteZBufferFlag3D;
	DrawZBuffer_Static.ZBufferCmpType2D   = GBASE.ZBufferCmpType2D;
	DrawZBuffer_Static.ZBufferCmpType3D   = GBASE.ZBufferCmpType3D;

	NS_SetDrawZ( WriteZMode == DX_ZWRITE_MASK ? 0.0f : 0.9f );
	NS_SetDrawBlendMode( DX_BLENDMODE_DESTCOLOR, 255 );
	NS_SetWriteZBufferFlag( TRUE );
	NS_SetZBufferCmpType( DX_CMP_ALWAYS );
}

static void FASTCALL DrawZBuffer_Post( void )
{
	NS_SetZBufferCmpType( DrawZBuffer_Static.ZBufferCmpType2D );
	NS_SetWriteZBufferFlag( DrawZBuffer_Static.WriteZBufferFlag2D );
	NS_SetDrawBlendMode( DrawZBuffer_Static.BlendMode, DrawZBuffer_Static.BlendParam );
	NS_SetDrawZ( DrawZBuffer_Static.DrawZ );

	NS_SetZBufferCmpType3D( DrawZBuffer_Static.ZBufferCmpType3D );
	NS_SetWriteZBuffer3D( DrawZBuffer_Static.WriteZBufferFlag3D );
}

// �y�o�b�t�@�ɑ΂��ĉ摜�̕`����s��
extern	int NS_DrawGraphToZBuffer( int X, int Y, int GrHandle, int WriteZMode )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawGraph( X, Y, GrHandle, TRUE );
	DrawZBuffer_Post();

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɑ΂��ĉ摜�̍��E���]�`��
extern	int NS_DrawTurnGraphToZBuffer( int x, int y, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawTurnGraph( x, y, GrHandle, TRUE );
	DrawZBuffer_Post();

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɑ΂��ĉ摜�̊g��k���`��
extern	int NS_DrawExtendGraphToZBuffer( int x1, int y1, int x2, int y2, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendGraph( x1, y1, x2, y2, GrHandle, TRUE );
	DrawZBuffer_Post();

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɑ΂��ĉ摜�̉�]�`��
extern	int NS_DrawRotaGraphToZBuffer( int x, int y, double ExRate, double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK �� */, int TurnFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaGraph(  x,  y,  ExRate,  Angle,  GrHandle,  TRUE, TurnFlag );
	DrawZBuffer_Post();

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɑ΂��ĉ摜�̉�]�`��Q
extern	int NS_DrawRotaGraph2ToZBuffer(  int x, int y, int cx, int cy, double ExtRate,                   double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ , int TurnFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaGraph2( x,  y,  cx,  cy,  ExtRate,  Angle,  GrHandle,  TRUE, TurnFlag ) ;
	DrawZBuffer_Post();

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɑ΂��ĉ摜�̉�]�`��R
extern	int NS_DrawRotaGraph3ToZBuffer(  int x, int y, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ , int TurnFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaGraph3(   x,  y,  cx,  cy,  ExtRateX,  ExtRateY,  Angle,  GrHandle, TRUE, TurnFlag );
	DrawZBuffer_Post();

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɑ΂��ĉ摜�̎��R�ό`�`��
extern	int NS_DrawModiGraphToZBuffer( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawModiGraph(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4,  GrHandle,  TRUE );
	DrawZBuffer_Post();

	// �I��
	return 0 ;
}

// �y�o�b�t�@�ɑ΂��ċ�`�̕`����s��
extern int NS_DrawBoxToZBuffer( int x1, int y1, int x2, int y2, int FillFlag, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawBox( x1, y1, x2, y2, NS_GetColor(255,255,255), FillFlag );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĉ~�̕`����s��
extern int NS_DrawCircleToZBuffer( int x, int y, int r, int FillFlag, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawCircle( x, y, r, GetColor(255,255,255), FillFlag );
	DrawZBuffer_Post();

	// �I��
	return 0;
}






#ifndef DX_NON_FONT

// �y�o�b�t�@�ɑ΂��ĕ�����̕`��
extern int NS_DrawStringToZBuffer( int x, int y, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawString(  x,  y, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĕ�����̕`��
extern int NS_DrawVStringToZBuffer( int x, int y, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVString(  x,  y, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĕ������`�悷��
extern int NS_DrawStringToHandleToZBuffer( int x, int y, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ , int VerticalFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawStringToHandle(  x,  y, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) ,  VerticalFlag );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĕ������`�悷��
extern int NS_DrawVStringToHandleToZBuffer( int x, int y, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVStringToHandle(  x,  y, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶�����`�悷��
extern int NS_DrawFormatStringToZBuffer( int x, int y, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawString(  x,  y, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶�����`�悷��
extern int NS_DrawFormatVStringToZBuffer( int x, int y, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVString(  x,  y, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶�����`�悷��
extern int NS_DrawFormatStringToHandleToZBuffer( int x, int y, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawStringToHandle(  x,  y, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶�����`�悷��
extern int NS_DrawFormatVStringToHandleToZBuffer( int x, int y, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVStringToHandle(  x,  y, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĕ�����̊g��`��
extern int NS_DrawExtendStringToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendString(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĕ�����̊g��`��
extern int NS_DrawExtendVStringToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVString(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĕ�������g��`�悷��
extern int NS_DrawExtendStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ , int VerticalFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendStringToHandle(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) ,  VerticalFlag );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ĕ�������g��`�悷��
extern int NS_DrawExtendVStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVStringToHandle(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶������g��`�悷��
extern int NS_DrawExtendFormatStringToZBuffer( int x, int y, double ExRateX, double ExRateY, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendString(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶������g��`�悷��
extern int NS_DrawExtendFormatVStringToZBuffer( int x, int y, double ExRateX, double ExRateY, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVString(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶������g��`�悷��
extern int NS_DrawExtendFormatStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendStringToHandle(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

// �y�o�b�t�@�ɑ΂��ď����w�蕶������g��`�悷��
extern int NS_DrawExtendFormatVStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK �� */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVStringToHandle(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// �I��
	return 0;
}

#endif // DX_NON_FONT










}

