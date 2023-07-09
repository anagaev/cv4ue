// Fill out your copyright notice in the Description page of Project Settings.


#include "CoCoDataStruct/CoCoAnnotationInfo.h"
#include "GenericPlatform/GenericPlatformMath.h"


UCoCoAnnotationInfo::UCoCoAnnotationInfo(int id, int imageId, int imgWidth, int imgHeight)
{   
    data.category_id = 1;
    data.iscrowd = 1;
    data.bbox.SetNum(4);
    data.bbox[0] = INT_MAX;
    data.bbox[1] = INT_MAX;
    data.id = id;
    data.image_id = imageId;

    width = imgWidth;
    height = imgHeight;
}

UCoCoAnnotationInfo::~UCoCoAnnotationInfo()
{
}

void UCoCoAnnotationInfo::Update(int initPos, int length){
    data.segmentation.Add(initPos);
    data.segmentation.Add(length);
    int x1 = initPos % width;
    int y1 = initPos / width;
    int x2 = (initPos + length) % width;
    int y2 = (initPos + length) / width;
    
    data.bbox[0] = FMath::Min(data.bbox[0], x1);
    data.bbox[1] = FMath::Min(data.bbox[1], y1);
    data.bbox[2] = FMath::Max(data.bbox[2], x2);
    data.bbox[3] = FMath::Max(data.bbox[3], y2);

    data.area += length;
}

FCoCoAnnotationInfo UCoCoAnnotationInfo::GetStructData(){
    return data;
}

float UCoCoAnnotationInfo::CalculateAreaPercentage(){
    return data.area / (width * height);
}

bool UCoCoAnnotationInfo::IsBboxCorrect(){
    return (data.bbox[0] < data.bbox[2]) && (data.bbox[1] < data.bbox[3]);
}