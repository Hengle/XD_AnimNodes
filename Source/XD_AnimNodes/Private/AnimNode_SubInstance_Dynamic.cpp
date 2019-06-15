// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_SubInstance_Dynamic.h"

bool FAnimNode_SubInstance_Dynamic::HasPreUpdate() const
{
	return true;
}

void FAnimNode_SubInstance_Dynamic::PreUpdate(const UAnimInstance* InAnimInstance)
{
	Super::PreUpdate(InAnimInstance);
	CheckAndReinitAnimInstance(InAnimInstance);
}

void FAnimNode_SubInstance_Dynamic::CheckAndReinitAnimInstance(const UAnimInstance* InAnimInstance)
{
	if (DynamicInstanceClass == nullptr)
	{
		return;
	}

	check(DynamicInstanceClass->IsChildOf(InstanceClass));

	if (InstanceToRun == nullptr || InstanceToRun->GetClass() != DynamicInstanceClass)
	{
		UAnimInstance* PreInstance = InstanceToRun;

		USkeletalMeshComponent* MeshComp = InAnimInstance->GetSkelMeshComponent();

		//延迟一帧避免PreUpdate检测的报错
		MeshComp->GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(MeshComp, [=]()
			{
				// Need an instance to run, so create it now
				// We use the tag to name the object, but as we verify there are no duplicates in the compiler we
				// dont need to verify it is unique here.
				UAnimInstance*& CachedAnimInstance = DynamicInstanceMap.FindOrAdd(DynamicInstanceClass);
				if (CachedAnimInstance == nullptr)
				{
					CachedAnimInstance = NewObject<UAnimInstance>(MeshComp, DynamicInstanceClass);
					// Initialize the new instance
					CachedAnimInstance->InitializeAnimation();
				}
				InstanceToRun = CachedAnimInstance;

				if (PreInstance)
				{
					MeshComp->SubInstances.Remove(InstanceToRun);
				}
				MeshComp->SubInstances.Add(InstanceToRun);
			}));
	}
}

void FAnimNode_SubInstance_Dynamic::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	Super::Update_AnyThread(Context);
}

void FAnimNode_SubInstance_Dynamic::OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	DynamicInstanceMap.Empty();
	if (*InstanceClass)
	{
		USkeletalMeshComponent* MeshComp = InAnimInstance->GetSkelMeshComponent();
		check(MeshComp);

		// Full reinit, kill old instances
		if (InstanceToRun)
		{
			MeshComp->SubInstances.Remove(InstanceToRun);
			InstanceToRun->MarkPendingKill();
			InstanceToRun = nullptr;
		}

		UClass* SourceClass = InAnimInstance->GetClass();
		InstanceProperties.Reset();
		SubInstanceProperties.Reset();
		for (TFieldIterator<UProperty> DestPropertyIterator(InstanceClass, EFieldIteratorFlags::ExcludeSuper); DestPropertyIterator; ++DestPropertyIterator)
		{
			UProperty* DestProperty = *DestPropertyIterator;

			if (UStructProperty* StructProperty = Cast<UStructProperty>(DestProperty))
			{
				if (StructProperty->Struct->IsChildOf(FAnimNode_Base::StaticStruct()))
				{
					continue;
				}
			}

			UProperty* SourceProperty = FindField<UProperty>(SourceClass, DestProperty->GetFName());
			if (SourceProperty)
			{
				InstanceProperties.Add(SourceProperty);
				SubInstanceProperties.Add(DestProperty);
			}
		}

		CheckAndReinitAnimInstance(InAnimInstance);
	}
	else if (InstanceToRun)
	{
		// We have an instance but no instance class
		TeardownInstance();
	}
}
