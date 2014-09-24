

#include "ThesisTest.h"
#include "LinkFragment.h"


ALinkFragment::ALinkFragment(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_dynamicMaterial(nullptr)
{
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