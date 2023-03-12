// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoCoAnnotationInfo.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FCoCoAnnotationInfo{
    GENERATED_BODY()

	UPROPERTY()
	int id;
	UPROPERTY()
	int image_id;
	UPROPERTY()
	int category_id;
	UPROPERTY()
	float area;
	UPROPERTY()
	int iscrowd;
	UPROPERTY()
    TArray<int> segmentation;
	UPROPERTY()
	TArray<int> bbox;
};



class UECOMES_API UCoCoAnnotationInfo
{
private:
	int width;
	int height;
public:
	FCoCoAnnotationInfo data;
	UCoCoAnnotationInfo(int id, int imageId, int imgWidth, int imgHeight);
	~UCoCoAnnotationInfo();
	FCoCoAnnotationInfo GetStructData();
	void Update(int initPos, int length);
	float CalculateAreaPercentage();
	bool IsBboxCorrect();
};
