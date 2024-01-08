// Fill out your copyright notice in the Description page of Project Settings.

#include "DeathMatchGameMode.h"
#include "EngineUtils.h"
#include "IgnitionOverdriveGameInstance.h"
#include "Kismet/GameplayStatics.h"

// spawnar in spelare i rätt player start, ser till att ingen spawnar på samma plats
// körs när en spelare skapas, är överskriven från vanliga FindPlayerStart

AActor *ADeathMatchGameMode::FindPlayerStart_Implementation(AController *Player, const FString &IncomingName)
{
    //APlayerController* PlayerController;
    int PlayerId;

    //får en array av playerstarts från gameinstance

    TArray<AActor*> PlayerStarts = Cast<UIgnitionOverdriveGameInstance>(GetGameInstance())->GetPlayerStarts();

    if (Cast<APlayerController>(Player))
    {
        // Sätter PlayerId till controller id och returnar korrekt playerstart för den spelaren, om den inte hittas körs ChoosePlayerStart som hittar "bästa" playerstart, den är overriden i blueprint

        PlayerId = UGameplayStatics::GetPlayerControllerID(Cast<APlayerController>(Player));
        UE_LOG(LogTemp, Warning, TEXT("PLAYER: %i") , PlayerId);

        if (PlayerStarts.Num() > 0 && PlayerStarts.Num() <= 4 && PlayerId >-1)
        {
            return PlayerStarts[PlayerId];
        }
    }
    
    
    return AGameModeBase::ChoosePlayerStart(Player);
}
