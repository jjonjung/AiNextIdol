#include "Jennie.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

namespace JennieInternal
{
    static const TCHAR* LikeJennieSoundtrackName = TEXT("JENNIE_-_like_JENNIE DMXPrevis_Soundtrack");
    static const TCHAR* LikeJennieAnimAssetPath = TEXT("/Script/Engine.AnimSequence'/Game/Assets/Animations/likejennie/jennie1_Anim.jennie1_Anim'");
    static const TCHAR* JennieBlueprintIdentifier = TEXT("BP_Jennie");
}

void AJennie::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoStartOnBeginPlay)
    {
        SetActorLocation(SongStartLocation);
        StartSongPerformance();
    }
}

void AJennie::HandleDMXPrevisSoundtrackStarted(const FString& SoundtrackName)
{
    using namespace JennieInternal;

    // Debug log
    UE_LOG(LogTemp, Log, TEXT("Received soundtrack start: %s"), *SoundtrackName);

    // Check soundtrack name - more flexible matching
    if (!SoundtrackName.Contains(TEXT("JENNIE"), ESearchCase::IgnoreCase) || 
        !SoundtrackName.Contains(TEXT("like"), ESearchCase::IgnoreCase))
    {
        UE_LOG(LogTemp, Warning, TEXT("Soundtrack name doesn't match: %s"), *SoundtrackName);
        return;
    }

    if (!GetClass()->GetPathName().Contains(JennieBlueprintIdentifier))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not a Jennie blueprint"));
        return;
    }

    SongAnimation = TSoftObjectPtr<UAnimSequence>(FSoftObjectPath(LikeJennieAnimAssetPath));

    SetActorLocation(SongStartLocation);
    StartSongPerformance();
}

void AJennie::StartSongPerformance()
{
    if (USkeletalMeshComponent* MeshComponent = GetMesh())
    {
        // Stop any current animation
        MeshComponent->Stop();
        
        MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);

        if (UAnimSequence* LoadedAnim = SongAnimation.LoadSynchronous())
        {
            MeshComponent->PlayAnimation(LoadedAnim, true);
            UE_LOG(LogTemp, Log, TEXT("Animation started: %s"), *LoadedAnim->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load animation"));
            
            // Try direct loading as fallback
            if (UAnimSequence* DirectAnim = LoadObject<UAnimSequence>(nullptr, JennieInternal::LikeJennieAnimAssetPath))
            {
                MeshComponent->PlayAnimation(DirectAnim, true);
                UE_LOG(LogTemp, Log, TEXT("Animation loaded via direct method"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No mesh component found"));
    }
}

void AJennie::NotifySongStarted(int32 SongIndex)
{
    using namespace JennieInternal;

    UE_LOG(LogTemp, Log, TEXT("Song started: Index %d (Target: %d)"), SongIndex, TargetSongIndex);

    if (SongIndex == TargetSongIndex)
    {
        HandleDMXPrevisSoundtrackStarted(LikeJennieSoundtrackName);
    }
}