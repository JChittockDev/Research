#include "include.h"

MTypeId mlDeform::id(0x00081152);
MObject	mlDeform::deviceType;
MObject	mlDeform::modelFilePath;
MObject	mlDeform::inputMeanPath;
MObject	mlDeform::inputStdPath;
MObject	mlDeform::outputMeanPath;
MObject	mlDeform::outputStdPath;
MObject	mlDeform::indexPath;
MObject	mlDeform::mappingPath;
MObject	mlDeform::inputs;

std::vector<std::valarray<float>> featNorm(std::vector<std::valarray<float>>  features, std::vector<std::valarray<float>> mean, std::vector<std::valarray<float>> std)
{
	std::vector<std::valarray<float>> out;
	for (unsigned int i = 0; i < features.size(); ++i) {
		float epsilon = 1.19209e-07;
		std::valarray<float> feats_norm = (features[i] - mean[i]) / (std[i] + epsilon);
		out.push_back(feats_norm);
	}
	return out;
}

std::valarray<float> featDenorm(std::valarray<float>  features_norm, std::valarray<float> mean, std::valarray<float> std)
{
	std::valarray<float> features = (features_norm * std) + mean;
	return features;
}

std::vector<std::vector<float>> castVector(std::vector<std::valarray<float>> in)
{
	std::vector<std::vector<float>> out;
	for (unsigned int i = 0; i < in.size(); ++i) {
		std::vector<float> convert_input;
		convert_input.assign(std::begin(in[i]), std::end(in[i]));
		out.push_back(convert_input);
	}
	return out;
}

std::vector<std::valarray<float>> castValarray(std::vector<std::vector<float>> in)
{
	std::vector<std::valarray<float>> out;
	for (unsigned int i = 0; i < in.size(); ++i) {
		std::valarray<float> convert_input(in[i].data(), in[i].size());
		out.push_back(convert_input);
	}
	return out;
}

std::vector<std::valarray<float>> getCSV(std::wstring inFile) {
	std::fstream myFile(inFile);
	std::vector<std::valarray<float>> array;
	if (myFile.is_open()) {
		std::string lineA;
		while (getline(myFile, lineA)) {
			std::string lineB;
			std::vector<float> line_array;
			std::istringstream ss(lineA);
			while (getline(ss, lineB, ' ')) {
				line_array.push_back(std::stof(lineB));
			}
			std::valarray<float> dataMap(line_array.data(), line_array.size());
			array.push_back(dataMap);
		}
	}
	return array;
}

std::vector<std::vector<int>> getCSVInt(std::wstring inFile) {
	std::fstream myFile(inFile);
	std::vector<std::vector<int>> array;
	if (myFile.is_open()) {
		std::string lineA;
		while (getline(myFile, lineA)) {
			std::string lineB;
			std::vector<int> line_array;
			std::istringstream ss(lineA);
			while (getline(ss, lineB, ' ')) {
				line_array.push_back(std::stoi(lineB));
			}
			array.push_back(line_array);
		}
	}
	return array;
}

std::vector<std::vector<std::string>> getCSVString(std::wstring inFile) {
	std::fstream myFile(inFile);
	std::vector<std::vector<std::string>> array;
	if (myFile.is_open()) {
		std::string lineA;
		while (getline(myFile, lineA)) {
			std::string lineB;
			std::vector<std::string> line_array;
			std::istringstream ss(lineA);
			while (getline(ss, lineB, ' ')) {
				line_array.push_back(lineB);
			}
			array.push_back(line_array);
		}
	}
	return array;
}

void mlDeform::returnCSV(std::wstring newDataPathA, std::wstring newDataPathB, std::wstring newDataPathC, std::wstring newDataPathD, std::wstring newDataPathE, std::wstring newDataPathF)
{
	if (newDataPathA != iMeanPath || newDataPathB != iStdPath || newDataPathC != oMeanPath || newDataPathD != oStdPath || newDataPathE != idxPath || newDataPathF != mapPath)
	{
		iMeanPath = newDataPathA;
		iStdPath = newDataPathB;
		oMeanPath = newDataPathC;
		oStdPath = newDataPathD;
		idxPath = newDataPathE;
		mapPath = newDataPathF;
		input_mean = getCSV(iMeanPath);
		input_std = getCSV(iStdPath);
		output_mean = getCSV(oMeanPath);
		output_std = getCSV(oStdPath);
		index_list = getCSVInt(idxPath);
		input_vars = getCSVString(mapPath);
	}
}

CNTK::FunctionPtr mlDeform::getModel(const std::wstring newFilePath, CNTK::DeviceDescriptor device)
{
	if (newFilePath == modelPath)
	{
		return modelPtr;
	}
	else
	{
		modelPath = newFilePath;
		modelPtr = CNTK::Function::Load(modelPath, device, CNTK::ModelFormat::ONNX);
		int threadCount = omp_get_max_threads();
		std::vector<CNTK::FunctionPtr> clone_vector;
		for (unsigned int x = 0; x < threadCount; ++x) {
			clone_vector.push_back(modelPtr->Clone(CNTK::ParameterCloningMethod::Share));
		} 
		clonePtrs = clone_vector;
		return modelPtr;
	}
}

std::string arrayReturnLongName(std::vector<std::string> array) {
	std::string name = "";
	if (array.size() > 0) {
		for (unsigned int x = 0; x < array.size(); ++x)
		{
			name += array[x];
			if (x + 1 < array.size()) {
				name += "_";
			}
			return name;
		}
	}
	else {
		return array[0];
	}
}

void remove_control_characters(std::string& s) {
	s.erase(std::remove_if(s.begin(), s.end(), [](char c) { return std::iscntrl(c); }), s.end());
}

void returnVariableNames(std::vector<std::vector<CNTK::Variable>> variableLists) {
	for (unsigned int i = 0; i < variableLists.size(); ++i) {
		for (std::vector<CNTK::Variable>::iterator it = variableLists[i].begin(); it != variableLists[i].end(); ++it) {
			std::string feature = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(it->Name());
			cout << feature << "\n";
		}
	}
}

bool GetVariableByName(std::vector<CNTK::Variable> variableLists, std::string varName, CNTK::Variable& var)
{
	for (std::vector<CNTK::Variable>::iterator it = variableLists.begin(); it != variableLists.end(); ++it)
	{
		std::string feature = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(it->Name());
		if (feature == varName) {
			var = *it;
			return true;
		}
	}
	return false;
}

bool GetInputVariableByName(CNTK::FunctionPtr evalFunc, std::string varName, CNTK::Variable& var)
{
	return GetVariableByName(evalFunc->Arguments(), varName, var);
}

bool GetOutputVaraiableByName(CNTK::FunctionPtr evalFunc, std::string varName, CNTK::Variable& var)
{
	return GetVariableByName(evalFunc->Outputs(), varName, var);
}

std::valarray<float> evaluateSample(CNTK::FunctionPtr model, CNTK::DeviceDescriptor device, std::valarray<float> input, std::valarray<float> mean, std::valarray<float> std, std::string input_name, std::string output_name)
{

	CNTK::Variable inputVar;
	CNTK::Variable outputVar;
	std::valarray<float> prediction;
	if (GetInputVariableByName(model, input_name, inputVar) && GetOutputVaraiableByName(model, output_name, outputVar))
	{
		std::vector<float> normalInputX;
		normalInputX.assign(std::begin(input), std::end(input));
		CNTK::ValuePtr inputVal = CNTK::Value::CreateBatch(inputVar.Shape(), normalInputX, device);
		std::unordered_map<CNTK::Variable, CNTK::ValuePtr> inputDataMap = { { inputVar, inputVal } };
		std::unordered_map<CNTK::Variable, CNTK::ValuePtr> outputDataMap = { { outputVar, nullptr } };
		model->Evaluate(inputDataMap, outputDataMap, device);
		CNTK::ValuePtr outputVal = outputDataMap[outputVar];
		std::vector<std::vector<float>> outputData;
		outputVal->CopyVariableValueTo(outputVar, outputData);
		std::vector<std::valarray<float>> outputs = castValarray(outputData);
		prediction = featDenorm(outputs[0], mean, std);
	}
	else {
		cout << "The features " << input_name << " and " << output_name << " are not avalible." << "\n";
	}
	return prediction;
}

std::valarray<float> evaluateParallel(std::vector<CNTK::FunctionPtr> ptrs, CNTK::DeviceDescriptor device, std::vector<std::valarray<float>> input,
	std::vector<std::valarray<float>> output_mean , std::vector<std::valarray<float>> output_std, std::vector<std::vector<int>> index_list,
	MPointArray& output, std::vector<std::vector<std::string>> variables, int index) {
	
	std::string input_name = "input_" + std::to_string(index+1);
	std::string output_name = "Add";
	if (index != 0) {
		output_name += std::to_string(index);
	}
	remove_control_characters(input_name);
	remove_control_characters(output_name);
	std::valarray<float> prediction = evaluateSample(ptrs[index], device, input[index], output_mean[index], output_std[index], input_name, output_name);
	return prediction;
}

std::vector<std::vector<float>> numberMatchingIndicies(std::vector<std::vector<int>> index_list) {
	std::vector<std::vector<float>> average_multiplier;
	// for every number in ever index list //
	for (unsigned int a = 0; a < index_list.size(); ++a) {
		std::vector<float> averages;
		for (unsigned int b = 0; b < index_list[a].size(); ++b) {
			// check if this number occurs in the list at all //
			int occurance_count = 0;
			for (unsigned int c = 0; c < index_list.size(); ++c) {
				for (unsigned int d = 0; d < index_list[c].size(); ++d) {
					// If it does add an occurance number //
					if (index_list[a][b] == index_list[c][d]) {
						occurance_count += 1;
					}
				}
			}
			// calculate the multiplication number for average //
			float multiplication_factor = occurance_count / 1.0;
			averages.push_back(multiplication_factor);
		}
		average_multiplier.push_back(averages);
	}
	return average_multiplier;
}

MStatus mlDeform::deform(MDataBlock& data, MItGeometry& geomIter, const MMatrix& matrix, unsigned int multiIndex)
{
	MStatus stat;
	MObject input_mesh;
	MPointArray input_points;
	geomIter.allPositions(input_points);
	MPointArray deformed_points = input_points;

	MArrayDataHandle hInput = data.outputArrayValue(input, &stat);
	hInput.jumpToElement(multiIndex);
	MDataHandle hInputGeom = hInput.outputValue().child(inputGeom);
	input_mesh = hInputGeom.asMesh();

	float envelope_value = data.inputValue(envelope).asFloat();
	MDataHandle deviceType_h = data.inputValue(deviceType, &stat);
	MDataHandle modelFilePath_h = data.inputValue(modelFilePath, &stat);
	MDataHandle inputMeanPath_h = data.inputValue(inputMeanPath, &stat);
	MDataHandle inputStdPath_h = data.inputValue(inputStdPath, &stat);
	MDataHandle outputMeanPath_h = data.inputValue(outputMeanPath, &stat);
	MDataHandle outputStdPath_h = data.inputValue(outputStdPath, &stat);
	MDataHandle iPath_h = data.inputValue(indexPath, &stat);
	MDataHandle mappingPath_h = data.inputValue(mappingPath, &stat);
	MArrayDataHandle inputs_h = data.inputArrayValue(inputs, &stat);

	unsigned int numElements = inputs_h.elementCount();
	if (numElements <= 0)
		return MS::kSuccess;
	int deviceTypeInt = deviceType_h.asInt();
	std::wstring modelPath = modelFilePath_h.asString().asWChar();
	std::wstring inMeanPath = inputMeanPath_h.asString().asWChar();
	std::wstring inStdPath = inputStdPath_h.asString().asWChar();
	std::wstring outMeanPath = outputMeanPath_h.asString().asWChar();
	std::wstring outStdPath = outputStdPath_h.asString().asWChar();
	std::wstring iPath = iPath_h.asString().asWChar();
	std::wstring mappedPath = mappingPath_h.asString().asWChar();
	returnCSV(inMeanPath, inStdPath, outMeanPath, outStdPath, iPath, mappedPath);

	int c = 0;
	std::vector<std::valarray<float>> rotation_data_array;
	for (unsigned int i = 0; i < input_vars.size(); ++i)
	{
		std::vector<float> rotation_data;
		for (unsigned int a = 0; a < input_vars[i].size(); ++a) {
			inputs_h.jumpToArrayElement(c);
			MVector rotation_data_cell = inputs_h.inputValue().asVector();
			rotation_data.push_back(float(rotation_data_cell[0]));
			rotation_data.push_back(float(rotation_data_cell[1]));
			rotation_data.push_back(float(rotation_data_cell[2]));
			c += 1;
		}
		std::valarray<float> rawInput(rotation_data.data(), rotation_data.size());
		rotation_data_array.push_back(rawInput);
	}

	if (rotation_data_array.size() == 0 || input_mean.size() == 0 || input_std.size() == 0 || output_mean.size() == 0 || output_std.size() == 0 || index_list.size() == 0) {
		cout << "Error: CSV data not correctly entered \n";
		return MS::kFailure;
	}

	if (rotation_data_array.size() != input_mean.size()) {
		cout << "Error: input size is " << rotation_data_array.size() << " and mean size is " << input_mean.size() << "\n";
		return MS::kFailure;
	}

	std::vector<std::valarray<float>> normalInput = featNorm(rotation_data_array, input_mean, input_std);
	std::vector<std::vector<float>> normalInputCast = castVector(normalInput);
	const CNTK::DeviceDescriptor device = CNTK::DeviceDescriptor::CPUDevice();
	if (deviceTypeInt == 1) {
		const CNTK::DeviceDescriptor device = CNTK::DeviceDescriptor::GPUDevice(0);
	}

	CNTK::FunctionPtr modelFunc = getModel(modelPath, device);
	if (modelFunc == NULL) {
		cout << "Unable to read Model" << "\n";
		return MS::kFailure;
	}
	
	#pragma omp parallel for private(prediction) collapse()
	for (int th = 0; th < input_vars.size(); ++th)
	{
		std::valarray<float> prediction = evaluateParallel(clonePtrs, device, normalInput, output_mean, output_std, index_list, deformed_points, input_vars, th);
		cout << "prediction sucsess \n";
		for (int x = 0; x < (int)index_list[th].size(); ++x) {
			deformed_points[index_list[th][x]] += MPoint(prediction[x * 3], prediction[x * 3 + 1], prediction[x * 3 + 2]) * envelope_value;
		}
	}
	geomIter.setAllPositions(deformed_points);
	return MS::kSuccess;
}

void* mlDeform::creator()
{
	return new mlDeform();
}

MStatus mlDeform::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnMatrixAttribute mAttr;
	MFnEnumAttribute enumAttr;
	
	deviceType = enumAttr.create("deviceType", "dt", 0);
	enumAttr.addField("CPU", 0);
	enumAttr.addField("GPU", 1);
	enumAttr.setHidden(false);
	enumAttr.setKeyable(true);
	addAttribute(deviceType);
	attributeAffects(deviceType, outputGeom);

	MFnStringData fileFnStringData;
	MObject fileNameDefaultObject = fileFnStringData.create("");
	modelFilePath = tAttr.create("modelFile", "mf", MFnData::kString, fileNameDefaultObject);
	tAttr.setStorable(true);
	tAttr.setUsedAsFilename(true);
	addAttribute(modelFilePath);
	attributeAffects(modelFilePath, outputGeom);

	inputMeanPath = tAttr.create("inputMeansFile", "imf", MFnData::kString, fileNameDefaultObject);
	tAttr.setStorable(true);
	tAttr.setUsedAsFilename(true);
	addAttribute(inputMeanPath);
	attributeAffects(inputMeanPath, outputGeom);

	inputStdPath = tAttr.create("inputStdFile", "istdf", MFnData::kString, fileNameDefaultObject);
	tAttr.setStorable(true);
	tAttr.setUsedAsFilename(true);
	addAttribute(inputStdPath);
	attributeAffects(inputStdPath, outputGeom);

	outputMeanPath = tAttr.create("outputMeansFile", "omf", MFnData::kString, fileNameDefaultObject);
	tAttr.setStorable(true);
	tAttr.setUsedAsFilename(true);
	addAttribute(outputMeanPath);
	attributeAffects(outputMeanPath, outputGeom);

	outputStdPath = tAttr.create("outputStdFile", "ostdf", MFnData::kString, fileNameDefaultObject);
	tAttr.setStorable(true);
	tAttr.setUsedAsFilename(true);
	addAttribute(outputStdPath);
	attributeAffects(outputStdPath, outputGeom);

	indexPath = tAttr.create("indexFile", "if", MFnData::kString, fileNameDefaultObject);
	tAttr.setStorable(true);
	tAttr.setUsedAsFilename(true);
	addAttribute(indexPath);
	attributeAffects(indexPath, outputGeom);

	mappingPath = tAttr.create("mappingFile", "mpf", MFnData::kString, fileNameDefaultObject);
	tAttr.setStorable(true);
	tAttr.setUsedAsFilename(true);
	addAttribute(mappingPath);
	attributeAffects(mappingPath, outputGeom);

	inputs = nAttr.create("inputs", "ins", MFnNumericData::k3Double);
	nAttr.setArray(true);
	nAttr.setConnectable(true);
	addAttribute(inputs);
	attributeAffects(inputs, outputGeom);

	return MS::kSuccess;
}