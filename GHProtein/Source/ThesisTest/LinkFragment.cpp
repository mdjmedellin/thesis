

#include "ThesisTest.h"
#include "LinkFragment.h"


ALinkFragment::ALinkFragment(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	//Create the static mesh component
	MeshComponent = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));

	RootComponent = MeshComponent;
}