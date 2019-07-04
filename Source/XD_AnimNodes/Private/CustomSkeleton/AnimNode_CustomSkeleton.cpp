// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_CustomSkeleton.h"
#include "AnimationRuntime.h"
#include "AnimInstanceProxy.h"

void FAnimNode_CustomSkeleton::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	// the way we apply transform is same as FMatrix or FTransform
	// we apply scale first, and rotation, and translation
	// if you'd like to translate first, you'll need two nodes that first node does translate and second nodes to rotate.
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();

	if (CustomCharacterRuntimeData.CustomSkeletonValues.Num() != CustomBoneDatas.Num())
	{
		return;
	}

	for (const FCustomSkeletonRuntimeEntry& Entry : CustomBoneDatas)
	{
		if (Entry.BoneReference.IsValidToEvaluate(BoneContainer))
		{
			FCompactPoseBoneIndex CompactPoseBoneToModify = Entry.BoneReference.GetCompactPoseIndex(BoneContainer);
			FTransform NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
			FTransform ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();

			// Convert to Bone Space.
			FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);
			if (Entry.ScaleModifies.Num() > 0)
			{
				FVector Scale{ FVector::ZeroVector };
				for (const auto& Data : Entry.ScaleModifies)
				{
					Scale[Data.Key] = CustomCharacterRuntimeData.GetCustomSkeletonValue(Data.Value);
				}
				NewBoneTM.SetScale3D(NewBoneTM.GetScale3D() + Scale);
			}

			if (Entry.RotationModifies.Num() > 0)
			{
				FVector Rotation{ FVector::ZeroVector };
				for (const auto& Data : Entry.RotationModifies)
				{
					Rotation[Data.Key] = CustomCharacterRuntimeData.GetCustomSkeletonValue(Data.Value);
				}
				NewBoneTM.SetRotation(FQuat(FRotator(Rotation.X, Rotation.Y, Rotation.Z)) * NewBoneTM.GetRotation());
			}

			if (Entry.OffsetModifies.Num() > 0)
			{
				FVector Offset{ FVector::ZeroVector };
				for (const auto& Data : Entry.OffsetModifies)
				{
					Offset[Data.Key] = CustomCharacterRuntimeData.GetCustomSkeletonValue(Data.Value);
				}
				NewBoneTM.AddToTranslation(Offset);
			}

			// Convert back to Component Space.
			FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_BoneSpace);

			OutBoneTransforms.Add(FBoneTransform(Entry.BoneReference.GetCompactPoseIndex(BoneContainer), NewBoneTM));
		}
	}
}

void FAnimNode_CustomSkeleton::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
	Super::CacheBones_AnyThread(Context);
}

bool FAnimNode_CustomSkeleton::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return true;
}

void FAnimNode_CustomSkeleton::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	if (!CustomCharacterRuntimeData.CustomConfig)
	{
		return;
	}

	if (CachedConfig.Get() == CustomCharacterRuntimeData.CustomConfig)
	{
		return;
	}

	CustomBoneDatas.Empty();
	CachedConfig = CustomCharacterRuntimeData.CustomConfig;

	for (int32 Idx = 0; Idx < CustomCharacterRuntimeData.CustomConfig->SkeletonData.Num(); ++Idx)
	{
		const FCustomSkeletonEntry& CustomSkeletonEntry = CustomCharacterRuntimeData.CustomConfig->SkeletonData[Idx];
		int32 Index = CustomBoneDatas.IndexOfByPredicate([&](const FCustomSkeletonRuntimeEntry& E) {return E.BoneReference.BoneName == CustomSkeletonEntry.BoneName; });
		if (Index == INDEX_NONE)
		{
			FBoneReference BoneReferences(CustomSkeletonEntry.BoneName);
			if (BoneReferences.Initialize(RequiredBones))
			{
				FCustomSkeletonRuntimeEntry CustomSkeletonRuntimeEntry;
				CustomSkeletonRuntimeEntry.BoneReference = BoneReferences;
				Index = CustomBoneDatas.Add(CustomSkeletonRuntimeEntry);
			}
			else
			{
				continue;
			}
		}
		FCustomSkeletonRuntimeEntry& Entry = CustomBoneDatas[Index];

		switch (CustomSkeletonEntry.Mode)
		{
		case ECustomSkeletonMode::OffsetX:
			Entry.OffsetModifies.Add({ 0, Idx });
			break;
		case ECustomSkeletonMode::OffsetY:
			Entry.OffsetModifies.Add({ 1, Idx });
			break;
		case ECustomSkeletonMode::OffsetZ:
			Entry.OffsetModifies.Add({ 2, Idx });
			break;
		case ECustomSkeletonMode::Pitch:
			Entry.ScaleModifies.Add({ 0, Idx });
			break;
		case ECustomSkeletonMode::Yaw:
			Entry.ScaleModifies.Add({ 1, Idx });
			break;
		case ECustomSkeletonMode::Roll:
			Entry.ScaleModifies.Add({ 2, Idx });
			break;
		case ECustomSkeletonMode::ScaleX:
			Entry.ScaleModifies.Add({ 0, Idx });
			break;
		case ECustomSkeletonMode::ScaleY:
			Entry.ScaleModifies.Add({ 1, Idx });
			break;
		case ECustomSkeletonMode::ScaleZ:
			Entry.ScaleModifies.Add({ 2, Idx });
			break;
		case ECustomSkeletonMode::ScaleXYZ:
			Entry.ScaleModifies.Add({ 0, Idx });
			Entry.ScaleModifies.Add({ 1, Idx });
			Entry.ScaleModifies.Add({ 2, Idx });
			break;
		}
	}

	CustomBoneDatas.Sort([&](const FCustomSkeletonRuntimeEntry& LHS, const FCustomSkeletonRuntimeEntry& RHS)
		{
			return LHS.BoneReference.BoneIndex < RHS.BoneReference.BoneIndex;
		});
}
