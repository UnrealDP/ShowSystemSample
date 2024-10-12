// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ShowSequencerEditorToolkit.h"
#include "ShowMaker/SShowMakerWidget.h"
#include "Misc/PathsUtil.h"
#include "ActorPreviewViewport.h"
#include "Runtime/ShowKeys/ShowAnimStatic.h"

FShowSequencerEditorHelper::FShowSequencerEditorHelper(FShowSequencerEditorToolkit* InShowSequencerEditorToolkit, TObjectPtr<UShowSequencer> InEditShowSequencer)
{
	checkf(InShowSequencerEditorToolkit, TEXT("FShowSequencerEditorHelper::FShowSequencerEditorHelper ShowSequencerEditorToolkit is nullptr"));
	checkf(InEditShowSequencer, TEXT("FShowSequencerEditorHelper::FShowSequencerEditorHelper InEditShowSequencer is nullptr"));

	ShowSequencerEditorToolkit = InShowSequencerEditorToolkit;

	EditShowSequencer = InEditShowSequencer;
	EditShowSequencer->EditorInitialize();
	EditShowSequencer->SetDontDestroy();
}

FShowSequencerEditorHelper::~FShowSequencerEditorHelper()
{
	EditShowSequencer->ReleaseDontDestroy();
	EditShowSequencer->EditorBeginDestroy();
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
		if (UShowBase* ShowBase = EditShowSequencer->EditorGetShowBase(SelectedShowKey))
		{
			FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
			FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

			if (PropertyName.IsEqual("AnimSequenceAsset"))
			{
				ShowBase->ExecuteReset();
			}
		}
	}

	EditShowSequencer->MarkPackageDirty();
}

void FShowSequencerEditorHelper::Play()
{
	if (EditShowSequencer)
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

TArray<FShowKey*> FShowSequencerEditorHelper::GetShowKeys()
{
	if (!EditShowSequencer)
	{
		return TArray<FShowKey*>();
	}

	TArray<FShowKey*> ShowKeyPointers;

	for (FInstancedStruct& Struct : EditShowSequencer->ShowKeys)
	{
		if (Struct.IsValid())
		{
			FShowKey* ShowKey = Struct.GetMutablePtr<FShowKey>();
			ShowKeyPointers.Add(ShowKey);
		}
	}

	return ShowKeyPointers;
}

void FShowSequencerEditorHelper::ShowSequencerDetailsViewForceRefresh()
{
	if (ShowSequencerEditorToolkit->DetailsView)
	{
		ShowSequencerEditorToolkit->DetailsView->ForceRefresh();
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
			EditShowSequencer->SetOwner(Owner);

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
			ActorPreviewViewport->ReplaceSkeletalMesh(SelectedSkeletalMesh);

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