#include "include.h"
#include <maya/MStreamUtils.h>
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MStatus status;
	std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
	std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());

    MFnPlugin plugin( obj, "None", "1.1", "Any" );

    status = plugin.registerNode( "mlDeform",
        mlDeform::id,
        &mlDeform::creator,
        &mlDeform::initialize,
		MPxNode::kDeformerNode);
    return status;
}


MStatus uninitializePlugin( MObject obj )
{
    MStatus status;

    MFnPlugin plugin( obj );

    status = plugin.deregisterNode( mlDeform::id );

	return status;
}