// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "CameraPlayerController.h"
#include "AminoAcidMovementComponent.h"
#include "AminoMovementComponent.h"
#include "ThesisTestHUD.h"

ACameraPlayerController::ACameraPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bEnableClickEvents = true;
	bCanPickupItem = false;
	m_actorHitByRayTrace = nullptr;
	//MAX_PICKUP_DISTANCE = 256.f;
}

bool ACameraPlayerController::TogglePickup(bool CanPickup)
{
	if (bCanPickupItem != CanPickup)
	{
		//this is where the functionality to trigger the ability to pickup items will happen
		bCanPickupItem = CanPickup;
		AThesisTestHUD* myCustomHUD = Cast< AThesisTestHUD >(MyHUD);

		if (bCanPickupItem)
		{
			myCustomHUD->ActivateCrosshair();
		}
		else
		{
			myCustomHUD->DeactivateCrosshair();
		}

		return true;
	}

	return false;
}

void ACameraPlayerController::ActivatePhysicsOnActor()
{
	const FVector Start = GetFocalLocation();

	if (m_actorHitByRayTrace != nullptr)
	{
		float _lengthToActorHitByRayTraceSquared = 
			(Start - m_actorHitByRayTrace->GetActorLocation()).SizeSquared();

		UAminoMovementComponent* movementComponent;
		movementComponent = Cast< UAminoMovementComponent >(
										m_actorHitByRayTrace->FindComponentByClass< UAminoMovementComponent >() );

		if ( movementComponent != nullptr)
		{
			//m_lengthTo
			movementComponent->ToggleGravity();
			//movementComponent->SetMovementMode(MOVE_Falling);
		}
	}
}

void ACameraPlayerController::TickActor(float DeltaSeconds, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaSeconds, TickType, ThisTickFunction);

	//Perform a ray trace to see if we have the ability to pickup items
	const FVector Start = GetFocalLocation();
	const FVector End = Start + GetControlRotation().Vector() * 500;

	FHitResult HitData(ForceInit);

	if (UThesisStaticLibrary::Trace(GetPawn(), Start, End, HitData))
	{
		//we hit something that is not our Pawn
		if (TogglePickup(true))
		{
			m_actorHitByRayTrace = HitData.GetActor();
		}
	}
	else
	{
		if (TogglePickup(false))
		{
			m_actorHitByRayTrace = nullptr;
		}
	}
}




