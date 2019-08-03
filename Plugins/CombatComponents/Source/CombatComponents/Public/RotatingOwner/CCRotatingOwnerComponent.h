// Copyright (C) 2019 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCRotatingOwnerComponent.generated.h"

/* Delegate called when rotating has started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotatingStart);
DECLARE_MULTICAST_DELEGATE(FOnRotatingStartNative);

/* Delegate called when rotating has ended */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotatingEnd);
DECLARE_MULTICAST_DELEGATE(FOnRotatingEndNative);

/**
 * Component which allows to rotate character towards desired rotation,
 * which should be defined in owning actor throught interface CCRotatingOwnerInterface
 * Example of use: Rotate character towards input direction while it's playing attack anim montage with enabled root motion
 */
UCLASS( ClassGroup=(CombatComponents), meta=(BlueprintSpawnableComponent) )
class COMBATCOMPONENTS_API UCCRotatingOwnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/* Default constructor */
	UCCRotatingOwnerComponent();

	/* Called every frame */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Retrurns true if rotating is activated, false otherwise */
	UFUNCTION(BlueprintCallable, Category = "RotatingOwnerComponent")
	bool IsRotating() const;

	/* Delegate called when rotating has started */
	UPROPERTY(BlueprintAssignable, Category = "RotatingOwnerComponent")
	FOnRotatingStart OnRotatingStart;
	/* Native version above, called before BP delegate */
	FOnRotatingStartNative OnRotatingStartNative;

	/* Delegate called when rotating has ended */
	UPROPERTY(BlueprintAssignable, Category = "RotatingOwnerComponent")
	FOnRotatingEnd OnRotatingEnd;
	/* Native version above, called before BP delegate */
	FOnRotatingEndNative OnRotatingEndNative;

	/* Starts rotating owner for given time with given rotating speed */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "RotatingOwnerComponent")
	void StartRotating(float time = 0.5f, float degressPerSecond = 540.f);

	/**
	 * Starts rotating owner where time is calculated based on maxPossibleRotation/degressPerSecond, with given rotating speed
	 * Example: maxPossibleRotation=180, degressPerSecond=360 -> rotation will be activated for 0.5 sec
	 */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "RotatingOwnerComponent")
	void StartRotatingWithLimit(float maxPossibleRotation = 180.f, float degressPerSecond = 540.f);

	/* Stops rotating */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "RotatingOwnerComponent")
	void StopRotating();

protected:
	/* Called when the game starts */
	virtual void BeginPlay() override;

	/* Calls the notify callbacks */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_NotifyOnRotatingStart();

	/* Calls the notify callbacks */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_NotifyOnRotatingEnd();

	/* Set owner rotation to server and all connected clients */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetOwnerRotation(FRotator newRotation);

private:
	/* Whether rotating is activated or not */
	UPROPERTY(Replicated)
	uint32 bIsRotating : 1;

	/* Time elapsed since rotating was activated */
	float TimeElapsed;

	/* How long rotating should be activated */
	float RotatingTime;

	/* Limit of degrees to rotate per second */
	float DegreesPerSecond;
};
