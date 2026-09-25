// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/VoxelProjectile.h"
#include "Character/VoxelCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GAS/VoxelAbilitySystemComponent.h"
#include "GAS/VoxelHitContext.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AVoxelProjectile::AVoxelProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->InitSphereRadius(25.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Sphere->SetGenerateOverlapEvents(true);
	RootComponent = Sphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Sphere);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.35f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = Sphere;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
}

void AVoxelProjectile::Init(AVXCharacterBase* InCaster, const FGameplayTag& InSkillTag, float InDamage, float InSpeed, float InMaxRange,
	const FLinearColor& InColor)
{
	Caster = InCaster;
	SkillTag = InSkillTag;
	Damage = InDamage;
	Speed = InSpeed;
	MaxRange = InMaxRange;

	if (InCaster)
	{
		Sphere->IgnoreActorWhenMoving(InCaster, true);
	}

	if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), InColor);
	}

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
}

void AVoxelProjectile::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AVoxelProjectile::OnSphereBeginOverlap);
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AVoxelProjectile::OnProjectileStopped);

	// 사거리를 수명으로 환산한다.
	SetLifeSpan(MaxRange / FMath::Max(Speed, 1.f));
}

void AVoxelProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AVXCharacterBase* SourceCharacter = Caster.Get();
	AVXCharacterBase* Target = Cast<AVXCharacterBase>(OtherActor);
	if (nullptr == SourceCharacter || nullptr == Target || Target->IsDead() || false == SourceCharacter->IsHostileTo(Target))
	{
		return;
	}

	if (HitActors.Contains(Target))
	{
		return;
	}
	HitActors.Add(Target);

	FVoxelHitContext Context;
	Context.SkillTag = SkillTag;
	Context.Source = SourceCharacter;
	Context.Target = Target;
	Context.Location = GetActorLocation();
	Context.Direction = GetVelocity().GetSafeNormal();
	Context.Damage = Damage;
	Context.bIsDerived = bIsDerived;

	if (UVoxelAbilitySystemComponent* ASC = SourceCharacter->GetVoxelAbilitySystemComponent())
	{
		ASC->ApplySkillHit(Context);
	}

	// 관통이 남아 있으면 계속 나아간다. 관통으로 맞힌 다음 적도 새로운 원본 명중이다. (DES-MOD-001 조합 규칙 5)
	if (PierceRemaining > 0)
	{
		--PierceRemaining;
		return;
	}

	Destroy();
}

void AVoxelProjectile::OnProjectileStopped(const FHitResult& ImpactResult)
{
	Destroy();
}
