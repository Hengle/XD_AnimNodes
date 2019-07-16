// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomSkeletionRuntimeData_Customization.h"
#include "DetailWidgetRow.h"
#include "CustomSkeletonConfig.h"
#include "STextBlock.h"
#include "SNumericEntryBox.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"

#define LOCTEXT_NAMESPACE "CustomSkeletionRuntimeData_Customization"

void FCustomSkeletionRuntimeData_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> CustomConfig_PropertyHandle = FPropertyCustomizeHelper::GetPropertyHandleByName(StructPropertyHandle, GET_MEMBER_NAME_CHECKED(FCustomCharacterRuntimeData, CustomConfig));

	FCustomCharacterRuntimeData CustomCharacterRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
	CustomCharacterRuntimeData.SyncConfigSize();
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
	//TODO 提供返回至默认值的按钮 SResetToDefaultPropertyEditor

	FCustomCharacterRuntimeData CustomSkeletionRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
	if (CustomSkeletionRuntimeData.CustomConfig)
	{
		TMap<FName, IDetailGroup*> GroupMap;
		for (const FText& Category : CustomSkeletionRuntimeData.CustomConfig->GetAllCategoryNames())
		{
			FName CategoryName = *Category.ToString();
			IDetailGroup*& DetailGroup = GroupMap.FindOrAdd(CategoryName);
			if (!DetailGroup)
			{
				DetailGroup = &StructBuilder.AddGroup(CategoryName, Category);
			}
		}

		TArray<FCustomSkeletonEntry>& SkeletonData = CustomSkeletionRuntimeData.CustomConfig->SkeletonData;
		for (int32 Idx = 0; Idx < SkeletonData.Num(); ++Idx)
		{
			const FCustomSkeletonEntry& Entry = SkeletonData[Idx];
			GroupMap[*Entry.Category.ToString()]->AddWidgetRow().NameContent()
				[
					SNew(STextBlock)
						.Text(Entry.DisplayName)
						.ToolTipText(Entry.GetBonesDesc())
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

		TArray<FCustomMorphEntry>& MorphData = CustomSkeletionRuntimeData.CustomConfig->MorphData;
		for (int32 Idx = 0; Idx < MorphData.Num(); ++Idx)
		{
			const FCustomMorphEntry& Entry = MorphData[Idx];
			GroupMap[*Entry.Category.ToString()]->AddWidgetRow().NameContent()
				[
					SNew(STextBlock)
					.Text(Entry.DisplayName)
				.ToolTipText(Entry.GetMorphsDesc())
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
							if (Idx < CustomSkeletionRuntimeData.CustomMorphValues.Num())
							{
								return CustomSkeletionRuntimeData.GetCustomMorphValue(Idx);
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
								if (Idx < Value->CustomMorphValues.Num())
								{
									Value->SetCustomMorphValue(Idx, NewValue);
								}
							}
						})
					.OnValueCommitted_Lambda([=](float NewValue, ETextCommit::Type CommitType)
						{
							FCustomCharacterRuntimeData CustomSkeletionRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
							CustomSkeletionRuntimeData.SetCustomMorphValue(Idx, NewValue);
							FPropertyCustomizeHelper::SetValue(StructPropertyHandle, CustomSkeletionRuntimeData);
						})
				];
		}

		TArray<FCustomMaterialFloatEntry>& MaterialFloatData = CustomSkeletionRuntimeData.CustomConfig->MaterialFloatData;
		for (int32 Idx = 0; Idx < MaterialFloatData.Num(); ++Idx)
		{
			const FCustomMaterialFloatEntry& Entry = MaterialFloatData[Idx];
			GroupMap[*Entry.Category.ToString()]->AddWidgetRow().NameContent()
				[
					SNew(STextBlock)
					.Text(Entry.DisplayName)
					//.ToolTipText(Entry.GetMorphsDesc())
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
							if (Idx < CustomSkeletionRuntimeData.CustomMaterialFloatValues.Num())
							{
								return CustomSkeletionRuntimeData.CustomMaterialFloatValues[Idx];
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
								if (Idx < Value->CustomMaterialFloatValues.Num())
								{
									Value->CustomMaterialFloatValues[Idx] = NewValue;
								}
							}
						})
					.OnValueCommitted_Lambda([=](float NewValue, ETextCommit::Type CommitType)
						{
							FCustomCharacterRuntimeData CustomSkeletionRuntimeData = FPropertyCustomizeHelper::GetValue<FCustomCharacterRuntimeData>(StructPropertyHandle);
							CustomSkeletionRuntimeData.CustomMaterialFloatValues[Idx] = NewValue;
							FPropertyCustomizeHelper::SetValue(StructPropertyHandle, CustomSkeletionRuntimeData);
						})
				];
		}

 		TArray<FCustomMaterialColorEntry>& MaterialColorData = CustomSkeletionRuntimeData.CustomConfig->MaterialColorData;
 		for (int32 Idx = 0; Idx < MaterialColorData.Num(); ++Idx)
 		{
 			const FCustomMaterialColorEntry& Entry = MaterialColorData[Idx];
			TSharedRef<IPropertyHandleArray> CustomMaterialColorValues_Handle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCustomCharacterRuntimeData, CustomMaterialColorValues))->AsArray().ToSharedRef();
			uint32 NumElements;
			CustomMaterialColorValues_Handle->GetNumElements(NumElements);
			if (int32(NumElements) > Idx)
			{
				//TODO 还是会显示Array存在的拖拽功能，直接自己写吧
				IDetailPropertyRow& DetailPropertyRow = GroupMap[*Entry.Category.ToString()]->AddPropertyRow(CustomMaterialColorValues_Handle->GetElement(Idx));
				DetailPropertyRow.DisplayName(Entry.DisplayName);
				DetailPropertyRow.GetPropertyHandle()->SetOnChildPropertyValuePreChange(FSimpleDelegate::CreateLambda([=]()
					{
						StructPropertyHandle->NotifyPreChange();
					}));
				DetailPropertyRow.GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
					{
						StructPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
					}));
			}
 		}
 
 		TArray<FCustomMaterialTextureEntry>& MaterialTextureData = CustomSkeletionRuntimeData.CustomConfig->MaterialTextureData;
 		for (int32 Idx = 0; Idx < MaterialTextureData.Num(); ++Idx)
 		{
			//TODO 从预制的地方选择
 			const FCustomMaterialTextureEntry& Entry = MaterialTextureData[Idx];
			TSharedRef<IPropertyHandleArray> CustomMaterialTextureValues_Handle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FCustomCharacterRuntimeData, CustomMaterialTextureValues))->AsArray().ToSharedRef();
			uint32 NumElements;
			CustomMaterialTextureValues_Handle->GetNumElements(NumElements);
			TSharedRef<IPropertyHandle> Element = CustomMaterialTextureValues_Handle->GetElement(Idx);
			Element->SetOnChildPropertyValuePreChange(FSimpleDelegate::CreateLambda([=]()
				{
					StructPropertyHandle->NotifyPreChange();
				}));
			Element->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
				{
					StructPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
				}));

			if (int32(NumElements) > Idx)
			{
				GroupMap[*Entry.Category.ToString()]->AddWidgetRow().NameContent()
					[
						SNew(STextBlock)
						.Text(Entry.DisplayName)
						//.ToolTipText(Entry.GetMorphsDesc())
					]
				.ValueContent()
					[
						Element->CreatePropertyValueWidget()
					];
			}
 		}
	}
}

#undef LOCTEXT_NAMESPACE
