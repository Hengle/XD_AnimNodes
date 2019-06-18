// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_SubInstance_Dynamic.h"
#include "AnimInstanceProxy.h"

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
	}
}

void FAnimNode_SubInstance_Dynamic::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	Super::Initialize_AnyThread(Context);
	GetEvaluateGraphExposedInputs().Execute(Context);
}

namespace ProxySkeletonRob
{
	template<typename Tag>
	struct result {
		/* export it ... */
		typedef typename Tag::type type;
		static type ptr;
	};

	template<typename Tag>
	typename result<Tag>::type result<Tag>::ptr;

	template<typename Tag, typename Tag::type p>
	struct rob : result<Tag> {
		/* fill it ... */
		struct filler {
			filler() { result<Tag>::ptr = p; }
		};
		static filler filler_obj;
	};
	template<typename Tag, typename Tag::type p>
	typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;

	struct ProxySkeleton { typedef USkeleton* FAnimInstanceProxy::*type; };
	template struct rob<ProxySkeleton, &FAnimInstanceProxy::Skeleton>;

	USkeleton* GetSkeleton(FAnimInstanceProxy& Proxy)
	{
		return Proxy.*result<ProxySkeleton>().ptr;
	}
}

void FAnimNode_SubInstance_Dynamic::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	struct UAnimInstanceProxyRob : public UAnimInstance
	{
		inline static FAnimInstanceProxy& Get(UAnimInstance* Instance)
		{
			return static_cast<UAnimInstanceProxyRob*>(Instance)->GetProxyOnAnyThread<FAnimInstanceProxy>();
		}
	};

	if (InstanceToRun && ProxySkeletonRob::GetSkeleton(UAnimInstanceProxyRob::Get(InstanceToRun)))
	{
		//Sequence播放时Proxy中的Skeleton可能为空，需要判断下
		Super::Update_AnyThread(Context);
	}
	else
	{
		InPose.Update(Context);
		GetEvaluateGraphExposedInputs().Execute(Context);
	}
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
