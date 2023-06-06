// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZAnimInstance.h"
#include "ProjectZCharacter.h"
#include "ProjectZ/Weapon/Weapon.h"
#include "ProjectZ/ProjectZTypes/CombatState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
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

	//is in air
	bIsInAir = ProjectZCharacter->GetCharacterMovement()->IsFalling();
	//is Accelerate
	bIsAccelerating = ProjectZCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true:false;
	//is Weapon Equipped
	bWeaponEquipped = ProjectZCharacter->IsWeaponEquipped();
	//get EquippedWeapon
	EquippedWeapon = ProjectZCharacter->GetEquippedWeapon();
	//is Crouched
	bIsCrouched = ProjectZCharacter->bIsCrouched;
	//is Aim
	bAiming = ProjectZCharacter->IsAiming();
	//TurnInPlace
	TurnInPlace = ProjectZCharacter->GetTurnInPlace();
	//RotateRootBone
	bRotateRootBone = ProjectZCharacter->ShouldRotateRootBone();
	//Elimmeted
	bElimmed = ProjectZCharacter->IsElimmed();
	//YawOffset
	FRotator AimRotation = ProjectZCharacter->GetBaseAimRotation();
	FRotator MovementRotaion = UKismetMathLibrary::MakeRotFromX(ProjectZCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotaion, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,DeltaRot,DeltaTime,6.f);
	YawOffset = DeltaRotation.Yaw;
	//Lean
	CharacterRotationLastFrame = CharacterRotationCurrentFrame;
	CharacterRotationCurrentFrame = ProjectZCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationCurrentFrame,CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean,Target,DeltaTime,1.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	//AimOffset
	AO_Yaw = ProjectZCharacter->GetAOYaw();
	AO_Pitch = ProjectZCharacter->GetAOPitch();

	if (bWeaponEquipped && EquippedWeapon&&EquippedWeapon->GetWeaponMesh()&&ProjectZCharacter->GetMesh())
	{
		if (ProjectZCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = ProjectZCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - ProjectZCharacter->GetHitTarget()));
			RightHandRotation = LookAtRotation;

			//Test player said it is weird cause of slow weapon rotate so just direct in LookAtRotation
			//RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 90.f);
		}

		bUseFABRIK = ProjectZCharacter->GetCombatState() != ECombatState::ECS_Reloading;
		bUseAimOffsets= ProjectZCharacter->GetCombatState() != ECombatState::ECS_Reloading&& !ProjectZCharacter->GetDisableGameplay();
		bTransformRightHand= ProjectZCharacter->GetCombatState() != ECombatState::ECS_Reloading && !ProjectZCharacter->GetDisableGameplay();

		//Get Weapon Socket and attach to Character Right Hand Bone(FABRIK)
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"),ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		ProjectZCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"),LeftHandTransform.GetLocation(),FRotator::ZeroRotator,OutPosition,OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		// weapon rotation debug line
		//FTransform MuzzleTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleTransform.GetLocation(), MuzzleTransform.GetLocation() + MuzzleX*1000.f, FColor::Red);
		//DrawDebugLine(GetWorld(), MuzzleTransform.GetLocation(),ProjectZCharacter->GetHitTarget(),FColor::Cyan);
		//

		
		
	}
}
