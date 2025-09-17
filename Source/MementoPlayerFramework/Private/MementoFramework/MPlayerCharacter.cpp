// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoFramework/MPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "MementoFramework/MCharacter.h"

AMPlayerCharacter::AMPlayerCharacter()
{
	// Size appropriate size for first person
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 96.0f);

	// Create and set up the Camera Component
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(
		FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f)); // Some magic numbers, but they work
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Make the world space mesh invisible to the local player
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	// Configure character movement better for first person
	GetCharacterMovement()->BrakingDecelerationWalking = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AMPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (!GetController()) return;

	FVector2D InputVector = Value.Get<FVector2D>();
	AddMovementInput(GetActorRightVector(), InputVector.X);
	AddMovementInput(GetActorForwardVector(), InputVector.Y);
}

void AMPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (!GetController()) return;

	FVector2D InputVector = Value.Get<FVector2D>();
	AddControllerYawInput(InputVector.X);
	AddControllerPitchInput(InputVector.Y);
}

void AMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogMCharacter, Error, TEXT("%s failed to find EnhancedInputComponent"), *GetNameSafe(this));
		return;
	}

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMPlayerCharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMPlayerCharacter::StopJumping);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMPlayerCharacter::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMPlayerCharacter::Look);
}
