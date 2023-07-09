#pragma once

class ASceneCapture2D;
class UMaterial;

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Queue.h"
#include "Chaos/Matrix.h"
#include "AsyncTasks/AsyncSaveImageToDiskTask.h"
#include "CoCoDataStruct/CoCoAnnotationInfo.h"
#include "CoCoDataStruct/CoCoFrameInfo.h"
#include "CameraCaptureManager.generated.h"


UENUM(BlueprintType)
enum OutputFormat {
     IMG UMETA(DisplayName = "IMG"),
     JSON UMETA(DisplayName = "JSON"),
};


USTRUCT()
struct FRenderRequestStruct{
    GENERATED_BODY()

    TArray<FColor> Image;
    FRenderCommandFence RenderFence;

    FRenderRequestStruct(){

    }
};


UCLASS(Blueprintable)
class CV4UE_API ACameraCaptureManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACameraCaptureManager();
    
    // Captured Data Sub-Directory Name 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capture")
    FString SubDirectoryName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capture")
    int NumDigits = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capture")
    int FrameWidth = 640;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capture")
    int FrameHeight = 480;

    // Color Capture Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capture")
    USceneCaptureComponent2D* CaptureComponent;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Capture")
    //ASceneCapture2D* SegmentationCapture = nullptr;

    // PostProcessMaterial used for segmentation
    UPROPERTY(EditAnywhere, Category="Capture")
    UMaterial* PostProcessMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="Logging")
    bool VerboseLogging = false;

    UPROPERTY(EditAnywhere, Category="Logging")
    TEnumAsByte<OutputFormat> outputFormat = IMG;

protected:
	// RenderRequest Queue
    TQueue<FRenderRequestStruct*> RenderRequestQueue;

    int ImgCounter = 0;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SetupCaptureComponent();

    // Creates an async task that will save the captured image to disk
    void RunAsyncImageSaveTask(TArray64<uint8>& Image, FString& ImageName);
    void RunAsyncJsonSaveTask(FCoCoFrameInfo data, FString& ImageName);

    FString ToStringWithLeadingZeros(int32 Integer, int32 MaxDigits);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "ImageCapture")
    void CaptureNonBlocking();

    FMatrix GetExtrinsicsMatrix();
    Chaos::PMatrix<float, 3, 3> GetIntrinsicMatrix();
};
