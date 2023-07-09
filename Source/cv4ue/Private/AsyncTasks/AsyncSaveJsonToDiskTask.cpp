#include "AsyncTasks/AsyncSaveJsonToDiskTask.h"


AsyncSaveJsonToDiskTask::AsyncSaveJsonToDiskTask(TSharedPtr<FJsonObject> pJsonObject , FString name){
    jsonName = name;
    FJsonSerializer::Serialize(pJsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&serialisedJson, 0));
}

AsyncSaveJsonToDiskTask::~AsyncSaveJsonToDiskTask(){
    //UE_LOG(LogTemp, Warning, TEXT("AsyncTaskDone"));
}

void AsyncSaveJsonToDiskTask::DoWork(){
    UE_LOG(LogTemp, Warning, TEXT("Starting Work"));
    FFileHelper::SaveStringToFile(*serialisedJson, *jsonName);
    UE_LOG(LogTemp, Log, TEXT("Stored Json: %s"), *jsonName);
}