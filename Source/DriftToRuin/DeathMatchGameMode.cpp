// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatchGameMode.h"
#include "EngineUtils.h"
#include "IgnitionOverdriveGameInstance.h"
#include "Kismet/GameplayStatics.h"

AActor *ADeathMatchGameMode::FindPlayerStart_Implementation(AController *Player, const FString &IncomingName)
{
    //APlayerController* PlayerController;
    int PlayerId;
    TArray<AActor*> PlayerStarts = Cast<UIgnitionOverdriveGameInstance>(GetGameInstance())->GetPlayerStarts();

    if (Cast<APlayerController>(Player))
    {
        
        PlayerId = UGameplayStatics::GetPlayerControllerID(Cast<APlayerController>(Player));
        UE_LOG(LogTemp, Warning, TEXT("PLAYER: %i") , PlayerId);

        if (PlayerStarts.Num() > 0 && PlayerStarts.Num() <= 4 && PlayerId >-1)
        {
            return PlayerStarts[PlayerId];
        }
    }
    
    
    return AGameModeBase::ChoosePlayerStart(Player);
}
