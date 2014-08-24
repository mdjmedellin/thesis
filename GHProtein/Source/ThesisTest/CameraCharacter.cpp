// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ThesisTest.h"
#include "CameraCharacter.h"
#include "CameraPlayerController.h"
#include "CustomMovementComponent.h"
#include "ThesisTestProjectile.h"


ACameraCharacter::ACameraCharacter(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
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
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ACameraCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ACameraCharacter::LookUpAtRate);
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

void ACameraCharacter::MoveUp(float Value)
{
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
}

void ACameraCharacter::MoveRight(float Value)
{
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


