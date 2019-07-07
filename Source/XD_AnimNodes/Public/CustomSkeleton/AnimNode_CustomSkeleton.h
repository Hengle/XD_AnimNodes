﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "CustomSkeletonConfig.h"
#include "AnimNode_CustomSkeleton.generated.h"

/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct XD_ANIMNODES_API FAnimNode_CustomSkeleton : public FAnimNode_Base
{
	GENERATED_BODY()
public:
	struct FCustomSkeletonRuntimeEntry
	{
		FBoneReference BoneReference;

		TArray<TKeyValuePair<FVector, int32>> OffsetModifies;
		TArray<TKeyValuePair<FVector, int32>> RotationModifies;
		TArray<TKeyValuePair<FVector, int32>> ScaleModifies;
	};

	TArray<FCustomSkeletonRuntimeEntry> CustomBoneDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Links)
	FPoseLink BasePose;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (PinShownByDefault))
	FCustomCharacterRuntimeData CustomCharacterRuntimeData;

	TWeakObjectPtr<UCustomCharacterConfig> CachedConfig;

	// FAnimNode_Base interface
	void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	void Evaluate_AnyThread(FPoseContext& Output) override;
	void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	// End of FAnimNode_Base interface
};
