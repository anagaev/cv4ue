// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/Matrix.h"
#include <vector>
#include "CoCoImageInfo.generated.h"


USTRUCT(BlueprintType)
struct FCoCoImageInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int id;
	UPROPERTY()
	int width;
	UPROPERTY()
	int height;
	UPROPERTY()
	FString file_name;
	UPROPERTY()
	int license;
	UPROPERTY()
	float fov;
	UPROPERTY()
	TMap<FString, float> offset;
	UPROPERTY()
	FVector camera_translation;
	UPROPERTY()
	FQuat camera_rotation;
};

class CV4UE_API UCoCoImageInfo
{
public:
	FCoCoImageInfo data;
	UCoCoImageInfo(int id, int width, int height, FString file_name, FVector cameraTranslation, FRotator cameraRotator, FMinimalViewInfo cameraInfo);
	~UCoCoImageInfo();
	FCoCoImageInfo GetStructData();
};