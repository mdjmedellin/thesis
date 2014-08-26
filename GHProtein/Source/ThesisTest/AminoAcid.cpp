// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "AminoAcid.h"
#include "ParticleDefinitions.h"

AAminoAcid::AAminoAcid(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{

	//Create the root SphereComponent to handle collision
	BaseCollisionComponent = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("BaseSphereComponent"));

	//Set the SphereComponent as the root component
	RootComponent = BaseCollisionComponent;

	//Create the static mesh component
	MeshComponent = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));

	//turn physics on for the static mesh
	//MeshComponent->SetSimulatePhysics(false);

	//Attach the static mesh component to the root
	MeshComponent->AttachTo(RootComponent);

	BeamParticleTemplate = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("BeamParticleTemplate"));
	BeamParticleTemplate->DeactivateSystem();			//no matter what, we do not want the template particle to be active
}

void AAminoAcid::SpawnBeamParticle(AAminoAcid* target)
{
	/*
	UParticleSystemComponent* newBeamParticle = UGameplayStatics::SpawnEmitterAttached(BeamParticleTemplate->Template,
		RootComponent,
		NAME_None,
		GetActorLocation(),
		GetActorRotation(),
		EAttachLocation::KeepWorldPosition,
		false);
		*/

	UParticleSystemComponent* PSC = NULL;
	UParticleSystem* EmitterTemplate = BeamParticleTemplate->Template;
	UObject* Actor = this;

	FVector actorLocation = GetActorLocation();
	FVector beamTargetLocation = target->GetActorLocation();

	if (EmitterTemplate)
	{
		PSC = ConstructObject<UParticleSystemComponent>(UParticleSystemComponent::StaticClass(), Actor);
		PSC->bAutoDestroy = false;
		PSC->SecondsBeforeInactive = 0.0f;
		PSC->bAutoActivate = false;
		PSC->SetTemplate(EmitterTemplate);
		PSC->bOverrideLODMethod = false;

		PSC->RegisterComponentWithWorld(this->GetWorld());

		PSC->AttachTo(RootComponent, NAME_None);
		PSC->SetWorldLocationAndRotation(GetActorLocation(), GetActorRotation());
		PSC->SetRelativeScale3D(FVector(1.f));

		PSC->ActivateSystem(true);
	}

	//The zero vector for the particle system is the place where the particle system is spawned
	//In this case, the particle system is spawned at the location of the ball
	//Therefore, in order to get the beam to point to the right location, we need to get the displacement
	//vector from this actor's location to the targetActor's location
	PSC->SetVectorParameter("TargetLocation", (beamTargetLocation - actorLocation));
}

void AAminoAcid::ReceiveActorOnClicked()
{
	int x = 1;
}
