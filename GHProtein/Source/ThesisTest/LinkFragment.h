

#pragma once

#include "GameFramework/Actor.h"
#include "Components/SplineMeshComponent.h"
#include "LinkFragment.generated.h"

/**
 * 
 */
UCLASS()
class THESISTEST_API ALinkFragment : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
		TSubobjectPtr<USplineMeshComponent> SplineMeshComponent;
};
