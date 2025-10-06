// Copyright Axel Woermann


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

// Sets default values
AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")));
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, const TSubclassOf<UGameplayEffect> GameplayEffectClass,
                                           const float EffectLevel, EEffectRemovalPolicy RemovalPolicy)
{
	UAbilitySystemComponent* TargetAbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetAbilitySystemComponent == nullptr) return;

	check(GameplayEffectClass);

	FGameplayEffectContextHandle EffectContextHandle = TargetAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(
		GameplayEffectClass, EffectLevel, EffectContextHandle);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
		*EffectSpecHandle.Data.Get());

	// Check if Gameplay Effect DurationPolicy is Infinite
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy ==
		EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && RemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetAbilitySystemComponent);
	}
}

void AAuraEffectActor::ApplyConfiguredEffects_OnOverlap(AActor* TargetActor)
{
	for (const auto& EffectRule : GameplayEffectRules)
	{
		if (EffectRule.Application == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, EffectRule.EffectClass, EffectRule.Level, EffectRule.Removal);
		}
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	// Apply any On End Overlap Rules
	for (const auto& Rule : GameplayEffectRules)
	{
		if (Rule.Application == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, Rule.EffectClass, Rule.Level, Rule.Removal);
		}
	}

	// Remove effects tracked for this target (Only if marked remove on end overlap get tracked)
	UAbilitySystemComponent* TargetAbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!IsValid(TargetAbilitySystemComponent)) return;

	TArray<FActiveGameplayEffectHandle> HandlesToRemove;
	for (const auto& EffectHandle : ActiveEffectHandles)
	{
		if (TargetAbilitySystemComponent == EffectHandle.Value)
		{
			TargetAbilitySystemComponent->RemoveActiveGameplayEffect(EffectHandle.Key, 1);
			HandlesToRemove.Add(EffectHandle.Key);
		}
	}
	for (FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		ActiveEffectHandles.FindAndRemoveChecked(Handle);
	}
}
