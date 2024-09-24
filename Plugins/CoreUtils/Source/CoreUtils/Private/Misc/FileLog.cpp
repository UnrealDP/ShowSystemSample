// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/FileLog.h"

void UFileLog::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 엔진 시작 시 시간을 기록 (YYYYMMDD_HHMMSS 형식)
    EngineStartTime = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

    // 로그 파일을 저장할 폴더 경로 설정 (프로젝트 경로/log_file/)
    LogFolder = FString::Printf(TEXT("%s%s"), *FPaths::ProjectLogDir(), TEXT("log_file."));
}

FString UFileLog::GetLogFilePath(UClass* ClassType) const
{
    if (ClassType == nullptr)
    {
        return FString::Printf(TEXT("%s%s%s%s"), *LogFolder, TEXT("DefaultLog_"), *EngineStartTime, TEXT(".log"));
    }

    // 클래스명 + 시작 시간으로 파일명 생성
    return FString::Printf(TEXT("%s%s%s%s%s"), *LogFolder, *ClassType->GetName(), TEXT("_"), *EngineStartTime, TEXT(".log"));
}

void UFileLog::LogToFile(UClass* ClassType, const TCHAR* LogMessage)
{
    // 폴더가 존재하지 않으면 생성
    IFileManager::Get().MakeDirectory(*LogFolder, true);

    FString LogFilePath = GetLogFilePath(ClassType);

    // 타임스탬프 추가
    FString Timestamp = FDateTime::Now().ToString();
    FString FormattedMessage = FString::Printf(TEXT("%s: %s%s"), *Timestamp, LogMessage, LINE_TERMINATOR);

    // 파일에 로그 메시지 추가
    FFileHelper::SaveStringToFile(FormattedMessage, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}