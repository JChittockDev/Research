// Copyright 2022 Joseph Chittock. All Rights Reserved.

#pragma once

#include "Math/Vector.h"
#include "Rendering/PositionVertexBuffer.h"
#include "tetgen.h"
#include <sstream>

class TetgenInterface
{
public:
	void GenerateTetMesh(double radius_edge_ratio, double volume_constraint, 
						FSkeletalMeshRenderData* RenderData, const TMap<const int, const FVector>& holes);
	
	TMap<const int, FVector>* GetInputVertexPositions() { return &_inputVertexPositions; };
	TMap<const int, FVector>* GetTetVertexPositions() { return &_tetVertexPositions; };
	TMap<const int, int>* GetTetrahedralIDs() { return &_tetrahedralIDs; };
	TMap<const int, TTuple<int, int>>* GetEdgeIDs() { return &_edgeIDs; };
	TMap<const int, TTuple<int, int, int>>* GetTriangleIDs() { return &_triangleIDs; };
	TMap<const int, int> GetClosestVertexIDs(const TMap<const int, FVector>& inputPoints, 
		const TMap<const int, int>& includeIDs, const TMap<const int, FVector>& targetPoints);
	void GetClosestBaraycenter(const TMap<const int, FVector>& inputPoints, 
		const TMap<const int, FVector>& targetPoints, TMap<const int, TTuple<int, int, int>>& triangleIDs, 
		TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates);
	
	int* GetVertexCount() { return &_vertexCount; };
	int* GetFaceCount() { return &_tetCount; };
	int* GetEdgeCount() { return &_edgeCount; };

protected:
	void ToTetgenIO(FSkeletalMeshRenderData* RenderData, const TMap<const int, const FVector>& hole);
	void OutTetMeshData(TMap<const int, FVector>& inputVertexPositions, TMap<const int, FVector>& tetVertexPositions,
		TMap<const int, int>& tetrahedralIDs, TMap<const int, TTuple<int, int>> edgeIDs, TMap<const int, TTuple<int, int, int>> triangleIDs,
		int& vertexCount, int& tetCount, int& edgeCount, int& triangleCount, TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates);
	void ToTetMesh(double radius_edge_ratio, double volume_constraint, FSkeletalMeshRenderData* RenderData,
		const TMap<const int, const FVector>& holes, TMap<const int, FVector>& inputVertexPositions, TMap<const int, FVector>& tetVertexPositions,
		TMap<const int, int>& tetrahedralIDs, TMap<const int, TTuple<int, int>>& edgeIDs, TMap<const int, TTuple<int, int, int>>& triangleIDs,
		int& vertexCount, int& tetCount, int& edgeCount, int& triangleCount, TMap<const int, TTuple<float, float, int, int>>& baraycentericCoordinates);
	void GetBarycentricCoord(FVector inputPoint, FVector a, FVector b, FVector c, float& u, float& v, float& w);
	TArray<int> GetClosestTriangleID(int targetID, FVector position, TMap<const int, TTuple<int, int, int>>& triangleIDs, const TMap<const int, FVector>& points);

private:
	int FindClosestIndex(const FVector inputPoint, const TMap<const int, FVector>& targetPoints);
	TMap<const int, int> InitializeIDMap(const int& size);

protected:
	tetgenio _in, _out;
	int _vertexCount;
	int _tetCount;
	int _edgeCount;
	int _triangleCount;
	TMap<const int, FVector> _inputVertexPositions;
	TMap<const int, FVector> _tetVertexPositions;
	TMap<const int, int> _tetrahedralIDs;
	TMap<const int, TTuple<int, int>> _edgeIDs;
	TMap<const int, TTuple<int, int, int>> _triangleIDs;
	TMap<const int, TTuple<float, float, int, int>> _baraycentericCoordinates;
};
