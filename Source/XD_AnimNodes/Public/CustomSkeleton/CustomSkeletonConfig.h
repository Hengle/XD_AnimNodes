// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CustomSkeletonConfig.generated.h"

/**
 * 
 */
UENUM()
enum class ECustomSkeletonMode
{
	OffsetX,
	OffsetY,
	OffsetZ,
	Pitch,
	Yaw,
	Roll,
	ScaleX,
	ScaleY,
	ScaleZ
};

USTRUCT()
struct XD_ANIMNODES_API FCustomCharacterRuntimeEntry
{
	GENERATED_BODY()
public:
	FCustomCharacterRuntimeEntry() = default;

private:
	UPROPERTY(EditAnywhere, Category = "角色定制")
	uint8 Value;
public:
	void SetValue(float InValue, float MinValue, float MaxValue)
	{
		Value = FMath::GetMappedRangeValueUnclamped({ MinValue, MaxValue }, { 0.f, 255.f }, InValue);
	}
	float GetValue(float MinValue, float MaxValue) const
	{
		return FMath::GetMappedRangeValueUnclamped({ 0.f, 255.f }, { MinValue, MaxValue }, Value);
	}
};

USTRUCT(BlueprintInternalUseOnly)
struct XD_ANIMNODES_API FCustomSkeletonEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	FText DisplayName;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	FName BoneName;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	ECustomSkeletonMode Mode;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	float MaxValue;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	float MinValue;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	float DefalutValue;

	FCustomCharacterRuntimeEntry ToRuntimeData() const
	{
		FCustomCharacterRuntimeEntry Entry;
		Entry.SetValue(DefalutValue, MinValue, MaxValue);
		return Entry;
	}
};

UCLASS()
class XD_ANIMNODES_API UCustomCharacterConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	TArray<FCustomSkeletonEntry> Data;
};

USTRUCT(BlueprintType)
struct XD_ANIMNODES_API FCustomCharacterRuntimeData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "骨架定制")
	UCustomCharacterConfig* CustomConfig;

	UPROPERTY(EditAnywhere, Category = "骨架定制")
	TArray<FCustomCharacterRuntimeEntry> CustomSkeletonValues;

	float GetCustomSkeletonValue(int32 Idx) const;
	void SetCustomSkeletonValue(int32 Idx, float InValue);
};
