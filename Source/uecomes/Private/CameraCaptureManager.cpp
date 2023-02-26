// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraCaptureManager.h"

//#include "Engine.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "ShowFlags.h"
#include "AsyncTasks/AsyncSaveImageToDiskTask.h"
#include "AsyncTasks/AsyncSaveJsonToDiskTask.h"
#include "Materials/Material.h"

#include "RHICommandList.h"

#include "ImageWrapper/Public/IImageWrapper.h"
#include "ImageWrapper/Public/IImageWrapperModule.h"

#include "ImageUtils.h"

#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"



// Sets default values
ACameraCaptureManager::ACameraCaptureManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
    CaptureComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ACameraCaptureManager::BeginPlay()
{
	Super::BeginPlay();

	if(CaptureComponent){ // nullptr check
		SetupCaptureComponent();
	} else{
		UE_LOG(LogTemp, Error, TEXT("No CaptureComponent set!"));
	}
	
}

// Called every frame
void ACameraCaptureManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    CaptureNonBlocking();

    if(!RenderRequestQueue.IsEmpty()){
        // Peek the next RenderRequest from queue
        FRenderRequestStruct* nextRenderRequest = nullptr;
        RenderRequestQueue.Peek(nextRenderRequest);

        if(nextRenderRequest){ //nullptr check
            if(nextRenderRequest->RenderFence.IsFenceComplete()){ // Check if rendering is done, indicated by RenderFence
                FString fileName = "img_" + ToStringWithLeadingZeros(ImgCounter, NumDigits);
                FString filePath = FPaths::ProjectSavedDir() + SubDirectoryName + "/" + fileName;
                if (outputFormat == IMG) {
                    // Load the image wrapper module 
                    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

                    // Decide storing of data, either jpeg or png
                    TArray64<uint8> ImgData;

                    filePath += ".png";
                    ImageWrapperModule.CompressImage(ImgData, EImageFormat::JPEG, FImageView(nextRenderRequest->Image.GetData(), FrameWidth, FrameHeight), 100);
                    RunAsyncImageSaveTask(ImgData, filePath);

                    if(VerboseLogging && !fileName.IsEmpty()){
                        UE_LOG(LogTemp, Warning, TEXT("%f"), *fileName);
                    }

                    // Delete the first element from RenderQueue
                    RenderRequestQueue.Pop();
                    delete nextRenderRequest;
                } else {
                    filePath += ".json";
                    TArray<FColor>& rawData = nextRenderRequest->Image;
                    UCoCoImageInfo imageInfo(ImgCounter, FrameWidth, FrameHeight, fileName + ".jpeg");
                    UCoCoFrameInfo frameInfo(imageInfo);

                    int currentColor = rawData[0].R;
                    int initPos = 0;
                    int length = 1;
                    for (int i=1; i < rawData.Num(); ++i){
                        if (currentColor != rawData[i].R){
                            UCoCoAnnotationInfo* ptrObAnnotation = frameInfo.Find(currentColor);
                            if (ptrObAnnotation != nullptr){
                                ptrObAnnotation->Update(initPos, length, FrameWidth, FrameHeight);
                            } else {
                                UCoCoAnnotationInfo newAnnotation(currentColor, ImgCounter);
                                newAnnotation.Update(initPos, length, FrameWidth, FrameHeight);
                                frameInfo.AddAnnotation(currentColor, newAnnotation);
                            }
                            initPos = i;
                            length = 1;
                            currentColor = rawData[i].R;
                        } else { ++length;}
                    }
                    RunAsyncJsonSaveTask(frameInfo.GetStructData(), filePath);
                }
                ImgCounter += 1;
            }
        }
    }
}

void ACameraCaptureManager::SetupCaptureComponent(){
    if(!IsValid(CaptureComponent)){
        UE_LOG(LogTemp, Error, TEXT("SetupCaptureComponent: CaptureComponent is not valid!"));
        return;
    }

    // Create RenderTargets
    UTextureRenderTarget2D* renderTarget2D = NewObject<UTextureRenderTarget2D>();

    renderTarget2D->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8; //8-bit color format
    renderTarget2D->InitCustomFormat(FrameWidth, FrameHeight, PF_B8G8R8A8, true); // PF... disables HDR, which is most important since HDR gives gigantic overhead, and is not needed!
    UE_LOG(LogTemp, Warning, TEXT("Set Render Format for Color-Like-Captures"));
    
    renderTarget2D->bGPUSharedFlag = true; // demand buffer on GPU

    // Assign RenderTarget
    CaptureComponent->TextureTarget = renderTarget2D;
    // Set Camera Properties
    CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    CaptureComponent->TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
    CaptureComponent->ShowFlags.SetTemporalAA(true);
    // lookup more showflags in the UE4 documentation..

    // Assign PostProcess Material if assigned
    if(PostProcessMaterial){ // check nullptr
        CaptureComponent->AddOrUpdateBlendable(PostProcessMaterial);
    } else {
        UE_LOG(LogTemp, Log, TEXT("No PostProcessMaterial is assigend"));
    }
    UE_LOG(LogTemp, Warning, TEXT("Initialized RenderTarget!"));
}

void ACameraCaptureManager::CaptureNonBlocking(){
    if(!IsValid(CaptureComponent)){
        UE_LOG(LogTemp, Error, TEXT("CaptureColorNonBlocking: CaptureComponent was not valid!"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Entering: CaptureNonBlocking"));
    CaptureComponent->TextureTarget->TargetGamma = 1.2f;//GEngine->GetDisplayGamma();

    // Get RenderConterxt
    FTextureRenderTargetResource* renderTargetResource = CaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();
    UE_LOG(LogTemp, Warning, TEXT("Got display gamma"));
    struct FReadSurfaceContext{
        FRenderTarget* SrcRenderTarget;
        TArray<FColor>* OutData;
        FIntRect Rect;
        FReadSurfaceDataFlags Flags;
    };
    UE_LOG(LogTemp, Warning, TEXT("Inited ReadSurfaceContext"));
    // Init new RenderRequest
    FRenderRequestStruct* renderRequest = new FRenderRequestStruct();
    UE_LOG(LogTemp, Warning, TEXT("inited renderrequest"));

    // Setup GPU command
    FReadSurfaceContext readSurfaceContext = {
        renderTargetResource,
        &(renderRequest->Image),
        FIntRect(0,0,renderTargetResource->GetSizeXY().X, renderTargetResource->GetSizeXY().Y),
        FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)
    };
    UE_LOG(LogTemp, Warning, TEXT("GPU Command complete"));

    // Above 4.22 use this
    ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
    [readSurfaceContext](FRHICommandListImmediate& RHICmdList){
        RHICmdList.ReadSurfaceData(
            readSurfaceContext.SrcRenderTarget->GetRenderTargetTexture(),
            readSurfaceContext.Rect,
            *readSurfaceContext.OutData,
            readSurfaceContext.Flags
        );
    });

    // Notifiy new task in RenderQueue
    RenderRequestQueue.Enqueue(renderRequest);

    // Set RenderCommandFence
    renderRequest->RenderFence.BeginFence();
}

FString ACameraCaptureManager::ToStringWithLeadingZeros(int32 Integer, int32 MaxDigits){
    FString result = FString::FromInt(Integer);
    int32 stringSize = result.Len();
    int32 stringDelta = MaxDigits - stringSize;
    if(stringDelta < 0){
        UE_LOG(LogTemp, Error, TEXT("MaxDigits of ImageCounter Overflow!"));
        return result;
    }
    //FIXME: Smarter function for this..
    FString leadingZeros = "";
    for(size_t i=0;i<stringDelta;i++){
        leadingZeros += "0";
    }
    result = leadingZeros + result;

    return result;
}

void ACameraCaptureManager::RunAsyncImageSaveTask(TArray64<uint8>& Image, FString& ImageName){
    UE_LOG(LogTemp, Warning, TEXT("Running Async Task"));
    (new FAutoDeleteAsyncTask<AsyncSaveImageToDiskTask>(Image, ImageName))->StartBackgroundTask();
}

void ACameraCaptureManager::RunAsyncJsonSaveTask(FCoCoFrameInfo data, FString& ImageName){
    UE_LOG(LogTemp, Warning, TEXT("Running Async Task"));
    TSharedPtr<FJsonObject> pJsonObject = FJsonObjectConverter::UStructToJsonObject(data);
    (new FAutoDeleteAsyncTask<AsyncSaveJsonToDiskTask>(pJsonObject, ImageName))->StartBackgroundTask();
}