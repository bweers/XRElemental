// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

// Prerequisite header files for SteamVR 
#include "HeadMountedDisplay.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h"

#include "../Projectile/Projectile.h"
#include "VRPlayerPawn.generated.h"

UCLASS()
class ELEMENTALCONTROLLER_API AVRPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVRPlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = Projectile)
		TSubclassOf<class AProjectile> ProjectileClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called to handle all component creation
	void CreateComponents();

	// Right motion controller component for VR Pawn
	UPROPERTY(VisibleAnywhere, Category = MotionController)
		UMotionControllerComponent* RightMotionControllerComponent;

	// Left motion controller component for VR Pawn
	UPROPERTY(VisibleAnywhere, Category = MotionController)
		UMotionControllerComponent* LeftMotionControllerComponent;

	// Called to add meshes to motion controller components
	UStaticMeshComponent* CreateHandMesh(UMotionControllerComponent* parentComponent, FName DisplayName, FName HandTypeName);

	// Controller position when trigger is first pressed
	UPROPERTY(VisibleAnywhere, Category = MovementTracking)
		FVector ControllerStartLocation;

	// Controller position when trigger is released
	UPROPERTY(VisibleAnywhere, Category = MovementTracking)
		FVector ControllerStopLocation;

	// Time, in seconds, of when controller is first pressed. Used 
	// to calcualte velocity of controller over duration that trigger 
	// is pressed.
	UPROPERTY(VisibleAnywhere, Category = MovementTracking)
		float ControllerTriggerStartTime;

	// Handles the start of movement tracking on trigger press
	// (i.e. saves starting position)
	UFUNCTION()
		void StartTrackingRightControllerMovement();

	// Handles the end of movement tracking on trigger release
	// (i.e. saves ending position)
	UFUNCTION()
		void StopTrackingRightControllerMovement();

	// Fires a new projectile at ControllerStopPosition in 
	// NormalizedMovementVector direction, with speed based on 
	// MovementVelocity.
	void Fire(FVector MovementVector, float MovementVelocity);
};
