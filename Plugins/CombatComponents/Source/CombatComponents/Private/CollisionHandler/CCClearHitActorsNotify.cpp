// Copyright (C) 2019 Grzegorz Szewczyk - All Rights Reserved

#include "CollisionHandler/CCClearHitActorsNotify.h"
#include "CollisionHandler/CCCollisionHandlerComponent.h"
#include "Components/SkeletalMeshComponent.h"

UCCClearHitActorsNotify::UCCClearHitActorsNotify()
{
	NotifyName = TEXT("ClearHitActors");
}

void UCCClearHitActorsNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp)
	{
		AActor* Owner = MeshComp->GetOwner();

		if (Owner)
		{
			// Call only from the server
			if (Owner->HasAuthority())
			{
				UCCCollisionHandlerComponent* collisionHandler = Cast<UCCCollisionHandlerComponent>(Owner->GetComponentByClass(UCCCollisionHandlerComponent::StaticClass()));

				if (collisionHandler)
				{
					collisionHandler->ClearHitActors();
				}
			}
		}
	}
}

FString UCCClearHitActorsNotify::GetNotifyName_Implementation() const
{
	return "ClearHitActors";
}