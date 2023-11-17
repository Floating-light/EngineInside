#pragma once 

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShaderBlueprintFunction.generated.h"

UCLASS()
class MYTEST_API UShaderBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static void DrawMyTestShader(const UObject* WorldContextObject,UTexture2D* InTex, TArray<UTexture2D*> TextureArray, class UTextureRenderTarget2D* OutputRenderTarget,FVector4f InColor);
};

