//-----------------------------------------------------------------------------
// 
// 		�c�w���C�u����		WindowsOS�p�`�揈���v���O����
// 
//  	Ver 3.12a
// 
//-----------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h ---------------------------------------------------------------
#include "DxGraphicsWin.h"
#include "DxWindow.h"
#include "DxWinAPI.h"
#include "../DxSystem.h"
#include "../DxLog.h"
#include "../DxModel.h"
#include "../DxBaseFunc.h"
#include "../DxGraphics.h"

namespace DxLib
{

// �}�N����` -----------------------------------------------------------------

// �\���̐錾 -----------------------------------------------------------------

// �f�[�^��` -----------------------------------------------------------------

static int UseGDIFlag ;							// GDI ���g�p���邩�ǂ���

// �֐��錾 -------------------------------------------------------------------

static int EnumDirect3DAdapter() ;												// Direct3D �̃A�_�v�^����񋓂���
static int UseChangeDisplaySettings( void ) ;									// ChangeDisplaySettings ���g�p���ĉ�ʃ��[�h��ύX����

// �v���O���� -----------------------------------------------------------------

// ���݂̉�ʂ̃��t���b�V�����[�g���擾����
extern int NS_GetRefreshRate( void )
{
	int RefreshRate = -1 ;

	// ���t���b�V�����[�g�̎擾
	if( WinData.WindowsVersion >= DX_WINDOWSVERSION_NT31 )
	{
		HDC hdc ;
		
		hdc = GetDC( NS_GetMainWindowHandle() ) ;
		if( hdc )
		{
			RefreshRate = GetDeviceCaps( hdc, VREFRESH ) ;
			ReleaseDC( NS_GetMainWindowHandle(), hdc ) ;

			if( RefreshRate < 50 ) RefreshRate = -1 ;
		}
	}

	return RefreshRate ;
}

// ChangeDisplaySettings ���g�p���ĉ�ʃ��[�h��ύX����
static int UseChangeDisplaySettings( void )
{
//	HRESULT hr ;
	int DesktopWidth, DesktopHeight, DesktopColorBitDepth ;
	DEVMODE DevMode;

	// ���̉�ʂ̑傫���𓾂�
	NS_GetDefaultState( &DesktopWidth, &DesktopHeight, &DesktopColorBitDepth ) ;

	DXST_ERRORLOGFMT_ADD(( _T( "��ʃ��[�h�̕ύX�������J�n���܂� %u x %u  %u bit " ), GRA2.MainScreenSizeX, GRA2.MainScreenSizeY, GRA2.MainScreenColorBitDepth )) ;
	DXST_ERRORLOG_TABADD ;

	// �Ή����Ă���J���[�r�b�g�[�x�� 16 �� 32 �̂�
	if( GRA2.MainScreenColorBitDepth != 16 &&
		GRA2.MainScreenColorBitDepth != 32 )
		return -1 ;

	// �f�B�X�v���C���[�h�̕ύX
	DXST_ERRORLOG_ADD( _T( "��ʉ𑜓x��ύX���܂�... " ) ) ;

	// Win32 API ���g���ĉ�ʃ��[�h��ύX����
	{
		// �p�����[�^�Z�b�g
		_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
		DevMode.dmSize             = sizeof( DevMode ) ;
		DevMode.dmPelsWidth        = GRA2.MainScreenSizeX ;
		DevMode.dmPelsHeight       = GRA2.MainScreenSizeY ;
		DevMode.dmBitsPerPel       = GRA2.MainScreenColorBitDepth ;
		DevMode.dmFields           = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT ;
		if( GRA2.MainScreenRefreshRate != 0 )
		{
			DevMode.dmDisplayFrequency  = GRA2.MainScreenRefreshRate ;
			DevMode.dmFields           |= DM_DISPLAYFREQUENCY ;
		}

		// ��ʃ��[�h�`�F���W
		if( ChangeDisplaySettings( &DevMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
		{
			// ���s�����烊�t���b�V�����[�g�w�薳���ł�����x����
			GRA2.MainScreenRefreshRate = 0 ;
			DevMode.dmDisplayFrequency = 0 ;
			DevMode.dmFields           &= ~DM_DISPLAYFREQUENCY ;
			if( ChangeDisplaySettings( &DevMode, 0/*CDS_FULLSCREEN*/ ) != DISP_CHANGE_SUCCESSFUL )
			{
				// ����ł��ʖڂŁA����ʃ��[�h�� 320x240 �������ꍇ��
				// �[�� 320x240 ���[�h�Ŏ���
				if( GRA2.MainScreenSizeX == 320 && GRA2.MainScreenSizeY == 240 )
				{
					SetMainScreenSize( 640, 480 ) ;
					NS_SetEmulation320x240( TRUE ) ;
					DevMode.dmPelsWidth        = GRA2.MainScreenSizeX ;
					DevMode.dmPelsHeight       = GRA2.MainScreenSizeY ;
					if( ChangeDisplaySettings( &DevMode, 0/*CDS_FULLSCREEN*/ ) != DISP_CHANGE_SUCCESSFUL )
					{
						goto ERRORLABEL ;
					}
					SetWindowStyle() ;
				}
				else
				{
ERRORLABEL:
					DXST_ERRORLOG_ADD( _T( "��ʃ��[�h�̕ύX�Ɏ��s���܂���\n" ) ) ;
					DXST_ERRORLOG_TABSUB ;
					return -1 ;
				}
			}
		}
	}

	GRA2.UseChangeDisplaySettings = TRUE ;
	DXST_ERRORLOG_ADD( _T( "�������܂���\n" ) ) ;

	return 0 ;
}

// DirectX �֘A�̏�����( 0:����  -1:���s )
extern int InitializeDirectX()
{
	int result ;

	// Direct3D9.DLL �̓ǂݍ���
	if( Graphics_LoadDLL() < 0 )
		return -1 ;

	// DirectDraw7 �̎擾�����݂�
	if( OldGraphicsInterface_Create() < 0 )
	{
		Graphics_FreeDLL() ;
		return -1 ;
	}

	// �n�[�h�E�G�A���g�p���Ȃ��A�t���O�������Ă��Ȃ��ꍇ�� Direct3D9 �̎擾�����݂�
	GRA2.ValidHardWare = FALSE ;
	if( GRA2.NotUseHardWare == FALSE )
	{
		// �t���X�N���[�����[�h�Ŋ��� Vista �ȍ~�̏ꍇ�ŕ\��ʂƗ���ʂ̑傫�����Ⴄ�ꍇ�͂����ŉ�ʃ��[�h��ύX����
		result = 0 ;
		if( NS_GetWindowModeFlag() == FALSE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA &&
			( GRA2.ValidGraphDisplayArea || GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) )
		{
			result = UseChangeDisplaySettings() ;
		}

		// ��ʃ��[�h��ύX���Ă��Ȃ����A��ʃ��[�h�̕ύX�����������ꍇ�̂� Direct3DDevice9 ���쐬����
		if( result == 0 )
		{
CREATEDIRECT3D9OBJECT :
			if( Graphics_Create() == 0 )
			{
				int Ret ;

				// Direct3DDevice9 �̎擾�����݂�
				Ret = CreateDirect3D9DeviceObject() ;
				if( Ret == -2 )
				{
					goto CREATEDIRECT3D9OBJECT ;
				}
				else
				if( Ret == 0 )
				{
					// Z�o�b�t�@ �̍쐬�����݂�
					if( CreateDirect3D9ZBufferObject() == 0 )
					{
						GRA2.ValidHardWare = TRUE ;

						// �V�F�[�_�[�̍쐬
						SetupDirect3D9Shader() ;

						// �V�F�[�_�[���g�p�ł���ꍇ�Ƃł��Ȃ��ꍇ�ŏ����𕪊�
						if( GRH.UseShader )
						{
							// ���_�錾�̍쐬
							CreateDirect3D9VertexDeclaration() ;

#ifndef DX_NON_MODEL
							// �{�����[���e�N�X�`���̍쐬
							CreateRGBtoVMaxRGBVolumeTexture() ;
#endif // DX_NON_MODEL
						}
					}
				}
			}
		}
		if( GRA2.ValidHardWare == FALSE )
		{
			// �ǂꂩ��ł����s�����ꍇ�� Direct3D ����̃I�u�W�F�N�g���������

			// Z�o�b�t�@ �̉��
			if( GRH.ZBufferSurface )
			{
				Graphics_ObjectRelease( GRH.ZBufferSurface ) ;
				GRH.ZBufferSurface = NULL ;
			}

			// Direct3DDevice9 �̉��
			GraphicsDevice_Release() ;
			GRH.BeginSceneFlag = FALSE ;

			// Direct3D9 �̉��
			Graphics_Release() ;
		}

#if 0
		// �f�B�X�v���C���[�h�̗񋓂��s��
		{
			int i, Num16, Num32 ;
			D_D3DDISPLAYMODE Mode ;
			DISPLAYMODEDATA *Disp ;

			Num32 = Graphics_GetAdapterModeCount( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_X8R8G8B8 ) ;
			Num16 = Graphics_GetAdapterModeCount( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_R5G6B5 ) ;

			if( GRH.DisplayMode )
			{
				DXFREE( GRH.DisplayMode ) ;
				GRH.DisplayMode = NULL ;
			}
			GRH.DisplayModeNum = Num32 + Num16 ;
			GRH.DisplayMode = ( DISPLAYMODEDATA * )DXALLOC( sizeof( DISPLAYMODEDATA ) * ( Num32 + Num16 ) ) ;
			if( GRH.DisplayMode == NULL )
			{
				return DXST_ERRORLOG_ADD( _T( "�f�B�X�v���C���[�h��ۑ����郁�����̈�̊m�ۂɎ��s���܂���\n" ) ) ;
			}

			Disp = GRH.DisplayMode ;
			for( i = 0 ; i < Num32 ; i ++, Disp ++ )
			{
				Graphics_EnumAdapterModes( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_X8R8G8B8, i, &Mode ) ;
				Disp->Width = Mode.Width ;
				Disp->Height = Mode.Height ;
				Disp->ColorBitDepth = 32 ;
				Disp->RefreshRate = Mode.RefreshRate ;
			}
			for( i = 0 ; i < Num16 ; i ++, Disp ++ )
			{
				Graphics_EnumAdapterModes( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_R5G6B5, i, &Mode ) ;
				Disp->Width = Mode.Width ;
				Disp->Height = Mode.Height ;
				Disp->ColorBitDepth = 16 ;
				Disp->RefreshRate = Mode.RefreshRate ;
			}
		}
#endif
	}

	// �n�[�h�E�G�A�A�N�Z�����[�V�������g�p���Ȃ��ꍇ�͂����ŉ�ʃ��[�h��ύX����
	if( GRA2.ValidHardWare == FALSE )
	{
		// �t���X�N���[���̏ꍇ�̂ݏ���
		if( NS_GetWindowModeFlag() == FALSE )
		{
			if( UseChangeDisplaySettings() < 0 )
				return -1 ;
		}

		// �u�r�x�m�b�҂��̎��Ԃ��擾����
		GraphicsDevice_WaitVSyncInitialize() ;

		// �I��
		return 0 ;
	}

	// �I��
	return 0 ;
}

// DirectX �֘A�̌�n��
extern int TerminateDirectX()
{
#ifndef DX_NON_MODEL
	// ���f���̒��_�o�b�t�@�����
	MV1TerminateVertexBufferAll() ;
#endif

	// ���_�錾�̉��
	TerminateDirect3D9VertexDeclaration() ;

	// �V�F�[�_�[�̉��
	TerminateDirect3D9Shader() ;

//	// �n�[�h�E�G�A�A�N�Z�����[�^���g�p���Ȃ��ꍇ�͂����ŉ�ʃ��[�h�����ɖ߂�
//	if( /*NS_GetWindowModeFlag() == FALSE &&*/
//		( GRA2.ValidHardWare == FALSE ||
//		 ( WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA && ( GRA2.ValidGraphDisplayArea || GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) ) ) )
	// ChangeDisplaySettings ���g�p���ĉ�ʃ��[�h��ύX�����ꍇ�͂����ŉ�ʃ��[�h�����ɖ߂�
	if( GRA2.UseChangeDisplaySettings )
	{
		GRA2.UseChangeDisplaySettings = FALSE ;
		ChangeDisplaySettings( NULL, 0 ) ;
	}

	// Z�o�b�t�@ �̉��
	if( GRH.ZBufferSurface )
	{
		Graphics_ObjectRelease( GRH.ZBufferSurface ) ;
		GRH.ZBufferSurface = NULL ;
	}

	// �T�u�o�b�N�o�b�t�@�̉��
	if( GRH.SubBackBufferSurface )
	{
		Graphics_ObjectRelease( GRH.SubBackBufferSurface ) ;
		GRH.SubBackBufferSurface = NULL ;
	}

	// �o�b�N�o�b�t�@�̉��
	if( GRH.BackBufferSurface )
	{
		Graphics_ObjectRelease( GRH.BackBufferSurface ) ;
		GRH.BackBufferSurface = NULL ;
	}

#ifndef DX_NON_MODEL
	// �{�����[���e�N�X�`���̉��
	if( GRH.RGBtoVMaxRGBVolumeTexture )
	{
		Graphics_ObjectRelease( GRH.RGBtoVMaxRGBVolumeTexture ) ;
		GRH.RGBtoVMaxRGBVolumeTexture = NULL ;
	}
#endif // DX_NON_MODEL

	// Direct3DDevice9 �̉��
	GraphicsDevice_Release() ;
	GRH.BeginSceneFlag = FALSE ;

	// Direct3D9 �̉��
	Graphics_Release() ;

	// DirectDraw �̉��
	OldGraphicsInterface_Release() ;

	// d3d9.dll �̉��
	Graphics_FreeDLL() ;

	// �f�B�X�v���C���[�h�p�Ɋm�ۂ����������̉��
	if( GRH.DisplayMode )
	{
		HeapFree( GetProcessHeap(), 0, GRH.DisplayMode ) ;
		GRH.DisplayMode = NULL ;
	}

	// �I��
	return 0 ;
}


















































// �@��ˑ��֐��̃R�[�h

// Direct3D �̃A�_�v�^����񋓂���
static int EnumDirect3DAdapter()
{
	int i ;

	if( GRH.ValidAdapterInfo ) return 0 ;

	// ����������
	if( DxSysData.DxLib_InitializeFlag == FALSE || DxLib_GetEndRequest() )
	{
		// Direct3D9.DLL �̓ǂݍ���
		if( Graphics_LoadDLL() < 0 )
			return -1 ;

		// Direct3D9 �̍쐬
		if( Graphics_Create() == 0 )
		{
			GRH.AdapterInfoNum = Graphics_GetAdapterCount() ;
			for( i = 0 ; i < GRH.AdapterInfoNum ; i ++ )
				Graphics_GetAdapterIdentifier( i, 0, &GRH.AdapterInfo[ i ] ) ;

			// Direct3D9 �̉��
			Graphics_Release() ;
		}

		// d3d9.dll �̉��
		Graphics_FreeDLL() ;
	}
	else
	{
		GRH.AdapterInfoNum = Graphics_GetAdapterCount() ;
		for( i = 0 ; i < GRH.AdapterInfoNum ; i ++ )
			Graphics_GetAdapterIdentifier( i, 0, &GRH.AdapterInfo[ i ] ) ;
	}

	GRH.ValidAdapterInfo = TRUE ;

	// ����I��
	return 0 ;
}

// �c�����������c���������g�p����f�t�c�h��ݒ肷��
extern	int		NS_SetDDrawUseGuid( const GUID FAR *Guid )
{
	int i ;

	if( EnumDirect3DAdapter() != 0 ) return -1 ;

	// �w��̂f�t�h�c�����邩�ǂ������ׂ�
	for( i = 0 ; i < GRH.AdapterInfoNum && _MEMCMP( Guid, &GRH.AdapterInfo[ i ].DeviceIdentifier, sizeof( GUID ) ) != 0 ; i ++ ){}
	if( i == GRH.AdapterInfoNum ) return -1 ;

	// �l��ۑ�����
	GRH.ValidAdapterNumber = TRUE ;
	GRH.UseAdapterNumber = i ;

	// �I��
	return 0 ;
}

/*�߂�l�� IDirectDraw7 * �ɃL���X�g���ĉ�����*/			// ���ݎg�p���Ă���c�����������c�������I�u�W�F�N�g�̃A�h���X���擾����
extern	const void					*NS_GetUseDDrawObj( void )
{
	// �I��
	return OldGraphicsInterface_GetObject() ;
}

// �L���� DirectDraw �f�o�C�X�� GUID ���擾����
extern	const GUID *				NS_GetDirectDrawDeviceGUID( int Number )
{
	if( EnumDirect3DAdapter() != 0 || Number >= GRH.AdapterInfoNum ) return NULL ;

	return &GRH.AdapterInfo[ Number ].DeviceIdentifier ;
}

// �L���� DirectDraw �f�o�C�X�̖��O�𓾂�
extern	int						NS_GetDirectDrawDeviceDescription( int Number, char *StringBuffer )
{
	if( EnumDirect3DAdapter() != 0 || Number >= GRH.AdapterInfoNum ) return -1 ;

	_STRCPY( StringBuffer, GRH.AdapterInfo[ Number ].Description ) ;

	// �I��
	return 0 ;
}

// �L���� DirectDraw �f�o�C�X�̐����擾����
extern	int						NS_GetDirectDrawDeviceNum( void )
{
	if( EnumDirect3DAdapter() != 0 ) return -1 ;

	return GRH.AdapterInfoNum ;
}


// �c�����������c���������g�p���邩�ǂ������Z�b�g����
extern	int		NS_SetUseDirectDrawFlag( int Flag )
{
	// �������O�̂ݎg�p�\
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	GRA2.NotUseHardWare = !Flag ;

	// �I��
	return 0 ;
}

// �f�c�h�`���K�v�Ƃ��邩�A��ύX����
extern	int		NS_SetUseGDIFlag( int Flag )
{
	// �t���O��ۑ�
	UseGDIFlag = Flag ;

	// �n�[�h�E�G�A���g�p���Ă��邩�ǂ����ŏ����𕪊�
	if( GRA2.ValidHardWare )
	{
		// �n�[�h�E�G�A�����_�����O���[�h�̏ꍇ
		if( GraphicsDevice_IsValid() == 0 ) return 0 ;

		// �f�o�C�X�̐ݒ�
		if( Flag == FALSE )
		{
			GraphicsDevice_SetDialogBoxMode( FALSE ) ;
		}
		else
		{
			GraphicsDevice_SetDialogBoxMode( FALSE ) ;
			GraphicsDevice_SetDialogBoxMode( TRUE ) ;
		}
	}
	else
	{
		// �\�t�g�E�G�A�����_�����O���[�h�ł͓��ɂ��邱�Ɩ���
	}

	// �I��
	return 0 ;
}

// �f�c�h�`���K�v�Ƃ��邩�̏�Ԃ��擾����
extern int NS_GetUseGDIFlag( void )
{
	return UseGDIFlag ;
}

// �f�B�X�v���C���[�h�f�[�^�̏����Z�b�g�A�b�v����
extern void SetupDisplayModeData( int *Num, DISPLAYMODEDATA **Buffer )
{
	int i, ModeNum ;
	DEVMODE DevMode ;
	DISPLAYMODEDATA *Disp ;

	// ���Ƀ��������m�ۂ���Ă�������
	if( *Buffer )
	{
		HeapFree( GetProcessHeap(), 0, *Buffer ) ;
		*Buffer = NULL ;
	}

	// ���𐔂���
	*Num = 0 ;
	ModeNum = 0 ;
	for( ModeNum ;  ; ModeNum ++ )
	{
		_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
		if( EnumDisplaySettings( NULL, ModeNum, &DevMode ) == 0 ) break ;

		// 16bit �ȉ��̃��[�h�͏��O����
		if( DevMode.dmBitsPerPel >= 16 )
		{
			( *Num ) ++ ;
		}
	}

	// �����i�[���郁�����̈�̊m��
	*Buffer = ( DISPLAYMODEDATA * )HeapAlloc( GetProcessHeap(), 0, sizeof( DISPLAYMODEDATA ) * *Num ) ;
	if( *Buffer == NULL )
	{
		*Num = 0 ;
		return ;
	}

	// �����擾����
	Disp = *Buffer ;
	for( i = 0 ; i < ModeNum ; i ++ )
	{
		_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
		EnumDisplaySettings( NULL, i, &DevMode ) ;
		if( DevMode.dmBitsPerPel >= 16 )
		{
			Disp->Width         = DevMode.dmPelsWidth ;
			Disp->Height        = DevMode.dmPelsHeight ;
			Disp->ColorBitDepth = DevMode.dmBitsPerPel ;
			Disp->RefreshRate   = DevMode.dmDisplayFrequency ;
			Disp ++ ;
		}
	}
}

}

