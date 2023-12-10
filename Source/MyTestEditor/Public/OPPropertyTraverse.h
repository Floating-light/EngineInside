#pragma once 

#include "CoreMinimal.h"
#include "EdGraphSchema_K2.h"

template <typename T>
class FOPPropertyTranverse
{
	using FTargetPropertyType = T;  
public:
	using FPropertyFuncType = TFunctionRef<void(UObject*,FTargetPropertyType*, uint8*)>;
	using FGraphPinFuncType = TFunctionRef<void(UObject*, UEdGraphPin*)>;

	FOPPropertyTranverse(FPropertyFuncType InPropertyFunc, FGraphPinFuncType InGraphPinFunc, FName InPinType = UEdGraphSchema_K2::PC_Text)
		: PropertyProccessor(InPropertyFunc)
		, GraphPinProccessor(InGraphPinFunc)
		, GraphPinType(InPinType) {}

	void DoProcessAssetData(const FAssetData& InAssets);
	void BeginProcessObject(UObject* InObj);
	int32 GetProccessedObjectNum() const { return ProccessedObjs.Num(); };
	void ResetProccessedObjectNum(int32 InNum) { ProccessedObjs.SetNum(InNum, false); }
	static void TEST_DirectoryAsset();
private:
	void ProcessBlueprintGraph(UBlueprint* InBlueprint); 
	void ProcessObject(UObject* InObj);
	void ProcessPropertyValue(FProperty* Prop, void* PropValue);
	void ProcessStruct(const UScriptStruct* InStruct, void* InData);
	void ProcessContainerEntryValue(FProperty* Prop, uint8* PropValue);
	void SkipTrackProccessed() { bTrackProccessed = false; };
	void KeepTrackProccessed() { bTrackProccessed = true;  };
	FPropertyFuncType PropertyProccessor;
	FGraphPinFuncType GraphPinProccessor; 
	FName GraphPinType;
	bool bTrackProccessed = true;
	TArray<UObject*> ProccessedObjs;
	TArray<FName> ProccessedLevels;
	TArray<UObject*> ObjContexts ;
};

template<typename T>
void FOPPropertyTranverse<T>::ProcessPropertyValue(FProperty* Prop, void* PropValue)
{
	if (T* TextProp = CastField<T>(Prop)) 
	{
		// UpdateTextNamespace(TextProp, (uint8*)PropValue); 
		// FText InOutText = TextPropFText->GetPropertyValue(PropValue);
		PropertyProccessor(ObjContexts.Top(), TextProp, (uint8*)PropValue);
	}
	else if (FObjectPtrProperty* ObjPtrProp = CastField<FObjectPtrProperty>(Prop))
	{
		UObject* ObjPtr = ObjPtrProp->GetObjectPropertyValue(PropValue); 
		ProcessObject(ObjPtr);
	}
	else if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop)) 
	{
		UObject* SubObj = ObjProp->GetPropertyValue(PropValue);
		ProcessObject(SubObj); 
	}
	else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
	{
		ProcessStruct(StructProp->Struct, PropValue);
	}
	else if (FArrayProperty* Ap = CastField<FArrayProperty>(Prop))
	{
		FScriptArrayHelper ArrHelper(Ap, PropValue);
		for (int Ind = 0; Ind < ArrHelper.Num(); ++Ind)
		{
			uint8* ArrayEntryData = ArrHelper.GetRawPtr(Ind);
			ProcessContainerEntryValue(Ap->Inner, ArrayEntryData);
		}
	}
	else if (FMapProperty* Mp = CastField<FMapProperty>(Prop))
	{
		if (!Mp->KeyProp || !Mp->ValueProp) return;
		Mp->KeyProp->IsA< FTextProperty>();
		Mp->ValueProp->IsA< FTextProperty>();
		FScriptMapHelper MapHelper(Mp, PropValue);
		for (int32 MapSparseIndex = 0; MapSparseIndex < MapHelper.GetMaxIndex(); ++MapSparseIndex)
		{
			if (MapHelper.IsValidIndex(MapSparseIndex))
			{
				uint8* MapKeyData = MapHelper.GetKeyPtr(MapSparseIndex);
				uint8* MapValueData = MapHelper.GetValuePtr(MapSparseIndex);
				check(MapKeyData);
				check(MapValueData);
				ProcessContainerEntryValue(Mp->KeyProp, MapKeyData);
				ProcessContainerEntryValue(Mp->ValueProp, MapValueData);
			}
		}
	}
	else if (FSetProperty* Sp = CastField<FSetProperty>(Prop))
	{
		if (!Sp->ElementProp) return;
		FScriptSetHelper SetHelper(Sp, PropValue);
		for (int32 SetSparseIndex = 0; SetSparseIndex < SetHelper.GetMaxIndex(); ++SetSparseIndex)
		{
			if (SetHelper.IsValidIndex(SetSparseIndex))
			{
				ProcessContainerEntryValue(SetHelper.GetElementProperty(), SetHelper.GetElementPtr(SetSparseIndex));
			}
		}
	}
}

template<typename T>
void FOPPropertyTranverse<T>::ProcessContainerEntryValue(FProperty* Prop, uint8* PropValue)
{
	if (T* TextProp = CastField<T>(Prop)) 
	{
		// UpdateTextNamespace(TextProp, (uint8*)PropValue); 
		// FText InOutText = TextPropFText->GetPropertyValue(PropValue);
		PropertyProccessor(ObjContexts.Top(),TextProp, (uint8*)PropValue);
	}
	else if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
	{
		UObject* SubObj = ObjProp->GetPropertyValue(PropValue);
		ProcessObject(SubObj);
	}
	else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
	{
		StructProp->Struct;
		ProcessStruct(StructProp->Struct, PropValue);
	}
	// can not nest container 
}
