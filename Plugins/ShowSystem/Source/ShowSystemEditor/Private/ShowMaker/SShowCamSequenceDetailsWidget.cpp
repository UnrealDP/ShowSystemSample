// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowCamSequenceDetailsWidget.h"
#include "SlateOptMacros.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "IStructureDetailsView.h"
#include "SAutoAspectRatioWidthWidget.h"
#include "SVerticalResizableSplitter.h"

#define LOCTEXT_NAMESPACE "SShowCamSequenceDetailsWidget"

SShowCamSequenceDetailsWidget::~SShowCamSequenceDetailsWidget()
{
    if (CameraRenderTargetTexture)
    {
        CameraRenderTargetTexture->RemoveFromRoot();
        CameraRenderTargetTexture = nullptr;
    }

    if (SceneCaptureComponent)
    {
        SceneCaptureComponent->RemoveFromRoot();
        SceneCaptureComponent->DestroyComponent();
        SceneCaptureComponent = nullptr;
    }
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowCamSequenceDetailsWidget::Construct(const FArguments& InArgs)
{
    ShowKeyStructureDetailsView = InArgs._ShowKeyStructureDetailsView;

    InitializeCameraRenderTarget();

    ChildSlot
        [
            CamSequenceWidget()
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SShowCamSequenceDetailsWidget::InitializeCameraRenderTarget()
{
    if (!CameraRenderTargetTexture)
    {
        // Render Target 생성 (초기에는 디폴트 회색 이미지)
        CameraRenderTargetTexture = NewObject<UTextureRenderTarget2D>();
        CameraRenderTargetTexture->InitAutoFormat(ImageWidth, ImageHeight);
        CameraRenderTargetTexture->RenderTargetFormat = RTF_RGBA8;
        CameraRenderTargetTexture->ClearColor = FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);
        CameraRenderTargetTexture->bAutoGenerateMips = false;
        CameraRenderTargetTexture->UpdateResourceImmediate(true);
        CameraRenderTargetTexture->AddToRoot();
    }

    // 초기 브러시 설정
    if (!CameraRenderTargetBrush.IsValid() && CameraRenderTargetTexture)
    {
        CameraRenderTargetBrush = MakeShareable(new FSlateBrush());
        CameraRenderTargetBrush->SetResourceObject(CameraRenderTargetTexture);
        CameraRenderTargetBrush->ImageSize = FVector2D(ImageWidth, ImageHeight);
    }
}

void SShowCamSequenceDetailsWidget::UpdateCameraImage(UWorld* World, FVector CameraLocation, FRotator CameraRotation, const TArray<AActor*>& ActorsToHide)
{
    checkf(CameraRenderTargetTexture, TEXT("SShowCamSequenceDetailsWidget::UpdateCameraImage CameraRenderTargetTexture is nullptr."));

    bool bUpdateWorld = false;
    if (BeforeWorld != World)
    {
        bUpdateWorld = true;
        BeforeWorld = World;
    }

    if (bUpdateWorld)
    {
        if (SceneCaptureComponent)
        {
            SceneCaptureComponent->RemoveFromRoot();
            SceneCaptureComponent->DestroyComponent();
            SceneCaptureComponent = nullptr;
        }

        SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(World);
        SceneCaptureComponent->TextureTarget = CameraRenderTargetTexture;
        SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
        SceneCaptureComponent->bCaptureEveryFrame = false; // 수동 캡처 설정
        SceneCaptureComponent->bCaptureOnMovement = false;
        SceneCaptureComponent->AddToRoot();
    }

    // SceneCaptureComponent 위치 및 회전 업데이트
    SceneCaptureComponent->SetWorldLocation(CameraLocation);
    SceneCaptureComponent->SetWorldRotation(CameraRotation);
    SceneCaptureComponent->HiddenActors = ActorsToHide;

    // 브러시 초기화 확인
    if (!CameraRenderTargetBrush.IsValid() && CameraRenderTargetTexture)
    {
        CameraRenderTargetBrush = MakeShareable(new FSlateBrush());
        CameraRenderTargetBrush->SetResourceObject(CameraRenderTargetTexture);
        CameraRenderTargetBrush->ImageSize = FVector2D(ImageWidth, ImageHeight);
    }

    SceneCaptureComponent->CaptureScene();
}

void SShowCamSequenceDetailsWidget::OnShowCameraCheckChanged(ECheckBoxState NewState)
{
    bShowCamera = (NewState == ECheckBoxState::Checked);
    // UI 업데이트나 추가 동작이 필요한 경우 추가 처리 가능
}

const FSlateBrush* SShowCamSequenceDetailsWidget::GetCameraRenderTargetBrush() const
{
    return CameraRenderTargetBrush.Get();
}

TSharedRef<SWidget> SShowCamSequenceDetailsWidget::CamSequenceWidget()
{
    return SNew(SVerticalResizableSplitter)
        .Widgets(
            {
                DetailScroll(),
                RenderView()
            }
        )
        .InitialRatios(
            { 0.9f, 0.1f }
        );

    //return SNew(SVerticalBox)

    //    // 스크롤 가능한 부분
    //    + SVerticalBox::Slot()
    //    .FillHeight(1.0f)
    //    [
    //        DetailScroll()
    //    ]

    //    // 스크롤되지 않는 고정 부분
    //    + SVerticalBox::Slot()
    //    .AutoHeight()
    //    [
    //        RenderView()
    //    ];
}

TSharedRef<SWidget> SShowCamSequenceDetailsWidget::DetailScroll()
{
    return SNew(SScrollBox)
        + SScrollBox::Slot()
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SCheckBox)
                        .OnCheckStateChanged(this, &SShowCamSequenceDetailsWidget::OnShowCameraCheckChanged)
                        .IsChecked(bShowCamera ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                        [
                            SNew(STextBlock).Text(FText::FromString("Show Camera"))
                        ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    ShowKeyStructureDetailsView.IsValid()
                        ? ShowKeyStructureDetailsView->GetWidget().ToSharedRef()
                        : SNullWidget::NullWidget
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SSpacer).Size(FVector2D(0, 8))
                ]
        ];
}

TSharedRef<SWidget> SShowCamSequenceDetailsWidget::RenderView()
{
    return SNew(SAutoAspectRatioWidthWidget)
        .WidthRatio(ImageHeight / ImageWidth)
        [
            SNew(SBorder)
                .Padding(5.0f)
                .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                .BorderBackgroundColor(FLinearColor::Gray)
                [
                    SNew(SImage)
                        .Image(this, &SShowCamSequenceDetailsWidget::GetCameraRenderTargetBrush)
                ]
        ];
}

#undef LOCTEXT_NAMESPACE
