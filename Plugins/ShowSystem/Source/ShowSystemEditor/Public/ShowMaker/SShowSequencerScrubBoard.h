// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnValueChanged, float);

/**
 * ShowSequencer 연출의 플레이되고 있는 위치를 바의 위치로 표현해주는 위젯
 */
class SHOWSYSTEMEDITOR_API SShowSequencerScrubBoard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerScrubBoard) {}
		/** 전체 시간 눈금들을 표시할 기본 높이 */
		SLATE_ATTRIBUTE(float, Height)
		/** 초당 프레임수 (0이면 프레임이 아닌 시간을 기준으로 잡는다) */
		SLATE_ATTRIBUTE(float, FPS)
		/** 연출의 전체 길이 (FPS 값이 0 이면 전체 시간(초), FPS 값이 0보다 크면 전체 프레임 수) */
		SLATE_ATTRIBUTE(float, TotalValue)
		/** 현제 위치 (FPS 값이 0이면 현재 시간(초), FPS 값이 0보다 크면 현재 프레임) */
		SLATE_ATTRIBUTE(float, CrrValue)
		/** 현재 값의 마크를 움직여서 바뀌는 CrrValue 값에 대한 이벤트 */
		SLATE_EVENT(FOnValueChanged, OnValueChanged)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	TAttribute<float> Height;
	TAttribute<float> FPS;
	TAttribute<float> TotalValue;
	TAttribute<float> CrrValue;
	FOnValueChanged OnValueChanged;

	bool bIsDragging = false;
	mutable FSlateRect MarkerBounds = FSlateRect(0, 0, 0, 0);
};
