// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomSkeletionRuntimeData_Customization.h"
#include "DetailWidgetRow.h"
#include "CustomSkeletonConfig.h"
#include "STextBlock.h"
#include "SNumericEntryBox.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "CustomSkeletionRuntimeData_Customization"

void FCustomSkeletionRuntimeData_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> CustomConfig_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_STRING_CHECKED(FCustomCharacterRuntimeData, CustomConfig));

	FCustomCharacterRuntimeData CustomCharacterRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
	if (CustomCharacterRuntimeData.CustomConfig)
	{
		CustomCharacterRuntimeData.CustomSkeletonValues.SetNumZeroed(CustomCharacterRuntimeData.CustomConfig->SkeletonData.Num());
	}
	FPropertyCustomizeHelper::SetValue(StructPropertyHandle, CustomCharacterRuntimeData, false);

	if (CustomConfig_PropertyHandle)
	{
		CustomConfig_PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([StructPropertyHandle]()
			{
				FCustomCharacterRuntimeData CustomSkeletionRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
				if (CustomSkeletionRuntimeData.CustomConfig)
				{
					TArray<FCustomSkeletonEntry>& Data = CustomSkeletionRuntimeData.CustomConfig->SkeletonData;
					int32 Length = Data.Num();
					CustomSkeletionRuntimeData.CustomSkeletonValues.SetNum(Length);
					for (int32 Idx = 0; Idx < Length; ++Idx)
					{
						CustomSkeletionRuntimeData.CustomSkeletonValues[Idx] = Data[Idx].ToRuntimeData();
					}
				}
				else
				{
					CustomSkeletionRuntimeData.CustomSkeletonValues.SetNum(0);
				}
				FPropertyCustomizeHelper::SetValue(StructPropertyHandle, CustomSkeletionRuntimeData);
			}));

		HeaderRow.NameContent()
			[
				CustomConfig_PropertyHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				CustomConfig_PropertyHandle->CreatePropertyValueWidget()
			];
	}
	else
	{
		HeaderRow.NameContent()
			[
				SNew(STextBlock)
					.Text(LOCTEXT("角色定制配置", "角色定制配置"))
			]
			.ValueContent()
			[
				SNew(STextBlock)
					.Text(FText::FromName(CustomCharacterRuntimeData.CustomConfig ? CustomCharacterRuntimeData.CustomConfig->GetFName() : NAME_None))
			];
	}
}

void FCustomSkeletionRuntimeData_Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FCustomCharacterRuntimeData CustomSkeletionRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
	if (CustomSkeletionRuntimeData.CustomConfig)
	{
		TArray<FCustomSkeletonEntry>& Data = CustomSkeletionRuntimeData.CustomConfig->SkeletonData;
		for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
		{
			const FCustomSkeletonEntry& Entry = Data[Idx];
			StructBuilder.AddCustomRow(Entry.DisplayName).NameContent()
				[
					SNew(STextBlock)
						.Text(Entry.DisplayName)
				]
			.ValueContent()
				[
					SNew(SNumericEntryBox<float>)
						.MinValue(Entry.MinValue)
						.MinSliderValue(Entry.MinValue)
						.MaxValue(Entry.MaxValue)
						.MaxSliderValue(Entry.MaxValue)
						.AllowSpin(true)
						.Value_Lambda([=]()
							{
								FCustomCharacterRuntimeData CustomSkeletionRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
								if (Idx < CustomSkeletionRuntimeData.CustomSkeletonValues.Num())
								{
									return CustomSkeletionRuntimeData.GetCustomSkeletonValue(Idx);
								}
								else
								{
									return 0.f;
								}
							})
						.OnValueChanged_Lambda([=](float NewValue)
							{
								if (FCustomCharacterRuntimeData* Value = FPropertyCustomizeHelper::Value<FCustomCharacterRuntimeData>(StructPropertyHandle))
								{
									if (Idx < Value->CustomSkeletonValues.Num())
									{
										Value->SetCustomSkeletonValue(Idx, NewValue);
									}
								}
							})
						.OnValueCommitted_Lambda([=](float NewValue, ETextCommit::Type CommitType)
							{
								FCustomCharacterRuntimeData CustomSkeletionRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
								CustomSkeletionRuntimeData.SetCustomSkeletonValue(Idx, NewValue);
								FPropertyCustomizeHelper::SetValue(StructPropertyHandle, CustomSkeletionRuntimeData);
							})
				];
		}
	}
}

#undef LOCTEXT_NAMESPACE
