#include "ThesisTest.h"
#include "LinkFragment.h"


ALinkFragment::ALinkFragment(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_dynamicMaterial(nullptr)
	, m_prevEndTangent(FVector::ZeroVector)
	, m_shake(false)
	, m_timeVal(0.f)
	, m_maxTime(0.15f)
	, m_minVals(FVector(0,-100, 0))
	, m_maxVals(FVector(0,100,0))
	, m_sizeInterpolator(new Interpolator())
	, m_colorInterpolator(new Interpolator())
	, testInterpolator(nullptr)
	, m_normalColor(FColor::White)
	, m_helixColor(FColor::White)
	, m_betaStrandColor(FColor::White)
	, m_hydrogenBondColor(FColor::White)
	, m_normalHeight(0.f)
	, m_normalWidth(0.f)
	, m_helixWidth(0.f)
	, m_betaStrandWidth(0.f)
	, m_hydrogenBondWidth(0.f)
	, m_linkType(ELinkType::ELink_None)
	, m_currentSizeScale(FVector::ZeroVector)
{
	PrimaryActorTick.bCanEverTick = true;
	//Create the static mesh component
	SplineMeshComponent = PCIP.CreateDefaultSubobject<USplineMeshComponent>(this, TEXT("SplineMeshComponent"));

	RootComponent = SplineMeshComponent;
}

void ALinkFragment::BeginPlay()
{
	Super::BeginPlay();

	//create the dynamic material
	m_dynamicMaterial = SplineMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
}

void ALinkFragment::SetColor(const FColor& linkColor)
{
	m_dynamicMaterial->SetVectorParameterValue("color", linkColor);
}

void ALinkFragment::Tick(float DeltaSeconds)
{
	if (m_shake)
	{
		float t = testInterpolator->Poll().X;
		m_prevEndTangent = m_minVals * (1.f - t) + m_maxVals * (t);

		SplineMeshComponent->SetEndTangent(m_prevEndTangent);
	}

	if (m_sizeInterpolator->IsPlaying())
	{
		m_currentSizeScale = m_sizeInterpolator->Poll();

		FVector2D newScale(m_currentSizeScale.X, m_currentSizeScale.Y);
		SplineMeshComponent->SetStartScale(newScale);
		SplineMeshComponent->SetEndScale(newScale);
	}

	if (m_colorInterpolator->IsPlaying())
	{
		FLinearColor newColor = m_colorInterpolator->Poll();
		SetColor(newColor);
	}
}

void ALinkFragment::ToggleShake()
{
	if (testInterpolator)
	{
		testInterpolator->TogglePlay();
	}
	else
	{
		testInterpolator = new Interpolator();
		testInterpolator->ResetInterpolator(FVector::ZeroVector, FVector(1.f, 0.f, 0.f), 0.75f, true, true);
	}

	float randVal = FMath::SRand();
	m_timeVal = randVal * m_maxTime;
	m_shake = !m_shake;
}

void ALinkFragment::ToggleBreaking()
{
	ChangeLinkType(ELinkType::ELink_None, true);
	//ChangeRenderType(ESecondaryStructure::ssCount, true);
}

void ALinkFragment::UpdateRenderProperties(const FColor& normalColor, const FColor& helixColor, const FColor& betaStrandColor,
	const FColor& hydrogenBondColor, float normalWidth, float helixWidth, float betaStrandWidth, float hydrogenBondWidth,
	float normalHeight)
{
	m_normalColor = normalColor;
	m_helixColor = helixColor;
	m_betaStrandColor = betaStrandColor;
	m_hydrogenBondColor = hydrogenBondColor;

	m_normalWidth = normalWidth;
	m_helixWidth = helixWidth;
	m_betaStrandWidth = betaStrandWidth;
	m_hydrogenBondWidth = hydrogenBondWidth;
	m_normalHeight = normalHeight;

	UpdateRendering();
}

void ALinkFragment::Hide()
{
	this->SetActorHiddenInGame(true);
}

void ALinkFragment::UpdateRendering(bool smoothInterpolate)
{
	FVector endScale = FVector::ZeroVector;
	FVector size = SplineMeshComponent->StaticMesh->GetBounds().GetBox().GetSize();
	FColor renderColor = m_normalColor;
	float width = m_normalWidth;
	float height = m_normalHeight;

	//this should be changed to link type
	switch (m_linkType)
	{
	case ELinkType::ELink_Helix:
		//calculate the size scale for the new structure
		width = m_helixWidth;
		renderColor = m_helixColor;
		break;

	case ELinkType::ELink_BetaStrand:
		//calculate the size scale for the new structure
		width = m_betaStrandWidth;
		renderColor = m_betaStrandColor;
		break;

	case ELinkType::ELink_HydrogenBond:
		width = m_hydrogenBondWidth;
		renderColor = m_hydrogenBondColor;
		break;

	case ELinkType::ELink_None:
		width = 0.0f;
		height = 0.0f;
		break;
		
	default:
		//this is the normal
		break;
	}

	endScale.X = height / size.X;
	endScale.Y = width / size.Y;

	if (smoothInterpolate)
	{
		m_sizeInterpolator->ResetInterpolator(m_currentSizeScale, endScale, 0.01f, false, false, 1);
		
		FLinearColor currentColor = FLinearColor::White;
		m_dynamicMaterial->GetVectorParameterValue("color", currentColor);
		
		FVector currentColorVector = currentColor;
		FVector desiredColor = renderColor.ReinterpretAsLinear();
		m_colorInterpolator->ResetInterpolator(currentColorVector, desiredColor, 0.01f, false, false, 1);
	}
	else
	{
		FVector2D newScale(endScale.X, endScale.Y);
		SplineMeshComponent->SetStartScale(newScale);
		SplineMeshComponent->SetEndScale(newScale);
		m_currentSizeScale = FVector(newScale.X, newScale.Y, 0.f);
	}

	SetColor(renderColor);
}

void ALinkFragment::ChangeLinkType(ELinkType::Type linkType, bool smoothInterpolate)
{
	if (m_linkType != linkType)
	{
		m_linkType = linkType;
		UpdateRendering(smoothInterpolate);
	}
}

void ALinkFragment::ChangeLinkType(ESecondaryStructure::Type secondaryStructureType, bool smoothInterpolate)
{
	ELinkType::Type linkType = UThesisStaticLibrary::GetLinkTypeFromSecondaryStructure(secondaryStructureType);

	if (m_linkType != linkType)
	{
		m_linkType = linkType;
		UpdateRendering(smoothInterpolate);
	}
}

void ALinkFragment::Translate(const FVector& deltaLocation)
{
	FVector currentLocation = GetActorLocation();
	currentLocation += deltaLocation;

	SetActorLocation(currentLocation);
}

void ALinkFragment::UpdateTangents(const FVector& startTangent, const FVector& endTangent)
{
	SplineMeshComponent->SetStartTangent(startTangent);
	SplineMeshComponent->SetEndTangent(endTangent);
}