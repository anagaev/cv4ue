#pragma once

#include "CoreMinimal.h"

class AsyncSaveJsonToDiskTask : public FNonAbandonableTask{
    public:
        AsyncSaveJsonToDiskTask(TSharedPtr<FJsonObject> jsonObject , FString jsonName);
        ~AsyncSaveJsonToDiskTask();

    // Required by UE4!
    FORCEINLINE TStatId GetStatId() const{
        RETURN_QUICK_DECLARE_CYCLE_STAT(AsyncSaveImageToDiskTask, STATGROUP_ThreadPoolAsyncTasks);
    }

    protected:
        FString serialisedJson;
        FString jsonName = "";

    public:
        void DoWork();
};