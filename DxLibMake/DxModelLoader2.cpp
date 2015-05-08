// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		�l�p�n���f���f�[�^�ǂݍ��݃v���O����
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

#define __DX_MAKE

#include "DxModelRead.h"

#ifndef DX_NON_MODEL

// �C���N���[�h ---------------------------------
#include "DxMath.h"
#include "DxLog.h"
#include "DxFile.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxMemory.h"
#include "Windows/DxWindow.h"
#include "Windows/DxGuid.h"

namespace DxLib
{

// �}�N����` -----------------------------------

#define ENDCHECK( Model )		( ( Model->TextNow - Model->Text ) >= Model->TextSize )
#define C16TOI( C )				( C >= 'A' ? C - 'A' + 10 : C - '0' )

// �f�[�^�錾 -----------------------------------

// ���^�Z�R�C�A�f�[�^�\����
struct MQO_MODEL
{
	char *Text ;								// �e�L�X�g�f�[�^
	int TextSize ;								// �e�L�X�g�T�C�Y
	char *TextNow ;								// ���݉�͒��̃A�h���X
} ;

// �֐��錾 -------------------------------------

static int SkipSpace( MQO_MODEL *Model, int ReturnStopFlag = 0 ) ;	// �X�y�[�X���΂�( 1:�I�[�ɒB����  0:�I�[�ɂ͒B���Ă��Ȃ� )
static int GetNextString( MQO_MODEL *Model, char *Buffer ) ;		// ���̈Ӗ��̂��镶������擾����( 1:�I�[�ɒB����  0:�I�[�ɂ͒B���Ă��Ȃ� -1:�G���[ )
static int GetString( MQO_MODEL *Model, char *Buffer ) ;			// ��������擾����( -1:�G���[ )
static char GetNextChar( MQO_MODEL *Model ) ;						// ���̈Ӗ��̂��镶�����擾����( -1:�G���[ )
static int GetInt( MQO_MODEL *Model ) ;								// �����l���擾����
static float GetFloat( MQO_MODEL *Model ) ;							// ���������_�l���擾����
static int SkipChunk( MQO_MODEL *Model ) ;							// ���ɓo�ꂷ��`�����N���΂�( -1:�G���[ )

// �v���O���� -----------------------------------

// ���ɓo�ꂷ��`�����N���΂�( -1:�G���[ )
static int SkipChunk( MQO_MODEL *Model )
{
	char Buffer[ 1024 ] ;
	int Count ;

	// �劇�ʂ��o��܂ŃX�L�b�v
	SkipSpace( Model ) ;
	while( !ENDCHECK( Model ) && *Model->TextNow != '{' )
	{
		GetNextString( Model, Buffer ) ;
		SkipSpace( Model ) ;
	}
	if( ENDCHECK( Model ) )
		return -1 ;

	// ���劇�ʂ�����܂ŕ�������X�L�b�v��������
	Model->TextNow ++ ;
	Count = 1 ;
	while( Count > 0 && !ENDCHECK( Model ) )
	{
		if( *Model->TextNow == '{' )
			Count ++ ;

		if( *Model->TextNow == '}' )
			Count -- ;

		GetNextString( Model, Buffer ) ;
		SkipSpace( Model ) ;
	}

	return Count != 0 ? -1 : 0 ;
}

// ���������_�l���擾����
static float GetFloat( MQO_MODEL *Model )
{
	char Buffer[ 1024 ] ;

	// ��������擾����
	if( GetNextString( Model, Buffer ) != 0 )
		return -1 ;

	// ���������_�l�ɕϊ�����
	return ( float )_ATOF( Buffer ) ;
}

// �����l���擾����
static int GetInt( MQO_MODEL *Model )
{
	char Buffer[ 1024 ] ;

	// ��������擾����
	if( GetNextString( Model, Buffer ) != 0 )
		return -1 ;

	// ���l�ɕϊ�����
	return _ATOI( Buffer ) ;
}

// ���̈Ӗ��̂��镶�����擾����( -1:�G���[ )
static char GetNextChar( MQO_MODEL *Model )
{
	// �X�y�[�X���΂�
	if( SkipSpace( Model ) == 1 )
		return -1 ;

	// �ŏ��̕�����Ԃ�
	return *Model->TextNow ;
}

// ��������擾����( -1:�G���[ )
static int GetString( MQO_MODEL *Model, char *Buffer )
{
	int Len ;

	// ������擾
	if( GetNextString( Model, Buffer ) != 0 )
		return -1 ;
	Len = lstrlenA( Buffer ) ;

	// �_�u���R�[�e�[�V�����Ŋ����Ă��Ȃ�������G���[
	if( Buffer[ 0 ] != '\"' || Buffer[ Len - 1 ] != '\"' )
		return -1 ;

	// �P�o�C�g�����炷
	_MEMMOVE( Buffer, Buffer + 1, Len - 2 ) ;
	Buffer[ Len - 2 ] = '\0' ;

	// �I��
	return 0 ;
}

// ���̈Ӗ��̂��镶������擾����( 1:�I�[�ɒB����  0:�I�[�ɂ͒B���Ă��Ȃ� -1:�G���[ )
static int GetNextString( MQO_MODEL *Model, char *Buffer )
{
	// �X�y�[�X���΂�
	if( SkipSpace( Model ) == 1 )
		return 1 ;

	// �ŏ��̕����ɂ���ď����𕪊�
	switch( *Model->TextNow )
	{
	case '\"' :		// ������
		// ���̃_�u���R�[�e�[�V�����܂Ŏ擾
		Model->TextNow ++ ;
		*Buffer = '\"' ;
		Buffer ++ ;
		while( !ENDCHECK( Model ) )
		{
			if( CheckMultiByteChar( *Model->TextNow, _GET_CHARSET() ) )
			{
				*( ( WORD * )Buffer ) = *( ( WORD * )Model->TextNow ) ;
				Buffer += 2 ;
				Model->TextNow += 2 ;
			}
			else
			{
				if( *Model->TextNow == '\"' ) break ;
				*Buffer = *Model->TextNow ;
				Buffer ++ ;
				Model->TextNow ++ ;
			}
		}
		if( ENDCHECK( Model ) )
			return -1 ;

		// �I�[�����̃Z�b�g
		Buffer[ 0 ] = '\"' ;
		Buffer[ 1 ] = '\0' ;

		// �_�u���R�[�e�[�V�����̎��̈ʒu�Ɉړ�
		Model->TextNow ++ ;
		break ;

	case '(' :		// ���ʂ���
	case ')' :
	case '{' :
	case '}' :
		Buffer[ 0 ] = *Model->TextNow ;
		Buffer[ 1 ] = '\0' ;
		Model->TextNow ++ ;
		break ;

	case '-' :		// ���l
	case '.' :
	case '0' : case '1' : case '2' : case '3' : case '4' :
	case '5' : case '6' : case '7' : case '8' : case '9' :
		while( !ENDCHECK( Model ) && 
			( *Model->TextNow == '-' || *Model->TextNow == '.' ||
			  *Model->TextNow == '0' || *Model->TextNow == '1' ||
			  *Model->TextNow == '2' || *Model->TextNow == '3' ||
			  *Model->TextNow == '4' || *Model->TextNow == '5' ||
			  *Model->TextNow == '6' || *Model->TextNow == '7' ||
			  *Model->TextNow == '8' || *Model->TextNow == '9' ) )
		{
			*Buffer = *Model->TextNow ;
			Buffer ++ ;
			Model->TextNow ++ ;
		}

		// �I�[�����̃Z�b�g
		*Buffer = '\0' ;
		break ;

	default :		// ����ȊO
		// �A���t�@�x�b�g�ȊO�̓G���[
		if( ( *Model->TextNow < 'A' || *Model->TextNow > 'Z' ) &&
			( *Model->TextNow < 'a' || *Model->TextNow > 'z' ) )
			return -1 ;

		// �A���t�@�x�b�g�Ɛ����ƃA���_�[�o�[�̂ݎ擾
		while( !ENDCHECK( Model ) && 
			( ( *Model->TextNow >= 'A' && *Model->TextNow >= 'Z' ) ||
			  ( *Model->TextNow >= 'a' && *Model->TextNow >= 'z' ) ||
			  ( *Model->TextNow >= '0' && *Model->TextNow >= '9' ) ||
			  *Model->TextNow == '_' ) )
		{
			*Buffer = *Model->TextNow ;
			Buffer ++ ;
			Model->TextNow ++ ;
		}

		// �I�[�����̃Z�b�g
		*Buffer = '\0' ;
		break ;
	}

	// �I��
	return 0 ;
}

// �X�y�[�X���΂�
static int SkipSpace( MQO_MODEL *Model, int ReturnStopFlag )
{
	if( ReturnStopFlag )
	{
		while( !ENDCHECK( Model ) &&
			( *Model->TextNow == ' '  ||
			  *Model->TextNow == '\t' ) )
			  Model->TextNow ++ ;
	}
	else
	{
		while( !ENDCHECK( Model ) &&
			( *Model->TextNow == ' '  ||
			  *Model->TextNow == '\t' ||
			  *Model->TextNow == '\r' ||
			  *Model->TextNow == '\n' ) )
			  Model->TextNow ++ ;
	}

	return ENDCHECK( Model ) ? 1 : 0 ;
}

// �l�p�n�t�@�C����ǂݍ���( -1:�G���[  0�ȏ�:���f���n���h�� )
extern int MV1LoadModelToMQO( const MV1_MODEL_LOAD_PARAM *LoadParam, int ASyncThread )
{
	int NewHandle = -1 ;
	int ErrorFlag = 1 ;
	int Num, i, j, k, l, uvcount, colcount, mat, uv, col ;
	MQO_MODEL MqoModel ;
	MV1_MODEL_R RModel ;
	MV1_TEXTURE_R *Texture ;
	MV1_MATERIAL_R *Material ;
	MV1_FRAME_R *Frame ;
	MV1_FRAME_R *FrameStack[ 1024 ] ;
	MV1_MESH_R *Mesh ;
	MV1_MESHFACE_R *MeshFace ;
	char String[ 1024 ] ;
	float r, g, b, a, f ;
	DWORD ColorCode ;
	int UseMaterialMap[ 1024 ], UseMaterialList[ 1024 ], UseMaterialTable[ 1024 ], UseMaterialNum, MatIndex ;
	int UseMaterialMapG[ 1024 ] ;
	int Mirror, Mirror_Axis, Depth ;
	BYTE *PositionUnionFlag = NULL ;
	int EnableShading;
	int Shading;

	// �ǂݍ��݂悤�f�[�^�̏�����
	MV1InitReadModel( &RModel ) ;
	RModel.MeshFaceRightHand = FALSE ;

	// MQO���f���f�[�^���O������
	_MEMSET( &MqoModel, 0, sizeof( MqoModel ) ) ;

	// MQO���f���f�[�^�̏����Z�b�g
	MqoModel.Text = ( char * )LoadParam->DataBuffer ;
	MqoModel.TextSize = LoadParam->DataSize ;
	MqoModel.TextNow = MqoModel.Text + 43 ;

	// MQO�t�@�C�����ǂ������m�F
	if( _MEMCMP( LoadParam->DataBuffer, "Metasequoia Document\r\nFormat Text Ver 1.0\r\n", 22 ) != 0 )
		return -1 ;

	// ���f�����ƃt�@�C�������Z�b�g
	RModel.FilePath = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->FilePath ) + 1 ) * sizeof( TCHAR ) ) ;
	RModel.Name     = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->Name     ) + 1 ) * sizeof( TCHAR ) ) ;
	lstrcpy( RModel.FilePath, LoadParam->FilePath ) ;
	lstrcpy( RModel.Name,     LoadParam->Name ) ;

	// �@���̎����������g�p
	RModel.AutoCreateNormal = TRUE ;

	// �}�e���A���`�����N�̓ǂݍ���
	while( SkipSpace( &MqoModel ) == 0 )
	{
		if( GetNextString( &MqoModel, String ) == -1 )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : �\������ EOF �ł�\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// �}�e���A���`�����N����Ȃ��ꍇ�̓X�L�b�v
		if( _STRCMP( String, "Material" ) != 0 )
		{
			SkipChunk( &MqoModel ) ;
			continue ;
		}

		// �}�e���A���̐����擾
		Num = GetInt( &MqoModel ) ;
		if( Num < 0 )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : �}�e���A���̐�( %d )���s���ł�\n" ), Num ) ) ;
			goto ENDLABEL ;
		}

		// ���ʂ̒��ɓ���
		GetNextString( &MqoModel, String ) ;
		if( String[ 0 ] != '{' )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : Material ���̌�� { ������܂���\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// �}�e���A���̐������J��Ԃ�
		SkipSpace( &MqoModel ) ;
		for( i = 0 ; i < Num ; i ++ )
		{
			// �}�e���A���̖��O���擾
			if( GetString( &MqoModel, String ) == -1 )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� Material ���̎擾�Ɏ��s���܂���\n" ), i ) ) ;
				goto ENDLABEL ;
			}

			// �}�e���A���̒ǉ�
			Material = MV1RAddMaterial( &RModel, String ) ;
			if( Material == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� Material �I�u�W�F�N�g�̒ǉ��Ɏ��s���܂���\n" ), i ) ) ;
				goto ENDLABEL ;
			}
			
			// ���̎擾
			r = 1.0f ;
			g = 1.0f ;
			b = 1.0f ;
			a = 1.0f ;
			for(;;)
			{
				static TCHAR *MaterialError1 = _T( "MQO Load Error : No.%d �� Material �� %s �̌�� ( ������܂���ł���\n" ) ;
				static TCHAR *MaterialError2 = _T( "MQO Load Error : No.%d �� Material �� %s �̐��l�̌�� ) ������܂���ł���\n" ) ;

				SkipSpace( &MqoModel, 1 ) ;
				if( *MqoModel.TextNow == '\r' ) break ;

				if( GetNextString( &MqoModel, String ) != 0 )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� Material �̓ǂݍ��ݒ��ɃG���[���������܂���\n" ), i ) ) ;
					goto ENDLABEL ;
				}

				// �J���[
				if( _STRCMP( String, "col" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "col" ) ) ) ;
						goto ENDLABEL ;
					}

					r = GetFloat( &MqoModel ) ;
					g = GetFloat( &MqoModel ) ;
					b = GetFloat( &MqoModel ) ;
					a = GetFloat( &MqoModel ) ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "col" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "dif" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "dif" ) ) ) ;
						goto ENDLABEL ;
					}

					f = GetFloat( &MqoModel ) ;
					Material->Diffuse.r = r * f ;
					Material->Diffuse.g = g * f ;
					Material->Diffuse.b = b * f ;
					Material->Diffuse.a = 1.0f ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "dif" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "amb" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "amb" ) ) ) ;
						goto ENDLABEL ;
					}

					f = GetFloat( &MqoModel ) ;
					Material->Ambient.r = f ;
					Material->Ambient.g = f ;
					Material->Ambient.b = f ;
					Material->Ambient.a = 0.0f ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "amb" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "emi" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "emi" ) ) ) ;
						goto ENDLABEL ;
					}

					f = GetFloat( &MqoModel ) ;
					Material->Emissive.r = r * f ;
					Material->Emissive.g = g * f ;
					Material->Emissive.b = b * f ;
					Material->Emissive.a = 0.0f ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "emi" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "spc" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "spc" ) ) ) ;
						goto ENDLABEL ;
					}

					f = GetFloat( &MqoModel ) ;
					Material->Specular.r = f ;
					Material->Specular.g = f ;
					Material->Specular.b = f ;
					Material->Specular.a = 0.0f ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "spc" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "power" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "power" ) ) ) ;
						goto ENDLABEL ;
					}

					Material->Power = GetFloat( &MqoModel ) * 4.0f ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "power" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "tex" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "tex" ) ) ) ;
						goto ENDLABEL ;
					}

					if( GetString( &MqoModel, String ) == -1 )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� tex �̕�����ǂݍ��݂Ɏ��s���܂���\n" ), i ) ) ;
						goto ENDLABEL ;
					}

					// �e�N�X�`����ǉ�
					Texture = MV1RAddTexture( &RModel, String, String ) ;
					if( Texture == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� �e�N�X�`���I�u�W�F�N�g�̍쐬�Ɏ��s���܂���\n" ), i ) ) ;
						goto ENDLABEL ;
					}
					Material->DiffuseTexNum = 1 ;
					Material->DiffuseTexs[ 0 ] = Texture ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "tex" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "aplane" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "aplane" ) ) ) ;
						goto ENDLABEL ;
					}

					if( GetString( &MqoModel, String ) == -1 )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� aplane �̕�����ǂݍ��݂Ɏ��s���܂���\n" ), i ) ) ;
						goto ENDLABEL ;
					}

					// �e�N�X�`���������ꍇ�͂����Œǉ�
					if( Material->DiffuseTexs[ 0 ] == NULL )
					{
						Texture = MV1RAddTexture( &RModel, String, NULL ) ;
						if( Texture == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� �e�N�X�`���I�u�W�F�N�g�̍쐬�Ɏ��s���܂���\n" ), i ) ) ;
							goto ENDLABEL ;
						}
						Material->DiffuseTexNum = 1 ;
						Material->DiffuseTexs[ 0 ] = Texture ;
					}

					// �A���t�@�v���[����ǉ�
					Material->DiffuseTexs[ 0 ]->AlphaFileName = MV1RAddString( &RModel, String ) ;
					if( Material->DiffuseTexs[ 0 ]->AlphaFileName == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� aplane �̕�����̕ۑ��Ɏ��s���܂���\n" ), i ) ) ;
						goto ENDLABEL ;
					}

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "aplane" ) ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "bump" ) == 0 )
				{
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '(' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError1, i, _T( "bump" ) ) ) ;
						goto ENDLABEL ;
					}

					if( GetString( &MqoModel, String ) == -1 )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� bump �̕�����ǂݍ��݂Ɏ��s���܂���\n" ), i ) ) ;
						goto ENDLABEL ;
					}

					// �e�N�X�`����ǉ�
					{
						char FileName[ 512 ] ;

						AnalysisFileNameAndDirPath_( String, FileName, NULL ) ;
						Texture = MV1RAddTexture( &RModel, FileName, String, NULL, TRUE ) ;
					}
					if( Texture == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : No.%d �� �e�N�X�`���I�u�W�F�N�g�̍쐬�Ɏ��s���܂���\n" ), i ) ) ;
						goto ENDLABEL ;
					}
					Material->NormalTexNum = 1 ;
					Material->NormalTexs[ 0 ] = Texture ;

					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != ')' )
					{
						DXST_ERRORLOGFMT_ADD( ( MaterialError2, i, _T( "bump" ) ) ) ;
						goto ENDLABEL ;
					}
				}
			}
			Material->Diffuse.a = a ;
		}

		// ���ʂ̒�����o��
		GetNextString( &MqoModel, String ) ;
		if( String[ 0 ] != '}' )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : Material �`�����N�̏I�[�� } ������܂���\n" ) ) ) ;
			goto ENDLABEL ;
		}
		break ;
	}

	// �}�e���A�������蓖�Ă��Ă��Ȃ����b�V���p�̃}�e���A����ǉ�
	Material = MV1RAddMaterial( &RModel, "NoMaterial" ) ;
	if( Material == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : Material �I�u�W�F�N�g�̒ǉ��Ɏ��s���܂���\n" ), i ) ) ;
		goto ENDLABEL ;
	}
	Material->DiffuseTexNum = 0 ;
	Material->SpecularTexNum = 0 ;
	Material->NormalTexNum = 0 ;
	Material->Diffuse.r = 1.0f ;
	Material->Diffuse.g = 1.0f ;
	Material->Diffuse.b = 1.0f ;
	Material->Diffuse.a = 1.0f ;

	// �`�����N�ǂݍ��݃��[�v
	_MEMSET( UseMaterialMapG, 0, sizeof( UseMaterialMapG ) ) ;
	MqoModel.TextNow = MqoModel.Text + 43 ;
	while( SkipSpace( &MqoModel ) == 0 )
	{
		if( GetNextString( &MqoModel, String ) == -1 )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : �`�����N��͒��ɕ������̓G���[���������܂���\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// �I�u�W�F�N�g�`�����N
		if( _STRCMP( String, "Object" ) == 0 )
		{
			char FrameName[ 512 ] ;

			// �~���[���̏�����
			Mirror = 0 ;
			Mirror_Axis = 0 ;

			// �V�F�[�f�B���O���̏�����
			EnableShading = 0;
			Shading = 0;

			// �I�u�W�F�N�g�����擾
			if( GetString( &MqoModel, FrameName ) != 0 )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : Object �`�����N�̖��O�̎擾�Ɏ��s���܂���\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// ���ʂ̒��ɓ���
			GetNextString( &MqoModel, String ) ;
			if( String[ 0 ] != '{' )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : Object �`�����N���̌�� { ������܂���\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// �[�����擾����
			Depth = 0 ;
			if( GetNextString( &MqoModel, String ) != 0 )
			{
				DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�̉�͒��ɃG���[���������܂���\n", Frame->Name ) ) ;
				goto ENDLABEL ;
			}
			if( _STRCMP( String, "depth" ) == 0 )
			{
				Depth = GetInt( &MqoModel ) ;
			}

			// �t���[���̒ǉ�
			Frame = MV1RAddFrame( &RModel, FrameName, Depth == 0 ? NULL : FrameStack[ Depth - 1 ] ) ;
			if( Frame == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : �t���[���I�u�W�F�N�g�̒ǉ��Ɏ��s���܂���\n" ) ) ) ;
				goto ENDLABEL ;
			}
			FrameStack[ Depth ] = Frame ;

			// �X�P�[���̏����l���Z�b�g
			Frame->Scale.x = 1.0f ;
			Frame->Scale.y = 1.0f ;
			Frame->Scale.z = 1.0f ;

			// �e�̍s��͖�������
//			Frame->IgnoreParentTransform = 1 ;

			// ��]�I�[�_�[�̏����l���Z�b�g
			Frame->RotateOrder = MV1_ROTATE_ORDER_XYZ ;

			// ���b�V���̒ǉ�
			Mesh = MV1RAddMesh( &RModel, Frame ) ;
			if( Mesh == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load Error : ���b�V���I�u�W�F�N�g�̒ǉ��Ɏ��s���܂���\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// �����ʂ�����܂ŉ��
			for(;;)
			{
				// ���̕������ǂݍ���
				if( GetNextString( &MqoModel, String ) != 0 )
				{
					DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�̉�͒��ɃG���[���������܂���\n", Frame->Name ) ) ;
					goto ENDLABEL ;
				}

				// �����ʂ�������I��
				if( _STRCMP( String, "}" ) == 0 )
				{
					break ;
				}
				else
				// �r�W�u���l�������ꍇ
				if( _STRCMP( String, "visible" ) == 0 )
				{
					Frame->Visible = GetInt( &MqoModel ) == 15 ? 1 : 0 ;
				}
				else
				// �V�F�[�f�B���O�������ꍇ
				if( _STRCMP( String, "shading" ) == 0 )
				{
					EnableShading = 1;
					Shading = GetInt( &MqoModel ) ;

					if( Shading == 0 )
					{
						Frame->SmoothingAngle = 0.0f ;
					}
				}
				else
				// �g��l�������ꍇ
				if( _STRCMP( String, "scale" ) == 0 )
				{
					Frame->Scale.x = GetFloat( &MqoModel ) ;
					Frame->Scale.y = GetFloat( &MqoModel ) ;
					Frame->Scale.z = GetFloat( &MqoModel ) ;
				}
				else
				// ��]�l�������ꍇ
				if( _STRCMP( String, "rotation" ) == 0 )
				{
					Frame->Rotate.y = -GetFloat( &MqoModel ) * DX_PI_F / 180.0f ;
					Frame->Rotate.x = -GetFloat( &MqoModel ) * DX_PI_F / 180.0f ;
					Frame->Rotate.z =  GetFloat( &MqoModel ) * DX_PI_F / 180.0f ;
				}
				else
				// ���s�ړ��l�������ꍇ
				if( _STRCMP( String, "translation" ) == 0 )
				{
					Frame->Translate.x =  GetFloat( &MqoModel ) ;
					Frame->Translate.y =  GetFloat( &MqoModel ) ;
					Frame->Translate.z = -GetFloat( &MqoModel ) ;
				}
				else
				// �X���[�W���O�p�x��񂾂����ꍇ
				if( _STRCMP( String, "facet" ) == 0 )
				{
					if( EnableShading == 0 || Shading == 1 )
					{
						Frame->SmoothingAngle = ( float )( GetFloat( &MqoModel ) * DX_PI_F / 180.0f ) ;
					}
					else
					{
						Frame->SmoothingAngle = 0.0f ;
					}
				}
				else
				// ���ʏ����������ꍇ
				if( _STRCMP( String, "mirror" ) == 0 )
				{
					Mirror = GetInt( &MqoModel ) ;
				}
				else
				// ���ʏ����̊p�x�������ꍇ
				if( _STRCMP( String, "mirror_axis" ) == 0 )
				{
					Mirror_Axis = GetInt( &MqoModel ) ;
				}
				else
				// ���_��񂾂����ꍇ
				if( _STRCMP( String, "vertex" ) == 0 )
				{
					// ���_�����擾
					Mesh->PositionNum = GetInt( &MqoModel ) ;
					if( Mesh->PositionNum < 0 )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� vertex �̐��̎擾�Ɏ��s���܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// ���_�f�[�^���i�[���邽�߂̃������̈�̊m��
					Mesh->Positions = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * Mesh->PositionNum, &RModel.Mem ) ;
					if( Mesh->Positions == NULL )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�̒��_���W��ۑ����郁�����̈�̊m�ۂɎ��s���܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// ���ʂ̒��ɓ���
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '{' )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� vertex ��� { ������܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// ���_�f�[�^���擾
					for( i = 0 ; i < ( int )Mesh->PositionNum ; i ++ )
					{
						Mesh->Positions[ i ].x =  GetFloat( &MqoModel ) ;
						Mesh->Positions[ i ].y =  GetFloat( &MqoModel ) ;
						Mesh->Positions[ i ].z = -GetFloat( &MqoModel ) ;
					}

					// ���ʂ̒�����o��
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '}' )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� vertex �̏I�[�� } ������܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}
				}
				else
				if( _STRCMP( String, "face" ) == 0 )
				{
					int FaceNum ;

					// �ʂ̐����擾
					FaceNum = GetInt( &MqoModel ) ;
					if( FaceNum < 0 )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �̐��̎擾�Ɏ��s���܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// �ʏ����i�[���郁�����̈�̊m��
					if( MV1RSetupMeshFaceBuffer( &RModel, Mesh, FaceNum, 4 ) < 0 )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�̖ʏ���ۑ����郁�����̈�̊m�ۂɎ��s���܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// �t�u�l���i�[���邽�߂̃������̈�̊m��
					Mesh->UVNum[ 0 ] = Mesh->FaceNum * 4 ;
					Mesh->UVs[ 0 ] = ( FLOAT4 * )ADDMEMAREA( sizeof( FLOAT4 ) * Mesh->UVNum[ 0 ], &RModel.Mem ) ;
					if( Mesh->UVs[ 0 ] == NULL )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� uv���W��ۑ����郁�����̈�̊m�ۂɎ��s���܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// ���_�J���[���i�[���邽�߂̃������̈�̊m��
					Mesh->VertexColorNum = Mesh->FaceNum * 4 ;
					Mesh->VertexColors = ( COLOR_F * )ADDMEMAREA( sizeof( COLOR_F ) * Mesh->VertexColorNum, &RModel.Mem ) ;
					if( Mesh->VertexColors == NULL )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� ���_�J���[��ۑ����郁�����̈�̊m�ۂɎ��s���܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// ���_�J���[�̏�����
					for( i = 0 ; i < ( int )Mesh->VertexColorNum ; i ++ )
					{
						Mesh->VertexColors[ i ].r = 1.0f ;
						Mesh->VertexColors[ i ].g = 1.0f ;
						Mesh->VertexColors[ i ].b = 1.0f ;
						Mesh->VertexColors[ i ].a = 1.0f ;
					}

					// ���ʂ̒��ɓ���
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '{' )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face ��� { ������܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}

					// �ʂ̏����擾
					MeshFace = Mesh->Faces ;
					uvcount = 0 ;
					colcount = 0 ;
					col = 0 ;
					_MEMSET( UseMaterialList, 0, sizeof( UseMaterialList ) ) ;
					_MEMSET( UseMaterialMap, 0, sizeof( UseMaterialMap ) ) ;
					UseMaterialNum = 0 ;
					for( i = 0 ; i < ( int )Mesh->FaceNum ; )
					{
						mat = 0 ;
						uv = 0 ;

						// �C���f�b�N�X�̐����擾
						MeshFace->IndexNum = GetInt( &MqoModel ) ;
						if( Mesh->FaceUnitMaxIndexNum < MeshFace->IndexNum )
						{
							int FaceIndex ;

							FaceIndex = ( int )( MeshFace - Mesh->Faces ) ;
							if( MV1RSetupMeshFaceBuffer( &RModel, Mesh, Mesh->FaceNum, MeshFace->IndexNum ) < 0 )
							{
								DXST_ERRORLOGFMT_ADD( ( _T( "MQO Load : �ʏ����i�[���郁�����̍Ċm�ۂɎ��s���܂���\n" ) ) ) ;
								return -1 ;
							}
							MeshFace = Mesh->Faces + FaceIndex ;
						}

						// ���_�C���f�b�N�X�̎擾
						{
							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != 'V' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �̒��_���̐��̌�� V ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != '(' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� V �� ( ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							for( j = 0 ; j < ( int )MeshFace->IndexNum ; j ++ )
							{
								MeshFace->VertexIndex[ j ] = GetInt( &MqoModel ) ;
								if( MeshFace->VertexIndex[ j ] == 0xffffffff )
								{
									DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� %d�Ԗڂ� V �̒��_�C���f�b�N�X���s���ł�\n", Frame->Name, j ) ) ;
									goto ENDLABEL ;
								}
							}

							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != ')' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� V �� ) ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}
						}

						// �}�e���A���C���f�b�N�X�̎擾
						SkipSpace( &MqoModel, 1 ) ;
						if( MqoModel.TextNow[ 0 ] == 'M' )
						{
							GetNextString( &MqoModel, String ) ;

							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != '(' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� M �� ( ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							MatIndex = GetInt( &MqoModel ) ;
							if( UseMaterialMap[ MatIndex ] == 0 )
							{
								UseMaterialMap[ MatIndex ] = 1 ;
								UseMaterialMapG[ MatIndex ] = 1 ;
								UseMaterialList[ UseMaterialNum ] = MatIndex ;
								UseMaterialTable[ MatIndex ] = UseMaterialNum ;
								UseMaterialNum ++ ;
							}
							MeshFace->MaterialIndex = UseMaterialTable[ MatIndex ] ;
							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != ')' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� M �� ) ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							mat = 1 ;
						}

						// �t�u�l�̎擾
						SkipSpace( &MqoModel, 1 ) ;
						if( MqoModel.TextNow[ 0 ] == 'U' && MqoModel.TextNow[ 1 ] == 'V' )
						{
							GetNextString( &MqoModel, String ) ;

							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != '(' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� UV �� ( ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							for( j = 0 ; j < ( int )MeshFace->IndexNum ; j ++, uvcount ++ )
							{
								MeshFace->UVIndex[ 0 ][ j ] = uvcount ;
								Mesh->UVs[ 0 ][ uvcount ].x = GetFloat( &MqoModel ) ;
								Mesh->UVs[ 0 ][ uvcount ].y = GetFloat( &MqoModel ) ;
							}
							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != ')' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� UV �� ) ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							uv = 1 ;
						}

						// ���_�J���[�̎擾
						SkipSpace( &MqoModel, 1 ) ;
						if( MqoModel.TextNow[ 0 ] == 'C' && MqoModel.TextNow[ 1 ] == 'O' && MqoModel.TextNow[ 2 ] == 'L' )
						{
							COLOR_F MatColor ;

							GetNextString( &MqoModel, String ) ;

							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != '(' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� COL �� ( ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							if( mat )
							{
								Material = MV1RGetMaterial( &RModel, UseMaterialList[ MeshFace->MaterialIndex ] ) ;
								MatColor = Material->Diffuse ;
							}
							else
							{
								MatColor.r = 1.0f ;
								MatColor.g = 1.0f ;
								MatColor.b = 1.0f ;
								MatColor.a = 1.0f ;
							}

							for( j = 0 ; j < ( int )MeshFace->IndexNum ; j ++, colcount ++ )
							{
								MeshFace->VertexColorIndex[ j ] = colcount ;
								ColorCode = ( DWORD )GetInt( &MqoModel ) ;
								Mesh->VertexColors[ colcount ].r = MatColor.r * ( ( ColorCode >>  0 ) & 0xff ) / 255.0f ;
								Mesh->VertexColors[ colcount ].g = MatColor.g * ( ( ColorCode >>  8 ) & 0xff ) / 255.0f ;
								Mesh->VertexColors[ colcount ].b = MatColor.b * ( ( ColorCode >> 16 ) & 0xff ) / 255.0f ;
								Mesh->VertexColors[ colcount ].a = MatColor.a * ( ( ColorCode >> 24 ) & 0xff ) / 255.0f ;
							}
							GetNextString( &MqoModel, String ) ;
							if( String[ 0 ] != ')' )
							{
								DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �� UV �� ) ������܂���\n", Frame->Name ) ) ;
								goto ENDLABEL ;
							}

							col = 1 ;
						}
						else
						{
							// �}�e���A���J���[�𒸓_�J���[�Ƃ��ăZ�b�g
							if( mat )
							{
								for( j = 0 ; j < ( int )MeshFace->IndexNum ; j ++, colcount ++ )
								{
									MeshFace->VertexColorIndex[ j ] = colcount ;
									Material = MV1RGetMaterial( &RModel, UseMaterialList[ MeshFace->MaterialIndex ] ) ;
									Mesh->VertexColors[ colcount ] = Material->Diffuse ;
								}
							}
						}

						// �}�e���A���������ꍇ�́A�}�e���A�������p�̃}�e���A�������蓖�Ă�
						if( mat == 0 )
						{
							MatIndex = RModel.MaterialNum - 1 ;
							if( UseMaterialMap[ MatIndex ] == 0 )
							{
								UseMaterialMap[ MatIndex ] = 1 ;
								UseMaterialMapG[ MatIndex ] = 1 ;
								UseMaterialList[ UseMaterialNum ] = MatIndex ;
								UseMaterialTable[ MatIndex ] = UseMaterialNum ;
								UseMaterialNum ++ ;
							}
							MeshFace->MaterialIndex = UseMaterialTable[ MatIndex ] ;
						}

						// ���_�����R���S�ł͖����ꍇ�͏ꍇ�͖ʂ�ǉ����Ȃ�
						if(MeshFace->IndexNum == 3 || MeshFace->IndexNum == 4 )
						{
							i ++ ;
							MeshFace ++ ;
						}
						else
						{
							Mesh->FaceNum -- ;
						}
					}

					// �L���Ȗʂ���������Ȃ����烁�b�V�����폜����
					if( Mesh->FaceNum == 0 )
					{
						MV1RSubMesh( &RModel, Frame, Mesh ) ;
					}
					else
					{
						// �g�p���Ă���}�e���A���̃Z�b�g
						Mesh->MaterialNum = UseMaterialNum ;
						for( i = 0 ; i < UseMaterialNum ; i ++ )
							Mesh->Materials[ i ] = MV1RGetMaterial( &RModel, UseMaterialList[ i ] ) ;

						// �g�p���Ă���}�e���A���������x�P�O�O���������烁�b�V�����폜����
						if( UseMaterialNum == 1 && Mesh->Materials[ 0 ]->Diffuse.a == 0.0f )
						{
							MV1RSubMesh( &RModel, Frame, Mesh ) ;
						}
					}

/*					// ���_�J���[��������������璸�_�J���[�𖳂��ɂ���
					if( col == 0 )
					{
						Mesh->VertexColorNum = 0 ;
						Mesh->VertexColors = NULL ;
					}
*/
					// ���ʂ̒�����o��
					GetNextString( &MqoModel, String ) ;
					if( String[ 0 ] != '}' )
					{
						DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�� face �̏I�[�� } ������܂���\n", Frame->Name ) ) ;
						goto ENDLABEL ;
					}
				}
			}

			// ���W�ϊ��p�����[�^�𔽉f����
			{
				// ���̃t���[���ł̍s����쐬����
				Frame->Matrix = MGetScale( Frame->Scale ) ;
				if( Frame->Rotate.x != 0.0f )
				{
					Frame->Matrix = MMult( Frame->Matrix, MGetRotX( Frame->Rotate.x ) ) ;
				}
				if( Frame->Rotate.y != 0.0f )
				{
					Frame->Matrix = MMult( Frame->Matrix, MGetRotY( Frame->Rotate.y ) ) ;
				}
				if( Frame->Rotate.z != 0.0f )
				{
					Frame->Matrix = MMult( Frame->Matrix, MGetRotZ( Frame->Rotate.z ) ) ;
				}
				Frame->Matrix.m[ 3 ][ 0 ] = Frame->Translate.x ;
				Frame->Matrix.m[ 3 ][ 1 ] = Frame->Translate.y ;
				Frame->Matrix.m[ 3 ][ 2 ] = Frame->Translate.z ;

				// ���̃t���[���ł̋t�s����쐬����
				MATRIX NowFrameInvMatrix ;
				if( Frame->Parent )
				{
					Frame->LocalWorldMatrix = MMult( Frame->Matrix, Frame->Parent->LocalWorldMatrix ) ;
				}
				else
				{
					Frame->LocalWorldMatrix = Frame->Matrix ;
				}
				NowFrameInvMatrix = MInverse( Frame->LocalWorldMatrix ) ;

				// ���_�f�[�^�ɋt�s����|����
				for( i = 0 ; i < ( int )Mesh->PositionNum ; i ++ )
				{
					Mesh->Positions[ i ] = VTransform( Mesh->Positions[ i ], NowFrameInvMatrix ) ;
				}

				Frame->Rotate.x = 0.0f ;
				Frame->Rotate.y = 0.0f ;
				Frame->Rotate.z = 0.0f ;
				Frame->Scale.x = 1.0f ;
				Frame->Scale.y = 1.0f ;
				Frame->Scale.z = 1.0f ;
				Frame->Translate.x = 0.0f ;
				Frame->Translate.y = 0.0f ;
				Frame->Translate.z = 0.0f ;
			}

			// �~���[�t���O�������Ă���ꍇ�̓~���[�����O���b�V�����ǉ�����
			if( Mirror == 1 && Frame->MeshNum != 0 )
			{
				VECTOR *Positions ;
				MV1_MESHFACE_R *Faces, *OrigFacesT ;
				int TotalMeshNum, SetCount, BitNum ;
				int OrigFaceNum ;

				// �~���[�����O���鐔���Z�o
				TotalMeshNum = 1 ;
				if( Mirror_Axis & 1 ) TotalMeshNum <<= 1 ;
				if( Mirror_Axis & 2 ) TotalMeshNum <<= 1 ;
				if( Mirror_Axis & 4 ) TotalMeshNum <<= 1 ;

				// ���_���W�����L���邩�ǂ����̃t���O���i�[���郁�����̈�̊m��
				PositionUnionFlag = ( BYTE * )DXALLOC( Mesh->PositionNum ) ;
				if( PositionUnionFlag == NULL )
				{
					DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�̃~���[�����O���_�f�[�^���t���O�i�[�p�������̊m�ۂɎ��s���܂���\n", Frame->Name ) ) ;
					goto ENDLABEL ;
				}

				// �~���[�����O���ꂽ���b�V���f�[�^���i�[���邽�߂̒��_�o�b�t�@�Ɩʃo�b�t�@���m�ۂ���
				Positions = Mesh->Positions ;
				Mesh->Positions = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * Mesh->PositionNum * TotalMeshNum, &RModel.Mem ) ;
				if( Mesh->Positions == NULL )
				{
					DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�̃~���[�����O���_�f�[�^�i�[�p�������̊m�ۂɎ��s���܂���\n", Frame->Name ) ) ;
					goto ENDLABEL ;
				}
				OrigFaceNum = Mesh->FaceNum ;
				if( MV1RSetupMeshFaceBuffer( &RModel, Mesh, OrigFaceNum * TotalMeshNum, Mesh->FaceUnitMaxIndexNum ) < 0 )
				{
					DXST_ERRORLOGFMT_ADDA( ( "MQO Load Error : Object %s �`�����N�̃~���[�����O�ʃf�[�^�i�[�p�������̊m�ۂɎ��s���܂���\n", Frame->Name ) ) ;
					goto ENDLABEL ;
				}

				// �m�ۂ��������Ɍ����_�ł̃f�[�^���R�s�[
				for( i = 0 ; i < TotalMeshNum ; i ++ )
				{
					_MEMCPY( ( BYTE * )Mesh->Positions + sizeof( VECTOR )         * Mesh->PositionNum * i, Positions, sizeof( VECTOR )         * Mesh->PositionNum ) ;
				}
				for( i = 1 ; i < TotalMeshNum ; i ++ )
				{
					Faces = Mesh->Faces + OrigFaceNum * i ;
					OrigFacesT = Mesh->Faces ;
					for( j = 0 ; j < OrigFaceNum ; j ++, Faces ++, OrigFacesT ++ )
					{
						Faces->IndexNum = OrigFacesT->IndexNum ;
						Faces->PolygonNum = OrigFacesT->PolygonNum ;
						for( k = 0 ; k < ( int )OrigFacesT->IndexNum ; k ++ )
						{
							Faces->VertexIndex[ k ] = OrigFacesT->VertexIndex[ k ] ;
							Faces->NormalIndex[ k ] = OrigFacesT->NormalIndex[ k ] ;
							Faces->VertexColorIndex[ k ] = OrigFacesT->VertexColorIndex[ k ] ;
							for( l = 0 ; l < MV1_READ_MAX_UV_NUM ; l ++ )
							{
								Faces->UVIndex[ l ][ k ] = OrigFacesT->UVIndex[ l ][ k ] ;
							}
						}
						Faces->MaterialIndex = OrigFacesT->MaterialIndex ;
						Faces->Normal = OrigFacesT->Normal ;
					}
				}

				// ���_�f�[�^�̍쐬
				SetCount = 1 ;
				for( j = 1 ; j < 8 ; j ++ )
				{
					if( ( j & Mirror_Axis ) != j ) continue ;

					// �����Ă���r�b�g�̐��𐔂���
					BitNum = 0 ;
					if( j & 1 ) BitNum ++ ;
					if( j & 2 ) BitNum ++ ;
					if( j & 4 ) BitNum ++ ;

					// ���_���W�����L���邩�ǂ����̃t���O���i�[���郁�����̈�̏�����
					_MEMSET( PositionUnionFlag, 0, Mesh->PositionNum ) ;

					// ���_���W�𔽓]

					// X���𔽓]
					if( j & 1 )
					{
						Positions = Mesh->Positions + Mesh->PositionNum * SetCount ;
						for( i = 0 ; i < ( int )Mesh->PositionNum ; i ++, Positions ++ )
						{
							if( Positions->x == 0.0f )
								PositionUnionFlag[ i ] ++ ;
							else
								Positions->x = -Positions->x ;
						}
					}

					// Y���𔽓]
					if( j & 2 )
					{
						Positions = Mesh->Positions + Mesh->PositionNum * SetCount ;
						for( i = 0 ; i < ( int )Mesh->PositionNum ; i ++, Positions ++ )
						{
							if( Positions->y == 0.0f )
								PositionUnionFlag[ i ] ++ ;
							else
								Positions->y = -Positions->y ;
						}
					}

					// Z���𔽓]
					if( j & 4 )
					{
						Positions = Mesh->Positions + Mesh->PositionNum * SetCount ;
						for( i = 0 ; i < ( int )Mesh->PositionNum ; i ++, Positions ++ )
						{
							if( Positions->z == 0.0f )
								PositionUnionFlag[ i ] ++ ;
							else
								Positions->z = -Positions->z ;
						}
					}

					// �ʂ��w�肷����W�C���f�b�N�X�̃Z�b�g
					Faces = Mesh->Faces + OrigFaceNum * SetCount ;
					OrigFacesT = Mesh->Faces ;
					for( i = 0 ; i < ( int )OrigFaceNum ; i ++, Faces ++, OrigFacesT ++ )
					{
						if( BitNum & 1 )
						{
							switch( Faces->IndexNum )
							{
							case 3 :
								Faces->VertexIndex[ 0 ]      = OrigFacesT->VertexIndex[ 0 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 0 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 0 ]      = OrigFacesT->NormalIndex[ 0 ] ;
								Faces->UVIndex[ 0 ][ 0 ]     = OrigFacesT->UVIndex[ 0 ][ 0 ] ;
								Faces->VertexColorIndex[ 0 ] = OrigFacesT->VertexColorIndex[ 0 ] ;

								Faces->VertexIndex[ 1 ]      = OrigFacesT->VertexIndex[ 2 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 2 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 1 ]      = OrigFacesT->NormalIndex[ 2 ] ;
								Faces->UVIndex[ 0 ][ 1 ]     = OrigFacesT->UVIndex[ 0 ][ 2 ] ;
								Faces->VertexColorIndex[ 1 ] = OrigFacesT->VertexColorIndex[ 2 ] ;

								Faces->VertexIndex[ 2 ]      = OrigFacesT->VertexIndex[ 1 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 1 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 2 ]      = OrigFacesT->NormalIndex[ 1 ] ;
								Faces->UVIndex[ 0 ][ 2 ]     = OrigFacesT->UVIndex[ 0 ][ 1 ] ;
								Faces->VertexColorIndex[ 2 ] = OrigFacesT->VertexColorIndex[ 1 ] ;
								break ;

							case 4 :
								Faces->VertexIndex[ 0 ]      = OrigFacesT->VertexIndex[ 3 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 3 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 0 ]      = OrigFacesT->NormalIndex[ 3 ] ;
								Faces->UVIndex[ 0 ][ 0 ]     = OrigFacesT->UVIndex[ 0 ][ 3 ] ;
								Faces->VertexColorIndex[ 0 ] = OrigFacesT->VertexColorIndex[ 3 ] ;

								Faces->VertexIndex[ 1 ]      = OrigFacesT->VertexIndex[ 2 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 2 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 1 ]      = OrigFacesT->NormalIndex[ 2 ] ;
								Faces->UVIndex[ 0 ][ 1 ]     = OrigFacesT->UVIndex[ 0 ][ 2 ] ;
								Faces->VertexColorIndex[ 1 ] = OrigFacesT->VertexColorIndex[ 2 ] ;

								Faces->VertexIndex[ 2 ]      = OrigFacesT->VertexIndex[ 1 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 1 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 2 ]      = OrigFacesT->NormalIndex[ 1 ] ;
								Faces->UVIndex[ 0 ][ 2 ]     = OrigFacesT->UVIndex[ 0 ][ 1 ] ;
								Faces->VertexColorIndex[ 2 ] = OrigFacesT->VertexColorIndex[ 1 ] ;

								Faces->VertexIndex[ 3 ]      = OrigFacesT->VertexIndex[ 0 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 0 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 3 ]      = OrigFacesT->NormalIndex[ 0 ] ;
								Faces->UVIndex[ 0 ][ 3 ]     = OrigFacesT->UVIndex[ 0 ][ 0 ] ;
								Faces->VertexColorIndex[ 3 ] = OrigFacesT->VertexColorIndex[ 0 ] ;
								break ;
							}
						}
						else
						{
							switch( Faces->IndexNum )
							{
							case 3 :
								Faces->VertexIndex[ 0 ]      = OrigFacesT->VertexIndex[ 0 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 0 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 0 ]      = OrigFacesT->NormalIndex[ 0 ] ;
								Faces->UVIndex[ 0 ][ 0 ]     = OrigFacesT->UVIndex[ 0 ][ 0 ] ;
								Faces->VertexColorIndex[ 0 ] = OrigFacesT->VertexColorIndex[ 0 ] ;

								Faces->VertexIndex[ 1 ]      = OrigFacesT->VertexIndex[ 1 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 1 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 1 ]      = OrigFacesT->NormalIndex[ 1 ] ;
								Faces->UVIndex[ 0 ][ 1 ]     = OrigFacesT->UVIndex[ 0 ][ 1 ] ;
								Faces->VertexColorIndex[ 1 ] = OrigFacesT->VertexColorIndex[ 1 ] ;

								Faces->VertexIndex[ 2 ]      = OrigFacesT->VertexIndex[ 2 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 2 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 2 ]      = OrigFacesT->NormalIndex[ 2 ] ;
								Faces->UVIndex[ 0 ][ 2 ]     = OrigFacesT->UVIndex[ 0 ][ 2 ] ;
								Faces->VertexColorIndex[ 2 ] = OrigFacesT->VertexColorIndex[ 2 ] ;
								break ;

							case 4 :
								Faces->VertexIndex[ 0 ]      = OrigFacesT->VertexIndex[ 0 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 0 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 0 ]      = OrigFacesT->NormalIndex[ 0 ] ;
								Faces->UVIndex[ 0 ][ 0 ]     = OrigFacesT->UVIndex[ 0 ][ 0 ] ;
								Faces->VertexColorIndex[ 0 ] = OrigFacesT->VertexColorIndex[ 0 ] ;

								Faces->VertexIndex[ 1 ]      = OrigFacesT->VertexIndex[ 1 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 1 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 1 ]      = OrigFacesT->NormalIndex[ 1 ] ;
								Faces->UVIndex[ 0 ][ 1 ]     = OrigFacesT->UVIndex[ 0 ][ 1 ] ;
								Faces->VertexColorIndex[ 1 ] = OrigFacesT->VertexColorIndex[ 1 ] ;

								Faces->VertexIndex[ 2 ]      = OrigFacesT->VertexIndex[ 2 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 2 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 2 ]      = OrigFacesT->NormalIndex[ 2 ] ;
								Faces->UVIndex[ 0 ][ 2 ]     = OrigFacesT->UVIndex[ 0 ][ 2 ] ;
								Faces->VertexColorIndex[ 2 ] = OrigFacesT->VertexColorIndex[ 2 ] ;

								Faces->VertexIndex[ 3 ]      = OrigFacesT->VertexIndex[ 3 ] + ( PositionUnionFlag[ OrigFacesT->VertexIndex[ 3 ] ] != BitNum ? Mesh->PositionNum * SetCount : 0 ) ;
								Faces->NormalIndex[ 3 ]      = OrigFacesT->NormalIndex[ 3 ] ;
								Faces->UVIndex[ 0 ][ 3 ]     = OrigFacesT->UVIndex[ 0 ][ 3 ] ;
								Faces->VertexColorIndex[ 3 ] = OrigFacesT->VertexColorIndex[ 3 ] ;
								break ;
							}
						}
					}

					SetCount ++ ;
				}

				// �|���S���̐��𑝂₷
				RModel.TriangleNum += Mesh->TriangleNum * ( TotalMeshNum - 1 ) ;
				Mesh->TriangleNum += Mesh->TriangleNum * ( TotalMeshNum - 1 ) ;

				// ���W�̐������₷
				Mesh->PositionNum += Mesh->PositionNum * ( TotalMeshNum - 1 ) ;

				// �t���O�i�[�p�̃����������
				DXFREE( PositionUnionFlag ) ;
				PositionUnionFlag = NULL ;
			}
		}
		else
		if( _STRCMP( String, "Eof" ) == 0 )
		{
			break ;
		}
		else
		{
			SkipChunk( &MqoModel ) ;
		}
	}

	// �g�p����Ă��Ȃ��}�e���A���̓e�N�X�`���ǂݍ��݃G���[���Ȃ����ׂɃ��C���[���O�ɂ���
	Material = RModel.MaterialFirst ;
	for( i = 0 ; i < ( int )RModel.MaterialNum ; i ++, Material = Material->DataNext )
	{
		if( UseMaterialMapG[ i ] == 0 )
			Material->DiffuseTexNum = 0 ;
	}

	// ���f����f�[�^�n���h���̍쐬
	NewHandle = MV1LoadModelToReadModel( &LoadParam->GParam, &RModel, LoadParam->CurrentDir, LoadParam->FileReadFunc, ASyncThread ) ;
	if( NewHandle < 0 ) goto ENDLABEL ;

	// �G���[�t���O��|��
	ErrorFlag = 0 ;

ENDLABEL :

	// �G���[�t���O�������Ă����烂�f���n���h�������
	if( ErrorFlag == 1 && NewHandle != -1 )
	{
		MV1SubModelBase( NewHandle ) ;
		NewHandle = -1 ;
	}

	// �������̉��
	if( PositionUnionFlag )
	{
		DXFREE( PositionUnionFlag ) ;
		PositionUnionFlag = NULL ;
	}

	// �ǂݍ��݂悤���f�������
	MV1TermReadModel( &RModel ) ; 

	// �n���h����Ԃ�
	return NewHandle ;
}

}

#endif


