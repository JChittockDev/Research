#include "CNTKLibrary.h"
#include <string.h>
#include <fstream>
#include <valarray>
#include <string>
#include <codecvt>
#include <locale>
#include <cctype>
#include <clocale>
#include <iostream>
#include <maya/MItMeshVertex.h>
#include <maya/MGlobal.h> 
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MAngle.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MPoint.h> 
#include <maya/MPointArray.h> 
#include <maya/MEulerRotation.h>
#include <maya/MItGeometry.h>
#include <omp.h>


class mlDeform : public MPxDeformerNode
{
public:
	static  void*   creator();
	static  MStatus initialize();
	MStatus deform(MDataBlock& block,
					MItGeometry& iterator,
					const MMatrix& matrix,
					unsigned int multiIndex) override;

	CNTK::FunctionPtr getModel(const std::wstring newFilePath, CNTK::DeviceDescriptor device);
	void returnCSV(std::wstring newDataPathA, std::wstring newDataPathB, std::wstring newDataPathC, std::wstring newDataPathD, std::wstring newDataPathE, std::wstring newDataPathF);
	static	MTypeId	id;
	static	MObject	deviceType;
	static	MObject	modelFilePath;
	static	MObject	inputMeanPath;
	static	MObject	inputStdPath;
	static	MObject	outputMeanPath;
	static	MObject	outputStdPath;
	static	MObject	indexPath;
	static	MObject	mappingPath;
	static	MObject	inputs;

private:
	std::wstring modelPath;
	std::wstring iMeanPath;
	std::wstring iStdPath;
	std::wstring oMeanPath;
	std::wstring oStdPath;
	std::wstring idxPath;
	std::wstring mapPath;
	CNTK::FunctionPtr modelPtr;
	std::vector<CNTK::FunctionPtr> clonePtrs;
	std::vector<std::valarray<float>> input_mean;
	std::vector<std::valarray<float>> input_std;
	std::vector<std::valarray<float>> output_mean;
	std::vector<std::valarray<float>> output_std;
	std::vector<std::vector<int>> index_list;
	std::vector<std::vector<std::string>> input_vars;
};
