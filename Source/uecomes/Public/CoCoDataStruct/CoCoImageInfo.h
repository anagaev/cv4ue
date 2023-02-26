// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoCoImageInfo.generated.h"


USTRUCT(BlueprintType)
struct FCoCoImageInfo{
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
};


class UECOMES_API UCoCoImageInfo
{
public:
	FCoCoImageInfo data;
	UCoCoImageInfo(int id, int width, int height, FString file_name);
	~UCoCoImageInfo();
	FCoCoImageInfo GetStructData();
};