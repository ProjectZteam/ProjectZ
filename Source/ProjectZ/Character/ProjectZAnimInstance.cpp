// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZAnimInstance.h"
#include "ProjectZCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
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

	FRotator AimRotation = ProjectZCharacter->GetBaseAimRotation();
	FRotator MovementRotaion = UKismetMathLibrary::MakeRotFromX(ProjectZCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotaion, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,DeltaRot,DeltaTime,6.f);
	YawOffset = DeltaRotation.Yaw;
	CharacterRotationLastFrame = CharacterRotationCurrentFrame;
	CharacterRotationCurrentFrame = ProjectZCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationCurrentFrame,CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean,Target,DeltaTime,1.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
}
