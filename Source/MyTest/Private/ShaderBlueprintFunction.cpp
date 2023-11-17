#include "ShaderBlueprintFunction.h"
#include "Engine/TextureRenderTarget2D.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "PixelShaderUtils.h"

class FMyTestPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FMyTestPS);
    SHADER_USE_PARAMETER_STRUCT(FMyTestPS, FGlobalShader);  
public:
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
        SHADER_PARAMETER(FVector4f, MyColor)
        SHADER_PARAMETER(uint32, InTextureNum)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, MyTexture)
        SHADER_PARAMETER_SAMPLER(SamplerState, MyTextureSampler)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2DArray<float4>, InTextures)
        RENDER_TARGET_BINDING_SLOTS()
    END_SHADER_PARAMETER_STRUCT()

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment); 
        // 添加自己的着色器代码定义
        OutEnvironment.SetDefine(TEXT("MY_DEFINE"), 1);
    }

    static bool ShouldCache(EShaderPlatform Platform)
    {
        // 例如，可跳过 "Platform == SP_METAL" 的编译
        return true;
    }

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& InParameters)
    {
        return true;
    }
    static void DrawSomething(FRDGBuilder& GraphBuilder, FParameters* InParameters, const FIntRect& InRenderTargetArea)
    {
        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        TShaderMapRef<FMyTestPS> PixelShader(ShaderMap);

        FPixelShaderUtils::AddFullscreenPass(
            GraphBuilder,
            ShaderMap,
            RDG_EVENT_NAME("MyTest_DrawColor"),
            PixelShader,
            InParameters,
            InRenderTargetArea
            );
    }
};

IMPLEMENT_SHADER_TYPE(, FMyTestPS, TEXT("/Plugin/MyTest/Private/MergeImage.usf"), TEXT("MainPS"), SF_Pixel);

UShaderBlueprintFunctionLibrary::UShaderBlueprintFunctionLibrary(FObjectInitializer const& Initilializer):
    Super(Initilializer)
{
}

void UShaderBlueprintFunctionLibrary::DrawMyTestShader(const UObject* WorldContextObject, UTexture2D* InTex, TArray<UTexture2D*> TextureArray, UTextureRenderTarget2D* OutputRenderTarget, FVector4f InColor)
{
    FTextureRenderTarget2DResource* OutputRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource()->GetTextureRenderTarget2DResource();
    
    ENQUEUE_RENDER_COMMAND(DrawMyTestShader)([InTex, TextureArray, OutputRenderTargetResource, InColor](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList, RDG_EVENT_NAME("DrawMyTestShader"));
            FRDGTextureRef OutputTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(OutputRenderTargetResource->GetTextureRHI(), TEXT("DrawMyTestShader"))); 

            FRHITexture* RHITexture = InTex->GetResource()->GetTexture2DRHI(); 
                // Texture Array
                FRDGTextureDesc Desc = FRDGTextureDesc::Create2DArray(FIntPoint(256), PF_A32B32G32R32F, FClearValueBinding::None,
                    TexCreate_RenderTargetable | TexCreate_TargetArraySlicesIndependently | TexCreate_ShaderResource,
                    8, /*InNumMips = */1, /*InNumSamples = */1);
                FRDGTextureRef InTexureArray = GraphBuilder.CreateTexture(Desc, TEXT("TestTextureArray"));
                FRDGTextureSRVRef TextureArrayRef = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(InTexureArray));

                TArray<UTexture2D*> InTexture = TextureArray;
                for (int32 i = 0; i < InTexture.Num(); ++i)
                {
                    FRHITexture* RHITex = InTexture[i]->GetResource()->GetTexture2DRHI(); 
                    TRefCountPtr<IPooledRenderTarget> LocalPooledRenderTarget = CreateRenderTarget(RHITex, TEXT("MyTestTexture"));
                    FRDGTextureRef FinalRDGTexture = GraphBuilder.RegisterExternalTexture(LocalPooledRenderTarget, TEXT("MyTestRDGTexture"));

                    FRHICopyTextureInfo CopyInfo;
                    CopyInfo.Size = FIntVector(256,256,0);
                    CopyInfo.DestSliceIndex = i;
                    CopyInfo.SourcePosition = FIntVector(0, 0,0);

                    AddCopyTexturePass(GraphBuilder, FinalRDGTexture, InTexureArray, CopyInfo);
                }
            TRefCountPtr<IPooledRenderTarget> PooledRenderTarget = CreateRenderTarget(RHITexture, TEXT("MyTestTexture"));
            FRDGTextureRef RDGTexture = GraphBuilder.RegisterExternalTexture(PooledRenderTarget, TEXT("MyTestRDGTexture")); 

            FMyTestPS::FParameters* Param = GraphBuilder.AllocParameters<FMyTestPS::FParameters>();
            Param->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::ENoAction);
            Param->MyColor = InColor;
            Param->MyTexture = RDGTexture;
            Param->MyTextureSampler = TStaticSamplerState<SF_Trilinear>::GetRHI(); 
            Param->InTextures = TextureArrayRef;

            FIntVector DrawSize = OutputTexture->Desc.GetSize();
            FMyTestPS::DrawSomething(GraphBuilder, Param, FIntRect(0,0,DrawSize.X, DrawSize.Y));

            GraphBuilder.Execute();
        });
}
