// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ShowSequencerEditorToolkit.h"
#include "ShowMaker/SShowMakerWidget.h"
#include "Misc/PathsUtil.h"
#include "ActorPreviewViewport.h"
#include "Runtime/ShowKeys/ShowAnimStatic.h"
#include "Runtime/ShowSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/ShowSequencerComponent.h"

#define LOCTEXT_NAMESPACE "FShowSequencerEditorHelper"

FShowSequencerEditorHelper::~FShowSequencerEditorHelper()
{
	Dispose();
}

void FShowSequencerEditorHelper::SetShowMakerWidget(TSharedPtr<SShowMakerWidget> InShowMakerWidget)
{
	ShowMakerWidget = InShowMakerWidget;
}

void FShowSequencerEditorHelper::Tick(float DeltaTime)
{
	if (bIsCreatedFromHlper)
	{
		if (EditShowSequencerPtr && EditShowSequencerPtr->GetOwner())
		{
			EShowSequencerState ShowSequencerState = EditShowSequencerPtr->GetShowSequencerState();
			switch (ShowSequencerState)
			{
			case EShowSequencerState::ShowSequencer_Wait:
				break;
			case EShowSequencerState::ShowSequencer_Playing:
				EditShowSequencerPtr->Tick(DeltaTime);
				break;
			case EShowSequencerState::ShowSequencer_Pause:
				break;
			case EShowSequencerState::ShowSequencer_End:
				ShowSequencerReset();
				break;
			default:
				break;
			}
		}
	}
}

void FShowSequencerEditorHelper::NotifyShowKeyChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged)
{
	FFieldVariant FieldVariant = PropertyChangedEvent.Property->Owner;

	if (UStruct* Struct = PropertyChangedEvent.Property->GetOwnerStruct())
	{
		if (SelectedShowBasePtr)
		{
			FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
			FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

			if (SelectedShowBasePtr->IsA<UShowAnimStatic>())
			{
				if (PropertyName.IsEqual("AnimSequenceAsset"))
				{
					SelectedShowBasePtr->ExecuteReset();
				}
			}
		}
	}

	EditShowSequencerPtr->ShowSequenceAsset->MarkPackageDirty();
}

void FShowSequencerEditorHelper::Dispose()
{
	if (EditShowSequencerPtr)
	{
		if (bIsCreatedFromHlper)
		{
			EditShowSequencerPtr->ReleaseDontDestroy();
			ShowSequencerClearShowObjects();
			EditShowSequencerPtr->RemoveFromRoot();
			EditShowSequencerPtr->Owner = nullptr;
		}
		
		EditShowSequencerPtr = nullptr;
	}

	ShowMakerWidget.Reset();
	ShowMakerWidget = nullptr;
	SelectedShowBasePtr = nullptr;
}

void FShowSequencerEditorHelper::HelperNewShowSequencer(TObjectPtr<UShowSequenceAsset> InShowSequenceAsset)
{
	checkf(InShowSequenceAsset, TEXT("FShowSequencerEditorHelper::ShowSequencerInitialize InShowSequenceAsset is nullptr."));
	checkf(!EditShowSequencerPtr, TEXT("FShowSequencerEditorHelper::ShowSequencerInitialize EditShowSequencerPtr is already initialized."));

	EditShowSequencerPtr = NewObject<UShowSequencer>(GetTransientPackage(), UShowSequencer::StaticClass());
	checkf(EditShowSequencerPtr, TEXT("FShowSequencerEditorHelper::ShowSequencerInitialize EditShowSequencerPtr is nullptr."));
	bIsCreatedFromHlper = true;

	EditShowSequencerPtr->AddToRoot();
	EditShowSequencerPtr->SetDontDestroy();
	EditShowSequencerPtr->ShowSequenceAsset = InShowSequenceAsset;
	EditShowSequencerPtr->ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;
	EditShowSequencerPtr->PassedTime = 0.0f;

	if (EditShowSequencerPtr->RuntimeShowKeys.IsEmpty())
	{
		EditShowSequencerPtr->RuntimeShowKeys.SetNum(EditShowSequencerPtr->ShowSequenceAsset->ShowKeys.Num());
	}

	for (int32 i = 0; i < EditShowSequencerPtr->ShowSequenceAsset->ShowKeys.Num(); ++i)
	{
		if (EditShowSequencerPtr->RuntimeShowKeys[i])
		{
			continue;
		}

		const FInstancedStruct& Key = EditShowSequencerPtr->ShowSequenceAsset->ShowKeys[i];
		checkf(Key.GetScriptStruct()->IsChildOf(FShowKey::StaticStruct()), TEXT("UShowSequencer::EditorPlay: not FShowKey."));

		const FShowKey* ShowKey = Key.GetPtr<FShowKey>();
		if (!ShowKey)
		{
			continue;
		}

		if (EditorPoolSettings.Num() == 0)
		{
			UObjectPoolManager::GetPoolSettings(EditorPoolSettings);
		}

		EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(ShowKey->KeyType);
		int32 Index = static_cast<int32>(PoolType);
		UShowBase* ShowBasePtr = NewObject<UShowBase>((UObject*)GetTransientPackage(), EditorPoolSettings[Index].ObjectClass);
		ShowBasePtr->AddToRoot();
		ShowBasePtr->InitShowKey(EditShowSequencerPtr, ShowKey);
		EditShowSequencerPtr->RuntimeShowKeys[i] = ShowBasePtr;
	}
}

void FShowSequencerEditorHelper::Play()
{
	AActor* Owner = EditShowSequencerPtr->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("FShowSequencerEditorHelper::Play: Owner is Invalid."));
		return;
	}

	ShowSequencerReset();

	if (EditShowSequencerPtr && ValidateRuntimeShowKeys())
	{
		EShowSequencerState showSequencerState = EditShowSequencerPtr->GetShowSequencerState();
		
		switch (showSequencerState)
		{
		case EShowSequencerState::ShowSequencer_Wait:
			EditShowSequencerPtr->Play();
			break;
		case EShowSequencerState::ShowSequencer_Playing:
			EditShowSequencerPtr->Pause();
			break;
		case EShowSequencerState::ShowSequencer_Pause:
			EditShowSequencerPtr->UnPause();
			break;
		case EShowSequencerState::ShowSequencer_End:
			EditShowSequencerPtr->Play();
			break;
		default:
			break;
		}
	}
}

void FShowSequencerEditorHelper::ShowSequencerReset()
{
	EditShowSequencerPtr->ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;
	EditShowSequencerPtr->PassedTime = 0.0f;

	for (UShowBase* ShowBasePtr : EditShowSequencerPtr->RuntimeShowKeys)
	{
		if (!ShowBasePtr)
		{
			continue;
		}

		ShowBasePtr->ExecuteReset();
	}
}

void FShowSequencerEditorHelper::ShowSequencerStop()
{
	EditShowSequencerPtr->ShowSequencerState = EShowSequencerState::ShowSequencer_End;
	ShowSequencerClearShowObjects();
}

void FShowSequencerEditorHelper::ShowSequencerClearShowObjects()
{
	for (UShowBase*& ShowBasePtr : EditShowSequencerPtr->RuntimeShowKeys)
	{
		ShowBasePtr->Dispose();
		ShowBasePtr->RemoveFromRoot();
		ShowBasePtr = nullptr;
	}
	EditShowSequencerPtr->RuntimeShowKeys.Empty();
}

float FShowSequencerEditorHelper::GetWidgetLengthAlignedToInterval(float Interval)
{
	float TotalLength = 0.0f;
	for (UShowBase* ShowBasePtr : EditShowSequencerPtr->RuntimeShowKeys)
	{
		if (!ShowBasePtr)
		{
			continue;
		}

		float StartTime = ShowBasePtr->GetStartTime();
		float ShowLength = ShowBasePtr->EditorInitializeAssetLength();

		TotalLength = FMath::Max(TotalLength, StartTime + ShowLength);
	}

	if (TotalLength <= 0)
	{
		return Interval;
	}

	int32 RoundedTotalLength = (FMath::FloorToInt((TotalLength - 0.001f) / Interval) + 1) * Interval;
	return static_cast<float>(RoundedTotalLength);
}

const TArray<UShowBase*>* FShowSequencerEditorHelper::RuntimeShowKeysPtr() const
{
	return &EditShowSequencerPtr->RuntimeShowKeys;
}

void FShowSequencerEditorHelper::SetShowBaseStartTime(UShowBase* InShowBasePtr, float StartTime)
{
	for (UShowBase* ShowBasePtr : EditShowSequencerPtr->RuntimeShowKeys)
	{
		if (ShowBasePtr == InShowBasePtr)
		{
			FShowKey* MutableShowKey = const_cast<FShowKey*>(ShowBasePtr->ShowKey);
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
			SetShowSequencerOwner(Owner);

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
				SetShowSequencerOwner(Owner);

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

UShowBase* FShowSequencerEditorHelper::AddKey(FInstancedStruct& NewKey)
{
	checkf(NewKey.GetScriptStruct()->IsChildOf(FShowKey::StaticStruct()), TEXT("FShowSequencerEditorHelper::EditorAddKey: not FShowKey."));

	EditShowSequencerPtr->ShowSequenceAsset->ShowKeys.Add(MoveTemp(NewKey));
	FShowKey* NewShowKey = EditShowSequencerPtr->ShowSequenceAsset->ShowKeys.Last().GetMutablePtr<FShowKey>();
	EditShowSequencerPtr->ShowSequenceAsset->MarkPackageDirty();

	AActor* Owner = EditShowSequencerPtr->GetOwner();
	UWorld* World = Owner->GetWorld();
	UShowBase* NewShowBasePtr = nullptr;

	if (bIsCreatedFromHlper)
	{
		if (EditorPoolSettings.Num() == 0)
		{
			UObjectPoolManager::GetPoolSettings(EditorPoolSettings);
		}

		EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(NewShowKey->KeyType);
		int32 Index = static_cast<int32>(PoolType);
		NewShowBasePtr = NewObject<UShowBase>((UObject*)GetTransientPackage(), EditorPoolSettings[Index].ObjectClass);
		NewShowBasePtr->AddToRoot();
	}	
	else
	{
		UObjectPoolManager* PoolManager = World->GetSubsystem<UObjectPoolManager>();
		checkf(PoolManager, TEXT("FShowSequencerEditorHelper::EditorAddKey: PoolManager is nullptr."));

		EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(NewShowKey->KeyType);
		NewShowBasePtr = PoolManager->GetPooledObject<UShowBase>(PoolType);
	}

	checkf(NewShowBasePtr, TEXT("FShowSequencerEditorHelper::EditorAddKey: NewShowBase is nullptr."));
	if (NewShowBasePtr)
	{
		NewShowBasePtr->InitShowKey(EditShowSequencerPtr, NewShowKey);
		EditShowSequencerPtr->RuntimeShowKeys.Add(NewShowBasePtr);
	}
	
	return NewShowBasePtr;
}

bool FShowSequencerEditorHelper::RemoveKey(UShowBase* RemoveShowBasePtr)
{
	if (!EditShowSequencerPtr->RuntimeShowKeys.Contains(RemoveShowBasePtr))
	{
		return false;
	}

	int32 KeyIndex = FindShowKeyIndex(RemoveShowBasePtr->ShowKey);
	checkf(KeyIndex != INDEX_NONE, TEXT("FShowSequencerEditorHelper::RemoveKey: ShowKey not found in ShowKeys."));

	AActor* Owner = EditShowSequencerPtr->GetOwner();
	UWorld* World = Owner->GetWorld();

	if (bIsCreatedFromHlper)
	{
		RemoveShowBasePtr->Dispose();
		RemoveShowBasePtr->RemoveFromRoot();
	}
	else
	{
		UObjectPoolManager* PoolManager = World->GetSubsystem<UObjectPoolManager>();
		checkf(PoolManager, TEXT("FShowSequencerEditorHelper::RemoveKey: PoolManager is nullptr."));

		EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(RemoveShowBasePtr->ShowKey->KeyType);
		PoolManager->ReturnPooledObject(RemoveShowBasePtr, PoolType);
	}

	EditShowSequencerPtr->RuntimeShowKeys.Remove(RemoveShowBasePtr);
	EditShowSequencerPtr->ShowSequenceAsset->ShowKeys.RemoveAt(KeyIndex);
	EditShowSequencerPtr->ShowSequenceAsset->MarkPackageDirty();
	return true;
}

int32 FShowSequencerEditorHelper::FindShowKeyIndex(const FShowKey* ShowKey) const
{
	for (int32 i = 0; i < EditShowSequencerPtr->ShowSequenceAsset->ShowKeys.Num(); ++i)
	{
		const FShowKey* Key = EditShowSequencerPtr->ShowSequenceAsset->ShowKeys[i].GetPtr<FShowKey>();
		if (Key == ShowKey)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

UScriptStruct* FShowSequencerEditorHelper::GetShowKeyStaticStruct(UShowBase* ShowBasePtr)
{
	return ShowSystem::GetShowKeyStaticStruct(ShowBasePtr->ShowKey->KeyType);
}

FShowKey* FShowSequencerEditorHelper::GetMutableShowKey(UShowBase* ShowBasePtr)
{
	return const_cast<FShowKey*>(ShowBasePtr->ShowKey);
}

UShowBase* FShowSequencerEditorHelper::CheckGetSelectedShowBase()
{
	for (UShowBase* ShowBasePtr : EditShowSequencerPtr->RuntimeShowKeys)
	{
		if (ShowBasePtr == SelectedShowBasePtr)
		{
			return SelectedShowBasePtr;
		}
	}

	if (EditShowSequencerPtr->RuntimeShowKeys.Num() > 0)
	{
		return EditShowSequencerPtr->RuntimeShowKeys[0];
	}
	return nullptr;
}

bool FShowSequencerEditorHelper::ValidateRuntimeShowKeys()
{
 	AActor* Owner = EditShowSequencerPtr->GetOwner();
	if (!Owner)
	{
		FText txt = LOCTEXT("ShowSequencerNoneOwner", "[UShowSequencer] None Owner Actor");
		FMessageDialog::Open(EAppMsgType::Ok, txt);
		return false;
	}

	for (UShowBase* ShowBasePtr : EditShowSequencerPtr->RuntimeShowKeys)
	{
		if (!ShowBasePtr)
		{
			return false;
		}

		if (ShowBasePtr->IsA(UShowAnimStatic::StaticClass()))
		{
			if (!ValidateShowAnimStatic(Owner, ShowBasePtr))
			{
				return false;
			}
		}
	}
	return true;
}

bool FShowSequencerEditorHelper::ValidateShowAnimStatic(AActor* Owner, UShowBase* ShowBasePtr)
{
	UShowAnimStatic* ShowAnimStatic = Cast<UShowAnimStatic>(ShowBasePtr);
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