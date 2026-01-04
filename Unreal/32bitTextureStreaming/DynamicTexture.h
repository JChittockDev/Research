#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Rendering/Texture2DResource.h"
#include "GameFramework/GameModeBase.h"


struct DynamicTexturePixel
{
	float b;
	float g;
	float r;
	float a;
};

void SetDynamicTexturePixelsToColor(DynamicTexturePixel* pixels, int32 numPixels,
                                    DynamicTexturePixel color);

struct DynamicTexture
{
private:
	TArray<class UMaterialInstanceDynamic*> DynamicMaterials;
	UTexture2D* Texture2D;

public:
	int32 Width;
	int32 Height;
	int32 NumPixels;
	DynamicTexturePixel* Pixels;
	~DynamicTexture();

	void Initialize(USkeletalMeshComponent* mesh, int32 materialIndex, int32 width,
	                int32 height);

	void Update();
};