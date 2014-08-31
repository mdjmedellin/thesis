// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ThesisTestGameMode.h"
#include "ThesisTestHUD.h"
#include "CameraPlayerController.h"
#include "ProteinBuilder.h"
#include "ProteinModel.h"
#include "AminoAcid.h"

AThesisTestGameMode::AThesisTestGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	/*
	FArchive* SaveFile = IFileManager::Get().CreateFileWriter(TEXT("FINDTHISFILE.txt") );
	if (SaveFile)
	{
		FString MyString = TEXT("TestString");
		*SaveFile << MyString;
		SaveFile->Close();
		delete SaveFile;
		SaveFile = NULL;
	}
	*/

	/*FPdbFile* PdbFile = new FPdbFile();
	FString testString = "../../../ThesisData/Lysozyme.pdb";
	PdbFile->LoadFile(testString);*/

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FObjectFinder<UBlueprint> PlayerPawnObject(TEXT("/Game/Blueprints/ThesisCameraCharacter"));
	if (PlayerPawnObject.Object != NULL)
	{
		DefaultPawnClass = (UClass*)PlayerPawnObject.Object->GeneratedClass;
	}

	//Find the blueprint we are to use for the amino acids
	static ConstructorHelpers::FObjectFinder<UBlueprint> AminoAcidBlueprint(TEXT("Blueprint'/Game/Blueprints/AminoAcid.AminoAcid'"));
	if (AminoAcidBlueprint.Object)
	{
		DefaultAminoAcidClass = (UClass*)AminoAcidBlueprint.Object->GeneratedClass;
	}
	else
	{
		DefaultAminoAcidClass = nullptr;
	}

	// use our custom HUD class
	HUDClass = AThesisTestHUD::StaticClass();
	//use our custom controller
	PlayerControllerClass = ACameraPlayerController::StaticClass();
}

void AThesisTestGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	//set the initial tension for the spline we use to get the tangents of each amino acid
	AAminoAcid::SetTangentTension(0.5);

	ProteinBuilder* PdbFile = new ProteinBuilder();
	FString testString = "../../../ThesisData/Lysozyme.dssp";
	PdbFile->LoadFile(testString);
	m_proteinModel = PdbFile->GetCurrentProteinModel();
}

void AThesisTestGameMode::StartMatch()
{
	Super::StartMatch();

	//check if we received a valid protein model
	if (m_proteinModel && DefaultAminoAcidClass)
	{
		UWorld* const World = GetWorld();
		FVector locationOffset = FVector::ZeroVector;
		locationOffset.Z = 900;
		FRotator defaultRotation = FRotator::ZeroRotator;
		m_proteinModel->SpawnAminoAcids(World, DefaultAminoAcidClass, locationOffset);
	}
}