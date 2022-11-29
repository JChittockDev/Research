// Copyright 2022 Joseph Chittock. All Rights Reserved.

#include "Math/Vector.h"
#include "Rendering/PositionVertexBuffer.h"
#include "tetgen.h"
#include <sstream>

struct FVectorTangent
{
	FPackedRGB10A2N TangentX;
	FPackedRGB10A2N TangentZ;
};

class TetgenInterface
{
public:
	void GenerateTetMesh(double radiusEdgeRatio, double volumeConstraint, 
						FSkeletalMeshRenderData* RenderData, const TMap<const int, const FVector>& holes);
	
	std::shared_ptr<TMap<const int, FVector>> GetInputVertexPositions() { return std::make_shared<TMap<const int, FVector>> (_inputVertexPositions); };
	std::shared_ptr<TMap<const int, TTuple<int, int, int>>> GetInputTriangleIDs() { return std::make_shared<TMap<const int, TTuple<int, int, int>>>(_inputTriangleIDs); };

	std::shared_ptr<TMap<const int, FVector>> GetTetVertexPositions() { return std::make_shared<TMap<const int, FVector>> (_tetVertexPositions); };
	std::shared_ptr<TMap<const int, TTuple<int, int, int>>> GetTetTriangleIDs() { return std::make_shared<TMap<const int, TTuple<int, int, int>>>(_tetTriangleIDs); };
	std::shared_ptr<TMap<const int, int>> GetTetrahedralIDs() { return std::make_shared<TMap<const int, int>> (_tetIDs); };
	std::shared_ptr<TMap<const int, TTuple<int, int>>> GetTetEdgeIDs() { return std::make_shared<TMap<const int, TTuple<int, int>>> (_tetEdgeIDs); };

	std::shared_ptr<int> GetTetVertexCount() { return std::make_shared<int>(_tetVertexCount); };
	std::shared_ptr<int> GetTetFaceCount() { return std::make_shared<int>(_tetCount); };
	std::shared_ptr<int> GetTetEdgeCount() { return std::make_shared<int>(_tetEdgeCount); };
	std::shared_ptr<int> GetTetTriangleCount() { return std::make_shared<int>(_tetTriangleCount); };

protected:

	void ParseMeshData(FSkeletalMeshRenderData* RenderData, TMap<const int, FVector>& InputVertexPositions,
		TMap<const int, TTuple<int, int, int>>& InputTriangleIDs, TMap<const int, FVectorTangent>& inputTangents,
		TArray<TMap<const int, FVector2D>>& inputTextureCoordinates);

	void ToTetMesh(double radius_edge_ratio, double volume_constraint, FSkeletalMeshRenderData* RenderData,
		const TMap<const int, const FVector>& holes, TMap<const int, FVector>& inputVertexPositions,
		TMap<const int, TTuple<int, int, int>>& inputTriangleIDs,
		TMap<const int, FVectorTangent>& _inputTangents,
		TArray<TMap<const int, FVector2D>>& _inputTextureCoordinates,
		TMap<const int, FVector>& tetVertexPositions,
		TMap<const int, int>& tetIDs, TMap<const int, TTuple<int, int>>& tetEdgeIDs,
		TMap<const int, TTuple<int, int, int>>& tetTriangleIDs, int& tetVertexCount,
		int& tetCount, int& tetEdgeCount, int& tetTriangleCount,
		TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates,
		tetgenio& in, tetgenio& out);

	void ToTetgenIO(const TMap<const int, const FVector>& hole, tetgenio& in,
		const TMap<const int, FVector>& inputVertexPositions, const TMap<const int, TTuple<int, int, int>>& inputTriangleIDs,
		const TMap<const int, FVectorTangent>& inputTangents, const TArray<TMap<const int, FVector2D>>& inputTextureCoordinates);
	
	void OutTetMeshData(TMap<const int, FVector>& inputVertexPositions,
		TMap<const int, FVector>& tetVertexPositions, TMap<const int, int>& tetIDs,
		TMap<const int, TTuple<int, int>> tetEdgeIDs, TMap<const int, TTuple<int, int, int>> tetTriangleIDs,
		int& tetVertexCount, int& tetCount, int& tetEdgeCount, int& tetTriangleCount,
		TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates,
		const tetgenio& in, tetgenio& out);
	
	void GetBarycentricCoord(FVector inputPoint, FVector a, FVector b, FVector c, float& u, float& v, float& w);
	
	TArray<int> GetClosestTriangleID(int targetID, FVector position, TMap<const int, TTuple<int, int, int>>& triangleIDs, const TMap<const int, FVector>& points);
	
	TMap<const int, int> GetClosestVertexIDs(const TMap<const int, FVector>& inputPoints,
		const TMap<const int, int>& includeIDs, const TMap<const int, FVector>& targetPoints);
	
	void GetClosestBaraycenter(const TMap<const int, FVector>& inputPoints,
		const TMap<const int, FVector>& targetPoints, TMap<const int, TTuple<int, int, int>>& triangleIDs,
		TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates);

private:
	int FindClosestIndex(const FVector inputPoint, const TMap<const int, FVector>& targetPoints);
	TMap<const int, int> InitializeIDMap(const int& size);

protected:
	tetgenio _in, _out;
	
	int _tetVertexCount;
	int _tetCount;
	int _tetEdgeCount;
	int _tetTriangleCount;
	
	TMap<const int, FVector> _inputVertexPositions;
	TMap<const int, TTuple<int, int, int>> _inputTriangleIDs;
	TArray<TMap<const int, FVector2D>> _inputTextureCoordinates;
	TMap<const int, FVectorTangent> _inputTangents;
	
	TMap<const int, FVector> _tetVertexPositions;
	TMap<const int, TTuple<int, int, int>> _tetTriangleIDs;
	TMap<const int, int> _tetIDs;
	TMap<const int, TTuple<int, int>> _tetEdgeIDs;
	
	TMap<const int, TTuple<float, float, int, int>> _baraycentericCoordinates;
};
