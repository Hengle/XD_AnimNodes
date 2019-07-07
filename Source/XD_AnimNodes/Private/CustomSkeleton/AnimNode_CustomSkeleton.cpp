// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_CustomSkeleton.h"
#include "AnimationRuntime.h"
#include "AnimInstanceProxy.h"

void FAnimNode_CustomSkeleton::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	BasePose.Initialize(Context);
}

void FAnimNode_CustomSkeleton::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
	BasePose.Update(Context);

	const FBoneContainer& RequiredBones = Context.AnimInstanceProxy->GetRequiredBones();

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
		for (const FCustomSkeletonBoneData& BoneData : CustomSkeletonEntry.BoneDatas)
		{
			const FName& BoneName = BoneData.BoneName;

			int32 Index = CustomBoneDatas.IndexOfByPredicate([&](const FCustomSkeletonRuntimeEntry& E) {return E.BoneReference.BoneName == BoneName; });
			if (Index == INDEX_NONE)
			{
				FBoneReference BoneReferences(BoneName);
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

			switch (BoneData.Mode)
			{
			case ECustomSkeletonMode::Offset:
				Entry.OffsetModifies.Add({ BoneData.ApplyAxis.GetSafeNormal(), Idx });
				break;
			case ECustomSkeletonMode::Scale:
				Entry.ScaleModifies.Add({ BoneData.ApplyAxis.GetSafeNormal(), Idx });
				break;
			case ECustomSkeletonMode::Rotation:
				Entry.RotationModifies.Add({ BoneData.ApplyAxis.GetSafeNormal(), Idx });
				break;
			}
		}
	}

	CustomBoneDatas.Sort([&](const FCustomSkeletonRuntimeEntry& LHS, const FCustomSkeletonRuntimeEntry& RHS)
		{
			return LHS.BoneReference.BoneIndex < RHS.BoneReference.BoneIndex;
		});
}

void FAnimNode_CustomSkeleton::Evaluate_AnyThread(FPoseContext& Output)
{
	BasePose.Evaluate(Output);

	if (CustomCharacterRuntimeData.CustomConfig && CustomCharacterRuntimeData.CustomSkeletonValues.Num() != CustomCharacterRuntimeData.CustomConfig->SkeletonData.Num())
	{
		return;
	}

	// the way we apply transform is same as FMatrix or FTransform
	// we apply scale first, and rotation, and translation
	// if you'd like to translate first, you'll need two nodes that first node does translate and second nodes to rotate.
	const FBoneContainer& BoneContainer = Output.Pose.GetBoneContainer();

	for (const FCustomSkeletonRuntimeEntry& Entry : CustomBoneDatas)
	{
		if (Entry.BoneReference.IsValidToEvaluate(BoneContainer))
		{
			FCompactPoseBoneIndex CompactPoseBoneToModify = Entry.BoneReference.GetCompactPoseIndex(BoneContainer);
			FTransform& NewBoneTM = Output.Pose[CompactPoseBoneToModify];

			if (Entry.ScaleModifies.Num() > 0)
			{
				FVector Scale{ FVector::ZeroVector };
				for (const auto& Data : Entry.ScaleModifies)
				{
					Scale += Data.Key * CustomCharacterRuntimeData.GetCustomSkeletonValueScaled(Data.Value);
				}
				NewBoneTM.SetScale3D(NewBoneTM.GetScale3D() + Scale);
			}

			if (Entry.RotationModifies.Num() > 0)
			{
				FVector Rotation{ FVector::ZeroVector };
				for (const auto& Data : Entry.RotationModifies)
				{
					Rotation += Data.Key * CustomCharacterRuntimeData.GetCustomSkeletonValueScaled(Data.Value);
				}
				NewBoneTM.SetRotation(FQuat(FRotator(Rotation.X, Rotation.Y, Rotation.Z)) * NewBoneTM.GetRotation());
			}

			if (Entry.OffsetModifies.Num() > 0)
			{
				FVector Offset{ FVector::ZeroVector };
				for (const auto& Data : Entry.OffsetModifies)
				{
					Offset += Data.Key * CustomCharacterRuntimeData.GetCustomSkeletonValueScaled(Data.Value);
				}
				NewBoneTM.AddToTranslation(Offset);
			}
		}
	}
}

void FAnimNode_CustomSkeleton::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	Super::CacheBones_AnyThread(Context);
}
