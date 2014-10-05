

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

private:
	UMaterialInstanceDynamic* m_dynamicMaterial;
	FVector m_prevEndTangent;
	bool m_shake;
	float m_timeVal;
	float m_maxTime;
	FVector m_minVals;
	FVector m_maxVals;

public:
	virtual void BeginPlay();
	void setColor(const FColor& linkColor);

	virtual void Tick(float DeltaSeconds) OVERRIDE;
	void ToggleShake();
	void Hide();
};
