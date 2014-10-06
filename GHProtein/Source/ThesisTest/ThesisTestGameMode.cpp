// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ThesisTestGameMode.h"
#include "ThesisStaticLibrary.h"
#include "ThesisTestHUD.h"
#include "CameraPlayerController.h"
#include "ProteinBuilder.h"
#include "ProteinModel.h"
#include "AminoAcid.h"
#include "ProteinModelSpawnPoint.h"

AThesisTestGameMode::AThesisTestGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	, m_proteinModel(nullptr)
	, m_aminoAcidSize(0.f)
	, m_proteinModelCenterLocation(FVector(0.f,0.f,0.f))
	, DefaultAminoAcidClass(nullptr)
	, m_linkWidth(100.f)
	, m_linkHeight(100.f)
	, m_distanceScale(1.f)
	, m_helixColor(FColor::White)
	, m_betaStrandColor(FColor::White)
	, m_normalColor(FColor::White)
	, m_helixLinkWidth(100.f)
	, m_betaStrandLinkWidth(100.f)
	, m_hydrogenBondLinkWidth(100.f)
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
	FString testString = "";
	bool debug_file = true;

	if (debug_file)
	{
		testString = "../../../ThesisData/Lysozyme.dssp";
	}
	else
	{
		testString = "./ThesisData/Lysozyme.dssp";
	}
	PdbFile->LoadFile(testString);
	m_proteinModel = PdbFile->GetCurrentProteinModel(GetWorld());

	/*FArchive* SaveFile = IFileManager::Get().CreateFileWriter(TEXT("FINDTHISFILE.txt"));
	if (SaveFile)
	{
		FString MyString = TEXT("TestString");
		*SaveFile << MyString;
		SaveFile->Close();
		delete SaveFile;
		SaveFile = NULL;
	}*/
}

void AThesisTestGameMode::Tick(float DeltaSeconds)
{
	Interpolator::UpdateInterpolators(DeltaSeconds);
	Super::Tick(DeltaSeconds);
}

void AThesisTestGameMode::StartMatch()
{
	Super::StartMatch();

	//Get the spawn location of the protein
	AProteinModelSpawnPoint* bestModelSpawnPoint = GetBestProteinModelSpawnPoint(EProteinSpawnPointType::ESpawn_ProteinModel);

	//check if we received a valid protein model
	if (m_proteinModel && DefaultAminoAcidClass && bestModelSpawnPoint)
	{
		m_proteinModelCenterLocation = bestModelSpawnPoint->GetActorLocation();

		UWorld* const World = GetWorld();
		m_proteinModel->SpawnAminoAcids(World, DefaultAminoAcidClass, m_aminoAcidSize, m_proteinModelCenterLocation,
			m_linkWidth, m_linkHeight, m_distanceScale, m_normalColor, m_helixColor, m_betaStrandColor,
			m_helixLinkWidth, m_betaStrandLinkWidth, m_hydrogenBondLinkWidth);
	}
}

AProteinModelSpawnPoint* AThesisTestGameMode::GetBestProteinModelSpawnPoint(EProteinSpawnPointType::Type spawnType)
{
	//at the moment we only expect to have one protein model spawn point
	if (ProteinModelSpawnPoints.Num() > 0)
	{
		for (int i = 0; i < ProteinModelSpawnPoints.Num(); ++i)
		{
			if (ProteinModelSpawnPoints[i]->m_typeOfSpawnPoint == spawnType)
			{
				return ProteinModelSpawnPoints[i];
			}
		}
		return nullptr;
	}

	return nullptr;
}

void AThesisTestGameMode::AddProteinModelSpawnPoint(AProteinModelSpawnPoint* NewProteinModelSpawnPoint)
{
	ProteinModelSpawnPoints.AddUnique(NewProteinModelSpawnPoint);
}

void AThesisTestGameMode::RemoveProteinModelSpawnPoint(AProteinModelSpawnPoint* RemovedProteinModelSpawnPoint)
{
	ProteinModelSpawnPoints.Remove(RemovedProteinModelSpawnPoint);
}