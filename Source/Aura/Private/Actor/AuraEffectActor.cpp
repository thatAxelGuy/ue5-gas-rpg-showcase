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

void AAuraEffectActor::ApplyEffectToTarget(
	AActor* TargetActor,
	const TSubclassOf<UGameplayEffect> EffectClass,
	const float Level,
	const EEffectRemovalPolicy RemovalPolicy
)
{
	UAbilitySystemComponent* TargetAbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetAbilitySystemComponent == nullptr) return;

	//check(GameplayEffectClass);
	if (!ensureMsgf(EffectClass != nullptr,
	                TEXT("%s: ApplyEffectToTarget called without GameplayEffectClass variable set"), *GetName()))
	{
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = TargetAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(
		EffectClass, Level, EffectContextHandle);

	/** ---- Validation ----*/
	if (!EffectSpecHandle.IsValid() || !EffectSpecHandle.Data.IsValid()) return;
	const UGameplayEffect* GEDef = EffectSpecHandle.Data->Def.Get();
	if (!GEDef) return;

	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
		*EffectSpecHandle.Data.Get());

	// Check if Gameplay Effect DurationPolicy is Infinite
	const bool bIsInfinite = GEDef->DurationPolicy ==
		EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && RemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetAbilitySystemComponent);
	}
}

void AAuraEffectActor::ProcessBeginOverlap(AActor* TargetActor)
{
	if (!HasAuthority()) return;
	if (MultipleEffectRules.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no multieffectrules defined"), *GetName());
		return;
	}
	for (const auto& Rule : MultipleEffectRules)
	{
		if (Rule.Application == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			if (!Rule.EffectClass) continue;
			ApplyEffectToTarget(TargetActor, Rule.EffectClass, Rule.EffectLevel, Rule.Removal);
		}
	}
}


void AAuraEffectActor::ProcessEndOverlap(AActor* TargetActor)
{
	if (!HasAuthority()) return;
	if (MultipleEffectRules.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no multi effects defined"), *GetName());
		return;
	}

	// Apply any On End Overlap Rules
	for (const auto& Rule : MultipleEffectRules)
	{
		if (Rule.Application == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			if (!Rule.EffectClass) continue;
			ApplyEffectToTarget(TargetActor, Rule.EffectClass, Rule.EffectLevel, Rule.Removal);
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
