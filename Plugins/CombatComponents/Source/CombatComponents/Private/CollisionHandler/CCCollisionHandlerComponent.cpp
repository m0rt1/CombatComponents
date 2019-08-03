// Copyright (C) 2019 Grzegorz Szewczyk - All Rights Reserved

#include "CollisionHandler/CCCollisionHandlerComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UCCCollisionHandlerComponent::UCCCollisionHandlerComponent()
	: TraceRadius(0.1f), TraceCheckInterval(0.025f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Enable replication on this component
	bReplicates = true;

	// Add EObjectTypeQuery::ObjectTypeQuery3(Pawn) value to objects to collide with
	ObjectTypesToCollideWith.Add(EObjectTypeQuery::ObjectTypeQuery3);
}

void UCCCollisionHandlerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCCCollisionHandlerComponent, CollidingComponents);
	DOREPLIFETIME(UCCCollisionHandlerComponent, bIsCollisionActivated);
	DOREPLIFETIME(UCCCollisionHandlerComponent, ActivatedCollisionPart);
	DOREPLIFETIME(UCCCollisionHandlerComponent, bDebug);
}

// Called when the game starts
void UCCCollisionHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCCollisionHandlerComponent::Multicast_NotifyOnHit_Implementation(const FHitResult& hitResult, UPrimitiveComponent* collidingComponent)
{
	// Notify native before blueprint
	OnHitNative.Broadcast(hitResult, collidingComponent);
	OnHit.Broadcast(hitResult, collidingComponent);
}

void UCCCollisionHandlerComponent::Multicast_NotifyOnCollisionActivated_Implementation(ECCCollisionPart collisionPart)
{
	// Notify native before blueprint
	OnCollisionActivatedtNative.Broadcast(collisionPart);
	OnCollisionActivated.Broadcast(collisionPart);
}

void UCCCollisionHandlerComponent::Multicast_NotifyOnCollisionDeactivated_Implementation()
{
	// Notify native before blueprint
	OnCollisionDeactivatedNative.Broadcast();
	OnCollisionDeactivated.Broadcast();
}

void UCCCollisionHandlerComponent::UpdateSocketLocations()
{
	for (auto& collidingComponent : CollidingComponents)
	{
		if (collidingComponent.Component)
		{
			// for each colliding component get its location and store in LastFrameSocketLocations map
			for (const FName& socketName : collidingComponent.Sockets)
			{
				// get socket name
				FVector socketLocation = collidingComponent.Component->GetSocketLocation(socketName);
				//PRINT(socketName);
				// generate unique socket name
				FName uniqueSocketName = GenerateUniqueSocketName(collidingComponent.Component, socketName);
				// store values in map
				LastFrameSocketLocations.Add(uniqueSocketName, socketLocation);
			}
		}
	}
}

void UCCCollisionHandlerComponent::PerformTraceCheck()
{
	for (auto& collidingComponent : CollidingComponents)
	{
		if (collidingComponent.Component)
		{
			for (const FName& socketName : collidingComponent.Sockets)
			{
				// generate unique socket name
				FName uniqueSocketName = GenerateUniqueSocketName(collidingComponent.Component, socketName);
				FVector startTrace = *LastFrameSocketLocations.Find(uniqueSocketName);
				FVector endTrace = collidingComponent.Component->GetSocketLocation(socketName);
				// array that will store hit results
				TArray<FHitResult> hitResults;

				// do the sphere trace check
				bool wasHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, startTrace, endTrace, TraceRadius, ObjectTypesToCollideWith,
					bTraceComplex, collidingComponent.HitActors, EDrawDebugTrace::Type::None, hitResults, true);

				if (wasHit)
				{
					for (const FHitResult& hitResult : hitResults)
					{
						// if there was a hit check
						// whether this actor wasn't already hit during this activation
						// whether its class is not ignored
						// whether its profile name is not ignored
						if (collidingComponent.HitActors.Contains(hitResult.GetActor()) == false &&
							IsIgnoredClass(hitResult.GetActor()->GetClass()) == false &&
							IsIgnoredProfileName(hitResult.Component->GetCollisionProfileName()) == false)
						{
							// add to hit actors
							collidingComponent.HitActors.Add(hitResult.GetActor());
							// call notify
							Multicast_NotifyOnHit(hitResult, collidingComponent.Component);
#if WITH_EDITOR
							if(bDebug)
								Client_DrawDebugSphere(hitResult.Location);
#endif
						}
					}
				}
#if WITH_EDITOR
				if(bDebug)
					Client_DrawDebugTrace(startTrace, endTrace);
#endif
			}
		}
	}
}

/* --------------------------------------------------- DEBUG ------------------------------------------- */

void UCCCollisionHandlerComponent::Client_DrawDebugSphere_Implementation(FVector location)
{
	UWorld* world = GetWorld();
	if (world)
	{
		float radius = TraceRadius >= 8.f ? TraceRadius : 8.f;
		DrawDebugSphere(world, location, radius, 12, FColor::Green, false, 5.f);
	}
}

void UCCCollisionHandlerComponent::Client_DrawDebugTrace_Implementation(FVector start, FVector end)
{
	UWorld* world = GetWorld();
	if (world)
	{
		DrawDebugCylinder(world, start, end, TraceRadius, 12, FColor::Red, false, 5.f);
	}
}
/* ----------------------------------------------------------------------------------------------------------- */

FName UCCCollisionHandlerComponent::GenerateUniqueSocketName(UPrimitiveComponent* collidingComponent, FName socket)
{
	// check if colliding component is valid
	if (collidingComponent == nullptr)
		return FName(TEXT("None"));
	// concanate colliding component name + socket to get unique name e.g 'SwordMeshSocket01'
	return FName(*collidingComponent->GetName().Append(socket.ToString()));
}

bool UCCCollisionHandlerComponent::IsIgnoredClass(TSubclassOf<AActor> actorClass)
{
	// if actor class is child or same class of any of ignored classes, return true, otherwise false
	for (const auto& ignoredClass : IgnoredClasses)
	{
		if (actorClass->IsChildOf(ignoredClass))
			return true;
	}
	return false;
}

bool UCCCollisionHandlerComponent::IsIgnoredProfileName(FName profileName)
{
	// is profile name in array of ignored profile names
	return IgnoredCollisionProfileNames.Contains(profileName);
}

void UCCCollisionHandlerComponent::TraceCheckLoop()
{
	// on first tick just update socket locations so on next tick it will be able to compare socket locations
	if (bCanPerformTrace)
	{
		PerformTraceCheck();
	}

	UpdateSocketLocations();
	bCanPerformTrace = true;

}

void UCCCollisionHandlerComponent::UpdateCollidingComponent(UPrimitiveComponent* component, const TArray<FName>& sockets)
{
	if (GetOwner()->HasAuthority())
		UpdateCollidingComponents(TArray<FCCCollidingComponent>{ FCCCollidingComponent(component, sockets) });
}

void UCCCollisionHandlerComponent::UpdateCollidingComponents(const TArray<FCCCollidingComponent>& collisionComponents)
{
	// allow only on server
	if (GetOwner()->HasAuthority())
	{
		// update CollidingComponents array
		CollidingComponents = collisionComponents;
		ClearHitActors();
		UpdateSocketLocations();
	}
}

void UCCCollisionHandlerComponent::ActivateCollision(ECCCollisionPart collisionPart)
{
	// allow to call only by server
	if (GetOwner()->HasAuthority())
	{
		bIsCollisionActivated = true;
		ClearHitActors();

		// call notify
		Multicast_NotifyOnCollisionActivated(collisionPart);

		// set timer which will check for collisions
		UWorld* world = GetWorld();
		if (world)
			world->GetTimerManager().SetTimer(TraceCheckTimerHandle, this, &UCCCollisionHandlerComponent::TraceCheckLoop, TraceCheckInterval, true);
	}
}

void UCCCollisionHandlerComponent::ClearHitActors()
{
	// allow to call only by server
	if (GetOwner()->HasAuthority())
	{
		// for each colliding component clear HitActors array except owner
		for (auto& collidingComponent : CollidingComponents)
		{
			collidingComponent.HitActors.Empty();
			collidingComponent.HitActors.Add(GetOwner());
		}
	}
}

void UCCCollisionHandlerComponent::DeactivateCollision()
{
	// allow to call only by server
	if (GetOwner()->HasAuthority())
	{
		bIsCollisionActivated = false;
		bCanPerformTrace = false;

		// clear timer checking for collision
		UWorld* world = GetWorld();
		if (world)
			world->GetTimerManager().ClearTimer(TraceCheckTimerHandle);

		// call notify
		Multicast_NotifyOnCollisionDeactivated();
	}
}

bool UCCCollisionHandlerComponent::IsCollisionActivated() const
{
	return bIsCollisionActivated;
}

ECCCollisionPart UCCCollisionHandlerComponent::GetActivatedCollisionPart() const
{
	return ActivatedCollisionPart;
}



