// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZAnimInstance.h"
#include "ProjectZCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
void UProjectZAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ProjectZCharacter=Cast<AProjectZCharacter>(TryGetPawnOwner());

}

void UProjectZAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ProjectZCharacter == nullptr)
	{
		ProjectZCharacter = Cast<AProjectZCharacter>(TryGetPawnOwner());
	}
	if (ProjectZCharacter == nullptr) return;

	FVector Velocity = ProjectZCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ProjectZCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = ProjectZCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true:false;

	bWeaponEquipped = ProjectZCharacter->IsWeaponEquipped();
	
	bIsCrouched = ProjectZCharacter->bIsCrouched;
	bAiming = ProjectZCharacter->IsAiming();
}
