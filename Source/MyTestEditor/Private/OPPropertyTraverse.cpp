#include "OPPropertyTraverse.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "WidgetBlueprint.h"
#include "InstancedStruct.h" 
#include "EngineUtils.h"
#include "LevelSequence.h"
#include "UObject/Package.h"

UE_DISABLE_OPTIMIZATION

static FAutoConsoleCommandWithWorldAndArgs DebugText(TEXT("U01.TESTPropertyTraverse"), TEXT("Arg1"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			FOPPropertyTranverse<FTextProperty>::TEST_DirectoryAsset();  
		}));

template<typename T>
void FOPPropertyTranverse<T>::TEST_DirectoryAsset()
{
	FString AssetDir = TEXT("/Game/Developers/liuyy2/TestAsset/");
	// 获取Asset Registry Module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	// 生成一个包含目录路径的过滤器
	FARFilter Filter;
	Filter.PackagePaths.Add(*AssetDir); 
	Filter.bRecursivePaths = true; // 如果你也想包含子目录中的资源，将其设置为true

	// 保存找到的资源/Script/Engine.Blueprint'/Game/Outpost/Blueprints/GameModule/InteractiveObject/Cars/OP_SIO_CarDoors_SitToilet.OP_SIO_CarDoors_SitToilet'
	TArray<FAssetData> AssetDataArray;
	//AssetRegistryModule.Get().GetAssets(Filter, AssetDataArray);
	//AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/Outpost/Blueprints/GameModule/InteractiveObject/Cars/OP_SIO_CarDoors_SitToilet.OP_SIO_CarDoors_SitToilet"))));
	//AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/Outpost/DataTable/Mission/DT_OP_OPMissionChain.DT_OP_OPMissionChain"))));
	//AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/Developers/liuyy2/NewWorld.NewWorld"))));
	//AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/Outpost/Maps/Logic/MainStory/MainStory01/Sequences/L1_RecoverDayIntro.L1_RecoverDayIntro"))));
	//AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/Outpost/Blueprints/UI/Settings/Window_Settings_Sound.Window_Settings_Sound"))));
	//AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/Outpost/Maps/Logic/LootMap/General/OP_Map_E_Small_WildCamp02_Intro.OP_Map_E_Small_WildCamp02_Intro"))));
	AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/TestKit/MyTestActor.MyTestActor"))));
	AssetDataArray.Add(AssetRegistryModule.Get().GetAssetByObjectPath(FName(TEXT("/Game/Map/MyTestLevel.MyTestLevel")))); 


	auto PropertyFunc = [](UObject*,FTextProperty* Prop, uint8* Val)
		{
			FText InOutText = Prop->GetPropertyValue(Val);
			FString Str = InOutText.ToString();
			if (Str == TEXT("<MISSING STRING TABLE ENTRY>"))
			{
				UE_LOG(LogTemp, Display, TEXT("Temp"));
			}
			TOptional<FString> Key = FTextInspector::GetKey(InOutText);
			TOptional<FString> NS = FTextInspector::GetNamespace(InOutText); 
			if (Key.IsSet()&&NS.IsSet()&&!InOutText.IsEmpty())
			{
				const FString StrKey = FString::Printf(TEXT("%s::%s"), *Key.GetValue(), *NS.GetValue()); 
				UE_LOG(LogTemp, Display, TEXT("Key : %s , Value : %s"), *StrKey, *InOutText.ToString());
			}
		};
	auto GraphPinFunc = [](UObject*, UEdGraphPin* InPin)
		{
			FText InText = InPin->GetDefaultAsText();
			TOptional<FString> Key = FTextInspector::GetKey(InText);
			TOptional<FString> NS = FTextInspector::GetNamespace(InText);
			if (Key.IsSet()&&NS.IsSet()&&!InText.IsEmpty())
			{
				const FString StrKey = FString::Printf(TEXT("%s::%s"), *Key.GetValue(), *NS.GetValue());
				UE_LOG(LogTemp, Display, TEXT("Key : %s , Value : %s"), *StrKey, *InText.ToString());
			}
		};

	FOPPropertyTranverse<FTextProperty> Tranverse(PropertyFunc, GraphPinFunc, UEdGraphSchema_K2::PC_Text);

	// 遍历并打印资源信息
	for (const FAssetData& AssetData : AssetDataArray)
	{
		UPackage* Pkg = AssetData.GetPackage();
		TArray<UPackage*> ExtPkg = Pkg->GetExternalPackages(); 
		TArray<UObject*> Objs;
		GetObjectsWithPackage(Pkg, Objs);
		UE_LOG(LogTemp, Log, TEXT("Found asset: %s"), *AssetData.AssetName.ToString());
		Tranverse.DoProcessAssetData(AssetData);
	}
}

// UImportAssetsCommandlet::LoadLevel
UWorld* LoadLevel(const FString& LevelToLoad) 
{
	bool bResult = false;

	if (!LevelToLoad.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Loading Map %s"), *LevelToLoad); 

		FString Filename;
		if (FPackageName::TryConvertLongPackageNameToFilename(LevelToLoad, Filename))
		{
			UPackage* Package = LoadPackage(NULL, *Filename, 0);

			UWorld* World = UWorld::FindWorldInPackage(Package);
			if (World)
			{
				// Clean up any previous world.  The world should have already been saved
				UWorld* ExistingWorld = GEditor->GetEditorWorldContext().World();

				GEngine->DestroyWorldContext(ExistingWorld);
				ExistingWorld->DestroyWorld(true, World);

				GWorld = World;

				World->WorldType = EWorldType::Editor;

				FWorldContext& WorldContext = GEngine->CreateNewWorldContext(World->WorldType);
				WorldContext.SetCurrentWorld(World);

				// add the world to the root set so that the garbage collection to delete replaced actors doesn't garbage collect the whole world
				World->AddToRoot();

				// initialize the levels in the world
				World->InitWorld(UWorld::InitializationValues().AllowAudioPlayback(false));
				World->GetWorldSettings()->PostEditChange();
				World->UpdateWorldComponents(true, false);
				World->SetShouldForceVisibleStreamingLevels(true);
				World->UpdateLevelStreaming();
				World->FlushLevelStreaming();
				
				bResult = true;
			}
			return World;
		}
	}
	return nullptr;
}

template<typename T>
void FOPPropertyTranverse<T>::DoProcessAssetData(const FAssetData& InAssets)
{
	UObject* Obj = InAssets.GetAsset();
	
	BeginProcessObject(Obj);
}

template<typename T>
void FOPPropertyTranverse<T>::BeginProcessObject(UObject* InObj)
{
	{
		ProcessObject(InObj);
	} 
}

template<typename T>
void FOPPropertyTranverse<T>::ProcessBlueprintGraph(UBlueprint* BP)
{
	TArray<UEdGraph*> Graphs; 
	BP->GetAllGraphs(Graphs);   
	// the default value of Graph node pin 
	for (auto G : Graphs) 
	{
		const UEdGraphSchema* Schema = G->GetSchema(); 
		for (auto Nodes : G->Nodes) 
		{
			for (auto Pins : Nodes->Pins) 
			{
				if (Pins->PinType.PinCategory == GraphPinType) 
				{
					GraphPinProccessor(ObjContexts.Top(), Pins);
				}
			}
		}
	}
	if (UWidgetBlueprint* WBP = Cast<UWidgetBlueprint>(BP))     
	{
		WBP->ForEachSourceWidget([&](UWidget* W)  
			{
				ProcessObject((UObject*)W);
			});
	}
}

template<typename T>
void FOPPropertyTranverse<T>::ProcessObject(UObject* Obj)
{
	if (!Obj || !Obj->GetClass() || Obj->IsA<UMaterial>()) return;  
	UClass* Cls = Obj->GetClass(); 
	if (ProccessedObjs.Find(Obj) != INDEX_NONE)
	{
		return;
	}
	else if(bTrackProccessed) 
	{
		ProccessedObjs.Add(Obj); 
	}

	if (UDataTable* DT = Cast<UDataTable>(Obj))
	{
		ObjContexts.Push(Obj);
		for (auto Itr = DT->GetRowMap().CreateConstIterator(); Itr; ++Itr)
		{
			ProcessStruct(DT->RowStruct, Itr->Value);
		}
		ObjContexts.Pop();
	}
	else if (UBlueprint* BP = Cast<UBlueprint>(Obj))
	{
		ObjContexts.Push(Obj);
		ProcessBlueprintGraph(BP);
		ObjContexts.Pop();

		ProcessObject(BP->GeneratedClass->GetDefaultObject());
		ProcessObject(BP->GeneratedClass);
	}
	else if (UWorld* World = Cast<UWorld>(Obj))
	{
		// Full initlialize the world 
		auto RealWorld = LoadLevel(World->GetPackage()->GetPathName());  
		UObject* LeveScriptObj = (UObject*)RealWorld->PersistentLevel->GetLevelScriptBlueprint();
		BeginProcessObject(LeveScriptObj);

		const int32 PreviousNum = GetProccessedObjectNum();
		//for (ULevel* Level : RealWorld->GetLevels())
		{
			/*FName LevelPath = FName(Level->GetPackage()->GetFullName());
			if (ProccessedLevels.Contains(LevelPath))
			{
				continue;
			}
			ProccessedLevels.Add(LevelPath); */
			ULevel* CurrentLevel = RealWorld->PersistentLevel;
			for (int32 Ind = 0; Ind < CurrentLevel->Actors.Num(); ++Ind) 
			{
				ProcessObject(CurrentLevel->Actors[Ind]); 
			}
		}
		ResetProccessedObjectNum(PreviousNum);
	}else if (ULevelSequence* LS = Cast<ULevelSequence>(Obj)) 
	{
		//G:\UnrealEngine5\Engine\Source\Runtime\LevelSequence\Public\LevelSequence.h
		ObjContexts.Push(LS);
		BeginProcessObject(LS->GetDirectorBlueprint()); 
		ObjContexts.Pop(); 
	}
	ObjContexts.Push(Obj);
	for (TFieldIterator<FProperty> PropItr(Cls); PropItr; ++PropItr)
	{
		FProperty* Prop = *PropItr;
		void* PropVlu = Prop->ContainerPtrToValuePtr<void>(Obj);

		ProcessPropertyValue(Prop, PropVlu); 
	}
	ObjContexts.Pop(); 
}

template<typename T>
void FOPPropertyTranverse<T>::ProcessStruct(const UScriptStruct* InStruct, void* InData)
{
	if (InStruct == FInstancedStruct::StaticStruct())
	{
		//G:\UnrealEngine5\Engine\Plugins\Experimental\StructUtils\Source\StructUtils\Public\InstancedStruct.h
		FInstancedStruct* InstancedStruct = reinterpret_cast<FInstancedStruct*>(InData); 
		const void* Data = reinterpret_cast<const void*>(InstancedStruct->GetMemory());
		ProcessStruct(InstancedStruct->GetScriptStruct(), const_cast<void*>(Data)); 
	}
	for (TFieldIterator<FProperty> Itr(InStruct); Itr; ++Itr)
	{
		FProperty* BaseProp = *Itr;
		void* Data = BaseProp->ContainerPtrToValuePtr<void>(InData);
		ProcessPropertyValue(BaseProp, Data);
	}
}

UE_ENABLE_OPTIMIZATION