// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "CustomMovementComponent.h"


UCustomMovementComponent::UCustomMovementComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	DefaultWaterMovementMode = MOVE_Swimming;
	DefaultLandMovementMode = MOVE_Flying;
}

//void UCustomMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
//{
//	UPawnMovementComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	if ( !IsValid(CharacterOwner) || !UpdatedComponent )
//	{
//		return;
//	}
//
//	if (AvoidanceLockTimer > 0.0f)
//	{
//		AvoidanceLockTimer -= DeltaTime;
//	}
//
//	if (CharacterOwner->Role > ROLE_SimulatedProxy)
//	{
//		if (CharacterOwner->Role == ROLE_Authority)
//		{
//			// Check we are still in the world, and stop simulating if not.
//			const bool bStillInWorld = (bCheatFlying || CharacterOwner->CheckStillInWorld());
//			if (!bStillInWorld || !IsValid(CharacterOwner))
//			{
//				return;
//			}
//		}
//
//		// If we are a client we might have received an update from the server.
//		const bool bIsClient = (GetNetMode() == NM_Client && CharacterOwner->Role == ROLE_AutonomousProxy);
//		if (bIsClient)
//		{
//			ClientUpdatePositionAfterServerUpdate();
//		}
//
//		// Allow root motion to move characters that have no controller.
//		if (CharacterOwner->IsLocallyControlled() || bRunPhysicsWithNoController || (!CharacterOwner->Controller && CharacterOwner->IsPlayingRootMotion()))
//		{
//			// We need to check the jump state before adjusting input acceleration, to minimize latency
//			// and to make sure acceleration respects our potentially new falling state.
//			CharacterOwner->CheckJumpInput(DeltaTime);
//
//			// apply input to acceleration
//			Acceleration = ScaleInputAcceleration(ConstrainInputAcceleration(GetInputVector()));
//			ConsumeInputVector();
//
//			if (CharacterOwner->Role == ROLE_Authority)
//			{
//				PerformMovement(DeltaTime);
//			}
//			else if (bIsClient)
//			{
//				ReplicateMoveToServer(DeltaTime, Acceleration);
//			}
//
//			//we will clear the jumpInput on release
//			//CharacterOwner->ClearJumpInput();
//		}
//	}
//	else if (CharacterOwner->Role == ROLE_SimulatedProxy)
//	{
//		AdjustProxyCapsuleSize();
//		SimulatedTick(DeltaTime);
//	}
//
//	UpdateDefaultAvoidance();
//
//	if (bEnablePhysicsInteraction)
//	{
//		// Apply downwards force when walking on top of physics objects
//		if (CharacterOwner->GetMovementBase() != NULL)
//		{
//			UPrimitiveComponent* BaseComp = CharacterOwner->GetMovementBase();
//
//			if (BaseComp->IsSimulatingPhysics())
//			{
//				UPrimitiveComponent* CharacterComp = CharacterOwner->Mesh.Get();
//
//#if WITH_EDITOR
//				// We need to calculate the mass here, as the bodies, when kinematic, and do not return proper mass
//				float CharacterMass = CharacterComp != NULL ? CharacterComp->CalculateMass() : 1.0f;
//#else
//				float CharacterMass = 1.0f;
//#endif
//
//				float GravZ = GetGravityZ();
//				BaseComp->AddForceAtLocation(FVector(0, 0, GravZ * CharacterMass), CharacterComp->GetComponentLocation());
//			}
//		}
//
//		ApplyRepulsionForce(DeltaTime);
//	}
//}


