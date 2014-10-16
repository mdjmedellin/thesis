// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
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

	//InputComponent->BindAction( "Jump" , IE_Pressed, this, &ACameraCharacter::MoveUp);
	//InputComponent->BindAction("Jump", IE_Released, this, &ACameraCharacter::MoveUp);

	InputComponent->BindAction("Fire", IE_Pressed, this, &ACameraCharacter::OnFire);
	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AThesisTestCharacter::TouchStarted);

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

	//The following is not used at the moment
	/*
	FVector currentLocation = GetActorLocation();
	//check if the model is zooming in or out
	if (m_enableZoom)
	{
		FVector direction = m_proteinModel->GetDirectionFromCenter(currentLocation);

		//check if we should some in more
		FVector halfDimensions = m_proteinModel->GetBoundingBoxDimensions() * 0.5f;
		float maxDimension = halfDimensions.GetMax() + m_zoomBuffer;

		maxDimension *= maxDimension;

		if (direction.SizeSquared() > maxDimension
			|| m_zoomDirection < 0.f)
		{
			direction.Normalize();
			direction *= m_zoomDirection;
			direction *= m_zoomStep;
			m_proteinModel->TranslateModel(direction);
		}
	}
	*/
}

void ACameraCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateModelRotation(DeltaSeconds);
	UpdateModelLocation(DeltaSeconds);
}

void ACameraCharacter::TranslateProteinModel(const FVector& translation)
{
	m_proteinModel->TranslateModel(translation);
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

			//
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
	FVector translationOffset(-m_customChainResidueDiameter, -m_customChainResidueDiameter, -m_customChainResidueDiameter);
	translationOffset *= m_customChainSlidingAxis;

	translationOffset *= residuesToSlide;
	TranslateCustomChain(translationOffset, index);
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

void ACameraCharacter::ToggleProteinInputs()
{
	m_allowInput = !m_allowInput;

	if (!m_allowInput)
	{
		//disable all of the input
		m_rotateProteinPitch = 0.f;
		m_rotateProteinYaw = 0.f;
		m_xDirection = 0.f;
		m_yDirection = 0.f;
	}
}