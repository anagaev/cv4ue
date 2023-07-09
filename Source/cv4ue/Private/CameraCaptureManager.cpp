#include "CameraCaptureManager.h"
#include "CoCoDataStruct/CoCoAnnotationInfo.h"
#include "CoCoDataStruct/CoCoFrameInfo.h"

//#include "Engine.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "AsyncTasks/AsyncSaveImageToDiskTask.h"
#include "AsyncTasks/AsyncSaveJsonToDiskTask.h"
#include "ShowFlags.h"
#include "Materials/Material.h"
#include "RHICommandList.h"
#include "ImageWrapper/Public/IImageWrapper.h"
#include "ImageWrapper/Public/IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"
#include "Camera/CameraTypes.h"
#include "Chaos/Matrix.h"
#include "Chaos/Real.h"


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
    

    // READ UINT8 IMAGE
	// Read pixels once RenderFence is completed
    if(!RenderRequestQueue.IsEmpty()){
        // Peek the next RenderRequest from queue
        FRenderRequestStruct* nextRenderRequest = nullptr;
        RenderRequestQueue.Peek(nextRenderRequest);

        if(nextRenderRequest){ //nullptr check
            if(nextRenderRequest->RenderFence.IsFenceComplete()){ // Check if rendering is done, indicated by RenderFence
                // Load the image wrapper module 
                IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

                // Decide storing of data, either jpeg or png
                FString fileName = "img_" + ToStringWithLeadingZeros(ImgCounter, NumDigits);
                FString filePath = FPaths::ProjectSavedDir() + SubDirectoryName + "/" + fileName;
                TArray64<uint8> ImgData;
                switch (outputFormat){
                case IMG:
                    filePath += ".jpeg";
                    ImageWrapperModule.CompressImage(ImgData, EImageFormat::JPEG, FImageView(nextRenderRequest->Image.GetData(), FrameWidth, FrameHeight));
                    RunAsyncImageSaveTask(ImgData, filePath);
                    break;
                case JSON:
                    filePath += ".json";
                    TArray<FColor>& rawData = nextRenderRequest->Image;

                    FTransform parentActorTransform = RootComponent->GetAttachParent()->GetOwner()->GetTransform();
                    FRotator cameraRotator = parentActorTransform.Rotator();
                    FVector cameraTranslation = parentActorTransform.GetTranslation();
                    FMinimalViewInfo cameraInfo;
                    CaptureComponent->GetCameraView(0.0, cameraInfo);

                    UCoCoImageInfo imageInfo(ImgCounter, FrameWidth, FrameHeight, fileName + ".jpeg", cameraTranslation, cameraRotator, cameraInfo);
                    UCoCoFrameInfo frameInfo(imageInfo);

                    int currentColor = rawData[0].R;
                    int initPos = 0;
                    int length = 1;
                    for (int i=1; i < rawData.Num(); ++i){
                        if (currentColor != rawData[i].R){
                            UCoCoAnnotationInfo* ptrObAnnotation = frameInfo.Find(currentColor);
                            if (ptrObAnnotation != nullptr){
                                ptrObAnnotation->Update(initPos, length);
                            } else {
                                UCoCoAnnotationInfo newAnnotation(currentColor, ImgCounter, FrameWidth, FrameHeight);
                                newAnnotation.Update(initPos, length);
                                frameInfo.AddAnnotation(currentColor, newAnnotation);
                            }
                            initPos = i;
                            length = 1;
                            currentColor = rawData[i].R;
                        } else { ++length;}
                    }
                    RunAsyncJsonSaveTask(frameInfo.GetStructData(), filePath);
                    break;
                }

                    
                if(VerboseLogging && !fileName.IsEmpty()){
                    UE_LOG(LogTemp, Warning, TEXT("%f"), *fileName);
                }
                
                ImgCounter += 1;

                // Delete the first element from RenderQueue
                RenderRequestQueue.Pop();
                delete nextRenderRequest;
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

FMatrix ACameraCaptureManager::GetExtrinsicsMatrix(){
    FTransform parentActorTransform = RootComponent->GetAttachParent()->GetOwner()->GetTransform();
    FRotator parentActorRotator = parentActorTransform.Rotator();
    FVector parentActorTranslation = parentActorTransform.GetTranslation();
    FVector translation = -parentActorRotator.UnrotateVector(parentActorTranslation);
    FMatrix m = FRotationTranslationMatrix(parentActorRotator, parentActorTranslation).GetTransposed();
    return m;
}

Chaos::PMatrix<float, 3, 3> ACameraCaptureManager::GetIntrinsicMatrix(){
    FMinimalViewInfo cameraInfo;
    CaptureComponent->GetCameraView(0.0, cameraInfo);
    float f = 0.5 * FrameWidth / FMath::Tan(0.5 * cameraInfo.FOV);
    FVector2D offset = cameraInfo.OffCenterProjectionOffset;
    Chaos::PMatrix<float, 3, 3> intrinsicMatrix(f, 0.0, offset[0], 0.0, f, offset[1], 0.0, 0.0, 1.0);
    return intrinsicMatrix;
}