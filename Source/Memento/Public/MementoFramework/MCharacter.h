// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "GameFramework/Character.h"
#include "SaveSystem/IMSaveable.h"

#include "MCharacter.generated.h"

class UCameraComponent;
class UMGuidComponent;
class UInputAction;
class USkeletalMeshComponent;
struct FInputActionValue;

MEMENTO_API DECLARE_LOG_CATEGORY_EXTERN(LogMCharacter, Log, All);

/** Base character for Memento. */
UCLASS(abstract)
class MEMENTO_API AMCharacter : public ACharacter, public IMSaveable
{
	GENERATED_BODY()

public:
	AMCharacter();

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Handles move inputs */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void Move(const FInputActionValue& Value);

	/** Handles look inputs */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void Look(const FInputActionValue& Value);

	/** Called when a UPlayerController possesses this Character */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// TODO: Replace with just a socket. Have the CamaeraManager attach the camera instead.
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/** Guid component for saving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save System", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMGuidComponent> GuidComponent;

	/** Get the save id */
	virtual FString GetSaveId_Implementation() const override;
};
