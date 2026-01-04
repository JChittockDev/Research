// Copyright Joseph Chittock @2022

#include "node.h"
#include <maya/MFnPlugin.h>
#include <maya/MStreamUtils.h>

MStatus initializePlugin(MObject obj)
{
    MStatus result;

    MFnPlugin plugin(obj, "Joseph Chittock ", "1.0", "Any");
    result = plugin.registerNode("collisionDeformer",
                                collisionDeformer::id,
                                &collisionDeformer::creator,
                                &collisionDeformer::initialize,
                                MPxNode::kDeformerNode);
    return result;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus result;
    MFnPlugin plugin(obj);

    MStreamUtils::stdErrorStream();
    MStreamUtils::stdOutStream();

    result = plugin.deregisterNode(collisionDeformer::id);

    return result;
}