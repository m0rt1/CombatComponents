// Copyright (C) 2019 Grzegorz Szewczyk - All Rights Reserved

#include "RotatingOwner/CCRotatingOwnerComponent.h"
#include "RotatingOwner/CCRotatingOwnerInterface.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"

#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UCCRotatingOwnerComponent::UCCRotatingOwnerComponent()
	: bIsRotating(false), DegreesPerSecond(540.f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Enable replication on this component
	bReplicates = true;
}

void UCCRotatingOwnerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCCRotatingOwnerComponent, bIsRotating);
}

// Called when the game starts
void UCCRotatingOwnerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCRotatingOwnerComponent::Multicast_NotifyOnRotatingStart_Implementation()
{
	OnRotatingStartNative.Broadcast();
	OnRotatingStart.Broadcast();
}

void UCCRotatingOwnerComponent::Multicast_NotifyOnRotatingEnd_Implementation()
{
	OnRotatingEndNative.Broadcast();
	OnRotatingEnd.Broadcast();
}


void UCCRotatingOwnerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// get owner
	AActor* owner = GetOwner();

	if (bIsRotating)
	{
		// check if owner is valid and implements interface UCCRotatingOwnerInterface
		if (owner &&
			owner->GetClass()->ImplementsInterface(UCCRotatingOwnerInterface::StaticClass()))
		{
			// get desired rotation from owner through interface function GetDesiredRotation
			FRotator targetRotation = ICCRotatingOwnerInterface::Execute_GetDesiredRotation(owner);
			// get current owner rotation
			FRotator currentRotation = owner->GetActorRotation();
			// calculate new rotation by interpolating current and desired rotations
			FRotator newRotation = UKismetMathLibrary::RInterpTo_Constant(currentRotation, targetRotation, DeltaTime, DegreesPerSecond);

			// update elapsed time only on server
			if (owner->HasAuthority())
			{
				// update elapsed time
				TimeElapsed += DeltaTime;
				if (TimeElapsed <= RotatingTime)
				{
					// update owner rotation to server and all clients
					Multicast_SetOwnerRotation(newRotation);
				}
				else
				{
					// stop rotating if time has ended
					StopRotating();
				}
			}
		}
		else
		{
			// stop rotation if owner is not valid or doesnt implement interface
			StopRotating();
		}
	}
}

void UCCRotatingOwnerComponent::Multicast_SetOwnerRotation_Implementation(FRotator newRotation)
{
	GetOwner()->SetActorRotation(newRotation);
}

bool UCCRotatingOwnerComponent::IsRotating() const
{
	return bIsRotating;
}

void UCCRotatingOwnerComponent::StartRotating(float time, float degressPerSecond)
{
	if (GetOwner()->HasAuthority())
	{
		// update variables
		RotatingTime = time;
		DegreesPerSecond = degressPerSecond;
		TimeElapsed = 0.f;
		bIsRotating = true;
		// call notify
		Multicast_NotifyOnRotatingStart();
	}
}

void UCCRotatingOwnerComponent::StartRotatingWithLimit(float maxPossibleRotation, float degressPerSecond)
{
	if (GetOwner()->HasAuthority())
	{
		// calculate time and call StartRotating
		float time = maxPossibleRotation / degressPerSecond;
		StartRotating(time, degressPerSecond);
	}
}

void UCCRotatingOwnerComponent::StopRotating()
{
	if (GetOwner()->HasAuthority())
	{
		bIsRotating = false;
		// call notify
		Multicast_NotifyOnRotatingEnd();
	}
}
