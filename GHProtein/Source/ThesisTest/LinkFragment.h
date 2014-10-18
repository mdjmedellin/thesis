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

protected:
	UMaterialInstanceDynamic* m_dynamicMaterial;
	bool m_shake;
	float m_timeVal;
	float m_maxTime;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
	float m_sizeInterpolationSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AminoAcidInterface)
	float m_colorInterpolationSpeed;

	Interpolator* m_sizeInterpolator;
	Interpolator* m_colorInterpolator;

	ELinkType::Type m_linkType;

public:
	virtual void BeginPlay();
	void UpdateRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
		const FColor& hydrogenBondColor, float normalWidth, float helixWidth, float betaStrandWidth, float hydrogenBondWidth,
		float normalHeight);
	void ChangeLinkType(ELinkType::Type linkType, bool smoothInterpolate = false);
	void ChangeLinkType(ESecondaryStructure::Type secondaryStructureType, bool smoothInterpolate = false);
	virtual void Tick(float DeltaSeconds) OVERRIDE;
	virtual void Break();
	void ToggleShake();
	void Hide();
	void Translate(const FVector& deltaLocation);
	void UpdateTangents(const FVector& startTangent, const FVector& endtangent);
	bool IsAnimating();
	void RotateAboutSpecifiedPoint(const FRotationMatrix& rotationMatrix, const FVector& rotationPoint);

protected:
	void SetColor(const FColor& linkColor);
	virtual void UpdateRendering(bool smoothInterpolate = false);
};