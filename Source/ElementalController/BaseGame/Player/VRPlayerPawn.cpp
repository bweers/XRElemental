// Fill out your copyright notice in the Description page of Project Settings.


#include "./VRPlayerPawn.h"

#include "Engine.h"

// Sets default values
AVRPlayerPawn::AVRPlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CreateComponents();
}

// Called when the game starts or when spawned
void AVRPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AVRPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVRPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind actions
	PlayerInputComponent->BindAction(
		"FireRight",											// Name of action defined in project settings
		IE_Pressed,												// Enum for type of key event (key pressed, released, double tab, etc.)
		this,													// Reference to self
		&AVRPlayerPawn::StartTrackingRightControllerMovement	// Delegate function to bind action to
	);
	PlayerInputComponent->BindAction("FireRight", IE_Released, this, &AVRPlayerPawn::StopTrackingRightControllerMovement);
}

// Called to handle all component creation
void AVRPlayerPawn::CreateComponents() {
	// Create default root component for Pawn
	// USceneComponent: A component that has a transform and supports attachements,
	// but has no rendering or collision capabilities. This is useful as a dummy 
	// component for building a hierarchy tree
	USceneComponent* rootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	// define the RootComponent for this actor as our dummy rootComponent we just created.
	RootComponent = rootComponent;
	
	// Create a new component to be used for movement, based on controller input 
	// TODO: maybe remove, movement isn't needed for Elemental
	UFloatingPawnMovement* compFloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovementComp"));

	// Create a new USceneComponent that will act as a parent for the camera.
	USceneComponent* VRCameraRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VR Camera Root"));

	// Attach the camera parent component to the rootComponent
	VRCameraRootComponent->SetupAttachment(rootComponent);

	// Set transforms to "defaults"
	// More precisely, we set the relative location to (0,0,0), and the rotation to
	// (1,0,0,0). This will reset the position to the positional origin of the parent
	// component, and reset the rotation to be perfectly aligned (read: "no rotation")
	VRCameraRootComponent->SetRelativeLocationAndRotation(
		FVector::ZeroVector,	// component position: 3D vector (0,0,0)
		FQuat::Identity			// component rotation: quaternion (1,0,0,0)
	);
	VRCameraRootComponent->SetRelativeScale3D(
		FVector::OneVector		// Component scale: 3D Vector (1, 1, 1)
	);

	// Create a new UCameraComponent, which will get the motion from our HMD
	// UCameraComponent:
	UCameraComponent* VRCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("VR Camera"));

	// Attach the camera to the camera root
	VRCameraComponent->SetupAttachment(VRCameraRootComponent);

	// Set transforms
	VRCameraRootComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	VRCameraRootComponent->SetRelativeScale3D(FVector::OneVector);

	// Create left and right controller components
	// Maybe there is a better way to do this, such that we don't have to 
	// basically have the same block of code twice for each motion controller.
	if (!RightMotionControllerComponent) {
		// Create a new UMotionControllerComponent that will track a VR Controller
		RightMotionControllerComponent = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionComponent"));

		// Reset transforms
		RightMotionControllerComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
		RightMotionControllerComponent->SetRelativeScale3D(FVector::OneVector);

		RightMotionControllerComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set the source of motion for this component (it'll be an ID for one of our VR controllers)
		RightMotionControllerComponent->MotionSource = FXRMotionControllerBase::RightHandSourceId;
		// Attach motion controller to a parent component
		RightMotionControllerComponent->SetupAttachment(VRCameraRootComponent);

		CreateHandMesh(RightMotionControllerComponent, FName(TEXT("RightHandMesh")), FXRMotionControllerBase::RightHandSourceId);
	}

	if (!LeftMotionControllerComponent) {
		// Create a new UMotionControllerComponent that will track a VR Controller
		LeftMotionControllerComponent = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionComponent"));

		// Reset transforms
		LeftMotionControllerComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
		LeftMotionControllerComponent->SetRelativeScale3D(FVector::OneVector);

		LeftMotionControllerComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set the source of motion for this component (it'll be an ID for one of our VR controllers)
		LeftMotionControllerComponent->MotionSource = FXRMotionControllerBase::LeftHandSourceId;
		// Attach motion controller to a parent component
		LeftMotionControllerComponent->SetupAttachment(VRCameraRootComponent);

		CreateHandMesh(LeftMotionControllerComponent, FName(TEXT("LeftHandMesh")), FXRMotionControllerBase::LeftHandSourceId);
	}
}

// Called to add meshes to motion controller components
UStaticMeshComponent* AVRPlayerPawn::CreateHandMesh(UMotionControllerComponent* parentComponent, FName DisplayName, FName HandTypeName) {
	UStaticMeshComponent* ComponentHandReference = NULL;

	// Find default cube (that ships with engine content)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshObject(TEXT("/Engine/BasicShapes/Cube.Cube"));
	// Check if CubeMeshObject was loaded properly
	if (!CubeMeshObject.Object) {
		UE_LOG(
			LogTemp,									// Logging category
			Error,										// Message verbosity level (i.e. severity)
			TEXT("Failed to load cube for hand mesh")	// Message to log
		);
	}

	// Create mesh component
	ComponentHandReference = CreateDefaultSubobject<UStaticMeshComponent>(DisplayName);
	// Set mesh of component to CubeMeshObject
	ComponentHandReference->SetStaticMesh(CubeMeshObject.Object);

	// Set defaults
	ComponentHandReference->SetAutoActivate(true);
	ComponentHandReference->SetVisibility(true);
	ComponentHandReference->SetHiddenInGame(false);

	ComponentHandReference->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Reset transforms
	ComponentHandReference->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);

	// Set vector scale transform to something a bit smaller, 
	// since the CubeMesh is normally pretty big
	FVector vec3Scale = FVector(0.1, 0.1, 0.1);
	ComponentHandReference->SetRelativeScale3D(vec3Scale);

	// Attach StaticMesh component to a parent component
	ComponentHandReference->SetupAttachment(parentComponent);

	return ComponentHandReference;
}

// TODO: Improve the controller tracking over the duration that the 
// trigger is pressed. Pretty clunky right now.

void AVRPlayerPawn::StartTrackingRightControllerMovement() {
	// Get the world so we can access current time
	UWorld* World = GetWorld();
	
	// Get world position of right motion controller
	ControllerStartLocation = RightMotionControllerComponent->GetComponentLocation();
	// Get current time at which right motion controller trigger was pressed
	ControllerTriggerStartTime = World->GetTimeSeconds();
}

void AVRPlayerPawn::StopTrackingRightControllerMovement() {
	// Get the world so we can access current time
	UWorld* World = GetWorld();

	if (World) {
		// Get world position of right motion controller
		ControllerStopLocation = RightMotionControllerComponent->GetComponentLocation();
		// Get current time at which right motion controller trigger was released
		float ControllerTriggerStopTime = World->GetTimeSeconds();

		// Get Movement Vector (distance) of controller between trigger press and release
		FVector ControllerMovementVector =  ControllerStopLocation - ControllerStartLocation;

		// Get magnitude of controller movement vector
		float ControllerMovementMagnitude = ControllerMovementVector.Size();

		// Normalize controller movement vector
		ControllerMovementVector.Normalize();

		// Get delta time between start and stop (stop is first so our overall delta is positive)
		float DeltaTime = ControllerTriggerStopTime - ControllerTriggerStartTime;

		// Calculate velocity of controller movement given controller movement magnitude 
		// and delta time between trigger press and release
		float MovementVelocity = ControllerMovementMagnitude / DeltaTime;

		// Fire new projectile 
		Fire(ControllerMovementVector, MovementVelocity);
	}
}

// Fires a new projectile at ControllerStopLocation in NormalizedMovementVector 
// direction, with speed based on MovementVelocity.
void AVRPlayerPawn::Fire(FVector NormalizedMovementVector, float MovementVelocity) {
	// Ensure ProjectileClass is loaded
	if (ProjectileClass) {
		UWorld* World = GetWorld();

		FRotator RightControllerRotation = RightMotionControllerComponent->GetComponentRotation();

		// Ensure World is loaded
		if (World) {
			// Create new Actor spawn parameters
			FActorSpawnParameters SpawnParams;

			// Set actor owner. "Owner" is the actor responsible for spawning in 
			// this new actor
			SpawnParams.Owner = this;

			// Set actor instigator. "Instigator" is the actor responsible for 
			// creating this new actor, which may be (but not always) the owner.
			// Think of "instigator" in terms of a battle (since Unreal was 
			// originally created with FPS in mind); the instigator is the one
			// responsible for the damage done by the actor. 
			SpawnParams.Instigator = GetInstigator();

			// Spawn the projectile at ControllerStopPosition
			AProjectile* Projectile = World->SpawnActor<AProjectile>(
				ProjectileClass,			// Class to spawn in
				ControllerStopLocation,		// Location at which to spawn in actor
				RightControllerRotation,	// Rotation to spawn actor at
				SpawnParams					// Spawn paramters to spawn in actor with
			);

			// Check if projectile has successfully spawned in
			if (Projectile) {
				// Fire projectile with initial trajectory set to NormalizedMovementVector
				Projectile->FireInDirection(NormalizedMovementVector, MovementVelocity);

				
			}
		}
	}
}