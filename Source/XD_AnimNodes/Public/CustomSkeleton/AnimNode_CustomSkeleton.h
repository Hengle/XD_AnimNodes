// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNode_SkeletalControlBase.h"
#include "CustomSkeletonConfig.h"
#include "AnimNode_CustomSkeleton.generated.h"

/**
 * 
 */
USTRUCT()
struct XD_ANIMNODES_API FAnimNode_CustomSkeleton : public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()
public:
	struct FCustomSkeletonRuntimeEntry
	{
		FBoneReference BoneReference;

		TArray<TKeyValuePair<int32, int32>> OffsetModifies;
		TArray<TKeyValuePair<int32, int32>> RotationModifies;
		TArray<TKeyValuePair<int32, int32>> ScaleModifies;
	};

	TArray<FCustomSkeletonRuntimeEntry> CustomBoneDatas;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (PinShownByDefault))
	FCustomCharacterRuntimeData CustomCharacterRuntimeData;

	TWeakObjectPtr<UCustomCharacterConfig> CachedConfig;

	void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;

	void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
};
