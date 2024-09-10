// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowPlayer.h"
#include "RunTime/ShowSequencer.h"

void UShowPlayer::PlaySoloShow(AActor* Owner, UShowSequencer* ShowSequencer)
{
    if (ShowSequencer && Owner)
    {
        ActiveShows.Add(Owner, ShowSequencer);
        ShowSequencer->Play(); // �� ����
    }
}

void UShowPlayer::StopSoloShow(AActor* Owner, UShowSequencer* ShowSequencer)
{
    if (ShowSequencer && Owner && ActiveShows.Contains(Owner))
    {
        ShowSequencer->Stop(); // �� ����
        ActiveShows.Remove(Owner);
    }
}