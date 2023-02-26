#include "CoCoDataStruct/CoCoImageInfo.h"
#include "GenericPlatform/GenericPlatformMath.h"


UCoCoImageInfo::UCoCoImageInfo(int id, int width, int height, FString file_name)
{   
    data.id = id;
    data.width = width;
    data.height = height;
    data.file_name = file_name;
}

UCoCoImageInfo::~UCoCoImageInfo()
{
}

FCoCoImageInfo UCoCoImageInfo::GetStructData(){
    return data;
}