#pragma once

#include "CoreMinimal.h"

class AsyncSaveImageToDiskTask : public FNonAbandonableTask{
    public:
        AsyncSaveImageToDiskTask(TArray64<uint8> Image, FString ImageName);
        ~AsyncSaveImageToDiskTask();

    // Required by UE4!
    FORCEINLINE TStatId GetStatId() const{
        RETURN_QUICK_DECLARE_CYCLE_STAT(AsyncSaveImageToDiskTask, STATGROUP_ThreadPoolAsyncTasks);
    }

    protected:
        TArray64<uint8> ImageCopy;
        FString FileName = "";

    public:
        void DoWork();
};