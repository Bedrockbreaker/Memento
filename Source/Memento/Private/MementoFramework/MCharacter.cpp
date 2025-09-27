// Copyright © Bedrockbreaker 2025. MIT License

#include "MementoFramework/MCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MGuidComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogMCharacter);

AMCharacter::AMCharacter()
{
	// Create an MGuidComponent for saving and loading
	GuidComponent = CreateDefaultSubobject<UMGuidComponent>(TEXT("GuidComponent"));

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 96.0f);

	// Create and set up the Camera Component
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(2.8f, 0.0f, 70.0f)); // Some magic numbers, but they work
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

void AMCharacter::Move(const FInputActionValue& Value)
{
	if (!GetController()) return;

	FVector2D InputVector = Value.Get<FVector2D>();
	AddMovementInput(GetActorRightVector(), InputVector.X);
	AddMovementInput(GetActorForwardVector(), InputVector.Y);
}

void AMCharacter::Look(const FInputActionValue& Value)
{
	if (!GetController()) return;

	FVector2D InputVector = Value.Get<FVector2D>();
	AddControllerYawInput(InputVector.X);
	AddControllerPitchInput(InputVector.Y);
}

void AMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogMCharacter, Error, TEXT("%s failed to find EnhancedInputComponent"), *GetNameSafe(this));
		return;
	}

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMCharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMCharacter::StopJumping);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMCharacter::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMCharacter::Look);
}

FString AMCharacter::GetSaveId_Implementation() const
{
	APlayerController* PlayerController = GetController<APlayerController>();
	return PlayerController ? TEXT("TheStranger") : GuidComponent->GetString();
}
