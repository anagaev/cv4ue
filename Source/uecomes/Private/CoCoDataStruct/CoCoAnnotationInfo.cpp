// Fill out your copyright notice in the Description page of Project Settings.


#include "CoCoDataStruct/CoCoAnnotationInfo.h"
#include "GenericPlatform/GenericPlatformMath.h"


UCoCoAnnotationInfo::UCoCoAnnotationInfo(int id, int imageId)
{   
    data.category_id = 1;
    data.iscrowd = 0;
    data.bbox.SetNum(4);
    data.id = id;
    data.image_id = imageId;
}

UCoCoAnnotationInfo::~UCoCoAnnotationInfo()
{
}

void UCoCoAnnotationInfo::Update(int initPos, int length, int imgWidth, int imgHeight){
    data.segmentation.Add(initPos);
    data.segmentation.Add(length);
    int x1 = initPos % imgWidth;
    int y1 = initPos / imgHeight;
    int x2 = (initPos + length) % imgWidth;
    int y2 = (initPos + length) / imgHeight;
    
    data.bbox[0] = FMath::Min(data.bbox[0], x1);
    data.bbox[1] = FMath::Min(data.bbox[1], y1);
    data.bbox[2] = FMath::Max(data.bbox[2], x2);
    data.bbox[3] = FMath::Max(data.bbox[3], y2);

    data.area = (data.bbox[2] - data.bbox[0]) * (data.bbox[3] - data.bbox[1]);
}

FCoCoAnnotationInfo UCoCoAnnotationInfo::GetStructData(){
    return data;
}