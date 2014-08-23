// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ThesisTestGameMode.h"
#include "ThesisTestHUD.h"
#include "CameraPlayerController.h"
//#include "PdbParser.h"
#include "ProteinBuilder.h"
#include "ProteinModel.h"

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
	if (currentProteinModel)
	{
		UWorld* const World = GetWorld();

		currentProteinModel->SpawnAminoAcids(World);
	}
}