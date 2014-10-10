// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Object.h"
#include "ThesisStaticLibrary.generated.h"

UENUM()
namespace ELinkType
{
	enum Type
	{
		ELink_Helix,
		ELink_BetaStrand,
		ELink_HydrogenBond,
		ELink_Backbone,
		ELink_None
	};
}

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

UENUM(BlueprintType)
namespace EResidueType
{
	enum Type
	{
		kUnknownResidue			UMETA(Hidden),

		//
		kAlanine				UMETA(DisplayName = "Alanine"),				// A	ala
		kArginine				UMETA(DisplayName = "Arginine"),			// R	arg
		kAsparagine				UMETA(DisplayName = "Asparagine"),			// N	asn
		kAsparticAcid			UMETA(DisplayName = "Aspartic Acid"),		// D	asp
		kCysteine				UMETA(DisplayName = "Cysteine"),			// C	cys
		kGlutamicAcid			UMETA(DisplayName = "Glutamic Acid"),		// E	glu
		kGlutamine				UMETA(DisplayName = "Glutamine"),			// Q	gln
		kGlycine				UMETA(DisplayName = "Glycine"),				// G	gly
		kHistidine				UMETA(DisplayName = "Histidine"),			// H	his
		kIsoleucine				UMETA(DisplayName = "Isoleucine"),			// I	ile
		kLeucine				UMETA(DisplayName = "Leucine"),				// L	leu
		kLysine					UMETA(DisplayName = "Lysine"),				// K	lys
		kMethionine				UMETA(DisplayName = "Methionine"),			// M	met
		kPhenylalanine			UMETA(DisplayName = "Phenylalanine"),		// F	phe
		kProline				UMETA(DisplayName = "Proline"),				// P	pro
		kSerine					UMETA(DisplayName = "Serine"),				// S	ser
		kThreonine				UMETA(DisplayName = "Threonine"),			// T	thr
		kTryptophan				UMETA(DisplayName = "Tryptophan"),			// W	trp
		kTyrosine				UMETA(DisplayName = "Tyrosine"),			// Y	tyr
		kValine					UMETA(DisplayName = "Valine"),				// V	val

		kResidueTypeCount		UMETA(HIDDEN),
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

class Interpolator
{
public:
	Interpolator();
	~Interpolator();
	void TogglePlay();
	void Update(float deltaTime);
	void OffsetValues(const FVector& offset);
	void RotateValuesFromSpecifiedPoint(const FRotationMatrix& rotationMatrix, const FVector& rotationPoint);
	FVector Poll() const;
	bool IsPlaying() const;
	void ResetInterpolator(const FVector& start, const FVector& goal, float interpSpeed,
		bool loop = false, bool startRandomlyWithinRange = false, int maxPlayCount = -1);

	static void UpdateInterpolators(float deltaTime);

private:
	static void AddToListOfInterpolators(Interpolator* interpolatorToAdd);
	static void RemoveInterpolatorFromList(Interpolator* interpolatorToRemove);

public:

private:
	FVector m_originalGoal;
	FVector m_originalStart;
	FVector m_currentGoal;
	FVector m_currentStart;
	FVector m_current;

	float m_interpSpeed;

	bool m_play;
	bool m_loop;
	
	int m_playCount;
	int m_maxPlayCount;
	int m_id;

	bool m_isInList;

	static int s_id;
	static TArray<Interpolator*> s_interpolatorList;
};

/**
 * 
 */
UCLASS()
class UThesisStaticLibrary : public UObject
{
	GENERATED_UCLASS_BODY()

	static ELinkType::Type GetLinkTypeFromSecondaryStructure(ESecondaryStructure::Type secondaryStructure);
	static void RotateVectorAroundSpecifiedPoint(FVector& out_vectorToRotate, const FMatrix& rotationMatrix,
		const FVector& rotationPoint);

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
