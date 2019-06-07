// Fill out your copyright notice in the Description page of Project Settings.


#include "AGNode_SubInstance_Dynamic.h"
#include "CompilerResultsLog.h"
#include "AnimationGraphSchema.h"
#include "EdGraphSchema_K2.h"
#include "Animation/AnimNode_SubInput.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "BlueprintEditorUtils.h"
#include "STextBlock.h"
#include "SBoxPanel.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "XD_AnimNode_SubInstance_Dynamic"

#define Super UAnimGraphNode_Base

FLinearColor UAGNode_SubInstance_Dynamic::GetNodeTitleColor() const
{
	return FLinearColor(0.2f, 0.2f, 0.8f);
}

FText UAGNode_SubInstance_Dynamic::GetTooltipText() const
{
	return LOCTEXT("ToolTip", "Runs a sub-anim instance to process animation");
}

FText UAGNode_SubInstance_Dynamic::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	UClass* TargetClass = *Node.InstanceClass;

	FFormatNamedArguments Args;
	Args.Add(TEXT("NodeTitle"), LOCTEXT("Title", "Sub Anim Instance Dynamic"));
	Args.Add(TEXT("TargetClass"), TargetClass ? FText::FromString(TargetClass->GetName()) : LOCTEXT("ClassNone", "None"));

	if (TitleType == ENodeTitleType::MenuTitle)
	{
		return LOCTEXT("NodeTitle", "Sub Anim Instance Dynamic");
	}
	if (TitleType == ENodeTitleType::ListView)
	{
		if (Node.Tag != NAME_None)
		{
			Args.Add(TEXT("Tag"), FText::FromName(Node.Tag));
			return FText::Format(LOCTEXT("TitleListFormatTagged", "{NodeTitle} - Target Class: {TargetClass} - Tag: {Tag}"), Args);
		}
		else
		{
			return FText::Format(LOCTEXT("TitleListFormat", "{NodeTitle} - Target Class: {TargetClass}"), Args);
		}
	}
	else
	{
		if (Node.Tag != NAME_None)
		{
			Args.Add(TEXT("Tag"), FText::FromName(Node.Tag));
			return FText::Format(LOCTEXT("TitleFormatTagged", "{NodeTitle}\nTarget Class: {TargetClass}\nTag: {Tag}"), Args);
		}
		else
		{
			return FText::Format(LOCTEXT("TitleFormat", "{NodeTitle}\nTarget Class: {TargetClass}"), Args);
		}
	}
}

void UAGNode_SubInstance_Dynamic::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

	UAnimBlueprint* AnimBP = CastChecked<UAnimBlueprint>(GetBlueprint());

	UObject* OriginalNode = MessageLog.FindSourceObject(this);

	// Check we have a class set
	if (!*Node.InstanceClass)
	{
		MessageLog.Error(TEXT("Sub instance node @@ has no valid instance class to spawn."), this);
		return;
	}

	for (TFieldIterator<UProperty> DestPropertyIterator(Node.InstanceClass, EFieldIteratorFlags::ExcludeSuper); DestPropertyIterator; ++DestPropertyIterator)
	{
		UProperty* DestProperty = *DestPropertyIterator;
		if (UStructProperty* StructProperty = Cast<UStructProperty>(DestProperty))
		{
			if (StructProperty->Struct->IsChildOf(FAnimNode_Base::StaticStruct()))
			{
				continue;
			}
		}

		UProperty* SourceProperty = FindField<UProperty>(AnimBP->GetAnimBlueprintSkeletonClass(), DestPropertyIterator->GetFName());
		if (SourceProperty == nullptr)
		{
			MessageLog.Error(TEXT("@@ 子蓝图的变量 @@ 在当前蓝图中不存在"), this, DestProperty);
			continue;
		}
		if (!SourceProperty->SameType(DestProperty))
		{
			MessageLog.Error(TEXT("@@ 子蓝图的变量 @@ 与当前蓝图中的类型不一致"), this, SourceProperty);
			continue;
		}
	}

	// Check we don't try to spawn our own blueprint
	if (*Node.InstanceClass == AnimBP->GetAnimBlueprintGeneratedClass())
	{
		MessageLog.Error(TEXT("Sub instance node @@ targets instance class @@ which it is inside, this would cause a loop."), this, AnimBP->GetAnimBlueprintGeneratedClass());
	}
}

void UAGNode_SubInstance_Dynamic::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);

 	UClass* TargetClass = *Node.InstanceClass;
 	if (!TargetClass)
 	{
 		// Nothing to search for properties
 		return;
 	}

	CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Class, TargetClass, GET_MEMBER_NAME_CHECKED(FAnimNode_SubInstance_Dynamic, DynamicInstanceClass));
}

void UAGNode_SubInstance_Dynamic::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	bool bRequiresNodeReconstruct = false;
	UProperty* ChangedProperty = PropertyChangedEvent.Property;

	if (ChangedProperty)
	{
		if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(FAnimNode_SubInstance, InstanceClass))
		{
			bRequiresNodeReconstruct = true;
		}
	}

	if (bRequiresNodeReconstruct)
	{
		ReconstructNode();
	}
}

bool UAGNode_SubInstance_Dynamic::HasExternalDependencies(TArray<class UStruct*>* OptionalOutput /*= NULL*/) const
{
	UClass* InstanceClassToUse = *Node.InstanceClass;

	// Add our instance class... If that changes we need a recompile
	if (InstanceClassToUse && OptionalOutput)
	{
		OptionalOutput->AddUnique(InstanceClassToUse);
	}

	bool bSuperResult = Super::HasExternalDependencies(OptionalOutput);
	return InstanceClassToUse || bSuperResult;
}

UObject* UAGNode_SubInstance_Dynamic::GetJumpTargetForDoubleClick() const
{
	UClass* InstanceClass = *Node.InstanceClass;

	if (InstanceClass)
	{
		return InstanceClass->ClassGeneratedBy;
	}

	return nullptr;
}

#undef Super

#undef LOCTEXT_NAMESPACE
