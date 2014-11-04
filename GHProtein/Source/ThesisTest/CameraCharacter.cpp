// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "ProteinUtilities.h"
#include "Residue.h"
#include "CameraCharacter.h"
#include "CameraPlayerController.h"
#include "CustomMovementComponent.h"
#include "ThesisTestGameMode.h"
#include "AminoAcid.h"
#include "ProteinModel.h"

ACameraCharacter::ACameraCharacter(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
	, m_rotationSpeedDegreesPerSecond(0.f)
	, m_movementSpeedPerSecond(0.f)
	, m_maxPickDistance(0.f)
	, m_allowCameraRotation(true)
	, m_selectedAminoAcid(nullptr)
	, m_prevLocation(FVector::ZeroVector)
	, m_enableZoom(false)
	, m_zoomDirection(0.f)
	, m_zoomStep(100.f)
	, m_zoomBuffer(100.f)
	, m_xDirection(0.f)
	, m_yDirection(0.f)
	, m_rotateProteinPitch(0.f)
	, m_rotateProteinYaw(0.f)
	, m_allowInput(true)
	, m_proteinModel(nullptr)
	, m_customChainModel(nullptr)
	, m_indexOfCustomChainResidueCurrentlyFocusedOn(0)
{
	// Set size for collision capsule
	// We actually want the  player to be a floating camera
	// Size for now is 42 by 42 of collision capsule
	CapsuleComponent->InitCapsuleSize(42.f, 21.f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	DefaultUpMovementRate = 5.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = CapsuleComponent;
	// We want the camera to be in the middle of the character
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 21.f); // Position the camera
}

void ACameraCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//attempt to modify the CharacterMovementComponent
	CharacterMovement->DefaultLandMovementMode = MOVE_Flying;
}

//////////////////////////////////////////////////////////////////////////
// Input
void ACameraCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Fire", IE_Pressed, this, &ACameraCharacter::OnFire);

	InputComponent->BindAxis("MoveForward", this, &ACameraCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACameraCharacter::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ACameraCharacter::MoveUp);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &ACameraCharacter::HandleControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ACameraCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ACameraCharacter::HandleControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ACameraCharacter::LookUpAtRate);

	//Custom protein zoom
	InputComponent->BindAxis("ZoomIn", this, &ACameraCharacter::Zoom);

	//Temperature
	InputComponent->BindAxis("ModifyTemperature", this, &ACameraCharacter::ModifyTemperatureInModel);

	//Custom protein translation
	InputComponent->BindAxis("TranslateModelX", this, &ACameraCharacter::TranslateModelX);
	InputComponent->BindAxis("TranslateModelY", this, &ACameraCharacter::TranslateModelY);

	//Custom Protein rotation
	InputComponent->BindAxis("RotateProteinYaw", this, &ACameraCharacter::RotateProteinYaw);
	InputComponent->BindAxis("RotateProteinPitch", this, &ACameraCharacter::RotateProteinPitch);
}

void ACameraCharacter::HandleControllerYawInput(float deltaYaw)
{
	if (m_allowCameraRotation)
	Super::AddControllerYawInput(deltaYaw);
}

void ACameraCharacter::HandleControllerPitchInput(float deltaPitch)
{
	if (m_allowCameraRotation)
	Super::AddControllerPitchInput(deltaPitch);
}

void ACameraCharacter::ClearJumpInput()
{
	//Do Nothing, this is in order to prevent the controller from resetting the move up command from some weird place
	int x = 1;
}

void ACameraCharacter::CustomClearJumpInput()
{
	Super::ClearJumpInput();
}

void ACameraCharacter::OnFire()
{
	ACameraPlayerController* localCameraController = Cast<ACameraPlayerController>(Controller);

	if ( localCameraController != nullptr )
	{
		if (localCameraController->bCanPickupItem)
		{
			localCameraController->ActivatePhysicsOnActor();	
		}
	}
}

void ACameraCharacter::TranslateModelX(float x_direction)
{
	if (m_allowInput)
	{
		m_xDirection = x_direction;
	}
}

void ACameraCharacter::TranslateModelY(float y_direction)
{
	if (m_allowInput)
	{
		m_yDirection = y_direction;
	}
}

void ACameraCharacter::RotateProteinYaw(float yawRotation)
{
	if (m_allowInput)
	{
		m_rotateProteinYaw = yawRotation;
	}
}

void ACameraCharacter::RotateProteinPitch(float pitchRotation)
{
	if (m_allowInput)
	{
		m_rotateProteinPitch = pitchRotation;
	}
}

void ACameraCharacter::Zoom(float Value)
{
	if (Value != 0.f)
	{
		m_zoomDirection = Value;
		m_enableZoom = true;
	}
	else
	{
		m_enableZoom = false;
	}
}

void ACameraCharacter::MoveUp(float Value)
{
	static bool wasPreviouslyZero = (Value == 0.f);

	if (Value != 0.f)
	{
		// find out which way is up
		const FRotator Rotation = GetControlRotation();
		const FRotator PitchYawRotation(Rotation.Pitch, Rotation.Yaw, 0);

		// Get right vector
		const FVector Direction = FRotationMatrix(PitchYawRotation).GetUnitAxis(EAxis::Z);

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
	else
	{
		//check if this axis was prevously giving an acceleration
		if (!wasPreviouslyZero)
		{
			//if we do not allow for camera rotation, then we are in the process of moving a part of the protein
			//therefore, we do not allow for the usual drag that the flying camera does when moving around the world
			GetMovementComponent()->StopMovementImmediately();
		}
	}

	wasPreviouslyZero = (Value == 0.f);
}

void ACameraCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// only fire for first finger down
	if (FingerIndex == 0)
	{
		OnFire();
	}
}

void ACameraCharacter::MoveForward(float Value)
{
	static bool wasPreviouslyZero = (Value == 0.f);

	//NOTE: This does not apply acceleration
	if (Value != 0.0f)
	{
		// find out which way is forward
		const FRotator Rotation = GetControlRotation();
		FRotator PitchYawRotation(Rotation.Pitch, Rotation.Yaw, 0);

		// Get forward vector
		const FVector Direction = FRotationMatrix(PitchYawRotation).GetUnitAxis(EAxis::X);

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
	else
	{
		if (!wasPreviouslyZero)
		{
			//if we do not allow for camera rotation, then we are in the process of moving a part of the protein
			//therefore, we do not allow for the usual drag that the flying camera does when moving around the world
			GetMovementComponent()->StopMovementImmediately();
		}
	}

	wasPreviouslyZero = (Value == 0.f);
}

void ACameraCharacter::MoveRight(float Value)
{
	static bool wasPreviouslyZero = (Value == 0.f);

	if (Value != 0.0f)
	{
		// find out which way is right
		const FRotator Rotation = GetControlRotation();
		const FRotator PitchYawRotation(Rotation.Pitch, Rotation.Yaw, 0);

		// Get right vector
		const FVector Direction = FRotationMatrix(PitchYawRotation).GetUnitAxis(EAxis::Y);

		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
	else
	{
		if (!wasPreviouslyZero)
		{
			//if we do not allow for camera rotation, then we are in the process of moving a part of the protein
			//therefore, we do not allow for the usual drag that the flying camera does when moving around the world
			GetMovementComponent()->StopMovementImmediately();
		}
	}

	wasPreviouslyZero = (Value == 0.f);
}

void ACameraCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACameraCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACameraCharacter::Restart()
{
	AThesisTestGameMode* gameMode = (AThesisTestGameMode*)GetWorld()->GetAuthGameMode();

	m_proteinModel = gameMode->m_proteinModel;
	m_customChainModel = gameMode->m_customChainModel;

	//translate the custom chain model to the start
	if (m_customChainModel)
	{
		TranslateCustomChainToSpecifiedResidue(m_indexOfCustomChainResidueCurrentlyFocusedOn);
	}
}

void ACameraCharacter::UpdateModelRotation(float DeltaSeconds)
{
	float deltaDegrees = m_rotationSpeedDegreesPerSecond * DeltaSeconds;
	FVector degreesRotated(m_rotateProteinPitch, m_rotateProteinYaw, 0.f);
	
	if (degreesRotated != FVector::ZeroVector)
	{
		degreesRotated.Normalize();
		degreesRotated *= deltaDegrees;
		m_proteinModel->RotateModel(degreesRotated);
	}
}

void ACameraCharacter::UpdateModelLocation(float DeltaSeconds)
{
	float translationDistance = m_movementSpeedPerSecond * DeltaSeconds;
	FVector translationVector(m_xDirection, m_yDirection, 0.f);

	if (translationVector != FVector::ZeroVector)
	{
		translationVector.Normalize();
		translationVector *= translationDistance;
		m_proteinModel->TranslateModel(translationVector);
	}
}

void ACameraCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateModelRotation(DeltaSeconds);
	UpdateModelLocation(DeltaSeconds);
}

void ACameraCharacter::TranslateProteinModel(const FVector& translation, bool interpolate, float speedMultiplier)
{
	m_proteinModel->TranslateModel(translation, interpolate, speedMultiplier);
}

FVector ACameraCharacter::GetProteinModelLocation()
{
	return m_proteinModel->GetCenterLocation();
}

void ACameraCharacter::AddResidueToCustomChain(TEnumAsByte<EResidueType::Type> residueType, bool translateOtherResidues, int32 index)
{
	//add the residue at the back of the chain
	UWorld* world = GetWorld();
	AThesisTestGameMode* gameMode = nullptr;
	if (world)
	{
		gameMode = (AThesisTestGameMode*)world->GetAuthGameMode();
		if (gameMode)
		{
			FVector dimensions = FVector::ZeroVector;
			FVector location = FVector::ZeroVector;
			if (m_customChain.Num() != 0)
			{
				if (index < 0 || index >= m_customChain.Num())
				{
					//add to the end of the chain
					location = m_customChain.Last()->GetActorLocation();
					//check if we are to slide the other amino acids or just offset the amino acid that is added to the chain
					if (translateOtherResidues)
					{
						SlideCustomChain(1);
					}
					else
					{
						dimensions.Set(m_customChainResidueDiameter, m_customChainResidueDiameter, m_customChainResidueDiameter);
					}
				}
				else
				{
					location = m_customChain[index]->GetActorLocation();

					//slide all of the other actors coming after this location
					SlideCustomChain(1, index);
				}
			}
			else
			{
				//if the chain is empty, then we spawn the custom peptyde chain at the best spawn point
				AProteinModelSpawnPoint* bestSpawnPoint = gameMode->GetBestProteinModelSpawnPoint(EProteinSpawnPointType::ESpawn_CustomPolypeptide);

				if (bestSpawnPoint)
				{
					location = bestSpawnPoint->GetActorLocation();
				}
			}

			dimensions *= m_customChainSlidingAxis;
			location += dimensions;

			AAminoAcid* newAminoAcid = UThesisStaticLibrary::SpawnBP<AAminoAcid>(world, gameMode->DefaultAminoAcidClass
				, location, FRotator::ZeroRotator);

			newAminoAcid->SetAminoAcidType(residueType);
			newAminoAcid->SetAminoAcidSize(m_customChainResidueDiameter);

			//Handles edge case where the index given is not valid
			if (index < 0 
				|| (index + 1 >= m_customChain.Num()))
			{
				m_customChain.Add(newAminoAcid);
			}
			else
			{
				m_customChain.Insert(newAminoAcid, index+1);
			}
		}
	}
}

void ACameraCharacter::EraseCustomChain()
{
	for (int i = 0; i < m_customChain.Num(); ++i)
	{
		m_customChain[i]->Destroy();
	}

	m_customChain.Empty();
}

void ACameraCharacter::RemoveResidueFromCustomChain(int32 index)
{
	if (m_customChain.Num() > 0)
	{
		AAminoAcid* residueToDestroy = nullptr;
		//any negative number removes the residue at the end of the chain
		if (index < 0
			|| index == (m_customChain.Num() - 1) )
		{
			residueToDestroy = m_customChain.Last();
			SlideCustomChain(-1);
			m_customChain.RemoveAt(m_customChain.Num() - 1);
		}
		else if (index == 0)
		{
			residueToDestroy = m_customChain[0];
			SlideCustomChain(1);
			m_customChain.RemoveAt(0);
		}
		else if (index < m_customChain.Num())
		{
			residueToDestroy = m_customChain[index];
			m_customChain.RemoveAt(index);
			SlideCustomChain(-1, index);
		}

		residueToDestroy->Destroy();
	}
}

void ACameraCharacter::EscapeFromPredictionMode()
{
	if (m_customChainModel)
	{
		delete m_customChainModel;
		m_customChainModel = nullptr;
	}

	//align the amino acids in a single row
	UWorld* world = GetWorld();
	AThesisTestGameMode* gameMode = nullptr;
	if (world)
	{
		gameMode = (AThesisTestGameMode*)world->GetAuthGameMode();
		if (gameMode)
		{
			//if the chain is empty, then we spawn the custom peptyde chain at the best spawn point
			AProteinModelSpawnPoint* bestSpawnPoint = gameMode->GetBestProteinModelSpawnPoint(EProteinSpawnPointType::ESpawn_CustomPolypeptide);
			FVector spawnLocation = FVector::ZeroVector;

			if (bestSpawnPoint)
			{
				spawnLocation = bestSpawnPoint->GetActorLocation();
			}

			FVector aminoAcidLocation = FVector::ZeroVector;
			float offsetDistance = m_customChainResidueDiameter;

			for (int i = 0; i < m_customChain.Num(); ++i)
			{
				aminoAcidLocation = spawnLocation + (offsetDistance * i * m_customChainSlidingAxis);
				m_customChain[i]->SetActorLocation(aminoAcidLocation);
				m_customChain[i]->SetAminoAcidSize(m_customChainResidueDiameter);
			}
		}

		//slide chain to focus on the residue we were looking at before we exited prediction mode
		SlideCustomChain(m_indexOfCustomChainResidueCurrentlyFocusedOn);
	}
}

void ACameraCharacter::PredictSecondaryStructureOfCustomChain(int32 indexOfResidueToFocusOn)
{
	UWorld* world = GetWorld();
	AThesisTestGameMode* gameMode = nullptr;

	if (world 
		&& m_customChain.Num() > 0)
	{
		gameMode = (AThesisTestGameMode*)world->GetAuthGameMode();

		m_customChainModel = gameMode->PredictSecondaryStructure(m_customChain);

		//check if we have a valid model from the chain
		if (m_customChainModel)
		{
			TranslateCustomChainToSpecifiedResidue(indexOfResidueToFocusOn);
		}
	}
}

void ACameraCharacter::TranslateCustomChainToSpecifiedResidue(int32 indexOfResidueToFocusOn)
{
	UWorld* world = GetWorld();
	AThesisTestGameMode* gameMode = nullptr;
	if (world
		&& m_customChain.Num() > 0)
	{
		gameMode = (AThesisTestGameMode*)world->GetAuthGameMode();
		if (gameMode)
		{
			//translate the chain to focus on the residue at the specified index
			if (indexOfResidueToFocusOn < 0)
			{
				m_indexOfCustomChainResidueCurrentlyFocusedOn = 0;
			}
			else if (indexOfResidueToFocusOn >= m_customChain.Num())
			{
				m_indexOfCustomChainResidueCurrentlyFocusedOn = m_customChain.Num() - 1;
			}
			else
			{
				m_indexOfCustomChainResidueCurrentlyFocusedOn = indexOfResidueToFocusOn;
			}

			//get the location of the residue to focus on
			FVector locationOfResidueToFocusOn = m_customChain[m_indexOfCustomChainResidueCurrentlyFocusedOn]->GetActorLocation();
			//calculate the displacement needed to translate the desired residue into the focus point
			AProteinModelSpawnPoint* bestSpawnPoint = gameMode->GetBestProteinModelSpawnPoint(EProteinSpawnPointType::ESpawn_CustomPolypeptide);
			if (bestSpawnPoint)
			{
				FVector displacement = bestSpawnPoint->GetActorLocation() - locationOfResidueToFocusOn;
				//we only want to displace along the y axis
				displacement.X = 0.f;
				displacement.Z = 0.f;

				//displace the entire chain
				TranslateCustomChain(displacement);
			}
		}
	}
}

void ACameraCharacter::AddResiduesInFileToCustomChain(const FString& fileLocation)
{
	//check the extension of the file
	int index = 0;

	if (fileLocation.FindLastChar('.', index))
	{
		//get a substring in order to compare the externsion of the file
		FString fileExtension = "";
		fileExtension = fileLocation.RightChop(index + 1);

		//check what type of file it is
		//at the moment we only support .jm
		FString fileDirLocation = FPaths::GameContentDir() + fileLocation;

		if (fileExtension == "jm")
		{
			TArray<FString> stringArray;
			FFileHelper::LoadANSITextFileToStrings(*(fileDirLocation), NULL, stringArray);
			FString currentString = "";

			for (int i = 0; i < stringArray.Num(); i+=3)
			{
				currentString = stringArray[i];

				for (int j = 0; j < currentString.Len(); ++j)
				{
					if (!GHProtein::IsWhiteSpace(currentString[j]))
					{
						//
						EResidueType::Type residueType = MapResidue(currentString[j]);
						AddResidueToCustomChain(residueType);
					}
				}
			}
		}
	}
}

void ACameraCharacter::TranslateCustomChain(const FVector& translation, int32 indexOfLastTranslatedResidue)
{
	if (indexOfLastTranslatedResidue < 0 || indexOfLastTranslatedResidue >= m_customChain.Num())
	{
		indexOfLastTranslatedResidue = m_customChain.Num() - 1;
	}

	for (int i = 0; i <= indexOfLastTranslatedResidue; ++i)
	{
		m_customChain[i]->Translate(translation);
	}
}

void ACameraCharacter::SlideCustomChain(int32 residuesToSlide, int32 index)
{
	//we only slide the custom chain by the residue's diameter when there is no model produced from it
	//otherwise we use the location fo the residue we plan on focusing on
	FVector translationOffset = FVector::ZeroVector;

	translationOffset.Set(-m_customChainResidueDiameter, -m_customChainResidueDiameter, -m_customChainResidueDiameter);
	translationOffset *= m_customChainSlidingAxis;
	translationOffset *= residuesToSlide;

	if (m_customChainModel)
	{
		m_indexOfCustomChainResidueCurrentlyFocusedOn += residuesToSlide;
		
		TranslateCustomChainToSpecifiedResidue(m_indexOfCustomChainResidueCurrentlyFocusedOn);
	}
	else
	{
		TranslateCustomChain(translationOffset, index);
	}
}

AAminoAcid* ACameraCharacter::GetResidueAtSpecifiedIndex(int32 index)
{
	if (index < 0 || index >= m_customChain.Num())
	{
		return nullptr;
	}
	else
	{
		return m_customChain[index];
	}
}

void ACameraCharacter::ToggleShake()
{
	m_proteinModel->ToggleShake();
}

void ACameraCharacter::ToggleBreaking()
{
	m_proteinModel->ToggleBreaking();
}

void ACameraCharacter::HideHydrogenBonds()
{
	m_proteinModel->HideHydrogenBonds();
}

void ACameraCharacter::SetModelTemperature(float temperatureCelsius)
{
	m_proteinModel->SetTemperature(temperatureCelsius);
}

void ACameraCharacter::ModifyTemperatureInModel(float temperatureModifierScale)
{
	m_proteinModel->ModifyTemperature(temperatureModifierScale);
}

float ACameraCharacter::GetModelTemperature()
{
	return m_proteinModel->GetCurrentTemperature();
}

void ACameraCharacter::ToggleProteinInputs(bool inputsState)
{
	m_allowInput = inputsState;

	if (!m_allowInput)
	{
		//disable all of the input
		m_rotateProteinPitch = 0.f;
		m_rotateProteinYaw = 0.f;
		m_xDirection = 0.f;
		m_yDirection = 0.f;
	}
}