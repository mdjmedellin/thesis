// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ThesisStaticLibrary.h"
#include "GameFramework/PlayerController.h"
#include "CameraPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACameraPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	AActor* m_actorHitByRayTrace;
	bool bCanPickupItem;

	/** This function toggles the players ability to pick up stuff */
	bool TogglePickup(bool CanPickup);
	void ActivatePhysicsOnActor();

	/** Begin AActorInterface */
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
};
