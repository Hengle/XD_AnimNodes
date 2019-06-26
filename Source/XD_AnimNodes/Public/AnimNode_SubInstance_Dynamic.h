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

	struct FBlendData
	{
		UAnimInstance* AnimInstance;
		float BlendTime;
		float Alpha = 1.f;
	};

	UPROPERTY(EditAnywhere, Category = "Settings")
	float BlendTime = 0.2f;

	TArray<FBlendData> BlendDatas;

	void CheckAndReinitAnimInstance(const UAnimInstance* InAnimInstance);

	void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	void Evaluate_AnyThread(FPoseContext& Output) override;

	void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;

private:
	void EvaluateSingleSubInstance(UAnimInstance* SubAnimInstance, FPoseContext& Output);
};
