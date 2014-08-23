// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "ThesisStaticLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UThesisStaticLibrary : public UObject
{
	GENERATED_UCLASS_BODY()

	static FORCEINLINE bool Trace(AActor* ActorToIgnore,
									const FVector& Start,
									const FVector& End,
									FHitResult& HitOut,
									ECollisionChannel CollisionChannel = ECC_Pawn,
									bool ReturnPhysMat = false)
	{
		FCollisionQueryParams TraceParams(FName(TEXT("Thesis Test Trace")), true, ActorToIgnore);
		TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;

		//Ignore Actors
		TraceParams.AddIgnoredActor(ActorToIgnore);

		//Re-initialize hit info
		HitOut = FHitResult(ForceInit);

		//Get World Context
		TObjectIterator< APlayerController > ThePC;
		if (!ThePC) return false;

		//Trace!
		ThePC->GetWorld()->LineTraceSingle(HitOut,		//result
											Start,	//start
											End, //end
											CollisionChannel, //collision channel
											TraceParams);

		//Hit any Actor?
		return (HitOut.GetActor() != NULL);
	}

	//Trace with an Array of Actors to Ignore
	//Ignore as many actors as you want!
	static FORCEINLINE bool Trace( TArray<AActor*>& ActorsToIgnore,
									const FVector& Start,
									const FVector& End,
									FHitResult& HitOut,
									ECollisionChannel CollisionChannel = ECC_Pawn,
									bool ReturnPhysMat = false) 
	{
		if (ActorsToIgnore.Num() <= 0) return false;   //MUST HAVE ACTOR ARRAY TO IGNORE
		if (ActorsToIgnore[0] == NULL) return false;   //use above function with NULL to have no ignore actors.
		//~~~~~~~~~~~~~~~~~~~~~~~~

		FCollisionQueryParams TraceParams(FName(TEXT("Thesis Test Trace")), true, ActorsToIgnore[0]);
		TraceParams.bTraceComplex = true;

		TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;

		//Ignore Actors
		TraceParams.AddIgnoredActors(ActorsToIgnore);

		//Re-initialize hit info
		HitOut = FHitResult(ForceInit);

		ActorsToIgnore[0]->GetWorld()->LineTraceSingle(HitOut,		//result
														Start,	//start
														End, //end
														CollisionChannel, //collision channel
														TraceParams);

		return (HitOut.GetActor() != NULL);
	}
	
};
