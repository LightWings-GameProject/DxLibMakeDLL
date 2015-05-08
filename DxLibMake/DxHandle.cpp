// -------------------------------------------------------------------------------
// 
// 		�c�w���C�u����		�n���h���Ǘ��v���O����
// 
// 				Ver 3.12a
// 
// -------------------------------------------------------------------------------

// �c�w���C�u�����쐬���p��`
#define __DX_MAKE

// �C���N���[�h ------------------------------------------------------------------
#include "DxHandle.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxASyncLoad.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxLog.h"

#ifndef DX_NON_ASYNCLOAD
#include "DxGraphics.h"
#include "DxSoftImage.h"
#include "DxSound.h"
#include "DxMask.h"
#include "DxNetwork.h"
#include "DxModel.h"
#include "DxFile.h"
#include "DxFont.h"
#endif // DX_NON_ASYNCLOAD

namespace DxLib
{

// �}�N����` --------------------------------------------------------------------

// �\���̒�` --------------------------------------------------------------------

// �������ϐ��錾 --------------------------------------------------------------

HANDLEMANAGE HandleManageArray[ DX_HANDLETYPE_MAX ] ;

// �֐��v���g�^�C�v�錾-----------------------------------------------------------

// �v���O���� --------------------------------------------------------------------


// �n���h�����ʊ֌W

// �n���h���Ǘ���������������
// ( InitializeFlag �ɂ� FALSE �������Ă���K�v������ )
extern int InitializeHandleManage(
	int HandleType,
	int OneSize,
	int MaxNum,
	int ( *InitializeFunction )( HANDLEINFO *HandleInfo ),
	int ( *TerminateFunction )( HANDLEINFO *HandleInfo ),
	const TCHAR *Name
)
{
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	// ���ɏ���������Ă����牽�����Ȃ�
	if( HandleManage->InitializeFlag )
		return -1 ;

	// NextID�ȊO�[��������
	int NextID = HandleManage->NextID ;
	_MEMSET( HandleManage, 0, sizeof( *HandleManage ) ) ;
	HandleManage->NextID = NextID ;

	// �p�����[�^�Z�b�g
	HandleManage->HandleTypeMask = HandleType << DX_HANDLETYPE_ADDRESS ;
	HandleManage->OneSize = OneSize ;
	HandleManage->MaxNum = MaxNum ;
	HandleManage->InitializeFunction = InitializeFunction ;
	HandleManage->TerminateFunction = TerminateFunction ;
	HandleManage->Name = Name ;

	// �n���h���̃f�[�^�|�C���^�A�h���X���i�[���郁�����A�h���X�z��̊m��
	HandleManage->Handle = ( HANDLEINFO ** )DXCALLOC( sizeof( HANDLEINFO * ) * MaxNum ) ;
	if( HandleManage->Handle == NULL )
		return -1 ;

	// �n���h�����X�g�̏�����
	InitializeHandleList( &HandleManage->ListFirst, &HandleManage->ListLast ) ;

	// �N���e�B�J���Z�N�V�����̏�����
	CriticalSection_Initialize( &HandleManage->CriticalSection ) ;

	// �������t���O�𗧂Ă�
	HandleManage->InitializeFlag = TRUE ;

	// �I��
	return 0 ;
}

// �n���h���Ǘ����̌�n�����s��
extern int TerminateHandleManage( int HandleType )
{
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	// ���Ɍ�n������Ă����牽�����Ȃ�
	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// ���ׂẴn���h�����폜
	AllHandleSub( HandleType ) ;

	// �n���h���|�C���^�i�[�p�̃������A�h���X�z������
	if( HandleManage->Handle != NULL )
	{
		DXFREE( HandleManage->Handle ) ;
		HandleManage->Handle = NULL ;
	}

	// �N���e�B�J���Z�N�V�����̍폜
	CriticalSection_Delete( &HandleManage->CriticalSection ) ;

	// NextID�ȊO�[��������
	int NextID = HandleManage->NextID ;
	_MEMSET( HandleManage, 0, sizeof( *HandleManage ) ) ;
	HandleManage->NextID = NextID ;

	// �������t���O��|��
	HandleManage->InitializeFlag = FALSE ;

	// �I��
	return 0 ;
}

// �n���h����ǉ�����
extern int AddHandle( int HandleType, int Handle )
{
	int NextIndex ;
	int NewHandle ;
	HANDLEINFO **ppHandleInfo ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// �N���e�B�J���Z�N�V�����̎擾
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

	// �ǉ��ł��Ȃ��ꍇ�͏I��
	if( HandleManage->Num == HandleManage->MaxNum )
	{
		// �N���e�B�J���Z�N�V�����̉��
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		DXST_ERRORLOGFMT_ADD(( _T( "%s�n���h���̐������E��( %d )�ɒB���Ă��ĐV���ȃn���h�����쐬�ł��܂���" ), HandleManage->Name, HandleManage->MaxNum )) ;
		return -1 ;
	}

	// �󂫔z��ԍ��̌���
	if( Handle != -1 && HandleManage->Handle[ Handle & DX_HANDLEINDEX_MASK ] == NULL )
	{
		NextIndex = Handle & DX_HANDLEINDEX_MASK ;
		ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
	}
	else
	{
		if( HandleManage->Num == 0 )
		{
			NextIndex = 0 ;
			ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
		}
		else
		{
			if( HandleManage->AreaMax + 1 < HandleManage->MaxNum )
			{
				NextIndex = HandleManage->AreaMax + 1 ;
				ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
			}
			else
			if( HandleManage->AreaMin - 1 > 0 )
			{
				NextIndex = HandleManage->AreaMin - 1 ;
				ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
			}
			else
			{
				ppHandleInfo = HandleManage->Handle ;
				for( NextIndex = 0 ; *ppHandleInfo != NULL ; NextIndex ++, ppHandleInfo ++ ){}
			}
		}
	}

	// �f�[�^�̈���m�ۂ���
	if( ( *ppHandleInfo = ( HANDLEINFO * )DXCALLOC( HandleManage->OneSize ) ) == NULL )
	{
		// �N���e�B�J���Z�N�V�����̉��
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		DXST_ERRORLOGFMT_ADD(( _T( "%s�n���h���̃f�[�^���i�[���郁�����̈�̊m�ۂɎ��s���܂���" ), HandleManage->Name )) ;
		return -1 ;
	}

	// �G���[�`�F�b�N�p�h�c�̐ݒ�
	if( Handle != -1 )
	{
		(*ppHandleInfo)->ID = ( Handle & DX_HANDLECHECK_MASK ) >> DX_HANDLECHECK_ADDRESS ;
	}
	else
	{
		HandleManage->NextID ++ ;
		if( HandleManage->NextID >= ( DX_HANDLECHECK_MASK >> DX_HANDLECHECK_ADDRESS ) )
			HandleManage->NextID = 0 ;

		(*ppHandleInfo)->ID = HandleManage->NextID ;
	}

	// �������̊m�ۃT�C�Y��ۑ�
	( *ppHandleInfo )->AllocSize = HandleManage->OneSize ;

	// �n���h���l��ۑ�
	NewHandle = (*ppHandleInfo)->Handle = NextIndex | HandleManage->HandleTypeMask | ( (*ppHandleInfo)->ID << DX_HANDLECHECK_ADDRESS ) ;

	// �n���h���̐��𑝂₷
	HandleManage->Num ++ ;

	// �g�p����Ă���n���h�������݂���͈͂��X�V����
	if( HandleManage->Num == 1 )
	{
		HandleManage->AreaMax = NextIndex ;
		HandleManage->AreaMin = NextIndex ;
	}
	else
	{
		if( HandleManage->AreaMax < NextIndex ) HandleManage->AreaMax = NextIndex ;
		if( HandleManage->AreaMin > NextIndex ) HandleManage->AreaMin = NextIndex ;
	}

	// ���X�g�֗v�f��ǉ�
	AddHandleList( &HandleManage->ListFirst, &(*ppHandleInfo)->List, (*ppHandleInfo)->Handle, *ppHandleInfo ) ;

	// �������֐����Ă�
	if( HandleManage->InitializeFunction )
	{
		HandleManage->InitializeFunction( *ppHandleInfo ) ;
	}

	// �N���e�B�J���Z�N�V�����̉��
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// �n���h����Ԃ�
	return NewHandle ;
}

// �n���h�����폜����
extern int SubHandle( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int Index ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// �N���e�B�J���Z�N�V�����̎擾
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

	Index = Handle & DX_HANDLEINDEX_MASK ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
	{
		// �N���e�B�J���Z�N�V�����̉��
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		return -1 ;
	}

	// �폜�t���O��-1��������
	if( HandleInfo->DeleteFlag != NULL )
	{
		*HandleInfo->DeleteFlag = -1 ;
	}

#ifndef DX_NON_ASYNCLOAD
	// �񓯊��ǂݍ��ݒ��ł���ꍇ�ł܂������������Ă��Ȃ������珈�����L�����Z������
	if( HandleInfo->ASyncLoadCount != 0 )
	{
		if( DeleteASyncLoadData( HandleInfo->ASyncDataNumber, TRUE ) < -1 )
		{
			// �N���e�B�J���Z�N�V�����̉��
			CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

			// �폜�Ɏ��s������ǂݍ��ݏ������I���܂ő҂�
			while( HandleInfo->ASyncLoadCount != 0 )
			{
				ProcessASyncLoadRequestMainThread() ;
				Sleep( 0 );
			}

			// �N���e�B�J���Z�N�V�����̎擾
			CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;
		}
	}
#endif // DX_NON_ASYNCLOAD

	// �n���h���^�C�v�X�̌�n������
	if( HandleManage->TerminateFunction )
	{
		// �߂�l���P�̏ꍇ�͍폜�L�����Z��
		if( HandleManage->TerminateFunction( HandleInfo ) == 1 )
			goto END ;
	}

	// ���X�g����v�f���O��
	SubHandleList( &HandleInfo->List ) ;

	// �f�[�^�̈���������
	DXFREE( HandleInfo ) ;

	// �e�[�u���� NULL ���Z�b�g����
	HandleManage->Handle[ Index ] = NULL ;

	// �n���h���̑��������炷
	HandleManage->Num -- ;

	// �L���ȃn���h�������݂���͈͂̍X�V
	if( HandleManage->Num == 0 )
	{
		HandleManage->AreaMax = 0 ;
		HandleManage->AreaMin = 0 ;
	}
	else
	{
		if( Index == HandleManage->AreaMax )
		{
			while( HandleManage->Handle[ HandleManage->AreaMax ] == NULL )
				HandleManage->AreaMax -- ; 
		}
		else
		if( Index == HandleManage->AreaMin )
		{
			while( HandleManage->Handle[ HandleManage->AreaMin ] == NULL )
				HandleManage->AreaMin ++ ; 
		}
	}

END :

	// �N���e�B�J���Z�N�V�����̉��
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// �I��
	return 0 ;
}

// �n���h���̏����i�[���郁�����̈�̃T�C�Y��ύX����A�񓯊��ǂݍ��ݒ��łȂ����Ƃ��O��
extern int ReallocHandle( int Handle, int NewSize )
{
	HANDLEINFO *HandleInfo ;
	int Index ;
	void *NewBuffer ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	Index = Handle & DX_HANDLEINDEX_MASK ;

	// �m�ۃ������̃T�C�Y���ω����Ȃ��ꍇ�͉��������I��
	if( HandleInfo->AllocSize == NewSize )
		return 0 ;

	// �N���e�B�J���Z�N�V�����̎擾
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

	// �������̍Ċm��
	NewBuffer = ( HANDLEINFO * )DXREALLOC( HandleInfo, NewSize ) ;

	// �������̍Ċm�ې����`�F�b�N
	if( NewBuffer == NULL )
	{
		// �N���e�B�J���Z�N�V�����̉��
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		// �G���[�I��
		return -1 ;
	}

	// �m�ۃ������A�h���X���ω������ꍇ�̓��X�g�̂Ȃ�����
	if( NewBuffer != ( void * )HandleInfo )
	{
		// �V�����������A�h���X���Z�b�g
		HandleInfo = HandleManage->Handle[ Index ] = ( HANDLEINFO * )NewBuffer ;

		// ���X�g�̌q������
		NewMemoryHandleList( &HandleInfo->List, HandleInfo ) ;
	}

	// �N���e�B�J���Z�N�V�����̉��
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// �I��
	return 0 ;
}

// �n���h���̏����擾����
extern HANDLEINFO *GetHandleInfo( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return NULL ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return NULL ;

	// �n���h���̏���Ԃ�
	return HandleInfo ;
}

// �n���h���Ǘ����ɓo�^����Ă��邷�ׂẴn���h�����폜
extern int AllHandleSub( int HandleType, int (*DeleteCancelCheckFunction)( HANDLEINFO *HandleInfo ) )
{
	int i ;
	HANDLEINFO **ppHandleInfo ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// �N���e�B�J���Z�N�V�����̎擾
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

#ifndef DX_NON_ASYNCLOAD
	// �񓯊��ǂݍ��݈˗�����������폜���Ă���
	ppHandleInfo = &HandleManage->Handle[ HandleManage->AreaMin ] ;
	for( i = HandleManage->AreaMin ; i <= HandleManage->AreaMax ; i ++, ppHandleInfo ++ )
	{
		if( *ppHandleInfo )
		{
			if( DeleteCancelCheckFunction == NULL ||
				DeleteCancelCheckFunction( *ppHandleInfo ) == FALSE )
			{
				if( (*ppHandleInfo)->ASyncLoadCount != 0 )
				{
					if( DeleteASyncLoadData( (*ppHandleInfo)->ASyncDataNumber, TRUE ) >= -1 )
					{
						(*ppHandleInfo)->ASyncLoadCount = 0 ;
					}
				}
			}
		}
	}
#endif // DX_NON_ASYNCLOAD

	// �N���e�B�J���Z�N�V�����̉��
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// ���ׂẴn���h���f�[�^���폜����
	ppHandleInfo = &HandleManage->Handle[ HandleManage->AreaMin ] ;
	for( i = HandleManage->AreaMin ; i <= HandleManage->AreaMax ; i ++, ppHandleInfo ++ )
	{
		if( *ppHandleInfo )
		{
			if( DeleteCancelCheckFunction == NULL ||
				DeleteCancelCheckFunction( *ppHandleInfo ) == FALSE )
			{
				SubHandle( (*ppHandleInfo)->Handle ) ;
			}
		}
	}

	// �I��
	return 0 ;
}









#ifndef DX_NON_ASYNCLOAD

// �񓯊��ǂݍ��݊֌W

// �n���h���̔񓯊��ǂݍ��݂��������Ă��邩�ǂ������擾����( TRUE:�������Ă���  FALSE:�܂��������Ă��Ȃ�  -1:�G���[ )
extern int NS_CheckHandleASyncLoad( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	return HandleInfo->ASyncLoadCount != 0 ? TRUE : FALSE ;
}

// �n���h���̔񓯊��ǂݍ��ݏ����̖߂�l���擾����( �񓯊��ǂݍ��ݒ��̏ꍇ�͈�O�̔񓯊��ǂݍ��ݏ����̖߂�l���Ԃ��Ă��܂� )
extern int NS_GetHandleASyncLoadResult( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	return HandleInfo->ASyncLoadResult ;
}

// �n���h���̔񓯊��ǂݍ��ݒ��J�E���g���C���N�������g����
extern int IncASyncLoadCount( int Handle, int ASyncDataNumber )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	HandleInfo->ASyncLoadCount ++ ;
	HandleInfo->ASyncDataNumber = ASyncDataNumber ;

	// �I��
	return 0 ;
}

// �n���h���̔񓯊��ǂݍ��ݒ��J�E���g���f�N�������g����
extern int DecASyncLoadCount( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	HandleInfo->ASyncLoadCount -- ;

	// �I��
	return 0 ;
}

#endif // DX_NON_ASYNCLOAD









// �n���h�����X�g

// ���X�g�̏�����
extern int InitializeHandleList( HANDLELIST *First, HANDLELIST *Last )
{
	First->Handle = -1 ;
	First->Data   = NULL ;
	First->Prev   = NULL ;
	First->Next   = Last ;
	Last->Handle  = -1 ;
	Last->Data    = NULL ;
	Last->Prev    = First ;
	Last->Next    = NULL ;

	// �I��
	return 0 ;
}

// ���X�g�֗v�f��ǉ�
extern int AddHandleList( HANDLELIST *First, HANDLELIST *List, int Handle, void *Data )
{
	List->Handle      = Handle ;
	List->Data        = Data ;
	List->Prev        = First ;
	List->Next        = First->Next ;
	First->Next->Prev = List ;
	First->Next       = List ;

	// �I��
	return 0 ;
}

// ���X�g����v�f���O��
extern int SubHandleList( HANDLELIST *List )
{
	List->Prev->Next = List->Next ;
	List->Next->Prev = List->Prev ;

	return 0 ;
}

// ���X�g�����݂��郁�������ύX���ꂽ�ꍇ�Ƀ��X�g�̑O����X�V����
extern int NewMemoryHandleList( HANDLELIST *List, void *Data )
{
	List->Data = Data ;

	List->Prev->Next = List ;
	List->Next->Prev = List ;

	// �I��
	return 0 ;
}







// �n���h�����폜���ꂽ�Ƃ��Ɂ|�P���ݒ肳���ϐ���o�^����
extern int NS_SetDeleteHandleFlag( int Handle, int *DeleteFlag )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// �G���[����
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	HandleInfo->DeleteFlag = DeleteFlag  ;

	// �I��
	return 0 ;
}










}
