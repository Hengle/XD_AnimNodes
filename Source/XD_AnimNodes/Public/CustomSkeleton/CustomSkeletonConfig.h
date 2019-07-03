// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
	ScaleZ,
	ScaleXYZ
};

USTRUCT()
struct XD_ANIMNODES_API FCustomCharacterRuntimeEntry
{
	GENERATED_BODY()
public:
	FCustomCharacterRuntimeEntry() = default;

private:
	UPROPERTY(EditAnywhere, Category = "角色定制")
	float Value;
public:
	void SetValue(float InValue, float MinValue, float MaxValue)
	{
		Value = InValue;
	}
	float GetValue(float MinValue, float MaxValue) const
	{
		return Value;
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
	float MaxValue = 1.f;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	float MinValue = -1.f;
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
	TArray<FCustomSkeletonEntry> SkeletonData;
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

UCLASS()
class XD_ANIMNODES_API UCustomCharacterFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "角色|定制")
	static float GetCustomSkeletonValue(const FCustomCharacterRuntimeData& Data, int32 Idx) { return Data.GetCustomSkeletonValue(Idx); }

	UFUNCTION(BlueprintCallable, Category = "角色|定制")
	static void SetCustomSkeletonValue(UPARAM(Ref)FCustomCharacterRuntimeData& Data, int32 Idx, float InValue) { Data.SetCustomSkeletonValue(Idx, InValue); }
};
