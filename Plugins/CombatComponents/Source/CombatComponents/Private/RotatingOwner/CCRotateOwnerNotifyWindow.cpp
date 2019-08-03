// Copyright (C) 2019 Grzegorz Szewczyk - All Rights Reserved

#include "RotatingOwner/CCRotateOwnerNotifyWindow.h"
#include "RotatingOwner/CCRotatingOwnerComponent.h"
#include "Components/SkeletalMeshComponent.h"

UCCRotateOwnerNotifyWindow::UCCRotateOwnerNotifyWindow()
	: DegreesPerSecond(540.f)
{
	NotifyName = TEXT("RotateOwner");
}

void UCCRotateOwnerNotifyWindow::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
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
					RotatingComponent->StartRotating(10.f, DegreesPerSecond);
				}
			}
		}
	}
}

void UCCRotateOwnerNotifyWindow::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
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
					RotatingComponent->StopRotating();
				}
			}
		}
	}
}

FString UCCRotateOwnerNotifyWindow::GetNotifyName_Implementation() const
{
	return "RotateOwner";
}