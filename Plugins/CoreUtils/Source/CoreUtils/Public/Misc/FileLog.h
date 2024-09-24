// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "FileLog.generated.h"

#define UE_FILE_LOG(ClassType, Verbosity, Format, ...) \
{ \
    if (ULogSubsystem* LogSubsystem = GEngine->GetEngineSubsystem<ULogSubsystem>()) \
    { \
        FString TempStr = FString::Printf(Format, ##__VA_ARGS__); \
        LogSubsystem->LogToFile(ClassType, *TempStr); \
    } \
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 
// 아직 테스트 하지 않음 나중에 필요할거 같아서 미리 만들어 둠
// 사용 전에 필히 시스템 검증해야함!!!
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/**
 * UFileLog 클래스는 엔진 서브시스템(UEngineSubsystem)을 상속받아,
 * Unreal Engine의 각 클래스 타입에 따라 로그를 파일로 기록하는 기능을 제공.
 *
 * 엔진 시작 시 초기화되며,
 * 각 클래스 타입마다 고유한 로그 파일을 생성하며, 
 * 엔진 시작 시간을 파일명에 포함시켜 동일 클래스의 여러 세션 로그를 구분할 수 있도록 동작.
 *
 * 주요 기능:
 * - 클래스 타입(UClass*)에 따라 개별 로그 파일을 생성하고, 해당 파일에 로그 메시지를 기록.
 * - 로그 파일은 프로젝트의 'log_file' 폴더에 저장되며, 파일명에는 클래스명과 엔진 시작 시간이 포함.
 *
 * 사용 예시:
 * - ADSLog 클래스를 만들어서 광고에 대한 로그를 기록할 때, 'log_file/ADSLog_20240101_120000.log' 형태로 파일이 생성됩니다.
 * - 각 클래스에 대한 로그 파일이 개별적으로 관리되며, 동일 클래스라도 매번 다른 세션의 로그 파일이 생성됩니다.
 * 
 * 중요 사항:
 * - Class 를 기준으로 남기는 이유는 오타 등으로 인해 로그가 잘못 남는 것을 방지하기 위해 지정한 Class로 남김
 * - 따라서 로그 그룹을 Class 단위로 관리한다고 보면 되며 그룹을 만들고 싶은 로그는 Class를 만들어서 사용
 * - 물론 내용도 없을 Class 를 만드는 것이 번거롭기도 하고 불필요하다고 생각할 수 있지만, 이는 로그 코드 실수 등을 방지하기 위함임
 * - 또한, 추후 로그를 확장할 가능성을 염두해 두고 Class 단위로 관리함 
 *      => Class 단위로 로그를 남길지 말지 데이터화 해서 관리가 가능함
 * - 특히 파일로 남기는 기능은 외부 라이브러리 (광고 시청, 스토어 빌드 배포로 가능한 테스트 등) 에서 사용할 때 유용함
 */
UCLASS()
class COREUTILS_API UFileLog : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override 
    { 
        // 여기서 return false; 하면 subsystem 자체를 생성하지 않고 로그도 안남음
        return true; 
    }

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 클래스 타입에 따라 로그 파일 경로 반환
    FString GetLogFilePath(UClass* ClassType) const;

    // 클래스 타입에 따라 로그 기록
    void LogToFile(UClass* ClassType, const TCHAR* LogMessage);

private:
    // 엔진 시작 시간을 저장하여 파일명에 반영
    FString EngineStartTime;

    // 로그 파일을 저장할 폴더 경로
    FString LogFolder;
};
