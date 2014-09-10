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
	, m_aminoAcidSize(0.f)
	, m_proteinModelCenterLocation(FVector(0.f,0.f,0.f))
	, DefaultAminoAcidClass(nullptr)
	, m_aminoAcidBlueprint(nullptr)
	, m_linkWidth(100.f)
	, m_linkHeight(100.f)
	, m_distanceScale(1.f)
	, m_helixColor(FColor::White)
	, m_betaStrandColor(FColor::White)
	, m_helixLinkWidth(100.f)
	, m_betaStrandLinkWidth(100.f)
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
	/*
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
	*/

	/*
	static ConstructorHelpers::FObjectFinder<UBlueprint> AminoAcidBlueprint(TEXT("Blueprint'/Game/Blueprints/AminoAcid.AminoAcid'"));
	if (AminoAcidBlueprint.Object)
	{
		DefaultAminoAcidClass = (UClass*)AminoAcidBlueprint.Object->GeneratedClass;
	}
	else
	{
		DefaultAminoAcidClass = nullptr;
	}
	*/
}

void AThesisTestGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	//Load the pdb file used to create the protein model and parse the data inside of it
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
		m_proteinModel->SpawnAminoAcids(World, DefaultAminoAcidClass, m_aminoAcidSize, m_proteinModelCenterLocation
			, m_linkWidth, m_linkHeight, m_distanceScale, m_helixColor, m_betaStrandColor, m_helixLinkWidth, m_betaStrandLinkWidth);
	}
}