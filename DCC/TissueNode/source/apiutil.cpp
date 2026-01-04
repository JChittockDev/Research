// Copyright Joseph Chittock @2022

#include "node.h"

// Check if an interger exists within a given vector
int tissueNode::contains(std::vector<int>& vec, const int elem)
{
    bool result = 0;
    if (find(vec.begin(), vec.end(), elem) != vec.end())
    {
        result = 1;
    }
    return result;
}

// Initialize MFloatArray with a given length and value
void tissueNode::initArray(MFloatArray& input, int length, float value)
{
    input.clear();
    for (int i = 0; i < length; i++)
    {
        input.append(value);
    }
}

// Flatten a MPointArray to an MFloatArray
void tissueNode::flattenPointArray(const MPointArray& input, MFloatArray& output)
{
    MFloatArray out;
    for (int i = 0; i < (int)input.length(); ++i)
    {
        out.append(input[i][0]);
        out.append(input[i][1]);
        out.append(input[i][2]);
    }
    output = out;
}

// Concatanate a MFloatArray to a MPointArray
void tissueNode::concatToPointArray(const MFloatArray& input, MPointArray& output)
{
    MPointArray out;
    int length = input.length() / 3;
    for (int i = 0; i < (int)length; ++i)
    {
        MPoint point = { input[i * 3], input[i * 3 + 1], input[i * 3 + 2] };
        out.append(point);
    }
    output = out;
}