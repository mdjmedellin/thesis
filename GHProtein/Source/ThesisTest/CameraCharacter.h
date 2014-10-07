// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "ThesisStaticLibrary.h"
#include "Residue.h"
#include "CameraCharacter.generated.h"

namespace GHProtein
{
	class ProteinModel;
}

class AAminoAcid;

/**
 * 
 */
UCLASS()
class ACameraCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TSubobjectPtr<class UCameraComponent> FirstPersonCameraComponent;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float DefaultUpMovementRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ProteinModel)
		float m_rotationSpeedDegreesPerSecond;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = ProteinModel)
		float m_maxPickDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CustomChain)
		FVector m_customChainSlidingAxis;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CustomChain)
		float m_customChainResidueDiameter;

	GHProtein::ProteinModel* m_proteinModel;
	float m_rotationSpeedSecondsPerDegrees;
	bool m_rotateProteinYaw;
	bool m_rotateProteinPitch;
	bool m_allowCameraRotation;
	bool m_enableZoom;
	float m_zoomDirection;
	float m_zoomStep;
	float m_zoomBuffer;
	AAminoAcid* m_selectedAminoAcid;
	FVector m_prevLocation;
	TArray<AAminoAcid*> m_customChain;

protected:

	/** Handler for a touch input beginning. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Fires a projectile. */
	void OnFire();

	/** Handles moving up/down */
	void MoveUp( float Val);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/** Handles moving the protein closer and farther away from the player */
	void Zoom(float Value);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

public:
	//Functions used to modify the model the player can access
	UFUNCTION(BlueprintCallable, Category = "ProteinModel")
		void TranslateProteinModel(const FVector& translation);

	UFUNCTION(BlueprintCallable, Category = "ProteinModel")
		FVector GetProteinModelLocation();

	UFUNCTION(BlueprintCallable, Category = "PeptideChainBuilder")
		void TranslateCustomChain(const FVector& translation, int32 indexOfLastTranslatedResidue = -1);

	UFUNCTION(exec, BlueprintCallable, Category = "PeptideChainBuilder")
		void SlideCustomChain(int32 residuesToSlide = 1, int32 index = -1);

	UFUNCTION(exec, BlueprintCallable, Category = "PeptideChainBuilder")
		void AddResidueToCustomChain(TEnumAsByte<EResidueType::Type> residueType, bool translateOtherResidues = true, int32 index = -1);

	UFUNCTION(BlueprintCallable, Category = "PeptideChainBuilder")
		AAminoAcid* GetResidueAtSpecifiedIndex(int32 index = -1);

	UFUNCTION(exec)
		void ToggleShake();

	UFUNCTION(exec)
		void ToggleBreaking();

	UFUNCTION(exec)
		void HideHydrogenBonds();

	UFUNCTION(exec)
		void BreakDownFirstSpiral();

	void CustomClearJumpInput();
	virtual void ClearJumpInput();
	virtual void Tick(float DeltaSeconds) OVERRIDE;

	//Functions used to toggle rotation of the protein
	virtual void ToggleProteinYawRotation();
	virtual void ToggleProteinPitchRotation();

	//world interaction functions
	virtual void StartInteraction();
	virtual void StopInteraction();

	virtual void HandleControllerYawInput(float deltaYaw);
	virtual void HandleControllerPitchInput(float deltaPitch);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) OVERRIDE;
	virtual void PostInitializeComponents() OVERRIDE;
	virtual void Restart() OVERRIDE;
	// End of APawn interface
};
