// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class IStructureDetailsView;
class USceneCaptureComponent2D;

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowCamSequenceDetailsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowCamSequenceDetailsWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IStructureDetailsView>, ShowKeyStructureDetailsView)
	SLATE_END_ARGS()

    ~SShowCamSequenceDetailsWidget();

    const float ImageWidth = 500.0f;
    const float ImageHeight = 300.0f;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
    void UpdateCameraImage(UWorld* World, FVector CameraLocation, FRotator CameraRotation, const TArray<AActor*>& ActorsToHide);

private:
    void InitializeCameraRenderTarget();

    void OnShowCameraCheckChanged(ECheckBoxState NewState);
    const FSlateBrush* GetCameraRenderTargetBrush() const;

    TSharedRef<SWidget> CamSequenceWidget();
    TSharedRef<SWidget> DetailScroll();
    TSharedRef<SWidget> RenderView();

    UPROPERTY()
    TObjectPtr<UTextureRenderTarget2D> CameraRenderTargetTexture = nullptr;

    UPROPERTY()
    TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent = nullptr;

    TSharedPtr<FSlateBrush> CameraRenderTargetBrush;    

    TSharedPtr<IStructureDetailsView> ShowKeyStructureDetailsView = nullptr;;
    bool bShowCamera = false;

    UWorld* BeforeWorld = nullptr;
};
