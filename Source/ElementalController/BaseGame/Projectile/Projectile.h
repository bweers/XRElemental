// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
// Updates the position of another component during its tick
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/PointLightComponent.h"
#include "TimerManager.h"
#include "Projectile.generated.h"

UCLASS()
class ELEMENTALCONTROLLER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Collision component for projectile
	// VisibleDefaultsOnly: allows editing, but only on archetype
	// The archetype is akin to the blueprint for the object, meaning 
	// that editing this is only possible in the topmost blueprint 
	// and will propogate to all instances
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		USphereComponent* ProjectileCollisionComponent;

	// Projectile movement component
	// VisibleAnywhere: does not allow editing but is 
	// visible in all property windows
	UPROPERTY(VisibleAnywhere, Category = Movement)
		UProjectileMovementComponent* ProjectileMovementComponent;

	// Initializes the projectile's velocity, in the direction it 
	// is to be shot.
	void FireInDirection(const FVector& ShootDirection, float ShootVelocity);

	// Projectile mesh
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		UStaticMeshComponent* ProjectileMeshComponent;

	// Projectile material
	UPROPERTY(VisibleDefaultsOnly, Category = Movement)
		UMaterialInstanceDynamic* ProjectileMaterialInstance;

	// Projectile Light
	UPROPERTY(VisibleDefaultsOnly, Category = Lighting)
		UPointLightComponent* ProjectilePointLightComponent;

	// Niagara particle component
	UPROPERTY(VisibleDefaultsOnly, Category = Particle)
		UNiagaraComponent* ProjectileNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, Category = Particle)
		UNiagaraSystem* ProjectileExplosionSystem;

	// Niagara particle system fire color
	UPROPERTY(VisibleDefaultsOnly, Category = Particle)
		FLinearColor FireColor;

	// Define minimum and maximum values to clip ShootVelocity 
	// within and use when normalizing ClippedShootVelocity
	UPROPERTY(VisibleDefaultsOnly, Category = Movement)
		float MinimumShootVelocityRange;
	UPROPERTY(VisibleDefaultsOnly, Category = Movement)
		float MaximumShootVelocityRange;

	// Define minimum and maximum range to normalize 
	// ClippedShootVelocity within.
	UPROPERTY(VisibleDefaultsOnly, Category = Movement)
		float MinimumShootVelocity;
	UPROPERTY(VisibleDefaultsOnly, Category = Movement)
		float MaximumShootVelocity;

	// Called by OnComponentHit event when projectile hits 
	// (or is hit) by something
	UFUNCTION()
		void OnHit(
			UPrimitiveComponent* HitComponent,		// ?
			AActor* OtherActor,						// Actor hit by projectile
			UPrimitiveComponent* OtherComponent,	// ?
			FVector NormalImpulse,					// Normal impulse of hit
			const FHitResult& Hit					// Struct w/ information about 
													// hit like point of impact and surface normal
		);

	void StopProjectileInteraction();

	void DestroySelf();
};
