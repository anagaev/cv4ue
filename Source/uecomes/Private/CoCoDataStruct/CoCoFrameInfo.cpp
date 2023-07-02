// Fill out your copyright notice in the Description page of Project Settings.


#include "CoCoDataStruct/CoCoFrameInfo.h"

UCoCoFrameInfo::UCoCoFrameInfo(UCoCoImageInfo& imageInfo): m_imageInfo(imageInfo){   
    m_imageInfo = imageInfo;
}

UCoCoFrameInfo::~UCoCoFrameInfo(){
}

void UCoCoFrameInfo::AddAnnotation(int key, UCoCoAnnotationInfo& annotationInfo){
   m_annotations.Add(key, annotationInfo);
}

FCoCoFrameInfo UCoCoFrameInfo::GetStructData(){
    FCoCoFrameInfo frameInfo;
    frameInfo.images.Add(m_imageInfo.GetStructData());
    // frameInfo.annotations.SetNum(m_annotations.Num());
    for(auto& object: m_annotations){
        if (object.Key != 0 && object.Value.CalculateAreaPercentage() > 0.0005 && object.Value.IsBboxCorrect()){
            frameInfo.annotations.Add(object.Value.GetStructData());
        }
    }
    return frameInfo;
}

UCoCoAnnotationInfo* UCoCoFrameInfo::Find(int key){
    return m_annotations.Find(key);
}
