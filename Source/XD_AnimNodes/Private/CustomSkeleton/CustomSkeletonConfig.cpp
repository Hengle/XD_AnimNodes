// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSkeletonConfig.h"

float FCustomCharacterRuntimeData::GetCustomSkeletonValue(int32 Idx) const
{
	const FCustomSkeletonEntry& CustomSkeletonEntry = CustomConfig->SkeletonData[Idx];
	return CustomSkeletonValues[Idx].GetValue(CustomSkeletonEntry.MinValue, CustomSkeletonEntry.MaxValue);
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
