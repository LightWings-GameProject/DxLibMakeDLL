// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		�`��`�o�h�v���O�����w�b�_�t�@�C��
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

#ifndef __DXGRAPHICSAPIWIN_H__
#define __DXGRAPHICSAPIWIN_H__

// �C���N���[�h ------------------------------------------------------------------
#include "../DxCompileConfig.h"
#include "../DxDirectX.h"

namespace DxLib
{

// �}�N����` --------------------------------------------------------------------

#define DX_DIRECTDRAWSURFACE7				class D_IDirectDrawSurface7
#define DX_DIRECT3DVERTEXSHADER9			class D_IDirect3DVertexShader9
#define DX_DIRECT3DPIXELSHADER9				class D_IDirect3DPixelShader9 
#define DX_DIRECT3DBASETEXTURE9				class D_IDirect3DBaseTexture9
#define DX_DIRECT3DTEXTURE9					class D_IDirect3DTexture9
#define DX_DIRECT3DCUBETEXTURE9				class D_IDirect3DCubeTexture9
#define	DX_DIRECT3DVOLUMETEXTURE9			class D_IDirect3DVolumeTexture9
#define DX_DIRECT3DSURFACE9					class D_IDirect3DSurface9
#define DX_DIRECT3DVERTEXDECLARATION9		class D_IDirect3DVertexDeclaration9
#define DX_DIRECT3DVERTEXBUFFER9			class D_IDirect3DVertexBuffer9
#define	DX_DIRECT3DINDEXBUFFER9				class D_IDirect3DIndexBuffer9

// �\���̒�` --------------------------------------------------------------------

struct GRAPHICSAPIINFO_WIN
{
	HINSTANCE				Direct3D9DLL ;							// �c�����������R�c�X�D�c�k�k
	D_IDirectDraw7			*DirectDraw7Object ;					// �c�����������c�������C���^�[�t�F�C�X
	D_IDirect3D9			*Direct3D9Object ;						// �c�����������R�c�X�C���^�[�t�F�C�X
	D_IDirect3D9Ex			*Direct3D9ExObject ;					// �c�����������R�c�X�d���C���^�[�t�F�C�X
	D_IDirect3DDevice9		*Direct3DDevice9Object ;				// �c�����������R�c�c�����������X�C���^�[�t�F�C�X
	D_IDirect3DDevice9Ex	*Direct3DDevice9ExObject ;				// �c�����������R�c�c�����������X�d���C���^�[�t�F�C�X
	D_IDirect3DSwapChain9	*Direct3DSwapChain9Object ;				// �c�����������R�c�r�������b���������X�C���^�[�t�F�C�X
} ;

// �������ϐ��錾 --------------------------------------------------------------

// �֐��v���g�^�C�v�錾-----------------------------------------------------------

}

#endif // __DXGRAPHICSAPIWIN_H__




