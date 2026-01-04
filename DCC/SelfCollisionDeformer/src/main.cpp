#include <math.h>
#include <maya/MAnimControl.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshVertex.h>
#include <maya/MMatrix.h>
#include <maya/MMeshIntersector.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MRampAttribute.h>
#include <maya/MThreadPool.h>
#include <maya/MTime.h>
#include <maya/MTypeId.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <omp.h>
#include <map>
#include <vector>


// Declaring SelfCollisionDeformer class & using MPxDeformerNode //
class SelfCollisionDeformer : public MPxDeformerNode {
 public:
  // Declare public functions
  static void* creator();
  static MStatus initialize();
  static const MTypeId id;
  static MObject collision_tolerance;
  static MObject collision_range;
  static MObject reverse_collision_range;
  static MObject collision_offset;
  static MObject smoothing_intensity;
  static MObject smoothing_iterations;
  static MObject weight_map;
  static MObject offset_strength_map;
  static MObject smooth_map;
  static MObject per_geo;
  virtual MStatus setDependentsDirty(const MPlug& plug, MPlugArray& plug_array);
  MStatus deform(MDataBlock& block, MItGeometry& iterator,
      const MMatrix& matrix, unsigned int multi_index) override;

 private:
  // Declare private functions
  MStatus jumpToElement(MArrayDataHandle& h_array, unsigned int index);
  float dotProduct(MVector f, MVector v);
  // Declare maps
  std::map<unsigned int, bool> _initialized;
  std::map<unsigned int, bool> _dirty_map;
  std::map<unsigned int, MFloatArray> _weight_map;
  std::map<unsigned int, MFloatArray> _offset_map;
  std::map<unsigned int, MFloatArray> _smooth_map;
  std::vector<MIntArray> _structured_vertex_array;
};

// Declare Mobjects
const MTypeId SelfCollisionDeformer::id(0x00081152);
MObject SelfCollisionDeformer::collision_tolerance;
MObject SelfCollisionDeformer::collision_range;
MObject SelfCollisionDeformer::reverse_collision_range;
MObject SelfCollisionDeformer::collision_offset;
MObject SelfCollisionDeformer::smoothing_intensity;
MObject SelfCollisionDeformer::smoothing_iterations;
MObject SelfCollisionDeformer::weight_map;
MObject SelfCollisionDeformer::offset_strength_map;
MObject SelfCollisionDeformer::smooth_map;
MObject SelfCollisionDeformer::per_geo;

// Build creator class
void* SelfCollisionDeformer::creator() { return new SelfCollisionDeformer(); }

// Initialize attributes
MStatus SelfCollisionDeformer::initialize() {
  MFnTypedAttribute tAttr;
  MFnNumericAttribute nAttr;
  MFnCompoundAttribute cAttr;

  collision_tolerance = nAttr.create("collisionTolerance", "pt", MFnNumericData::kFloat, 0.1);
  nAttr.setKeyable(true);
  nAttr.setStorable(true);
  addAttribute(collision_tolerance);
  attributeAffects(collision_tolerance, outputGeom);

  collision_range = nAttr.create("collisionRange", "cr", MFnNumericData::kFloat, 99999);
  nAttr.setKeyable(true);
  nAttr.setStorable(true);
  addAttribute(collision_range);
  attributeAffects(collision_range, outputGeom);

  reverse_collision_range = nAttr.create("reverseCollisionRange", "rcr", MFnNumericData::kFloat,
                     99999);
  nAttr.setKeyable(true);
  nAttr.setStorable(true);
  addAttribute(reverse_collision_range);
  attributeAffects(reverse_collision_range, outputGeom);

  collision_offset = nAttr.create("collisionOffset", "co", MFnNumericData::kFloat, 0.0);
  nAttr.setKeyable(true);
  nAttr.setStorable(true);
  addAttribute(collision_offset);
  attributeAffects(collision_offset, outputGeom);

  smoothing_intensity = nAttr.create("smoothingIntensity", "si", MFnNumericData::kFloat, 1.0);
  nAttr.setMax(1.0);
  nAttr.setMin(0.0);
  nAttr.setKeyable(true);
  nAttr.setStorable(true);
  addAttribute(smoothing_intensity);
  attributeAffects(smoothing_intensity, outputGeom);

  smoothing_iterations = nAttr.create("smoothingIterations", "sit", MFnNumericData::kInt, 2);
  nAttr.setMin(0);
  nAttr.setKeyable(true);
  nAttr.setStorable(true);
  addAttribute(smoothing_iterations);
  attributeAffects(smoothing_iterations, outputGeom);

  weight_map = nAttr.create("weightMap", "wm", MFnNumericData::kFloat, 1.0);
  nAttr.setStorable(true);
  nAttr.setArray(true);
  nAttr.setConnectable(true);
  nAttr.setUsesArrayDataBuilder(true);
  addAttribute(weight_map);
  attributeAffects(weight_map, outputGeom);

  offset_strength_map = nAttr.create("offsetStrengthMap", "osm", MFnNumericData::kFloat, 1.0);
  nAttr.setStorable(true);
  nAttr.setArray(true);
  nAttr.setConnectable(true);
  nAttr.setUsesArrayDataBuilder(true);
  addAttribute(offset_strength_map);
  attributeAffects(offset_strength_map, outputGeom);

  smooth_map = nAttr.create("smoothingMap", "sm", MFnNumericData::kFloat, 1.0);
  nAttr.setStorable(true);
  nAttr.setArray(true);
  nAttr.setConnectable(true);
  nAttr.setUsesArrayDataBuilder(true);
  addAttribute(smooth_map);
  attributeAffects(smooth_map, outputGeom);

  per_geo = cAttr.create("perGeometry", "pg");
  cAttr.setStorable(true);
  cAttr.setArray(true);
  cAttr.setConnectable(true);
  cAttr.setUsesArrayDataBuilder(true);
  cAttr.addChild(weight_map);
  cAttr.addChild(offset_strength_map);
  cAttr.addChild(smooth_map);
  addAttribute(per_geo);
  attributeAffects(per_geo, outputGeom);

  // Enable paintable maps 
  MGlobal::executeCommand(
      "makePaintable -attrType multiFloat -sm deformer selfCollisionDeformer "
      "weightMap");
  MGlobal::executeCommand(
      "makePaintable -attrType multiFloat -sm deformer selfCollisionDeformer "
      "offsetStrengthMap");
  MGlobal::executeCommand(
      "makePaintable -attrType multiFloat -sm deformer selfCollisionDeformer "
      "smoothingMap");

  return MStatus::kSuccess;
}

// Function to set the map status to dirty so it can be recalculated
MStatus SelfCollisionDeformer::setDependentsDirty(const MPlug& plug,
                                                  MPlugArray& plug_array) {
  if (plug == weight_map || plug == offset_strength_map || plug == smooth_map) {
    unsigned int geom_index = 0;
    if (plug.isArray()) {
      geom_index = plug.parent().logicalIndex();
    } else {
      geom_index = plug.array().parent().logicalIndex();
    }
    _dirty_map[geom_index] = true;
  }
  return MS::kSuccess;
}

// Function to jump to a designated index, but it if dosent exist create it
MStatus SelfCollisionDeformer::jumpToElement(MArrayDataHandle& h_array,
                                             unsigned int index) {
  MStatus status;
  status = h_array.jumpToElement(index);
  if (MFAIL(status)) {
    MArrayDataBuilder builder = h_array.builder(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    builder.addElement(index, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = h_array.set(builder);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = h_array.jumpToElement(index);
    CHECK_MSTATUS_AND_RETURN_IT(status);
  }
  return status;
}

// Function to calculate the dot product of two vectors
float SelfCollisionDeformer::dotProduct(MVector vector_a, MVector vector_b) {
  return vector_a[0] * vector_b[0] + vector_a[1] * vector_b[1] + vector_a[2] * vector_b[2];
}

// Main deform function to manipulate the mesh
MStatus SelfCollisionDeformer::deform(MDataBlock& data_block,
                                      MItGeometry& geom_iter,
                                      const MMatrix& matrix,
                                      unsigned int multi_index) {
  
  // Declare initial variables  
  MStatus status;
  MObject input_mesh;
  MPointArray input_points;
  geom_iter.allPositions(input_points);
  MPointArray deformed_points = input_points;

  // Retrive the Input Mesh & fnMesh
  MArrayDataHandle h_input = data_block.outputArrayValue(input, &status);
  h_input.jumpToElement(multi_index);
  MDataHandle h_input_geom = h_input.outputValue().child(inputGeom);
  input_mesh = h_input_geom.asMesh();
  MFnMesh fnMesh(input_mesh, &status);

  // Retrieve attribute values
  float envelope_value = data_block.inputValue(envelope).asFloat();
  float reverse_collision_range_value = data_block.inputValue(reverse_collision_range).asFloat();
  float collision_range_value = data_block.inputValue(collision_range).asFloat();
  float collision_tolerance_range = data_block.inputValue(collision_tolerance).asFloat();
  float collision_offset_value = data_block.inputValue(collision_offset).asFloat();
  float smoothing_intensity_value = data_block.inputValue(smoothing_intensity).asFloat();
  int smoothing_iterations_value = data_block.inputValue(smoothing_iterations).asInt();

  // Retrieve data handle
  MArrayDataHandle h_geo = data_block.inputArrayValue(per_geo);
  jumpToElement(h_geo, multi_index);
  MDataHandle h_per_geometry = h_geo.inputValue();

  // If first first calculation pass 
  if (!_initialized[multi_index]) {
    _dirty_map[multi_index] = true;
    _initialized[multi_index] = true;
  }

  // Create referenced map variables
  MFloatArray& ref_weight_map = _weight_map[multi_index];
  MFloatArray& ref_offset_map = _offset_map[multi_index];
  MFloatArray& ref_smooth_map = _smooth_map[multi_index];
  // If the maps need to be recalculated
  if (_dirty_map[multi_index]) {
    // Set their length 
    ref_weight_map.setLength(geom_iter.count());
    ref_offset_map.setLength(geom_iter.count());
    ref_smooth_map.setLength(geom_iter.count());
    // Retrieve their data handles
    MArrayDataHandle h_weight_map = h_per_geometry.child(weight_map);
    MArrayDataHandle h_offset_map = h_per_geometry.child(offset_strength_map);
    MArrayDataHandle h_smooth_map = h_per_geometry.child(smooth_map);
    int index = 0;
    // For each vertex grab there corresponding weight value for each map
    for (geom_iter.reset(); !geom_iter.isDone(); geom_iter.next(), index++) {
      status = jumpToElement(h_weight_map, geom_iter.index());
      CHECK_MSTATUS_AND_RETURN_IT(status);
      ref_weight_map[index] = h_weight_map.inputValue().asFloat();
      status = jumpToElement(h_offset_map, geom_iter.index());
      CHECK_MSTATUS_AND_RETURN_IT(status);
      ref_offset_map[index] = h_offset_map.inputValue().asFloat();
      status = jumpToElement(h_smooth_map, geom_iter.index());
      CHECK_MSTATUS_AND_RETURN_IT(status);
      ref_smooth_map[index] = h_smooth_map.inputValue().asFloat();
    }
    // Turn off dirty map so the maps dont recalcuate
    _dirty_map[multi_index] = false;
  }

  // Generate intersector parameters
  MMeshIsectAccelParams accel_params = fnMesh.autoUniformGridParams();
  for (int index = 0; index < (int)deformed_points.length(); ++index) {
    // Declare variables
    MVector normal;
    MVector hit_normal;
    MFloatPointArray hit_points_a;
    MFloatPointArray hit_points_b;
    MFloatArray ray_params_a;
    MFloatArray ray_params_b;
    MIntArray hit_faces;
    MPoint point = deformed_points[index];
    // Retrive vertex normal
    fnMesh.getVertexNormal(index, true, normal, MSpace::kObject);
    // Cast ray in the normal direction
    bool intersect_a = fnMesh.allIntersections(
        deformed_points[index], normal, NULL, NULL, true, MSpace::kObject,
        collision_range_value, false, &accel_params, true, hit_points_a, &ray_params_a,
        &hit_faces, NULL, NULL, NULL, collision_tolerance_range);
    // If there is a hit
    if (intersect_a) {
      // If there is an odd number of hits
      if ((hit_points_a.length() - 1) % 2 == 1) {
        // Get the vertex normals for the first hit face
        MFloatVectorArray face_normals;
        fnMesh.getFaceVertexNormals(hit_faces[1], face_normals, MSpace::kObject);
        // Average the face normals
        MVector average_normal = MVector(0.0, 0.0, 0.0);
        for (int x = 0; x < (int)face_normals.length(); ++x) {
          average_normal += face_normals[x];
        }
        average_normal = average_normal / face_normals.length();
        // Check to see if the hit is a backface
        float product_check = dotProduct(normal, average_normal);
        if (product_check > 0.0) {
          // Cast ray in the inverse normal direction
          bool intersect_b = fnMesh.allIntersections(
              deformed_points[index], -normal, NULL, NULL, true, MSpace::kObject,
              reverse_collision_range_value, false, &accel_params, true, hit_points_b,
              &ray_params_b, NULL, NULL, NULL, NULL, collision_tolerance_range);
          // If there is a hit
          if (intersect_b) {
            // If the hit is not a self cast
            if ((hit_points_b.length() - 1) > 0) {
              // If the number of hits is even
              if ((hit_points_b.length() - 1) % 2 == 0) {
                // Calculate the relative deformation
                MPoint relative_deformation =
                    (MPoint(hit_points_b[1][0], hit_points_b[1][1],
                            hit_points_b[1][2]) - deformed_points[index]) *0.5;
                // Set the new position
                deformed_points[index] =
                    (deformed_points[index] + relative_deformation) +
                    (relative_deformation * collision_offset_value * ref_offset_map[index]);
              }
            }
          }
        }
      }
    }
  }

  // Declare iterator
  MItMeshVertex vert_iter(input_mesh);
#pragma omp parallel for private(index, structural_vertices)
  for (; !vert_iter.isDone(); vert_iter.next()) {
    // grab each vertices connected vertes and store them in array
    int index = vert_iter.index();
    MIntArray structural_vertices;
    vert_iter.getConnectedVertices(structural_vertices);
    _structured_vertex_array.push_back(structural_vertices);
  }

  // For each smoothing iteration
#pragma omp parallel for private(average_point, vertex_smoothing_intensity, length) collapse()
  for (int y = 0; y < smoothing_iterations_value; ++y) {
    // For each vertex
    for (int i = 0; i < (int)input_points.length(); ++i) {
      // Average the surrounding vertices with the main vertex
      MPoint average_point = deformed_points[i] - input_points[i];
      for (int x = 0; x < (int)_structured_vertex_array[i].length(); ++x) {
        average_point += deformed_points[_structured_vertex_array[i][x]] -
                        input_points[_structured_vertex_array[i][x]];
      }
      // Factor in the smoothing map
      float vertex_smoothing_intensity = smoothing_intensity_value * ref_smooth_map[i];
      // Set the new position
      int length = _structured_vertex_array[i].length();
      deformed_points[i] =
          (deformed_points[i] * (1.0 - vertex_smoothing_intensity)) +
          (((average_point / (length + 1)) +
            input_points[i]) *vertex_smoothing_intensity);
    }
  }

  // Calculate the envelope attribute
#pragma omp parallel for private(envelope_vertex)
  for (int i = 0; i < (int)input_points.length(); ++i) {
    // Factor in the weight map
    float envelope_vertex = envelope_value * ref_weight_map[i];
    // Set the new Position
    deformed_points[i] = (deformed_points[i] * envelope_vertex) +
                         (input_points[i] * (1.0 - envelope_vertex));
  }

  // Set the final position
  geom_iter.setAllPositions(deformed_points);
  return status;
}

// Initialize the plugin
MStatus initializePlugin(MObject obj) {
  MStatus result;
  MFnPlugin plugin(obj, "Joseph Chittock ", "1.0", "Any");
  result = plugin.registerNode(
      "selfCollisionDeformer", SelfCollisionDeformer::id,
      &SelfCollisionDeformer::creator, &SelfCollisionDeformer::initialize,
      MPxNode::kDeformerNode);

  return result;
}

// Unitialize the plugin
MStatus uninitializePlugin(MObject obj) {
  MStatus result;
  MFnPlugin plugin(obj);
  result = plugin.deregisterNode(SelfCollisionDeformer::id);

  return result;
}
