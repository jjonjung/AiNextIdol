#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Http.h"
#include "PathBt.generated.h"

class UButton;

UCLASS()
class POTENUP_NEXTIDOL_API UPathBt : public UUserWidget
{
    GENERATED_BODY()

public:
    UPathBt(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void HandleUserWeightClicked();

    void HandleUploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    bool TrySelectMp4Files(TArray<FString>& OutSelectedFiles) const;
    bool BuildMultipartPayload(const TArray<FString>& FilePaths, const FString& Boundary, TArray<uint8>& OutPayload) const;
    void DispatchUploadRequest(const TArray<FString>& FilePaths);

    UPROPERTY(meta=(BindWidget))
    UButton* UserWeightButton = nullptr;

    UPROPERTY(EditAnywhere, Category="Upload")
    FString UploadEndpoint = TEXT("http://127.0.0.1:8000/upload");

    UPROPERTY(EditAnywhere, Category="Upload")
    FString DefaultBrowseFolder = TEXT("C:/Users/user/Desktop/Fork/AI_NewIdol/Http/get");

    UPROPERTY(EditAnywhere, Category="Upload", meta=(ClampMin="1"))
    int32 ExpectedFileCount = 2;
};
