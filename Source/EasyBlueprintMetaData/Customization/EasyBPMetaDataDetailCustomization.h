#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "IDetailCustomization.h"
#include "SMyBlueprint.h"
#include "Styling/SlateTypes.h"
#include "UObject/WeakFieldPtr.h"

class IDetailLayoutBuilder;
class IBlueprintEditor;
class UBlueprint;

struct FMetaDataPair
{
	FName Key;
	FString Value;

	FMetaDataPair()
	{
		
	}
	FMetaDataPair(const FName& InKey, const FString& InValue)
		:Key(InKey)
		,Value(InValue)
	{
		
	}
};

class EasyBPMetaDataDetailCustomization  : public IDetailCustomization
{

public:
	EasyBPMetaDataDetailCustomization()
	{
		
	}

	EasyBPMetaDataDetailCustomization(TSharedPtr<IBlueprintEditor> InBlueprintEditor, UBlueprint* Blueprint)
		: BlueprintEditorPtr(InBlueprintEditor)
		, Blueprint(Blueprint)
	{
		if(FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(InBlueprintEditor.Get()))
		{
			MyBlueprint = BlueprintEditor->GetMyBlueprintWidget();
		}
	}

	virtual ~EasyBPMetaDataDetailCustomization() override
	{
		if(MyBlueprint.IsValid())
		{
			// Remove the callback delegate we registered for
			TWeakPtr<FBlueprintEditor> BlueprintEditor = MyBlueprint.Pin()->GetBlueprintEditor();
			if( BlueprintEditor.IsValid() )
			{
				BlueprintEditor.Pin()->OnRefresh().RemoveAll(this);
			}
		}
	}
	
public:
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> InBlueprintEditor);
	
public:
	UK2Node_Variable* EdGraphSelectionAsVar() const;
	bool IsALocalVariable(FProperty* VariableProperty) const;
	bool IsAUserVariable(FProperty* VariableProperty) const;
	bool IsABlueprintVariable(FProperty* VariableProperty) const;
	FProperty* SelectionAsProperty() const;
	FName GetVariableName() const;
	void OnPostEditorRefresh();
	UBlueprint* GetPropertyOwnerBlueprint() const;
	EVisibility BPMetaDataVisibility() const;

public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	UBlueprint* GetBlueprintObj() const { return Blueprint.Get(); }

protected:
	TArray< TSharedPtr< FMetaDataPair > > MetaDataList;

	/** The listview widget for displaying property flags */
	TWeakPtr< SListView< TSharedPtr< FMetaDataPair > > > MetaDataWidget;

	TWeakPtr< SEditableTextBox > EditKeyWidget;
	TWeakPtr< SEditableTextBox > EditValueWidget;
protected:
	TSharedRef<ITableRow> OnGenerateWidgetForPropertyList( TSharedPtr< FMetaDataPair > Item, const TSharedRef<STableViewBase>& OwnerTable );
	void RefreshMetaDataList();
	FReply OnButtonAddClicked();
	FReply OnButtonRemoveClicked();
	FReply OnButtonRefreshClicked();
protected:

	/** The Blueprint editor we are embedded in */
	TWeakPtr<IBlueprintEditor> BlueprintEditorPtr;

	
	TWeakObjectPtr<UBlueprint> Blueprint;
	
	/** Cached property for the variable we are affecting */
	TWeakFieldPtr<FProperty> CachedVariableProperty;

	/** Cached name for the variable we are affecting */
	FName CachedVariableName;

	TWeakPtr<SMyBlueprint> MyBlueprint;
};

