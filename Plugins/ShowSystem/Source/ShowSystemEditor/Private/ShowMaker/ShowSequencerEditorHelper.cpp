// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ShowSequencerEditorToolkit.h"
#include "ShowMaker/SShowMakerWidget.h"
#include "Misc/PathsUtil.h"
#include "ActorPreviewViewport.h"
#include "Runtime/ShowKeys/ShowAnimStatic.h"
#include "Runtime/ShowSystem.h"

#define LOCTEXT_NAMESPACE "FShowSequencerEditorHelper"

FShowSequencerEditorHelper::FShowSequencerEditorHelper(TObjectPtr<UShowSequencer> InEditShowSequencer)
{
	checkf(InEditShowSequencer, TEXT("FShowSequencerEditorHelper::FShowSequencerEditorHelper InEditShowSequencer is nullptr"));
	EditShowSequencer = InEditShowSequencer;
}

FShowSequencerEditorHelper::~FShowSequencerEditorHelper()
{
	ShowMakerWidget.Reset();
	ShowMakerWidget = nullptr;
	SelectedShowBase = nullptr;
	EditShowSequencer = nullptr;
}

void FShowSequencerEditorHelper::SetShowMakerWidget(TSharedPtr<SShowMakerWidget> InShowMakerWidget)
{
	ShowMakerWidget = InShowMakerWidget;
}

void FShowSequencerEditorHelper::Tick(float DeltaTime)
{
	if (EditShowSequencer && EditShowSequencer->GetOwner())
	{
		EShowSequencerState ShowSequencerState = EditShowSequencer->GetShowSequencerState();
		switch (ShowSequencerState)
		{
		case EShowSequencerState::ShowSequencer_Wait:
			break;
		case EShowSequencerState::ShowSequencer_Playing:
			EditShowSequencer->Tick(DeltaTime);
			break;
		case EShowSequencerState::ShowSequencer_Pause:
			break;
		case EShowSequencerState::ShowSequencer_End:
			EditShowSequencer->EditorReset();
			break;
		default:
			break;
		}
	}
}

void FShowSequencerEditorHelper::NotifyShowKeyChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged)
{
	FFieldVariant FieldVariant = PropertyChangedEvent.Property->Owner;

	if (UStruct* Struct = PropertyChangedEvent.Property->GetOwnerStruct())
	{
		if (SelectedShowBase)
		{
			FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
			FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

			if (SelectedShowBase->IsA<UShowAnimStatic>())
			{
				if (PropertyName.IsEqual("AnimSequenceAsset"))
				{
					SelectedShowBase->ExecuteReset();
				}
			}
		}
	}

	EditShowSequencer->ShowSequenceAsset->MarkPackageDirty();
}

void FShowSequencerEditorHelper::Play()
{
	EditShowSequencer->EditorReset();

	if (EditShowSequencer && ValidateRuntimeShowKeys())
	{
		EShowSequencerState showSequencerState = EditShowSequencer->GetShowSequencerState();
		
		switch (showSequencerState)
		{
		case EShowSequencerState::ShowSequencer_Wait:
			EditShowSequencer->EditorPlay();
			break;
		case EShowSequencerState::ShowSequencer_Playing:
			EditShowSequencer->EditorPause();
			break;
		case EShowSequencerState::ShowSequencer_Pause:
			EditShowSequencer->EditorUnPause();
			break;
		case EShowSequencerState::ShowSequencer_End:
			EditShowSequencer->EditorPlay();
			break;
		default:
			break;
		}
	}
}

TArray<TObjectPtr<UShowBase>>* FShowSequencerEditorHelper::RuntimeShowKeysPtr()
{
	return &EditShowSequencer->RuntimeShowKeys;
}

void FShowSequencerEditorHelper::SetShowBaseStartTime(UShowBase* InShowBase, float StartTime)
{
	for (TObjectPtr<UShowBase>& ShowBase : EditShowSequencer->RuntimeShowKeys)
	{
		if (ShowBase.Get() == InShowBase)
		{
			FShowKey* MutableShowKey = const_cast<FShowKey*>(ShowBase->ShowKey);
			MutableShowKey->StartTime = StartTime;
			break;
		}
	}
}

UClass* FShowSequencerEditorHelper::GetLastSelectedActorClass()
{
	FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
	if (GConfig)
	{
		FString SelectedActorPath;
		if (GConfig->GetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedActor"), SelectedActorPath, *ConfigFilePath))
		{
			UClass* SavedActorClass = FindObject<UClass>(nullptr, *SelectedActorPath);
			if (SavedActorClass && SavedActorClass->IsChildOf(AActor::StaticClass()))
			{
				return SavedActorClass;
			}
		}
	}
	return nullptr;
}

USkeletalMesh* FShowSequencerEditorHelper::LoadLastSelectedOrDefaultSkeletalMesh()
{
	FString SelectedMeshPath;
	// 이전에 로드했던 스켈레털 메쉬 있는지 확인
	// 플러그인 폴더 경로에서 설정 파일 경로를 가져오기
	FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
	// 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
	if (GConfig)
	{
		GConfig->GetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), SelectedMeshPath, *ConfigFilePath);
	}

	USkeletalMesh* SkeletalMesh = nullptr;
	if (SelectedMeshPath.IsEmpty())
	{
		// 디폴트 어셋 로드
		SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/EngineMeshes/SkeletalCube.SkeletalCube"));

		SelectedMeshPath = SkeletalMesh->GetPathName();
		// 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
		if (GConfig)
		{
			GConfig->SetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), *SelectedMeshPath, *ConfigFilePath);
			GConfig->Flush(false, *ConfigFilePath);
		}
	}
	else
	{
		SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *SelectedMeshPath);
	}

	return SkeletalMesh;
}

UClass* FShowSequencerEditorHelper::GetLastSelectedAnimInstanceClass()
{
	FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
	if (GConfig)
	{
		FString SelectedAnimInterfacePath;
		if (GConfig->GetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedAnim"), SelectedAnimInterfacePath, *ConfigFilePath))
		{
			UClass* SavedAnimInstanceClass = FindObject<UClass>(nullptr, *SelectedAnimInterfacePath);
			if (SavedAnimInstanceClass && SavedAnimInstanceClass->IsChildOf(UAnimInstance::StaticClass()))
			{
				return SavedAnimInstanceClass;
			}
		}
	}
	return nullptr;
}

void FShowSequencerEditorHelper::ReplaceActorPreviewWorld(UClass* ActorClass)
{
	if (ShowMakerWidget.IsValid())
	{
		if (SActorPreviewViewport* ActorPreviewViewport = ShowMakerWidget->GetPreviewViewportPtr())
		{
			AActor* Owner = ActorPreviewViewport->ReplaceActorPreviewWorld(ActorClass);
			EditShowSequencer->EditorSetOwner(Owner);

			FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
			if (GConfig)
			{
				GConfig->SetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedActor"), *ActorClass->GetPathName(), *ConfigFilePath);
				GConfig->Flush(false, *ConfigFilePath);
			}
		}
	}
}

void FShowSequencerEditorHelper::ReplaceSkeletalMeshPreviewWorld(USkeletalMesh* SelectedSkeletalMesh)
{
	if (ShowMakerWidget.IsValid())
	{
		if (SActorPreviewViewport* ActorPreviewViewport = ShowMakerWidget->GetPreviewViewportPtr())
		{
			AActor* PreviewActor = ActorPreviewViewport->GetActor();
			UClass* PreviewActorClass = PreviewActor->GetClass();
			if (PreviewActorClass != AActor::StaticClass() || PreviewActorClass->ClassGeneratedBy != nullptr)
			{
				AActor* Owner = ActorPreviewViewport->ReplaceActorPreviewWorld(AActor::StaticClass());
				EditShowSequencer->EditorSetOwner(Owner);

				ActorPreviewViewport->SetSkeletalMesh(SelectedSkeletalMesh);
			}
			else
			{
				ActorPreviewViewport->ReplaceSkeletalMesh(SelectedSkeletalMesh);
			}

			FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
			if (GConfig)
			{
				FString SelectedSkeletalMeshPath = SelectedSkeletalMesh->GetPathName();
				GConfig->RemoveKey(TEXT("SShowMakerWidget"), TEXT("LastSelectedActor"), *ConfigFilePath);
				GConfig->SetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), *SelectedSkeletalMeshPath, *ConfigFilePath);
				GConfig->Flush(false, *ConfigFilePath);
			}
		}
	}
}

void FShowSequencerEditorHelper::ReplaceAnimInstancePreviewWorld(UClass* AnimInstanceClass)
{
	if (ShowMakerWidget.IsValid())
	{
		if (SActorPreviewViewport* ActorPreviewViewport = ShowMakerWidget->GetPreviewViewportPtr())
		{
			ActorPreviewViewport->ReplaceAnimInstancePreviewWorld(AnimInstanceClass);

			FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
			if (GConfig)
			{
				GConfig->SetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedAnim"), *AnimInstanceClass->GetPathName(), *ConfigFilePath);
				GConfig->Flush(false, *ConfigFilePath);
			}
		}
	}
}

TObjectPtr<UShowBase> FShowSequencerEditorHelper::AddKey(FInstancedStruct& NewKey)
{
	checkf(NewKey.GetScriptStruct()->IsChildOf(FShowKey::StaticStruct()), TEXT("FShowSequencerEditorHelper::EditorAddKey: not FShowKey."));

	EditShowSequencer->ShowSequenceAsset->ShowKeys.Add(MoveTemp(NewKey));

	FShowKey* NewShowKey = EditShowSequencer->ShowSequenceAsset->ShowKeys.Last().GetMutablePtr<FShowKey>();
	if (!NewShowKey)
	{
		return nullptr;
	}

	if (EditShowSequencer->EditorPoolSettings.Num() == 0)
	{
		UObjectPoolManager::GetPoolSettings(EditShowSequencer->EditorPoolSettings);
	}

	EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(NewShowKey->KeyType);
	int32 Index = static_cast<int32>(PoolType);
	UShowBase* NewShowBase = NewObject<UShowBase>((UObject*)GetTransientPackage(), EditShowSequencer->EditorPoolSettings[Index].ObjectClass);
	NewShowBase->AddToRoot();
	NewShowBase->InitShowKey(EditShowSequencer, NewShowKey);
	EditShowSequencer->RuntimeShowKeys.Add(NewShowBase);

	EditShowSequencer->ShowSequenceAsset->MarkPackageDirty();
	return NewShowBase;
}

bool FShowSequencerEditorHelper::RemoveKey(TObjectPtr<UShowBase> RemoveShowBase)
{
	if (!EditShowSequencer->RuntimeShowKeys.Contains(RemoveShowBase))
	{
		return false;
	}

	int32 KeyIndex = FindShowKeyIndex(RemoveShowBase->ShowKey);
	checkf(KeyIndex != INDEX_NONE, TEXT("FShowSequencerEditorHelper::RemoveKey: ShowKey not found in ShowKeys."));

	RemoveShowBase->Dispose();
	RemoveShowBase->RemoveFromRoot();
	EditShowSequencer->RuntimeShowKeys.Remove(RemoveShowBase);

	EditShowSequencer->ShowSequenceAsset->ShowKeys.RemoveAt(KeyIndex);
	EditShowSequencer->ShowSequenceAsset->MarkPackageDirty();
	return true;
}

int32 FShowSequencerEditorHelper::FindShowKeyIndex(const FShowKey* ShowKey) const
{
	for (int32 i = 0; i < EditShowSequencer->ShowSequenceAsset->ShowKeys.Num(); ++i)
	{
		const FShowKey* Key = EditShowSequencer->ShowSequenceAsset->ShowKeys[i].GetPtr<FShowKey>();
		if (Key == ShowKey)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

UScriptStruct* FShowSequencerEditorHelper::GetShowKeyStaticStruct(UShowBase* ShowBase)
{
	return ShowSystem::GetShowKeyStaticStruct(ShowBase->ShowKey->KeyType);
}

FShowKey* FShowSequencerEditorHelper::GetMutableShowKey(UShowBase* ShowBase)
{
	return const_cast<FShowKey*>(ShowBase->ShowKey);
}

TObjectPtr<UShowBase> FShowSequencerEditorHelper::CheckGetSelectedShowBase()
{
	for (TObjectPtr<UShowBase>& ShowBase : EditShowSequencer->RuntimeShowKeys)
	{
		if (ShowBase == SelectedShowBase)
		{
			return SelectedShowBase;
		}
	}

	if (EditShowSequencer->RuntimeShowKeys.Num() > 0)
	{
		return EditShowSequencer->RuntimeShowKeys[0];
	}
	return nullptr;
}

bool FShowSequencerEditorHelper::ValidateRuntimeShowKeys()
{
 	AActor* Owner = EditShowSequencer->GetOwner();
	if (!Owner)
	{
		FText txt = LOCTEXT("ShowSequencerNoneOwner", "[UShowSequencer] None Owner Actor");
		FMessageDialog::Open(EAppMsgType::Ok, txt);
		return false;
	}

	for (TObjectPtr<UShowBase>& ShowBase : EditShowSequencer->RuntimeShowKeys)
	{
		if (!ShowBase)
		{
			return false;
		}

		if (ShowBase->IsA(UShowAnimStatic::StaticClass()))
		{
			if (!ValidateShowAnimStatic(Owner, ShowBase))
			{
				return false;
			}
		}
	}
	return true;
}

bool FShowSequencerEditorHelper::ValidateShowAnimStatic(AActor* Owner, TObjectPtr<UShowBase>& ShowBase)
{
	UShowAnimStatic* ShowAnimStatic = Cast<UShowAnimStatic>(ShowBase);
	if (!ShowAnimStatic->GetAnimSequenceBase())
	{
		FText txt = LOCTEXT("ShowBaseNoneAnim", "[UShowAnimStatic] An animation is missing for one or more keys.");
		FMessageDialog::Open(EAppMsgType::Ok, txt);
		return false;
	}

	USkeletalMeshComponent* SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!SkeletalMeshComp)
	{
		FText txt = LOCTEXT("ShowBaseNoneAnim", "[UShowAnimStatic] None SkeletalMeshComp");
		FMessageDialog::Open(EAppMsgType::Ok, txt);
		return false;
	}

	if (!SkeletalMeshComp->AnimClass)
	{
		FText txt = LOCTEXT("ShowBaseNoneAnim", "[UShowAnimStatic] None SkeletalMeshComp->AnimClass");
		FMessageDialog::Open(EAppMsgType::Ok, txt);
		return false;
	}

	UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		FText txt = LOCTEXT("ShowBaseNoneAnim", "[UShowAnimStatic] None AnimInstance");
		FMessageDialog::Open(EAppMsgType::Ok, txt);
		return false;
	}

	return true;
}


#undef LOCTEXT_NAMESPACE