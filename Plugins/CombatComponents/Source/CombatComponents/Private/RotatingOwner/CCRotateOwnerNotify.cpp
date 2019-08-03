// Copyright (C) 2019 Grzegorz Szewczyk - All Rights Reserved

#include "RotatingOwner/CCRotateOwnerNotify.h"
#include "RotatingOwner/CCRotatingOwnerComponent.h"
#include "Components/SkeletalMeshComponent.h"

UCCRotateOwnerNotify::UCCRotateOwnerNotify()
	: DegreesPerSecond(540.f), MaxPossibleRotation(180.f)
{
	NotifyName = TEXT("RotateOwner");
}

void UCCRotateOwnerNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp)
	{
		AActor* Owner = MeshComp->GetOwner();

		if (Owner)
		{
			// Call only from the server
			if (Owner->HasAuthority())
			{
				UCCRotatingOwnerComponent* RotatingComponent = Cast<UCCRotatingOwnerComponent>(Owner->GetComponentByClass(UCCRotatingOwnerComponent::StaticClass()));

				if (RotatingComponent)
				{
					RotatingComponent->StartRotatingWithLimit(MaxPossibleRotation, DegreesPerSecond);
				}
			}
		}
	}
}

FString UCCRotateOwnerNotify::GetNotifyName_Implementation() const
{
	return "RotateOwner";
}
