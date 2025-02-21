#include "EasyBPMetaDataDetailCustomization.h"


#include "Kismet2/BlueprintEditorUtils.h"
#include "DetailLayoutBuilder.h"
#include "BlueprintEditorModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "K2Node_Variable.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Engine/BlueprintGeneratedClass.h"

#define LOCTEXT_NAMESPACE "EasyBPMetaDataDetailCustomization"

TSharedPtr<IDetailCustomization> EasyBPMetaDataDetailCustomization::MakeInstance(TSharedPtr<IBlueprintEditor> InBlueprintEditor)
{
	const TArray<UObject*>* Objects = (InBlueprintEditor.IsValid() ? InBlueprintEditor->GetObjectsCurrentlyBeingEdited() : nullptr);
	if (Objects && Objects->Num() == 1)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>((*Objects)[0]))
		{
			return MakeShareable(new EasyBPMetaDataDetailCustomization(InBlueprintEditor, Blueprint));
		}
	}
	return nullptr;
}

UK2Node_Variable* EasyBPMetaDataDetailCustomization::EdGraphSelectionAsVar() const
{
	TWeakPtr<FBlueprintEditor> BlueprintEditor = MyBlueprint.Pin()->GetBlueprintEditor();

	if( BlueprintEditor.IsValid() )
	{
		/** Get the currently selected set of nodes */
		FGraphPanelSelectionSet Objects = BlueprintEditor.Pin()->GetSelectedNodes();

		if (Objects.Num() == 1)
		{
			FGraphPanelSelectionSet::TIterator Iter(Objects);
			UObject* Object = *Iter;

			if (Object && Object->IsA<UK2Node_Variable>())
			{
				return Cast<UK2Node_Variable>(Object);
			}
		}
	}
	return nullptr;
}

bool EasyBPMetaDataDetailCustomization::IsALocalVariable(FProperty* VariableProperty) const
{
	return VariableProperty && (VariableProperty->GetOwner<UFunction>() != NULL);
}

bool EasyBPMetaDataDetailCustomization::IsAUserVariable(FProperty* VariableProperty) const
{
	FObjectProperty* VariableObjProp = VariableProperty ? CastField<FObjectProperty>(VariableProperty) : NULL;

	if (VariableObjProp != NULL && VariableObjProp->PropertyClass != NULL)
	{
		return FBlueprintEditorUtils::IsVariableCreatedByBlueprint(GetBlueprintObj(), VariableObjProp);
	}
	return true;
}

bool EasyBPMetaDataDetailCustomization::IsABlueprintVariable(FProperty* VariableProperty) const
{
	UClass* VarSourceClass = VariableProperty ? VariableProperty->GetOwner<UClass>() : NULL;
	if(VarSourceClass)
	{
		return (VarSourceClass->ClassGeneratedBy != NULL);
	}
	return false;
}

FProperty* EasyBPMetaDataDetailCustomization::SelectionAsProperty() const
{
	if (FEdGraphSchemaAction_BlueprintVariableBase* BPVar = MyBlueprint.Pin()->SelectionAsBlueprintVariable())
	{
		return BPVar->GetProperty();
	}
	else if (UK2Node_Variable* GraphVar = EdGraphSelectionAsVar())
	{
		return GraphVar->GetPropertyForVariable();
	}

	return nullptr;
}

FName EasyBPMetaDataDetailCustomization::GetVariableName() const
{
	if (FEdGraphSchemaAction_BlueprintVariableBase* BPVar = MyBlueprint.Pin()->SelectionAsBlueprintVariable())
	{
		return BPVar->GetVariableName();
	}
	else if (UK2Node_Variable* GraphVar = EdGraphSelectionAsVar())
	{
		return GraphVar->GetVarName();
	}

	return NAME_None;
}

void EasyBPMetaDataDetailCustomization::OnPostEditorRefresh()
{
	CachedVariableProperty = SelectionAsProperty();
	CachedVariableName = GetVariableName();
}

UBlueprint* EasyBPMetaDataDetailCustomization::GetPropertyOwnerBlueprint() const
{
	FProperty* VariableProperty = CachedVariableProperty.Get();

	// Cache the Blueprint which owns this VariableProperty
	if (UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(VariableProperty->GetOwnerClass()))
	{
		return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
	}
	return nullptr;
}

EVisibility EasyBPMetaDataDetailCustomization::BPMetaDataVisibility() const
{
	FProperty* VariableProperty = CachedVariableProperty.Get();

	// Cache the Blueprint which owns this VariableProperty
	if (VariableProperty && GetPropertyOwnerBlueprint())
	{
		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
		FEdGraphPinType VariablePinType;
		K2Schema->ConvertPropertyToPinType(VariableProperty, VariablePinType);

		const bool bShowPrivacySetting = IsABlueprintVariable(VariableProperty) && IsAUserVariable(VariableProperty) && !IsALocalVariable(VariableProperty);
		FObjectPropertyBase* ObjectProperty  = CastField<FObjectPropertyBase>(VariableProperty);
		if (bShowPrivacySetting && (K2Schema->FindSetVariableByNameFunction(VariablePinType) != NULL) && ObjectProperty == nullptr)
		{
			return EVisibility::Visible;
		}
	}
	return EVisibility::Collapsed;
}

void EasyBPMetaDataDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CachedVariableProperty = SelectionAsProperty();

	if(!CachedVariableProperty.IsValid())
	{
		return;
	}

	CachedVariableName = GetVariableName();

	TWeakPtr<FBlueprintEditor> BlueprintEditor = MyBlueprint.Pin()->GetBlueprintEditor();
	if( BlueprintEditor.IsValid() )
	{
		BlueprintEditor.Pin()->OnRefresh().AddSP(this, &EasyBPMetaDataDetailCustomization::OnPostEditorRefresh);
	}
	
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Easy Blueprint Meta Data",FText::GetEmpty(),ECategoryPriority::Uncommon);
	const FSlateFontInfo DetailFontInfo = IDetailLayoutBuilder::GetDetailFont();

	Category.AddCustomRow(LOCTEXT("EasyBPMetaData", "Defined Meta Data List"), true)
		.WholeRowWidget
		[
			SNew(STextBlock)
			.ToolTipText(LOCTEXT("EasyBPMetaData", "Defined Meta Data List"))
			.Text( LOCTEXT("EasyBPMetaData", "Defined Meta Data List"))
			.Font( IDetailLayoutBuilder::GetDetailFontBold() )
		];

	Category.AddCustomRow(LOCTEXT("EasyBPMetaData", "Modify Meta Data"), true)
			.WholeRowWidget
			[
				SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(STextBlock)
							.Text(FText::FromString("Key"))
							.ToolTipText(FText::FromString("Key"))
							.Font( IDetailLayoutBuilder::GetDetailFontBold() )
					]

					+SHorizontalBox::Slot()
						.AutoWidth()
					[
						SNew(STextBlock)
							.Text(FText::FromString("Value"))
							.ToolTipText(FText::FromString("Value"))
							.Font( IDetailLayoutBuilder::GetDetailFontBold() )
					]
			];

	Category.AddCustomRow(FText::GetEmpty(), true)
	.WholeRowWidget
	[
		SAssignNew(MetaDataWidget, SListView< TSharedPtr< FMetaDataPair > >)
			.OnGenerateRow(this, &EasyBPMetaDataDetailCustomization::OnGenerateWidgetForPropertyList)
			.ListItemsSource(&MetaDataList)
			.SelectionMode(ESelectionMode::None)
	];
	
	Category.AddCustomRow(LOCTEXT("EasyBPMetaData", "Modify Meta Data"), true)
			.Visibility(TAttribute<EVisibility>(this, &EasyBPMetaDataDetailCustomization::BPMetaDataVisibility))
			.WholeRowWidget
			[
				SNew(STextBlock)
				.ToolTipText(LOCTEXT("EasyBPMetaData", "Modify Meta Data"))
				.Text( LOCTEXT("EasyBPMetaData", "Modify Meta Data"))
				.Font( IDetailLayoutBuilder::GetDetailFontBold() )
			];

	
	
	Category.AddCustomRow(FText::GetEmpty(), true)
	.Visibility(TAttribute<EVisibility>(this, &EasyBPMetaDataDetailCustomization::BPMetaDataVisibility))
	.WholeRowWidget
	[
		SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(5)
			[
				SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
							.Text(FText::FromString("Key: "))
							.Font( IDetailLayoutBuilder::GetDetailFont() )
					]

					+SHorizontalBox::Slot()
					[
						SAssignNew(EditKeyWidget, SEditableTextBox)
							.Font( IDetailLayoutBuilder::GetDetailFont() )
					]
			]

			+SVerticalBox::Slot()
			.Padding(5)
			[
				SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
							.Text(FText::FromString("Value: "))
							.Font( IDetailLayoutBuilder::GetDetailFont() )
					]

					+SHorizontalBox::Slot()
					[
						SAssignNew(EditValueWidget, SEditableTextBox)
							.Font( IDetailLayoutBuilder::GetDetailFont() )
					]
			]

			+SVerticalBox::Slot()
			.Padding(5)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.Padding(5,3)
					[
						SNew(SButton)
							.Text(FText::FromString("Add"))
							.OnClicked(this,&EasyBPMetaDataDetailCustomization::OnButtonAddClicked)
							
					]

					+SHorizontalBox::Slot()
					.Padding(5,3)
					[
						SNew(SButton)
							.Text(FText::FromString("Remove"))
							.OnClicked(this,&EasyBPMetaDataDetailCustomization::OnButtonRemoveClicked)
					]

					+SHorizontalBox::Slot()
					.Padding(5,3)
					[
						SNew(SButton)
							.Text(FText::FromString("Refresh"))
							.OnClicked(this,&EasyBPMetaDataDetailCustomization::OnButtonRefreshClicked)
					]
			]
		
	];

	RefreshMetaDataList();
	
}

TSharedRef<ITableRow> EasyBPMetaDataDetailCustomization::OnGenerateWidgetForPropertyList( TSharedPtr< FMetaDataPair > Item, const TSharedRef<STableViewBase>& OwnerTable )
{
	return SNew(STableRow< TSharedPtr< FString > >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(STextBlock)
					.Text(FText::FromName(Item->Key))
					.ToolTipText(FText::FromName(Item->Key))
					.Font( IDetailLayoutBuilder::GetDetailFont() )
			]

			+SHorizontalBox::Slot()
				.AutoWidth()
			[
				SNew(STextBlock)
					.Text(FText::FromString(Item->Value))
					.ToolTipText(FText::FromString(Item->Value))
					.Font( IDetailLayoutBuilder::GetDetailFont() )
			]
		];
}

void EasyBPMetaDataDetailCustomization::RefreshMetaDataList()
{
	FProperty* VariableProperty = CachedVariableProperty.Get();
	if(VariableProperty)
	{
		MetaDataList.Empty();
		const TMap<FName, FString>* MetaDataMap = VariableProperty->GetMetaDataMap();
		if(MetaDataMap)
		{
			for(const auto& It : (*MetaDataMap))
			{
				MetaDataList.Add(MakeShareable<FMetaDataPair>(new FMetaDataPair(It.Key,It.Value)));
			}
		}
		

		MetaDataWidget.Pin()->RequestListRefresh();
	}
}

FReply EasyBPMetaDataDetailCustomization::OnButtonAddClicked()
{
	const FName VarName = CachedVariableName;
	if(!(EditKeyWidget.IsValid()) || !(EditValueWidget.IsValid()))
	{
		return FReply::Handled();
	}
	TSharedPtr< SEditableTextBox > KeyBox = EditKeyWidget.Pin();
	TSharedPtr< SEditableTextBox > ValueBox = EditValueWidget.Pin();
	if(KeyBox->GetText().IsEmpty())
	{
		return FReply::Handled();
	}
	FBlueprintEditorUtils::SetBlueprintVariableMetaData(GetBlueprintObj(), VarName, nullptr, FName(KeyBox->GetText().ToString()), ValueBox->GetText().ToString());
	RefreshMetaDataList();
	return FReply::Handled();
}

FReply EasyBPMetaDataDetailCustomization::OnButtonRemoveClicked()
{
	const FName VarName = CachedVariableName;
	
	if(!(EditKeyWidget.IsValid()))
	{
		return FReply::Handled();
	}
	TSharedPtr< SEditableTextBox > KeyBox = EditKeyWidget.Pin();
	if(KeyBox->GetText().IsEmpty())
	{
		return FReply::Handled();
	}
	FBlueprintEditorUtils::RemoveBlueprintVariableMetaData(GetBlueprintObj(), VarName, nullptr, FName(KeyBox->GetText().ToString()));

	RefreshMetaDataList();
	return FReply::Handled();
}

FReply EasyBPMetaDataDetailCustomization::OnButtonRefreshClicked()
{
	RefreshMetaDataList();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
