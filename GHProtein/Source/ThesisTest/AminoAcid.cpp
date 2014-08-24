// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "AminoAcid.h"


AAminoAcid::AAminoAcid(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	//if we have a static messh assigned, then add the static mesh component to the actor
	if (StaticMeshAsset)
	{
		//MeshComponent->SetStaticMesh(StaticMeshAsset);
	}
}

void AAminoAcid::ReceiveActorOnClicked()
{
	int x = 1;
}
