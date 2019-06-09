// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNode_SubInstance.h"
#include "AnimNode_SubInstance_Dynamic.generated.h"

class UAnimInstance;

/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct XD_ANIMNODES_API FAnimNode_SubInstance_Dynamic : public FAnimNode_SubInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TSubclassOf<UAnimInstance> DynamicInstanceClass;
	
	UPROPERTY(Transient)
	TMap<TSubclassOf<UAnimInstance>, UAnimInstance*> DynamicInstanceMap;

	bool HasPreUpdate() const override;
	void PreUpdate(const UAnimInstance* InAnimInstance) override;

	void CheckAndReinitAnimInstance(const UAnimInstance* InAnimInstance);

	void Update_AnyThread(const FAnimationUpdateContext& Context) override;

	void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;
};
