// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "ThesisStaticLibrary.generated.h"

// a limited set of known atoms.
UENUM()
namespace EAtomType
{
	enum Type
	{
		kUnknownAtom,
		kHydrogen,
		// ...
		kCarbon,
		kNitrogen,
		kOxygen,
		kFluorine,
		// ...
		kPhosphorus,
		kSulfur,
		kChlorine,
		kMagnesium,
		kPotassium,
		kCalcium,
		kZinc,
		kSelenium,

		kAtomTypeCount
	};
}

UENUM()
namespace EHelixFlag
{
	enum Type
	{
		helixNone
		, helixStart
		, helixEnd
		, helixStartAndEnd
		, helixMiddle
		, helixCount
	};
}

UENUM()
namespace ESecondaryStructure
{
	enum Type
	{
		ssLoop			//' '
		, ssAlphaHelix	// H
		, ssBetaBridge	// B
		, ssStrand		// E
		, ssHelix_3		// G
		, ssHelix_5		// I
		, ssTurn		// T
		, ssBend		// S
		, ssCount
	};
}

UENUM()
namespace EResidueType
{
	enum Type
	{
		kUnknownResidue,

		//
		kAlanine,				// A	ala
		kArginine,				// R	arg
		kAsparagine,			// N	asn
		kAsparticAcid,			// D	asp
		kCysteine,				// C	cys
		kGlutamicAcid,			// E	glu
		kGlutamine,				// Q	gln
		kGlycine,				// G	gly
		kHistidine,				// H	his
		kIsoleucine,			// I	ile
		kLeucine,				// L	leu
		kLysine,				// K	lys
		kMethionine,			// M	met
		kPhenylalanine,			// F	phe
		kProline,				// P	pro
		kSerine,				// S	ser
		kThreonine,				// T	thr
		kTryptophan,			// W	trp
		kTyrosine,				// Y	tyr
		kValine,				// V	val

		kResidueTypeCount
	};
}

UENUM()
namespace EBridgeType
{
	enum Type
	{
		btNoBridge
		, btParallel
		, btAntiParallel
		, btCount
	};
}

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
	
	//function used to spawn blueprint actors
	template <typename BlueprintType>
	static FORCEINLINE BlueprintType* SpawnBP(UWorld* world,
												UClass* blueprint,
												const FVector& location,
												const FRotator& rotation,
												AActor* owner = NULL,
												const bool bNoCollisionFail = false,
												APawn* instigator = NULL)
	{
		if (!world || !blueprint)
		{
			//we need to have a valid world and blueprint
			return NULL;
		}
		else
		{
			FActorSpawnParameters spawnInfo;
			spawnInfo.bNoCollisionFail = bNoCollisionFail;
			spawnInfo.Owner = owner;
			spawnInfo.Instigator = instigator;
			spawnInfo.bDeferConstruction = false;

			return world->SpawnActor<BlueprintType>(blueprint, location, rotation, spawnInfo);
		}
	}
};
