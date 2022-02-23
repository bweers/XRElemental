// Fill out your copyright notice in the Description page of Project Settings.


#include "./Projectile.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  
	// You can turn this off to improve performance if 
	// you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize root component
	if (!RootComponent) {
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileRoot"));
	}

	// Initialize projectile collision sphere
	if (!ProjectileCollisionComponent) {
		// Create a new sphere, as a simple collision representation
		ProjectileCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileSphereComponent"));

		// Set collision profile to "Projectile", a custom collision profile we 
		// made that will allow the projectile to collide with everything in the 
		// world EXCEPT for pawns.
		ProjectileCollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
		
		// Set the spheres collision radius
		ProjectileCollisionComponent->InitSphereRadius(15.f);

		TArray<AActor*> IgnoreArray;

		IgnoreArray.Add(GetOwner());

		ProjectileCollisionComponent->MoveIgnoreActors = IgnoreArray;
		
		// Set root component to this collision component
		RootComponent = ProjectileCollisionComponent;
	}

	// Initialize projectile movement component
	if (!ProjectileMovementComponent) {
		// This component is used to drive the projectile's movement
		// Create a new UProjectileMovementComponent
		ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
		
		// Assign the Component to update and move
		ProjectileMovementComponent->SetUpdatedComponent(ProjectileCollisionComponent);
		
		// Set initial speed of component
		ProjectileMovementComponent->InitialSpeed = 175.f;
		
		// Set maximum possible speed of component
		ProjectileMovementComponent->MaxSpeed = 1000.f;
		
		// update rotation of component each frame to match the 
		// direction of velocity vector
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		
		// simulate simple bounces with component
		ProjectileMovementComponent->bShouldBounce = true;
		
		// set "bounciness" factor. This factor represents the percentage
		// of velocity mainted after the bounch in the direction of the 
		// normal of impact (i.e. coefficient of restitution)
		ProjectileMovementComponent->Bounciness = 0.3f;
		
		// set gravity scale. This factor represents the percentage at 
		// which gravity affects the direction of velocity of this 
		// component.
		ProjectileMovementComponent->ProjectileGravityScale = 0.1f;
	}

	// Initalize projectile mesh component
	if (!ProjectileMeshComponent) {
		// Create a new UStaticMeshComponent
		ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));

		// Retrieve new mesh from disk
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Game/Assets/Projectile/ProjectileSphere.ProjectileSphere"));
		
		// Check if Mesh successfully loaded
		if (Mesh.Succeeded()) {
			// Set projectile mesh component to loaded mesh
			ProjectileMeshComponent->SetStaticMesh(Mesh.Object);
		}

		// Retrieve new material from disk
		static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Game/Assets/Projectile/ProjectileMaterial.ProjectileMaterial"));

		// Check if Material successfully loaded
		if (Material.Succeeded()) {
			// Create a new material instance using the Material we just loaded
			// and the projectile mesh
			ProjectileMaterialInstance = UMaterialInstanceDynamic::Create(Material.Object, NULL);
		}
		else UE_LOG(LogTemp, Warning, TEXT("Material failed to laod"));

		// Set the material of projectile mesh
		ProjectileMeshComponent->SetMaterial(
			0,							// Element index to access material of
			ProjectileMaterialInstance	// Material to change element to
		);

		// Set scale of projectile mesh, relative to parent 
		ProjectileMeshComponent->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));

		// Attach projectile mesh to RootComponent
		ProjectileMeshComponent->SetupAttachment(RootComponent);
	}

	if (!ProjectilePointLightComponent) {
		// Create a new UPointLightComponent
		ProjectilePointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("ProjectileLightComponent"));

		// Set point light intensity
		ProjectilePointLightComponent->Intensity = 3000.f;

		// Set point light attenuation radius
		ProjectilePointLightComponent->AttenuationRadius = 200.f;

		// Set point light color
		ProjectilePointLightComponent->SetLightColor(FLinearColor(40.f,4.f,0.4f,1.f));

		// Attach to mesh component
		ProjectilePointLightComponent->SetupAttachment(RootComponent);
	}

	// Initalize niagara particle system component
	if (!ProjectileNiagaraComponent) {
		// Create a new UNiagaraComponent 
		ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));

		// Load a new NiagaraSystem from Content files.
		// NiagaraSystem:
		UNiagaraSystem* ProjectileNiagaraFlameSystem = LoadObject<UNiagaraSystem>(
			nullptr,							// Optional object to narrow where to find/load the object from
			TEXT("NS_Projectile"),	// Name of object to load
			TEXT("/Game/Assets/Projectile/")	// Optional file to load from 
		);

		// Inisitalize default FireColor if not already instantiated
		if (! &FireColor) {
			FireColor = FLinearColor(40.f, 4.f, 0.4f, 1.f);
		}

		// Set Niagara system variable of type FLinearColor to FireColor UPROPERTY
		ProjectileNiagaraComponent->SetNiagaraVariableLinearColor(TEXT("FireColor"), FireColor);

		// Attach NiagaraComponent to RootComponent
		ProjectileNiagaraComponent->SetupAttachment(RootComponent);
	}
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Event called when projectile collision component hits something
	ProjectileCollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

	// Create a new TimerManager to handle destruction timer
	FTimerHandle DestroyTimerHandle;

	// Wait about 5.8 seconds before calling the StopProjectileInteraction function, 
	// which will initiate the destruction of the particle
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AProjectile::StopProjectileInteraction, 5.8f, false);
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Initializes the projectile's velocity, in the direction it 
// is to be shot.
void AProjectile::FireInDirection(const FVector& ShootDirection, float ShootVelocity) {
	// MinimumShootVelocity and MaximumShootVelocity represent the values that 
	//ShootVelocity should be clipped between, should it be outside of that range.
	MinimumShootVelocity = 0.f;
	MaximumShootVelocity = 500.f;

	// Clip the ShootVelocity such that it is between MinimumShootVelocity 
	// and MaximumShootVelocity
	float ClippedShootVelocity = FMath::Clamp(ShootVelocity, MinimumShootVelocity, MaximumShootVelocity);

	// MinimumShootVelocityRange and MaximumShootVelocityRange represent the range 
	// that ClippedShootVelocity will be normalized within, after being clipped
	MinimumShootVelocityRange = 1.f;
	MaximumShootVelocityRange = 5.5f;

	// Normalize ClippedShootVelocity between MinimumShootVelocityRange and 
	// MaximumShootVelocityRange 
	float NormalizedShootVelocity = 
		(MaximumShootVelocityRange - MinimumShootVelocityRange) * 
		((ClippedShootVelocity - MinimumShootVelocity) / (MaximumShootVelocity - MinimumShootVelocity)) + 
		MinimumShootVelocityRange;

	// Shoot the projectile at the speed defined by the 
	// ProjectileMovementComponent in the ShootDirection. We only need 
	// to supply a direction to shoot in because the speed at which 
	// the projectile is shot is defined by ProjectileMovementComponent.
	ProjectileMovementComponent->Velocity = ShootDirection * (ProjectileMovementComponent->InitialSpeed * NormalizedShootVelocity);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {

	const float MinimumVelocityMagnitude = 100.f;
	const float ExplosionScaleFactor = 1000.f;

	// Check that hit actor isn't the projectile itself
	if (OtherActor != this && OtherActor != this->GetInstigator()) {

		float VelocityMagnitude = this->GetVelocity().Size();

		// Check if other component is simulating physics
		if (OtherComponent->IsSimulatingPhysics() && OtherComponent->IsCollisionEnabled()) {

			// Check if explosion system is valid, and that velocity is high enough
			if (ProjectileExplosionSystem->IsValid() && VelocityMagnitude >= MinimumVelocityMagnitude) {
				// Spawn a new Niagara system comprised of a explosion particle system
				UNiagaraComponent* ProjectileExplosionSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ProjectileExplosionSystem,
					Hit.ImpactPoint,
					FRotator::ZeroRotator,
					FVector((VelocityMagnitude - MinimumVelocityMagnitude) / ExplosionScaleFactor)
				);
			}

			// Add an impulse at the location of hit
			OtherComponent->AddImpulseAtLocation(
				ProjectileMovementComponent->Velocity * 100.f,	// Force of impact
				Hit.ImpactPoint									// Location of impact
			);

			StopProjectileInteraction();
		}
	}
}

void AProjectile::StopProjectileInteraction() {
	// Disable collision and mesh so the projectile doesn't interact 
	// with the world anymore.
	ProjectileCollisionComponent->DestroyComponent();
	ProjectileMeshComponent->DestroyComponent();

	// Deactivate the Niagara System, so we don't keep generating new particles
	ProjectileNiagaraComponent->Deactivate();

	// Create a new TimerManager to handle timer
	FTimerHandle UnusedHandle;

	// Wait about 1.2 seconds before calling the DestroySelf function, 
	// since that is the maximum time it would take for any remaining 
	// particles to despawn.
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProjectile::DestroySelf, 1.2f, false);
}

void AProjectile::DestroySelf() {
	Destroy();
}