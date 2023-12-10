#pragma once 

#include "Kismet/BlueprintFunctionLibrary.h"
#include "LandscapeUtil.generated.h"

UCLASS()
class MYTEST_API ULandscapeBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void PrintLandscapeComponentSizeInfo(const UObject* WorldContextObject);
};

