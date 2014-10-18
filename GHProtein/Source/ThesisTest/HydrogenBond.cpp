#include "ThesisTest.h"
#include "HydrogenBond.h"
#include "ProteinModel.h"
#include "AminoAcid.h"

AHydrogenBond::AHydrogenBond(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_renderColor(FColor::White)
	, m_renderWidth(0.f)
	, m_renderHeight(0.f)
	, m_wasAnimating(false)
	, m_canReverseChange(true)
	, m_prevTemperatureCelsius(0.f)
	, m_regularTemperatureCelsius(0.f)
	, m_breakTemperatureCelsius(0.f)
	, m_irreversibleChangeTemperatureCelsius(0.f)
	, m_temperatureState(ETemperatureState::ETemperatureState_Stable)
{
	m_linkType = ELinkType::ELink_HydrogenBond;
}

void AHydrogenBond::UpdateRenderProperties(const FColor& renderColor, float bondWidth, float bondHeight)
{
	m_renderColor = renderColor;
	m_renderWidth = bondWidth;
	m_renderHeight = bondHeight;

	UpdateRendering();
}

void AHydrogenBond::SetEnviromentalProperties(float currentTemperatureCelsius, float regularTemperatureCelsius,
	float breakTemperatureCelsius, float irreversibleChangeTemperatureCelsius)
{
	m_prevTemperatureCelsius = currentTemperatureCelsius;
	m_regularTemperatureCelsius = regularTemperatureCelsius;
	m_breakTemperatureCelsius = breakTemperatureCelsius;
	m_irreversibleChangeTemperatureCelsius = irreversibleChangeTemperatureCelsius;

	UpdateBondAccordingToSpecifiedTemperature(currentTemperatureCelsius);
}

bool AHydrogenBond::UpdateBondAccordingToSpecifiedTemperature(float temperatureCelsius)
{
	m_canReverseChange = true;
	bool changeInTemperatureTriggeredAnimation = false;

	if (temperatureCelsius > m_irreversibleChangeTemperatureCelsius)
	{
		if (m_temperatureState != ETemperatureState::ETemperatureState_Melting)
		{
			Break();
			changeInTemperatureTriggeredAnimation = true;
			m_temperatureState = ETemperatureState::ETemperatureState_Melting;
		}

		m_canReverseChange = false;
	}
	else if (temperatureCelsius > m_breakTemperatureCelsius)
	{
		if (m_temperatureState != ETemperatureState::ETemperatureState_Melting)
		{
			Break();
			changeInTemperatureTriggeredAnimation = true;
			m_temperatureState = ETemperatureState::ETemperatureState_Melting;
		}
	}
	else
	{
		if (m_temperatureState != ETemperatureState::ETemperatureState_Stable)
		{
			//in this case we were previously breaking and now need to repair
			Stabilize();
			changeInTemperatureTriggeredAnimation = true;
			m_temperatureState = ETemperatureState::ETemperatureState_Stable;
		}
	}

	return changeInTemperatureTriggeredAnimation;
}

void AHydrogenBond::UpdateRendering(bool smoothInterpolate)
{
	FVector endScale = FVector::ZeroVector;
	FVector size = SplineMeshComponent->StaticMesh->GetBounds().GetBox().GetSize();
	FColor renderColor = m_renderColor;
	float width = m_renderWidth;
	float height = m_renderHeight;

	if (m_linkType == ELinkType::ELink_None)
	{
		//this is indicating a link that should vanish
		width = 0.f;
		height = 0.f;
	}

	endScale.X = height / size.X;
	endScale.Y = width / size.Y;

	if (smoothInterpolate)
	{
		m_sizeInterpolator->ResetInterpolator(m_currentSizeScale, endScale, m_sizeInterpolationSpeed, false, false, 1);

		FLinearColor currentColor = FLinearColor::White;
		m_dynamicMaterial->GetVectorParameterValue("color", currentColor);

		FVector currentColorVector = currentColor;
		FVector desiredColor = renderColor.ReinterpretAsLinear();
		m_colorInterpolator->ResetInterpolator(currentColorVector, desiredColor, m_colorInterpolationSpeed, false, false, 1);
	}
	else
	{
		FVector2D newScale(endScale.X, endScale.Y);
		SplineMeshComponent->SetStartScale(newScale);
		SplineMeshComponent->SetEndScale(newScale);
		m_currentSizeScale = FVector(newScale.X, newScale.Y, 0.f);
		SetColor(renderColor);
	}
}

void AHydrogenBond::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (m_wasAnimating
		&& !IsAnimating())
	{
		//finished animating
		m_wasAnimating = IsAnimating();
		//notify the parent model that this bond is done animating
		m_parentModel->RemoveFromListOfModifiedHydrogenBonds(this);
	}
}

void AHydrogenBond::SetParentModelAndResiduesInformation(GHProtein::ProteinModel* parentModel,
	AAminoAcid* startResidue, AAminoAcid* endResidue)
{
	m_parentModel = parentModel;
	m_bondResidues[0] = startResidue;
	m_bondResidues[1] = endResidue;

	startResidue->AddHydrogenBond(this);
	endResidue->AddHydrogenBond(this);
}

bool AHydrogenBond::ContainsSpecifiedResidue(const AAminoAcid* residueToLookFor)
{
	for (int i = 0; i < 2; ++i)
	{
		if (m_bondResidues[i] == residueToLookFor)
		{
			return true;
		}
	}

	return false;
}

void AHydrogenBond::ChangeLocationOfAssociatedEnd(AAminoAcid* aminoAcidEnd, const FVector& newLocation)
{
	if (aminoAcidEnd == m_bondResidues[0])
	{
		//the residue is the starting residue
		FVector transformedLocation = SplineMeshComponent->GetComponentTransform().InverseSafe().TransformPosition(newLocation);
		SplineMeshComponent->SetStartPosition(transformedLocation);
	}
	else if (aminoAcidEnd == m_bondResidues[1])
	{
		//the residue is the end
		FVector transformedLocation = SplineMeshComponent->GetComponentTransform().InverseSafe().TransformPosition(newLocation);
		SplineMeshComponent->SetEndPosition(transformedLocation);
	}
}

bool AHydrogenBond::SetTemperature(float newTemperatureCelsius)
{
	m_prevTemperatureCelsius = newTemperatureCelsius;
	return UpdateBondAccordingToSpecifiedTemperature(newTemperatureCelsius);
}

void AHydrogenBond::Stabilize()
{
	ChangeLinkType(ELinkType::ELink_HydrogenBond, true);
	m_wasAnimating = true;
	m_parentModel->AddToListOfModifiedHydrogenBonds(this);
}

void AHydrogenBond::Break()
{
	Super::Break();
	m_wasAnimating = true;
	m_parentModel->AddToListOfModifiedHydrogenBonds(this);
}