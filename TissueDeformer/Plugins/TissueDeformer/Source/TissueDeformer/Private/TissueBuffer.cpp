// Copyright 2022 Joseph Chittock. All Rights Reserved.

#include "TissueBuffer.h"

template<typename B>
EPixelFormat ReturnPixelFormat()
{
	if (std::is_same<B, uint32>::value)
	{
		return PF_R32_UINT;
	}
	else
	{
		return PF_R32_FLOAT;
	}
}

template<typename T>
void RHIPush(const char* BufferName, TMap<const int, T>& VertexData, FBufferRHIRef& VertexBuffer, FShaderResourceViewRHIRef& ShaderResource)
{
	if (VertexData.Num() > 0)
	{
		FRHIResourceCreateInfo CreateInfo(*BufferName);
		VertexBuffer = RHICreateVertexBuffer(VertexData.Num() * sizeof(T), BUF_Static | BUF_ShaderResource, CreateInfo);
		T* Data = reinterpret_cast<T*>(RHILockBuffer(VertexBuffer, 0, VertexData.Num() * sizeof(T), RLM_WriteOnly));

		for (int Index = 0; Index < VertexData.Num(); ++Index)
		{
			Data[Index] = static_cast<T>(VertexData[Index]);
		}

		RHIUnlockBuffer(VertexBuffer);
		ShaderResource = RHICreateShaderResourceView(VertexBuffer, sizeof(T), ReturnPixelFormat<T>());
	}
	else
	{
		VertexBuffer = nullptr;
		ShaderResource = nullptr;
	}
}

void TissueIntegerVertexBuffer::InitRHI()
{
	RHIPush(uniqueBufferName, VertexData, VertexBufferRHI, ShaderResourceViewRHI);
}

void TissueFloatVertexBuffer::InitRHI()
{
	RHIPush(uniqueBufferName, VertexData, VertexBufferRHI, ShaderResourceViewRHI);
}