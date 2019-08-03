// Copyright (C) 2019 Grzegorz Szewczyk - All Rights Reserved

#include "CollisionHandler/CCActivateCollisionNotifyWindow.h"
#include "Components/SkeletalMeshComponent.h"

UCCActivateCollisionNotifyWindow::UCCActivateCollisionNotifyWindow()
	: CollisionPart(ECCCollisionPart::PrimaryItem)
{
	NotifyName = TEXT("ActivateCollision");
}

// activate collision on notify begin
void UCCActivateCollisionNotifyWindow::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	if (MeshComp)
	{
		AActor* Owner = MeshComp->GetOwner();

		if (Owner)
		{
			// Call only from the server
			if (Owner->HasAuthority())
			{
				UCCCollisionHandlerComponent* CollisionHandlerComponent = Cast<UCCCollisionHandlerComponent>(Owner->GetComponentByClass(UCCCollisionHandlerComponent::StaticClass()));

				if (CollisionHandlerComponent)
				{
					CollisionHandlerComponent->ActivateCollision(CollisionPart);
				}
			}
		}
	}
}

// deactivate collision on notify end
void UCCActivateCollisionNotifyWindow::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	if (MeshComp)
	{
		AActor* Owner = MeshComp->GetOwner();

		if (Owner)
		{
			// Call only from the server
			if (Owner->HasAuthority())
			{
				UCCCollisionHandlerComponent* CollisionHandlerComponent = Cast<UCCCollisionHandlerComponent>(Owner->GetComponentByClass(UCCCollisionHandlerComponent::StaticClass()));

				if (CollisionHandlerComponent)
				{
					CollisionHandlerComponent->DeactivateCollision();
				}
			}
		}
	}
}

FString UCCActivateCollisionNotifyWindow::GetNotifyName_Implementation() const
{
	return "ActColl";
}

