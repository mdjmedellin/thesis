

#include "ThesisTest.h"
#include "ThesisTestGameMode.h"
#include "ProteinModelSpawnPoint.h"


AProteinModelSpawnPoint::AProteinModelSpawnPoint(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> ProteinSpawnLocationTextureObject;
		FName ID_ProteinSpawnLocation;
		FText NAME_ProteinSpawnLocation;
		FName ID_Navigation;
		FText NAME_Navigation;
		FConstructorStatics()
			: ProteinSpawnLocationTextureObject(TEXT("/Engine/EditorResources/S_Player"))
			, ID_ProteinSpawnLocation(TEXT("ProteinModelSpawnLocation"))
			, NAME_ProteinSpawnLocation(NSLOCTEXT("SpriteCategory", "ProteinModelSpawnLocation", "Protein Model Spawn Location"))
			, ID_Navigation(TEXT("Navigation"))
			, NAME_Navigation(NSLOCTEXT("SpriteCategory", "Navigation", "Navigation"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	CapsuleComponent = PCIP.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("CollisionCapsule"));
	CapsuleComponent->ShapeColor = FColor(255, 138, 5, 255);
	CapsuleComponent->bDrawOnlyIfSelected = true;
	CapsuleComponent->InitCapsuleSize(50.0f, 50.0f);
	CapsuleComponent->BodyInstance.bEnableCollision_DEPRECATED = false;
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	CapsuleComponent->bShouldCollideWhenPlacing = true;
	CapsuleComponent->bShouldUpdatePhysicsVolume = true;
	CapsuleComponent->Mobility = EComponentMobility::Static;
	RootComponent = CapsuleComponent;
	bCollideWhenPlacing = true;

	GoodSprite = PCIP.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Sprite"));
#if WITH_EDITORONLY_DATA
	if (GoodSprite)
	{
		GoodSprite->bHiddenInGame = true;
		GoodSprite->bAbsoluteScale = true;
		GoodSprite->bIsScreenSizeScaled = true;
		GoodSprite->Sprite = ConstructorStatics.ProteinSpawnLocationTextureObject.Get();
		GoodSprite->SpriteInfo.Category = ConstructorStatics.ID_ProteinSpawnLocation;
		GoodSprite->SpriteInfo.DisplayName = ConstructorStatics.NAME_ProteinSpawnLocation;
		GoodSprite->AttachParent = CapsuleComponent;
	}
#endif // WITH_EDITORONLY_DATA
}

void AProteinModelSpawnPoint::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (!IsPendingKill() && GetWorld()->GetAuthGameMode())
	{
		AThesisTestGameMode* authGameMode = (AThesisTestGameMode*)(GetWorld()->GetAuthGameMode());
		
		if (authGameMode)
		{
			authGameMode->AddProteinModelSpawnPoint(this);
		}
	}
}

void AProteinModelSpawnPoint::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	UWorld* ActorWorld = GetWorld();
	if (ActorWorld && ActorWorld->GetAuthGameMode())
	{
		AThesisTestGameMode* authGameMode = (AThesisTestGameMode*)(ActorWorld->GetAuthGameMode());

		if (authGameMode)
		{
			authGameMode->RemoveProteinModelSpawnPoint(this);
		}
	}
}