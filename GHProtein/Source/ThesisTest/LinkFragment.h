#pragma once

#include "GameFramework/Actor.h"
#include "ThesisStaticLibrary.h"
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
	FVector m_currentSizeScale;
	FColor m_normalColor;
	FColor m_helixColor;
	FColor m_betaStrandColor;
	FColor m_hydrogenBondColor;
	float m_normalWidth;
	float m_helixWidth;
	float m_betaStrandWidth;
	float m_hydrogenBondWidth;
	float m_normalHeight;

	ELinkType::Type m_linkType;

	Interpolator* m_sizeInterpolator;
	Interpolator* m_colorInterpolator;

public:
	virtual void BeginPlay();
	void UpdateRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
		const FColor& hydrogenBondColor, float normalWidth, float helixWidth, float betaStrandWidth, float hydrogenBondWidth,
		float normalHeight);
	void ChangeLinkType(ELinkType::Type linkType, bool smoothInterpolate = false);
	void ChangeLinkType(ESecondaryStructure::Type secondaryStructureType, bool smoothInterpolate = false);
	virtual void Tick(float DeltaSeconds) OVERRIDE;
	void ToggleShake();
	void ToggleBreaking();
	void Hide();
	void Translate(const FVector& deltaLocation);
	void UpdateTangents(const FVector& startTangent, const FVector& endtangent);
	bool IsAnimating();

private:
	void SetColor(const FColor& linkColor);
	void UpdateRendering(bool smoothInterpolate = false);
};