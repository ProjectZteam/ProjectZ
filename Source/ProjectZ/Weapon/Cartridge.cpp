// Fill out your copyright notice in the Description page of Project Settings.


#include "Cartridge.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundCue.h"
ACartridge::ACartridge()
{
	PrimaryActorTick.bCanEverTick = false;
	CartridgeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CartridgeMesh"));
	SetRootComponent(CartridgeMesh);
	CartridgeMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	CartridgeMesh->SetSimulatePhysics(true);
	CartridgeMesh->SetEnableGravity(true);
	CartridgeMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectImpulse = 10.f;
}

// Called when the game starts or when spawned
void ACartridge::BeginPlay()
{
	Super::BeginPlay();
	CartridgeMesh->OnComponentHit.AddDynamic(this, &ACartridge::OnHit);
	CartridgeMesh->AddImpulse(GetActorForwardVector() * ShellEjectImpulse);

	SetLifeSpan(3.f);
}

void ACartridge::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	CartridgeMesh->SetNotifyRigidBodyCollision(false);
}
