// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ThesisStaticLibrary.h"
#include "GameFramework/PlayerController.h"
#include "CameraPlayerController.generated.h"

/**
 * 
 */
UCLASS( dependson=UThesisStaticLibrary )
class ACameraPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

private:
	float MAX_PICKUP_DISTANCE;

public:
	AActor* m_actorHitByRayTrace;
	bool bCanPickupItem;

	/** This function toggles the players ability to pick up stuff */
	bool TogglePickup(bool CanPickup);

	void ActivatePhysicsOnActor();


	/** Begin AActorInterface */
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) OVERRIDE;
};
