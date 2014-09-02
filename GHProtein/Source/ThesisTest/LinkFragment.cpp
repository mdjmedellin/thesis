

#include "ThesisTest.h"
#include "LinkFragment.h"


ALinkFragment::ALinkFragment(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	//Create the static mesh component
	SplineMeshComponent = PCIP.CreateDefaultSubobject<USplineMeshComponent>(this, TEXT("SplineMeshComponent"));

	RootComponent = SplineMeshComponent;
}