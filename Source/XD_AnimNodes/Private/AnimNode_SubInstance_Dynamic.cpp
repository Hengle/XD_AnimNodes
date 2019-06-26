// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_SubInstance_Dynamic.h"
#include "AnimInstanceProxy.h"
#include "AnimationRuntime.h"

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

	USkeletalMeshComponent* MeshComp = InAnimInstance->GetSkelMeshComponent();
	if (InstanceToRun == nullptr || InstanceToRun->GetClass() != DynamicInstanceClass)
	{
		UAnimInstance* PreInstance = InstanceToRun;

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

		check(!BlendDatas.ContainsByPredicate([&](const FBlendData& Data) {return Data.AnimInstance == PreInstance; }));

		if (BlendTime > 0.f)
		{
			if (PreInstance)
			{
				FBlendData BlendData;
				BlendData.AnimInstance = PreInstance;
				BlendData.BlendTime = BlendTime;
				BlendDatas.Add(BlendData);
			}
			BlendDatas.RemoveAll([&](const FBlendData& Data) {return Data.AnimInstance == InstanceToRun; });
		}
		MeshComp->SubInstances.AddUnique(InstanceToRun);
	}

	if (BlendDatas.Num() > 0)
	{
		for (const FBlendData& BlendData : BlendDatas)
		{
			if (BlendData.Alpha <= 0.f)
			{
				MeshComp->SubInstances.Remove(BlendData.AnimInstance);
			}
		}
		BlendDatas.RemoveAll([&](const FBlendData& Data) {return Data.Alpha <= 0.f; });
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

	//Sequence播放时Proxy中的Skeleton可能为空，需要判断下
	if (InstanceToRun && ProxySkeletonRob::GetSkeleton(UAnimInstanceProxyRob::Get(InstanceToRun)))
	{
		Super::Update_AnyThread(Context);
	}
	else
	{
		InPose.Update(Context);
		GetEvaluateGraphExposedInputs().Execute(Context);
	}

	//混合中的子动画实例也同步下
	for (FBlendData& Data : BlendDatas)
	{
		// First copy properties
		check(InstanceProperties.Num() == SubInstanceProperties.Num());
		for (int32 PropIdx = 0; PropIdx < InstanceProperties.Num(); ++PropIdx)
		{
			UProperty* CallerProperty = InstanceProperties[PropIdx];
			UProperty* SubProperty = SubInstanceProperties[PropIdx];

			check(CallerProperty && SubProperty);

#if WITH_EDITOR
			if (ensure(CallerProperty->SameType(SubProperty)))
#endif
			{
				uint8* SrcPtr = CallerProperty->ContainerPtrToValuePtr<uint8>(Context.AnimInstanceProxy->GetAnimInstanceObject());
				uint8* DestPtr = SubProperty->ContainerPtrToValuePtr<uint8>(InstanceToRun);

				CallerProperty->CopyCompleteValue(DestPtr, SrcPtr);
			}
		}

		if (Data.AnimInstance->bNeedsUpdate)
		{
			struct FUpdateAnimationRob : public FAnimInstanceProxy
			{
				inline static void TryUpdateAnimation(FAnimInstanceProxy& Proxy)
				{
					static_cast<FUpdateAnimationRob&>(Proxy).UpdateAnimation();
				}
			};

			FAnimInstanceProxy& Proxy = UAnimInstanceProxyRob::Get(Data.AnimInstance);
			FUpdateAnimationRob::TryUpdateAnimation(Proxy);
		}

		Data.Alpha -= Context.GetDeltaTime() / Data.BlendTime;
	}
}

void FAnimNode_SubInstance_Dynamic::Evaluate_AnyThread(FPoseContext& Output)
{
	TArray<int32> ValidBlendPose;
	ValidBlendPose.Reset(BlendDatas.Num());
	for (int32 i = 0; i < BlendDatas.Num(); ++i)
	{
		if (BlendDatas[i].Alpha > 0.f)
		{
			ValidBlendPose.Add(i);
		}
	}

	int32 NumPoses = ValidBlendPose.Num() + 1;
	if (NumPoses > 1)
	{
		TArray<float> BlendWeights;
		BlendWeights.SetNum(NumPoses);
		const int32 ActiveInstanceIdx = NumPoses - 1;
		BlendWeights[ActiveInstanceIdx] = 0.f;

		TArray<FCompactPose, TInlineAllocator<8>> FilteredPoses;
		TArray<FBlendedCurve, TInlineAllocator<8>> FilteredCurve;
		FilteredPoses.SetNum(NumPoses, false);
		FilteredCurve.SetNum(NumPoses, false);

		for (int32 i = 0; i < ValidBlendPose.Num(); ++i)
		{
			const FBlendData& BlendData = BlendDatas[ValidBlendPose[i]];

			FPoseContext EvaluateContext(Output);

			EvaluateSingleSubInstance(BlendData.AnimInstance, EvaluateContext);

			FilteredPoses[i].MoveBonesFrom(EvaluateContext.Pose);
			FilteredCurve[i].MoveFrom(EvaluateContext.Curve);
			BlendWeights[i] = BlendData.Alpha / ValidBlendPose.Num();
			BlendWeights[ActiveInstanceIdx] += (1.f - BlendData.Alpha) / ValidBlendPose.Num();
		}

		{
			FPoseContext EvaluateContext(Output);
			EvaluateSingleSubInstance(InstanceToRun, EvaluateContext);
			FilteredPoses[ActiveInstanceIdx].MoveBonesFrom(EvaluateContext.Pose);
			FilteredCurve[ActiveInstanceIdx].MoveFrom(EvaluateContext.Curve);
		}

		FAnimationRuntime::BlendPosesTogether(FilteredPoses, FilteredCurve, BlendWeights, Output.Pose, Output.Curve);
	}
	else
	{
		Super::Evaluate_AnyThread(Output);
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

void FAnimNode_SubInstance_Dynamic::EvaluateSingleSubInstance(UAnimInstance* SubAnimInstance, FPoseContext& Output)
{
	UAnimInstance* CacheInstanceToRun = InstanceToRun;
	InstanceToRun = SubAnimInstance;
	Super::Evaluate_AnyThread(Output);
	InstanceToRun = CacheInstanceToRun;
}
