#pragma once

#include "LinkFragment.h"
#include "HydrogenBond.generated.h"

namespace GHProtein
{
	class ProteinModel;
}

class AAminoAcid;

/**
 * 
 */
UCLASS()
class THESISTEST_API AHydrogenBond : public ALinkFragment
{
	GENERATED_UCLASS_BODY()

public:
	void UpdateRenderProperties(const FColor& renderColor, float bondWidth, float bondHeight);
	void SetEnviromentalProperties(float currentTemperatureCelsius, float regularTemperatureCelsius,
		float breakTemperatureCelsius, float irreversibleChangeTemperatureCelsius);

	void ChangeLocationOfAssociatedEnd(AAminoAcid* aminoAcidEnd, const FVector& newLocation);
	void SetParentModelAndResiduesInformation(GHProtein::ProteinModel* parentModel,
		AAminoAcid* startResidue, AAminoAcid* endResidue);
	void SetTemperature(float newTemperatureCelsius);
	bool ContainsSpecifiedResidue(const AAminoAcid* residueToLookFor);
	
	void Tick(float DeltaSeconds);

	void Stabilize();
	void Break();

private:
	//private functions
	void UpdateRendering(bool smoothInterpolate = false);

public:
	//public data members

private:
	//private variables
	FColor m_renderColor;
	float m_renderWidth;
	float m_renderHeight;
	GHProtein::ProteinModel* m_parentModel;
	bool m_wasAnimating;

	//temperature variables
	bool m_canReverseChange;
	float m_prevTemperatureCelsius;
	float m_regularTemperatureCelsius;
	float m_breakTemperatureCelsius;
	float m_irreversibleChangeTemperatureCelsius;

	float m_sizeInterpolationSpeed;
	float m_colorInterpolationSpeed;

	AAminoAcid* m_bondResidues[2];
};
