// Copyright Joseph Chittock @2022

#include "node.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj)
{
    MStatus result;
    std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
    std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());
    MFnPlugin plugin(obj, "Joseph Chittock ", "1.0", "Any");
    result = plugin.registerNode(
        "tissueNode",
        tissueNode::id,
        &tissueNode::creator,
        &tissueNode::initialize);

    return result;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus result;

    MFnPlugin plugin(obj);
    result = plugin.deregisterNode(tissueNode::id);

    return result;
}