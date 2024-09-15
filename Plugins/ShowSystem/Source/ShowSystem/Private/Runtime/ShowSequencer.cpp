// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencer.h"
//#include "RunTime/ShowKey.h"

void UShowSequencer::InitShowKeys()
{
    //for (const FShowKey Key : ShowKeys)
    //{
    //    // FShowKey 타입에 따라 UShowKey 인스턴스 생성
    //    // RuntimeShowKeys에 추가
    //}
}

void UShowSequencer::Play()
{
    // RuntimeShowKeys 로직을 사용하여 시퀀스를 재생
}

void UShowSequencer::Stop()
{
    // 시퀀스 중지
}

void UShowSequencer::Pause()
{
    // 시퀀스 일시 중지
}

void UShowSequencer::UnPause()
{
    // 시퀀스 다시 재생
}

void UShowSequencer::ChangeSpeed(float Speed)
{
    // 모든 키에 대해 속도 변경
}