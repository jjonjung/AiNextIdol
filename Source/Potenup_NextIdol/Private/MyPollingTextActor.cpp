#include "MyPollingTextActor.h"

#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Math/UnrealMathUtility.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"
#include "Components/TextRenderComponent.h"

AMyPollingTextActor::AMyPollingTextActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMyPollingTextActor::BeginPlay()
{
    Super::BeginPlay();

    GetTextRender()->SetHorizontalAlignment(EHTA_Center);
    GetTextRender()->SetText(FText::FromString(TEXT("waiting...")));

    FMath::RandInit(static_cast<int32>(FPlatformTime::Cycles64()));

    RequestScore();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PollTimer, this, &AMyPollingTextActor::RequestScore, PollInterval, true);
    }
}

void AMyPollingTextActor::RequestScore()
{
    const FString Url = EndpointUrl.IsEmpty() ? TEXT("http://127.0.0.1:8000/status") : EndpointUrl;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindUObject(this, &AMyPollingTextActor::HandleResponse);
    Request->ProcessRequest();
}

void AMyPollingTextActor::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    bool bDisplayed = false;

    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        TSharedPtr<FJsonObject> JsonObject;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            const TArray<TSharedPtr<FJsonValue>>* PartScoreArray = nullptr;
            if (JsonObject->TryGetArrayField(TEXT("part_score"), PartScoreArray) && PartScoreArray)
            {
                ScoreMatrix.Empty(PartScoreArray->Num());
                for (const TSharedPtr<FJsonValue>& RowValue : *PartScoreArray)
                {
                    const TArray<TSharedPtr<FJsonValue>> RowArray = RowValue->AsArray();
                    TArray<float> ParsedRow;
                    ParsedRow.Reserve(RowArray.Num());
                    for (const TSharedPtr<FJsonValue>& CellValue : RowArray)
                    {
                        ParsedRow.Add(CellValue->AsNumber());
                    }
                    ScoreMatrix.Add(MoveTemp(ParsedRow));
                }

                if (ScoreMatrix.Num() > 0)
                {
                    CurrentRow = (CurrentRow + 1) % ScoreMatrix.Num();
                    if (ScoreMatrix[CurrentRow].IsValidIndex(TextSlot))
                    {
                        const float Value = ScoreMatrix[CurrentRow][TextSlot];
                        const FString DisplayText = GetJudgementString(Value);
                        GetTextRender()->SetText(FText::FromString(DisplayText));
                        bDisplayed = true;
                    }
                }
            }
        }
    }

    if (!bDisplayed)
    {
        DisplayRandomValue();
    }
}

void AMyPollingTextActor::DisplayRandomValue()
{
    const float RandomValue = FMath::FRandRange(0.1f, 1.0f);
    const FString DisplayText = GetJudgementString(RandomValue);
    GetTextRender()->SetText(FText::FromString(DisplayText));
}

FString AMyPollingTextActor::GetJudgementString(float Value) const
{
    if (Value >= 0.8f)
    {
        return TEXT("GOOD");
    }

    if (Value >= 0.4f)
    {
        return TEXT("NORMAL");
    }

    return TEXT("MISS");
}
