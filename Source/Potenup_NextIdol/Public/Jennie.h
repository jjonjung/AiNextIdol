#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Jennie.generated.h"

class UAnimSequence;

UCLASS()
class POTENUP_NEXTIDOL_API AJennie : public ACharacter
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    // Song animation asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimSequence> SongAnimation;

    // Whether to auto-start performance on BeginPlay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoStartOnBeginPlay;

    // Location where the character should be positioned for the song
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVector SongStartLocation;

    // Target song index to respond to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetSongIndex;

public:
    // Called when DMX Previs soundtrack starts
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void HandleDMXPrevisSoundtrackStarted(const FString& SoundtrackName);

    // Start the song performance
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartSongPerformance();

    // Called when a song starts (by index)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void NotifySongStarted(int32 SongIndex);

    

    
};
