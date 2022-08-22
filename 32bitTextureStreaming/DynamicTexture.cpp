#include "DynamicTexture.h"

void SetDynamicTexturePixelsToColor(DynamicTexturePixel* pixels, int32 numPixels,
                                    DynamicTexturePixel color)
{
	if (pixels)
	{
		for (int i = 0; i < numPixels; i++)
			pixels[i] = color;
	}
}

DynamicTexture::~DynamicTexture()
{
	if (Pixels)
		delete[] Pixels;
}

void DynamicTexture::Initialize(USkeletalMeshComponent* mesh, int32 materialIndex, int32 width,
                                int32 height)
{
	if (!mesh)
		return;

	Width = width;
	Height = height;

	if (!Width || !Height)
	{
		return;
	}

	{
		Texture2D = UTexture2D::CreateTransient(Width, Height, PF_R32_FLOAT);
		Texture2D->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		Texture2D->SRGB = 0;
		Texture2D->AddToRoot();
		Texture2D->UpdateResource();
	}

	{
		NumPixels = Width * Height;
		if (Pixels)
			delete[] Pixels;
		Pixels = new DynamicTexturePixel[NumPixels];

		SetDynamicTexturePixelsToColor(Pixels, NumPixels, {1.0, 0.0, 0.0, 1.0});
	}
	
	{
		UMaterialInstanceDynamic* material =
		    mesh->CreateAndSetMaterialInstanceDynamic(materialIndex);

		if (!material)
			return;

		DynamicMaterials.Empty();
		DynamicMaterials.Add(material);

		for (UMaterialInstanceDynamic* dynamicMaterial : DynamicMaterials)
		{
			if (dynamicMaterial)
				dynamicMaterial->SetTextureParameterValue("DynamicTextureParam", Texture2D);
		}
	}
}

void DynamicTexture::Update()
{
	uint8* data = reinterpret_cast<uint8*>(&Pixels);

	UTexture2D* inputText = Texture2D;

	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)
	([data, inputText](FRHICommandListImmediate& RHICmdList)
	{
		FUpdateTextureRegion2D region;
		region.SrcX = 0;
		region.SrcY = 0;
		region.DestX = 0;
		region.DestY = 0;
		region.Width = 138;
		region.Height = 139;
		FTexture2DResource* resource = (FTexture2DResource*)inputText->Resource;

		RHIUpdateTexture2D(
			resource->GetTexture2DRHI(),
			0,
			region,
			region.Width * sizeof(DynamicTexturePixel),
			data);
	});
}