// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencer.h"
//#include "RunTime/ShowKey.h"

void UShowSequencer::InitShowKeys()
{
    for (const FShowKey Key : ShowKeys)
    {
        // FShowKey Ÿ�Կ� ���� UShowKey �ν��Ͻ� ����
        // RuntimeShowKeys�� �߰�
    }
}

void UShowSequencer::Play()
{
    // RuntimeShowKeys ������ ����Ͽ� �������� ���
}

void UShowSequencer::Stop()
{
    // ������ ����
}

void UShowSequencer::Pause()
{
    // ������ �Ͻ� ����
}

void UShowSequencer::UnPause()
{
    // ������ �ٽ� ���
}

void UShowSequencer::ChangeSpeed(float Speed)
{
    // ��� Ű�� ���� �ӵ� ����
}