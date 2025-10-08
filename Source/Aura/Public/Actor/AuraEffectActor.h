// Copyright Axel Woermann

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraEffectActor.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EEffectApplicationPolicy :uint8
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply
};

UENUM(BlueprintType)
enum class EEffectRemovalPolicy :uint8
{
	RemoveOnEndOverlap,
	DoNotRemove
};

USTRUCT(BlueprintType)
struct FGameplayEffectRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AppliedEffects", meta=(ClampMin="0.0"))
	float EffectLevel = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplicationPolicy Application = EEffectApplicationPolicy::DoNotApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectRemovalPolicy Removal = EEffectRemovalPolicy::DoNotRemove;
};

UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AAuraEffectActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(
		AActor* TargetActor,
		TSubclassOf<UGameplayEffect> EffectClass,
		float Level = 1.f,
		EEffectRemovalPolicy RemovalPolicy = EEffectRemovalPolicy::DoNotRemove
	);

	UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="TargetActor", DisplayName="Process Effects (Begin Overlap)",
		ToolTip="Processes Multi Effect Rules configured in Details Panel."))
	void ProcessBeginOverlap(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="TargetActor", DisplayName="Process Effects (End Overlap)",
		ToolTip="Processes Multi Effect Rules configured in Details Panel"))
	void ProcessEndOverlap(AActor* TargetActor);

	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects",
		meta=(DisplayName= "Gameplay Effect Class",
			ToolTip="Use with 'Apply Effect To Target' Function Node"))
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects", meta=(ClampMin="0.0"))
	float EffectLevel = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects|Multi",
		meta=(DisplayName="Multi Effect Rules",
			ToolTip="Effects are auto-applied/removed by Process Overlap functions."))
	TArray<FGameplayEffectRule> MultipleEffectRules;

	
};
