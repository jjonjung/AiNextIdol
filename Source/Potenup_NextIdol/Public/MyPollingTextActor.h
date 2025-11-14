#pragma once

#include "CoreMinimal.h"
#include "Engine/TextRenderActor.h"
#include "Http.h"
#include "MyPollingTextActor.generated.h"

UCLASS()
class POTENUP_NEXTIDOL_API AMyPollingTextActor : public ATextRenderActor
{
    GENERATED_BODY()

public:
    AMyPollingTextActor();

protected:
    virtual void BeginPlay() override;

private:
    void RequestScore();
    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void DisplayRandomValue();
    FString GetJudgementString(float Value) const;

    UPROPERTY(EditAnywhere, Category="Polling", meta=(AllowPrivateAccess="true"))
    uint8 TextSlot = 0;

    UPROPERTY(EditAnywhere, Category="Polling", meta=(AllowPrivateAccess="true"))
    float PollInterval = 5.f;

    UPROPERTY(EditAnywhere, Category="Polling", meta=(AllowPrivateAccess="true"))
    FString EndpointUrl = TEXT("http://127.0.0.1:8000/status");

    FTimerHandle PollTimer;
    TArray<TArray<float>> ScoreMatrix;
    int32 CurrentRow = INDEX_NONE;
};
