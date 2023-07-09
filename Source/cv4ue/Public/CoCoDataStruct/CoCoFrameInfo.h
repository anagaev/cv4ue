#pragma once

#include "CoreMinimal.h"
#include "CoCoAnnotationInfo.h"
#include "CoCoImageInfo.h"
#include "CoCoFrameInfo.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FCoCoFrameInfo{
    GENERATED_BODY()

	UPROPERTY()
	TArray<FCoCoImageInfo> images;
	UPROPERTY()
	TArray<FCoCoAnnotationInfo> annotations;
};



class CV4UE_API UCoCoFrameInfo
{
private:
    TMap<int, UCoCoAnnotationInfo> m_annotations;
    UCoCoImageInfo& m_imageInfo;
public:
	UCoCoFrameInfo(UCoCoImageInfo& imageInfo);
	~UCoCoFrameInfo();
	FCoCoFrameInfo GetStructData();
    void AddAnnotation(int key, UCoCoAnnotationInfo& annotationInfo);
    UCoCoAnnotationInfo* Find(int key);
};