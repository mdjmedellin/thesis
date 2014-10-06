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

void ALinkFragment::setColor(const FColor& linkColor)
{
	m_dynamicMaterial->SetVectorParameterValue("color", linkColor);
}

void ALinkFragment::Tick(float DeltaSeconds)
{
	if (m_shake)
	{
		float t = 0.f;
		m_timeVal += DeltaSeconds;
		if (m_timeVal > m_maxTime)
		{
			if (m_timeVal < (m_maxTime * 2.f))
			{
				float overflow = m_timeVal - m_maxTime;
				t = m_maxTime - overflow;
			}
			else
			{
				float overflow = m_timeVal - (m_maxTime * 2.f);
				t = overflow;
				m_timeVal = overflow;
			}
		}
		else
		{
			t = m_timeVal;
		}

		t /= m_maxTime;
		m_prevEndTangent = m_minVals * (1.f - t) + m_maxTime * (t);

		SplineMeshComponent->SetEndTangent(m_prevEndTangent);
	}
}

void ALinkFragment::ToggleShake()
{
	float randVal = FMath::SRand();
	m_timeVal = randVal * m_maxTime;
	m_shake = !m_shake;
}

void ALinkFragment::Hide()
{
	this->SetActorHiddenInGame(true);
}

void ALinkFragment::ChangeRenderType(ESecondaryStructure::Type linkType)
{
	switch (linkType)
	{
	case ESecondaryStructure::ssAlphaHelix:
		break;
	case ESecondaryStructure::ssStrand:
		break;
	default:
		break;
	}
}