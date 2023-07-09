#include "CoCoDataStruct/CoCoImageInfo.h"
#include "Chaos/Matrix.h"
#include "GenericPlatform/GenericPlatformMath.h"


UCoCoImageInfo::UCoCoImageInfo(int id, int width, int height, FString file_name, FVector cameraTranslation, FRotator cameraRotator, FMinimalViewInfo cameraInfo)
{   
    data.id = id;
    data.width = width;
    data.height = height;
    data.file_name = file_name;
    data.fov = cameraInfo.FOV;
    
    data.offset.Add("x", cameraInfo.OffCenterProjectionOffset[0]);
    data.offset.Add("y", cameraInfo.OffCenterProjectionOffset[1]);

    data.camera_translation = cameraTranslation;
    data.camera_rotation = cameraRotator.Quaternion();
}

UCoCoImageInfo::~UCoCoImageInfo()
{
}

FCoCoImageInfo UCoCoImageInfo::GetStructData()
{
    return data;
}