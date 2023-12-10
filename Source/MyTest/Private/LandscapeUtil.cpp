#include "LandscapeUtil.h"

#include "Landscape.h"
#include "EngineUtils.h"
DEFINE_LOG_CATEGORY_STATIC(LogLandscapeUtils,Log, All)

//G:\workspace\UnrealEngine\Engine\Source\Runtime\Landscape\Classes\Landscape.h
void ULandscapeBlueprintFunctionLibrary::PrintLandscapeComponentSizeInfo(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return;
	}
	UWorld* World = WorldContextObject->GetWorld();

	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		UE_LOG(LogLandscapeUtils, Display, TEXT("============================ Landscape: %s ============================"),* It->GetName());

		for (int32 i = 0; i < It->LandscapeComponents.Num(); ++i)
		{
			// https://blog.csdn.net/qq_29523119/article/details/125532143
			// https://zhuanlan.zhihu.com/p/668278748?utm_id=0
			ULandscapeComponent* Comp = It->LandscapeComponents[i];
			check(Comp->GetHeightmap()->GetSizeX() == Comp->GetHeightmap()->GetSizeY());
			UE_LOG(LogLandscapeUtils, Display, TEXT("%3d. SectionBaseXY: (%3d, %3d), ComponentSizeQuads: %3d, SubsectionSizeQuads: %3d, NumSubsections: %3d, Heightmap Texture: %p, Heightmap Size: %3d,HeightmapScaleBias: %s"), 
				i, Comp->SectionBaseX,Comp->SectionBaseY,Comp->ComponentSizeQuads,Comp->SubsectionSizeQuads, Comp->NumSubsections, Comp->GetHeightmap(),Comp->GetHeightmap()->GetSizeX(), *Comp->HeightmapScaleBias.ToString());
		}
		UE_LOG(LogLandscapeUtils, Display, TEXT("============================ End Landscape: %s ========================="), *It->GetName());

	}
}
