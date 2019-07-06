// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSkeletonConfig.h"
#include "Components/SkeletalMeshComponent.h"

FText FCustomSkeletonEntry::GetBonesDesc() const
{
	FString Desc = TEXT("|");
	for (const FCustomSkeletonBoneData& BoneData : BoneDatas)
	{
		Desc += BoneData.BoneName.ToString() + TEXT("|");
	}
	return FText::FromString(Desc);
}

FText FCustomMorphEntry::GetMorphsDesc() const
{
	FString Desc = TEXT("|");
	for (const FName& MorphTargetName : MorphTargetNames)
	{
		Desc += MorphTargetName.ToString() + TEXT("|");
	}
	return FText::FromString(Desc);
}

float FCustomCharacterRuntimeData::GetCustomSkeletonValue(int32 Idx) const
{
	const FCustomSkeletonEntry& CustomSkeletonEntry = CustomConfig->SkeletonData[Idx];
	return CustomSkeletonValues[Idx].GetValue(CustomSkeletonEntry.MinValue, CustomSkeletonEntry.MaxValue);
}

float FCustomCharacterRuntimeData::GetCustomSkeletonValueScaled(int32 Idx) const
{
	const FCustomSkeletonEntry& CustomSkeletonEntry = CustomConfig->SkeletonData[Idx];
	return GetCustomSkeletonValue(Idx) * CustomSkeletonEntry.Scale;
}

void FCustomCharacterRuntimeData::SetCustomSkeletonValue(int32 Idx, float InValue)
{
	const FCustomSkeletonEntry& CustomSkeletonEntry = CustomConfig->SkeletonData[Idx];
	CustomSkeletonValues[Idx].SetValue(InValue, CustomSkeletonEntry.MinValue, CustomSkeletonEntry.MaxValue);
}

float FCustomCharacterRuntimeData::GetCustomMorphValue(int32 Idx) const
{
	const FCustomMorphEntry& CustomMorphEntry = CustomConfig->MorphData[Idx];
	return CustomMorphValues[Idx].GetValue(CustomMorphEntry.MinValue, CustomMorphEntry.MaxValue);
}

void FCustomCharacterRuntimeData::SetCustomMorphValue(int32 Idx, float InValue)
{
	const FCustomMorphEntry& CustomMorphEntry = CustomConfig->MorphData[Idx];
	CustomMorphValues[Idx].SetValue(InValue, CustomMorphEntry.MinValue, CustomMorphEntry.MaxValue);
}

void FCustomCharacterRuntimeData::ApplyMorphTarget(USkeletalMeshComponent* SkeletalMeshComponent) const
{
	for (int32 Idx = 0; Idx < CustomSkeletonValues.Num(); ++Idx)
	{
		const FCustomMorphEntry& Entry = CustomConfig->MorphData[Idx];
		float Value = GetCustomMorphValue(Idx);
		for (const FName& MorphTargetName : Entry.MorphTargetNames)
		{
			SkeletalMeshComponent->SetMorphTarget(MorphTargetName, Value);
		}
	}
}
