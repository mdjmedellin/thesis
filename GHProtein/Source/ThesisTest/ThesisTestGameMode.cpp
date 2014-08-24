// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ThesisTestGameMode.h"
#include "ThesisTestHUD.h"
#include "CameraPlayerController.h"
#include "ProteinBuilder.h"
#include "ProteinModel.h"
#include "AminoAcid.h"
//#include "PdbParser.h"
//#include "ProteinModel.h"

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

	ProteinBuilder* PdbFile = new ProteinBuilder();
	FString testString = "../../../ThesisData/Lysozyme.dssp";
	PdbFile->LoadFile(testString);
	GHProtein::ProteinModel* currentProteinModel = PdbFile->GetCurrentProteinModel();


	//check if we received a valid protein model
	if (currentProteinModel && DefaultAminoAcidClass)
	{
		UWorld* const World = GetWorld();
		FVector locationOffset = FVector::ZeroVector;
		locationOffset.Z = 900;
		FRotator defaultRotation = FRotator::ZeroRotator;
		currentProteinModel->SpawnAminoAcids<AAminoAcid>(World, DefaultAminoAcidClass, locationOffset);

		FActorSpawnParameters spawnInfo;
		spawnInfo.bNoCollisionFail = true;
		spawnInfo.Owner = NULL;
		spawnInfo.Instigator = NULL;
		spawnInfo.bDeferConstruction = false;

		//AAminoAcid* test = World->SpawnActor<AAminoAcid>(DefaultAminoAcidClass, locationOffset, defaultRotation, spawnInfo);
	}
}