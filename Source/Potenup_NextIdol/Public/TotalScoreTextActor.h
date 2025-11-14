#pragma once

#include "CoreMinimal.h"
#include "Engine/TextRenderActor.h"
#include "Http.h"
#include "TotalScoreTextActor.generated.h"

UCLASS()
class POTENUP_NEXTIDOL_API ATotalScoreTextActor : public ATextRenderActor
{
    GENERATED_BODY()

public:
    ATotalScoreTextActor();

protected:
    virtual void BeginPlay() override;

private:
    void RequestScore();
    void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void DisplayFallback();

    UPROPERTY(EditAnywhere, Category="Polling", meta=(AllowPrivateAccess="true"))
    float PollInterval = 5.f;

    UPROPERTY(EditAnywhere, Category="Polling", meta=(AllowPrivateAccess="true"))
    FString EndpointUrl = TEXT("http://172.16.100.57:8000/docs#/default/upload_status_post");

    FTimerHandle PollTimer;
};
