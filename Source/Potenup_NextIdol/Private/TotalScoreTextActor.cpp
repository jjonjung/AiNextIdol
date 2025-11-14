#include "TotalScoreTextActor.h"

#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"

namespace
{
    constexpr float FinalScoreTextWorldSize = 100.f;
    constexpr float ScoreTextCameraOffset = 250.f;

    FString ScoreLabelForValue(float ScoreValue)
    {
        const float ClampedScore = FMath::Clamp(ScoreValue, 0.f, 100.f);

        if (ClampedScore >= 71.f)
        {
            return TEXT("GOOD");
        }

        if (ClampedScore >= 51.f)
        {
            return TEXT("NORMAL");
        }

        if (ClampedScore >= 1.f)
        {
            return TEXT("MISS");
        }

        return TEXT("MISS");
    }

    void PositionActorInFrontOfCamera(ATotalScoreTextActor& Actor)
    {
        if (UWorld* World = Actor.GetWorld())
        {
            if (APlayerController* PlayerController = World->GetFirstPlayerController())
            {
                if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
                {
                    const FVector CameraLocation = CameraManager->GetCameraLocation();
                    const FRotator CameraRotation = CameraManager->GetCameraRotation();
                    const FVector Forward = CameraRotation.Vector();
                    const FVector TargetLocation = CameraLocation + Forward * ScoreTextCameraOffset;
                    const FRotator FacingRotation = CameraRotation + FRotator(0.f, 180.f, 0.f);

                    Actor.SetActorLocation(TargetLocation);
                    Actor.SetActorRotation(FacingRotation);
                }
            }
        }
    }

    void PresentScoreText(ATotalScoreTextActor& Actor, const FString& Text)
    {
        if (UTextRenderComponent* TextComponent = Actor.GetTextRender())
        {
            TextComponent->SetHorizontalAlignment(EHTA_Center);
            TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
            TextComponent->SetWorldSize(FinalScoreTextWorldSize);
            TextComponent->SetText(FText::FromString(Text));
        }

        PositionActorInFrontOfCamera(Actor);
    }
}

ATotalScoreTextActor::ATotalScoreTextActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATotalScoreTextActor::BeginPlay()
{
    Super::BeginPlay();

    GetTextRender()->SetHorizontalAlignment(EHTA_Center);
    GetTextRender()->SetVerticalAlignment(EVRTA_TextCenter);
    GetTextRender()->SetText(FText::FromString(TEXT("waiting...")));

    FMath::RandInit(static_cast<int32>(FPlatformTime::Cycles64()));

    RequestScore();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PollTimer, this, &ATotalScoreTextActor::RequestScore, PollInterval, true);
    }
}

void ATotalScoreTextActor::RequestScore()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(EndpointUrl);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindUObject(this, &ATotalScoreTextActor::HandleResponse);
    Request->ProcessRequest();
}

void ATotalScoreTextActor::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        TSharedPtr<FJsonObject> JsonObject;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            double TotalValue = 0.0;
            if (JsonObject->TryGetNumberField(TEXT("total"), TotalValue))
            {
                const FString DisplayText = ScoreLabelForValue(static_cast<float>(TotalValue));
                PresentScoreText(*this, DisplayText);
                return;
            }
        }
    }

    DisplayFallback();
}

void ATotalScoreTextActor::DisplayFallback()
{
    const float RandomValue = FMath::FRandRange(0.1f, 1.0f);
    FString DisplayText;

    if (RandomValue <= 0.3f)
    {
        DisplayText = TEXT("MISS");
    }
    else if (RandomValue <= 0.7f)
    {
        DisplayText = TEXT("NORMAL");
    }
    else
    {
        DisplayText = TEXT("GOOD");
    }

    PresentScoreText(*this, DisplayText);
}




