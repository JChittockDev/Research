// Copyright 2022 Joseph Chittock. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "RHICommandList.h"

class TissueIntegerVertexBuffer : public FVertexBufferWithSRV
{
public:
	void Init(const TMap<const int, int>& InVertexData, const char* bufferName)
	{
		VertexData = InVertexData;
		uniqueBufferName = bufferName;
	}

private:
	void InitRHI() override;
	TMap<const int, int> VertexData;
	const char* uniqueBufferName;
};

class TissueFloatVertexBuffer : public FVertexBufferWithSRV
{
public:
	void Init(const TMap<const int, float>& InVertexData, const char* bufferName)
	{
		VertexData = InVertexData;
		uniqueBufferName = bufferName;
	}

private:
	void InitRHI() override;
	TMap<const int, float> VertexData;
	const char* uniqueBufferName;
};