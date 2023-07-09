#include "AsyncTasks/AsyncSaveImageToDiskTask.h"


AsyncSaveImageToDiskTask::AsyncSaveImageToDiskTask(TArray64<uint8> Image, FString ImageName){
    ImageCopy = Image;
    FileName = ImageName;
}

AsyncSaveImageToDiskTask::~AsyncSaveImageToDiskTask(){
    //UE_LOG(LogTemp, Warning, TEXT("AsyncTaskDone"));
}

void AsyncSaveImageToDiskTask::DoWork(){
    UE_LOG(LogTemp, Warning, TEXT("Starting Work"));
    FFileHelper::SaveArrayToFile(ImageCopy, *FileName);
    UE_LOG(LogTemp, Log, TEXT("Stored Image: %s"), *FileName);
}
