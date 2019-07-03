// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimGraphNode_CustomSkeleton.h"

#define LOCTEXT_NAMESPACE "CustomSkeleton"

FText UAnimGraphNode_CustomSkeleton::GetTooltipText() const
{
	return LOCTEXT("CustomSkeleton", "CustomSkeleton");
}

FText UAnimGraphNode_CustomSkeleton::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("CustomSkeleton", "CustomSkeleton");
}

#undef LOCTEXT_NAMESPACE
