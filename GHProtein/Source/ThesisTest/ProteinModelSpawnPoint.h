#pragma once

#include "GameFramework/Actor.h"
#include "ProteinModelSpawnPoint.generated.h"

/**
 * 
 */
UCLASS(hidecategories=(Lighting, LightColor, Force, MinimalAPI), ClassGroup=Protein)
class THESISTEST_API AProteinModelSpawnPoint : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	TSubobjectPtr<class UCapsuleComponent> CapsuleComponent;

	UPROPERTY()
	TSubobjectPtr<class UBillboardComponent> GoodSprite;

	virtual void PostInitializeComponents() OVERRIDE;
	virtual void PostUnregisterAllComponents() OVERRIDE;
};
