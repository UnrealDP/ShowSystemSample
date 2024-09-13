// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/PathsUtil.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"

/// <summary>
/// 파일 경로에서 모듈명을 추출하는 함수.
/// 주어진 파일 경로에서 "Source" 디렉토리 이후의 첫 번째 폴더명을 모듈명으로 간주하고 추출합니다.
/// </summary>
/// <param name="FilePath">모듈명을 추출할 파일 경로.</param>
/// <returns>추출된 모듈명을 반환. 만약 "Source" 경로가 포함되지 않은 잘못된 경로라면 빈 문자열을 반환.</returns>
FString PathsUtil::ExtractModuleNameFromPath(const FString& FilePath)
{
    // 경로에서 "Source" 이후의 경로 추출
    FString PathAfterSource;
    if (FilePath.Split(TEXT("/Source/"), nullptr, &PathAfterSource))
    {
        // "Source/" 이후의 첫 번째 폴더명을 모듈명으로 간주
        FString ModuleName = PathAfterSource.Left(PathAfterSource.Find(TEXT("/")));
        return ModuleName;
    }

    return FString();
}

/// <summary>
/// 주어진 파일 경로를 기준으로 언리얼 엔진상에서 관리하는 어셋 경로로 변환
/// </summary>
/// <param name="FilePath">파일 경로 (예: "C:/Unreal Project/MyProject/Content/MyFolder/MyAsset").</param>
/// <returns>엔진에서 관리하는 어셋 경로 (예: "/Game/MyFolder/MyAsset").</returns>
FString PathsUtil::GetGameAssetPathFromFilePath(const FString& FilePath)
{
    // 파일 경로가 상대 경로일 수 있으므로 절대 경로로 변환
    FString FullFilePath = FPaths::ConvertRelativePathToFull(FilePath);

    // 프로젝트 콘텐츠 디렉터리의 절대 경로
    FString FullContentDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());

    // 파일 경로가 프로젝트 콘텐츠 디렉터리 안에 있는지 확인
    if (FullFilePath.StartsWith(FullContentDir))
    {
        // /Content/ 뒤로 경로 추출
        FString RelativePath = FullFilePath.RightChop(FullContentDir.Len());

        // 슬래시 중복 방지
        if (RelativePath.StartsWith(TEXT("/")))
        {
            RelativePath.RemoveAt(0);
        }

        // /Game 경로로 변환
        return FString::Printf(TEXT("/Game/%s"), *RelativePath);
    }
    else
    {
        // 플러그인의 콘텐츠 디렉터리에서 확인
        const TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetEnabledPlugins();
        for (const TSharedRef<IPlugin>& Plugin : Plugins)
        {
            FString PluginContentDir = FPaths::ConvertRelativePathToFull(Plugin->GetContentDir());

            // 파일 경로가 플러그인 콘텐츠 디렉터리 안에 있는지 확인
            if (FullFilePath.StartsWith(PluginContentDir))
            {
                // /Content/ 뒤로 경로 추출
                FString RelativePath = FullFilePath.RightChop(PluginContentDir.Len());

                // 슬래시 중복 방지
                if (RelativePath.StartsWith(TEXT("/")))
                {
                    RelativePath.RemoveAt(0);
                }

                // 플러그인의 경로는 /PluginName 경로로 변환
                return FString::Printf(TEXT("/%s/%s"), *Plugin->GetName(), *RelativePath);
            }
        }
    }

    return FString();
}

/// <summary>
/// 언리얼 엔진의 어셋 경로를 파일 시스템의 전체 경로로 변환
/// </summary>
/// <param name="AssetPath">어셋 경로 (예: "/Game/MyFolder/MyAsset").</param>
/// <returns>전체 파일 경로 (예: "C:/Unreal Project/MyProject/Content/MyFolder/MyAsset").</returns>
FString PathsUtil::GetFilePathFromGameAssetPath(const FString& AssetPath)
{
    // "/Game"으로 시작하는지 확인
    if (AssetPath.StartsWith(TEXT("/Game")))
    {
        // "/Game"을 "/Content"로 대체
        FString RelativePath = AssetPath.RightChop(5); // "/Game" 이후의 경로 추출

        // 절대 경로로 변환
        return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + RelativePath);
    }
    else
    {
        // 플러그인의 어셋 경로 처리
        const TArray<TSharedRef<IPlugin>> Plugins = IPluginManager::Get().GetEnabledPlugins();
        for (const TSharedRef<IPlugin>& Plugin : Plugins)
        {
            FString PluginAssetPathPrefix = FString::Printf(TEXT("/%s"), *Plugin->GetName());

            // "/PluginName"으로 시작하는지 확인
            if (AssetPath.StartsWith(PluginAssetPathPrefix))
            {
                // "/PluginName" 이후의 경로 추출
                FString RelativePath = AssetPath.RightChop(PluginAssetPathPrefix.Len());

                // 절대 경로로 변환
                return FPaths::ConvertRelativePathToFull(Plugin->GetContentDir() + RelativePath);
            }
        }
    }

    return FString();
}