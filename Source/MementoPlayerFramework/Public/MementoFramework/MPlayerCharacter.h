// Copyright © Bedrockbreaker 2025. MIT License

#pragma once

#include "MementoFramework/MCharacter.h"

#include "MPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class USkeletalMeshComponent;

/** Player character base class for Memento. Handles player input */
UCLASS(abstract)
class MEMENTOPLAYERFRAMEWORK_API AMPlayerCharacter : public AMCharacter
{
	GENERATED_BODY()

public:
	AMPlayerCharacter();

	/** Returns the first person camera */
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

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
};
