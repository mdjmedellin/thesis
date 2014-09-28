#pragma once

#include "GameFramework/Actor.h"
#include "ProteinModelSpawnPoint.generated.h"

UENUM()
namespace EProteinSpawnPointType
{
	enum Type
	{
		ESpawn_ProteinModel,
		ESpawn_CustomPolypeptide
	};
}

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

	UPROPERTY(EditDefaultsOnly, Category = "Protein")
		TEnumAsByte<EProteinSpawnPointType::Type> m_typeOfSpawnPoint;


	virtual void PostInitializeComponents() OVERRIDE;
	virtual void PostUnregisterAllComponents() OVERRIDE;
};
