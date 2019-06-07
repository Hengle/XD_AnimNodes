// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_SubInstance_Dynamic.h"
#include "AGNode_SubInstance_Dynamic.generated.h"

/**
 * 
 */
UCLASS()
class XD_ANIMNODES_EDITOR_API UAGNode_SubInstance_Dynamic : public UAnimGraphNode_Base
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_SubInstance_Dynamic Node;

	//~ Begin UEdGraphNode Interface.
	FLinearColor GetNodeTitleColor() const override;
	FText GetTooltipText() const override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog) override;
	void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	bool HasExternalDependencies(TArray<class UStruct*>* OptionalOutput = NULL) const override;
	UObject* GetJumpTargetForDoubleClick() const override;
	//~ End UEdGraphNode Interface.
};
