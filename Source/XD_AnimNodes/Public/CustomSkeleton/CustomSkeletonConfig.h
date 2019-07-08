// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CustomSkeletonConfig.generated.h"

class USkeletalMeshComponent;

/**
 * 
 */
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

UENUM()
enum class ECustomSkeletonMode
{
	Offset,
	Rotation,
	Scale
};

USTRUCT(BlueprintInternalUseOnly)
struct XD_ANIMNODES_API FCustomSkeletonBoneData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	FName BoneName;

	UPROPERTY(EditAnywhere, Category = "骨架定制")
	ECustomSkeletonMode Mode;

	UPROPERTY(EditAnywhere, Category = "骨架定制")
	FVector ApplyAxis = FVector(0.f, 1.f, 0.f);
};

USTRUCT(BlueprintType)
struct XD_ANIMNODES_API FCustomSkeletonEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "骨架定制")
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "骨架定制")
	FText Category;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	TArray<FCustomSkeletonBoneData> BoneDatas;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "骨架定制")
	float MaxValue = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "骨架定制")
	float MinValue = -1.f;
	UPROPERTY(EditAnywhere, Category = "骨架定制")
	float Scale = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "骨架定制")
	float DefalutValue;

	FText GetBonesDesc() const;

	FCustomCharacterRuntimeEntry ToRuntimeData() const
	{
		FCustomCharacterRuntimeEntry Entry;
		Entry.SetValue(DefalutValue, MinValue, MaxValue);
		return Entry;
	}
};

USTRUCT(BlueprintType)
struct XD_ANIMNODES_API FCustomMorphEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "混合变形定制")
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "混合变形定制")
	FText Category;
	UPROPERTY(EditAnywhere, Category = "混合变形定制")
	TArray<FName> MorphTargetNames;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "混合变形定制")
	float MaxValue = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "混合变形定制")
	float MinValue = -1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "混合变形定制")
	float DefalutValue;

	FText GetMorphsDesc() const;
};

UCLASS()
class XD_ANIMNODES_API UCustomCharacterConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "角色定制")
	TArray<FCustomSkeletonEntry> SkeletonData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "角色定制")
	TArray<FCustomMorphEntry> MorphData;
};

USTRUCT(BlueprintType)
struct XD_ANIMNODES_API FCustomCharacterRuntimeData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "角色定制")
	UCustomCharacterConfig* CustomConfig;

	UPROPERTY(EditAnywhere, Category = "角色定制")
	TArray<FCustomCharacterRuntimeEntry> CustomSkeletonValues;

	UPROPERTY(EditAnywhere, Category = "角色定制")
	TArray<FCustomCharacterRuntimeEntry> CustomMorphValues;

	float GetCustomSkeletonValue(int32 Idx) const;
	float GetCustomSkeletonValueScaled(int32 Idx) const;
	void SetCustomSkeletonValue(int32 Idx, float InValue);

	float GetCustomMorphValue(int32 Idx) const;
	void SetCustomMorphValue(int32 Idx, float InValue);

	void ApplyMorphTarget(USkeletalMeshComponent* SkeletalMeshComponent) const;
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

	UFUNCTION(BlueprintPure, Category = "角色|定制")
	static FCustomSkeletonEntry GetCustomSkeletonConfig(const FCustomCharacterRuntimeData& Data, int32 Idx) { return Data.CustomConfig ? Data.CustomConfig->SkeletonData[Idx] : FCustomSkeletonEntry(); }

	UFUNCTION(BlueprintPure, Category = "角色|定制")
	static FCustomMorphEntry GetCustomMorphConfig(const FCustomCharacterRuntimeData& Data, int32 Idx) { return Data.CustomConfig ? Data.CustomConfig->MorphData[Idx] : FCustomMorphEntry(); }
};
