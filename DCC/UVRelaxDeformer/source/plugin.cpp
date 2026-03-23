// Copyright Joseph Chittock @2022

#include "node.h"
#include <maya/MFnPlugin.h>

MTypeId UVSpringRelaxNode::id(0x00081152);

MStatus initializePlugin(MObject obj)
{
    MStatus result;

    MFnPlugin plugin(obj, "Joseph Chittock ", "1.0", "Any");
    result = plugin.registerNode("UVSpringRelaxNode",
        UVSpringRelaxNode::id,
                                &UVSpringRelaxNode::creator,
                                &UVSpringRelaxNode::initialize,
                                MPxNode::kDependNode);
    return result;
}

MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);
    return plugin.deregisterNode(UVSpringRelaxNode::id);
}