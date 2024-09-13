// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 파일 경로와 어셋 경로 간의 변환을 돕는 유틸리티 함수 모음.
 */
class COREUTILS_API PathsUtil
{
public:

	/**
	 * 파일 경로에서 모듈명을 추출.
	 * @param FilePath 모듈명을 추출할 파일 경로.
	 * @return 추출된 모듈명.
	 */
	static FString ExtractModuleNameFromPath(const FString& FilePath);

	/**
	 * 주어진 파일 경로를 기준으로 언리얼 엔진상에서 관리하는 어셋 경로로 변환.
	 * @param FilePath 파일 경로 (예: "C:/Unreal Project/MyProject/Content/MyFolder/MyAsset").
	 * @return 엔진에서 관리하는 어셋 경로 (예: "/Game/MyFolder/MyAsset").
	 */
	static FString GetGameAssetPathFromFilePath(const FString& FilePath);

	/**
	 * 언리얼 엔진상에서 관리하는 어셋 경로를 기준으로 파일 경로로 변환.
	 * @param AssetPath 어셋 경로 (예: "/Game/MyFolder/MyAsset").
	 * @return 파일 경로 (예: "C:/Unreal Project/MyProject/Content/MyFolder/MyAsset").
	 */
	static FString GetFilePathFromGameAssetPath(const FString& AssetPath);
};
