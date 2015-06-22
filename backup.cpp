//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
//  File: finalproject.cc
//
//  Description:
// 		Example implementation of a threaded deformer. This node
//		deforms one mesh using another.
//

#include <maya/MIOStream.h>

#include <maya/MPxDeformerNode.h> 
#include <maya/MItGeometry.h>
#include <maya/MFnPlugin.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MPoint.h>
#include <maya/MTimer.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MMeshIntersector.h>
#include <maya/MFnTransform.h>
#include <maya/MVector.h>

#include <maya/MThreadUtils.h>

#include "tbb/parallel_for.h"
#include <tbb/blocked_range.h>

#include "finalproject.h"
#include "float.h"
#include "math.h"

// Macros
//
#define MCheckStatus(status,message)	\
	if( MStatus::kSuccess != status ) {	\
		cerr << message << "\n";		\
		return status;					\
	}

class finalproject : public MPxDeformerNode
{
public:
						finalproject();
	virtual				~finalproject();

	static  void*		creator();
	static  MStatus		initialize();

	// deformation function
	//
	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock);

public:
	// local node attributes

	static  MTypeId		id;

	static MObject deformingMesh;  
	static MObject transX;
	static MObject transY;
	static MObject transZ;
	static MObject offload;
	static MObject tesla;
	static MObject positivelycharged;  

private:
};

MTypeId     finalproject::id( 0x8104D );
MObject		finalproject::deformingMesh;
MObject		finalproject::transX;
MObject		finalproject::transY;
MObject		finalproject::transZ;
MObject		finalproject::offload;
MObject     finalproject::tesla;
MObject     finalproject::positivelycharged;

finalproject::finalproject() {}
finalproject::~finalproject() {}

void* finalproject::creator()
{
	return new finalproject();
}

MStatus finalproject::initialize()
{
	// local attribute initialization
	MStatus status;
	MFnTypedAttribute mAttr;
 	deformingMesh=mAttr.create( "deformingMesh", "dm", MFnMeshData::kMesh);
 	
 	MFnNumericAttribute nAttrt;
 	transX = nAttrt.create( "transX", "tx", MFnNumericData::kDouble);
 	nAttrt.setStorable(true);
 	
 	status = addAttribute( transX );
	MCheckStatus(status, "ERROR in addAttribute\n");

 	status = attributeAffects( transX, outputGeom );
	MCheckStatus(status, "ERROR in attributeAffects\n");
	
	transY = nAttrt.create( "transY", "ty", MFnNumericData::kDouble);
 	nAttrt.setStorable(true);
 	
 	status = addAttribute( transY );
	MCheckStatus(status, "ERROR in addAttribute\n");

 	status = attributeAffects( transY, outputGeom );
	MCheckStatus(status, "ERROR in attributeAffects\n");
	
	transZ = nAttrt.create( "transZ", "tz", MFnNumericData::kDouble);
 	nAttrt.setStorable(true);
 	
 	status = addAttribute( transZ );
	MCheckStatus(status, "ERROR in addAttribute\n");

 	status = attributeAffects( transZ, outputGeom );
	MCheckStatus(status, "ERROR in attributeAffects\n");
	
	tesla = nAttrt.create( "tesla", "tes", MFnNumericData::kDouble);
	nAttrt.setStorable(true);
	nAttrt.setKeyable(true);
	nAttrt.setDefault(0.0);
	nAttrt.setMin(0.0);
	nAttrt.setMax(300.0);
	
 	status = addAttribute( tesla );
	MCheckStatus(status, "ERROR in addAttribute\n");

 	status = attributeAffects( tesla, outputGeom );
	MCheckStatus(status, "ERROR in attributeAffects\n");
	
   positivelycharged = nAttrt.create( "positivelycharged", "pc", MFnNumericData::kBoolean);
	nAttrt.setStorable(true);
	nAttrt.setKeyable(true);
	nAttrt.setDefault(true);
	
 	status = addAttribute( positivelycharged );
	MCheckStatus(status, "ERROR in addAttribute\n");

	MCheckStatus(status, "ERROR in attributeAffects\n");
 	
 	MFnNumericAttribute nAttrO;
 	offload=nAttrO.create( "offload", "ol", MFnNumericData::kBoolean);
 	nAttrO.setStorable(true);
 	nAttrO.setDefault(false);
 	nAttrO.setKeyable(true);
	
 	//  deformation attributes
 	status = addAttribute( deformingMesh );
	MCheckStatus(status, "ERROR in addAttribute\n");

 	status = attributeAffects( deformingMesh, outputGeom );
	MCheckStatus(status, "ERROR in attributeAffects\n");
	
   status = addAttribute( offload );
	MCheckStatus(status, "ERROR in addAttribute\n");

 	status = attributeAffects( offload, outputGeom );
	MCheckStatus(status, "ERROR in attributeAffects\n");

	return MStatus::kSuccess;
}

MStatus finalproject::compute(const MPlug& plug, MDataBlock& data)
{
	// do this if we are using an OpenMP implementation that is not the same as Maya's.
	// Even if it is the same, it does no harm to make this call.
	MThreadUtils::syncNumOpenMPThreads();

	MStatus status = MStatus::kUnknownParameter;
 	if (plug.attribute() != outputGeom) {
		return status;
	}

	unsigned int index = plug.logicalIndex();
	MObject thisNode = this->thisMObject();

	// get input value
	MPlug inPlug(thisNode,input);
	inPlug.selectAncestorLogicalIndex(index,input);
	MDataHandle hInput = data.inputValue(inPlug, &status);
	MCheckStatus(status, "ERROR getting input mesh\n");
	
	// get the input geometry
	MDataHandle inputData = hInput.child(inputGeom);
	if (inputData.type() != MFnData::kMesh) {
 		printf("Incorrect input geometry type\n");
		return MStatus::kFailure;
 	}

	// get the input groupId - ignored for now...
	MDataHandle hGroup = inputData.child(groupId);
	unsigned int groupId = hGroup.asLong();

	// get deforming mesh
	MDataHandle deformData = data.inputValue(deformingMesh, &status);
	MCheckStatus(status, "ERROR getting deforming mesh\n");
    if (deformData.type() != MFnData::kMesh) {
		printf("Incorrect deformer geometry type %d\n", deformData.type());
		return MStatus::kFailure;
	}
	
   MDataHandle offloadData = data.inputValue(offload, &status);
   //MDataHandle translate_Z = data.inputValue(translateZ, &status);

  	MObject dSurf = deformData.asMeshTransformed();
  	MObject iSurf = inputData.asMeshTransformed();
 	MFnMesh fnDeformingMesh, fnInputMesh;
 	fnDeformingMesh.setObject( dSurf ) ;
 	fnInputMesh.setObject( iSurf ) ;
 	
 	MFnTransform inTrans(iSurf);

	MDataHandle outputData = data.outputValue(plug);
	outputData.copy(inputData);
 	if (outputData.type() != MFnData::kMesh) {
		printf("Incorrect output mesh type\n");
		return MStatus::kFailure;
	}
	
	MItGeometry iter(outputData, groupId, false);

	// create fast intersector structure
	MMeshIntersector intersector;
	intersector.create(dSurf);

	// get all points at once. Faster to query, and also better for
	// threading than using iterator
	MPointArray objVerts;
	iter.allPositions(objVerts);
	int objNumPoints = objVerts.length();

	// use bool variable as lightweight object for failure check in loop below
	bool failed = false;

 	MTimer timer; timer.beginTimer();
 	
 	MPointArray magVerts, tempverts;
 	fnDeformingMesh.getPoints(magVerts);
 	fnInputMesh.getPoints(tempverts);
 	int magNumPoints = magVerts.length();
 	
 	double min = DBL_MAX, max = -DBL_MAX;
 	for (int i = 0; i < magNumPoints; i++) {
      min = magVerts[i].z < min ? magVerts[i].z : min;
      max = magVerts[i].z > max ? magVerts[i].z : max;
   }

   double middle = (min + max) / 2;
   double polarity[magNumPoints];
   fprintf(stderr, "\n\n\nmin: %lf, max: %lf\n", min, max);
   
   //assigns polarity based on middle point of mesh
   for (int i = 0; i < magNumPoints; i++) {
      polarity[i] = magVerts[i].z > middle ? max / magVerts[i].z : -min / magVerts[i].z;
      fprintf(stderr, "z = %lf, polarity[%d]: %lf\n", magVerts[i].z, i, polarity[i]);
   }
 	
 	double* objdVerts = (double *)malloc(sizeof(double) * objNumPoints * 3);
 	double* magdVerts = (double *)malloc(sizeof(double) * magNumPoints * 3);
 	int i;
 	
 	MDataHandle vecX = data.inputValue(transX, &status);
   MDataHandle vecY = data.inputValue(transY, &status);
   MDataHandle vecZ = data.inputValue(transZ, &status);
   double moveX = vecX.asFloat();
   double moveY = vecY.asFloat();
   double moveZ = vecZ.asFloat();
 	
 	fprintf(stderr, "1st finalVector: (%lf, %lf, %lf)\n", moveX, moveY, moveZ);
 	
 	//Mpoints array to double with x,y,z
 	for (i=0; i<objNumPoints; i++) {
 	   objdVerts[i * 3] = tempverts[i].x + moveX;
 	   objdVerts[i * 3 + 1] = tempverts[i].y + moveY;
 	   objdVerts[i * 3 + 2] = tempverts[i].z + moveZ;
 	}
 	
 	for (i=0; i<magNumPoints; i++) {
 	   magdVerts[i * 3] = magVerts[i].x;
 	   magdVerts[i * 3 + 1] = magVerts[i].y;
 	   magdVerts[i * 3 + 2] = magVerts[i].z;
 	}
 	
 	double teslaData = data.inputValue(tesla, &status).asDouble();
   MDataHandle posiData = data.inputValue(positivelycharged, &status);
   
   double pivot[6] = {DBL_MAX, -DBL_MAX,DBL_MAX, -DBL_MAX,DBL_MAX, -DBL_MAX};
 	for (int i = 0; i < tempverts.length(); i++) {
      pivot[0] = tempverts[i].x < pivot[0] ? tempverts[i].x : pivot[0];
      pivot[1] = tempverts[i].x > pivot[1] ? tempverts[i].x : pivot[1];
      pivot[2] = tempverts[i].y < pivot[2] ? tempverts[i].y : pivot[2];
      pivot[3] = tempverts[i].y > pivot[3] ? tempverts[i].y : pivot[3];
      pivot[4] = tempverts[i].z < pivot[4] ? tempverts[i].z : pivot[4];
      pivot[5] = tempverts[i].z > pivot[5] ? tempverts[i].z : pivot[5];
   }
 	fprintf(stderr, "pivot: %lf, %lf, %lf\n", (pivot[0] + pivot[1]) / 2, 
 	   (pivot[2] + pivot[3]) / 2, (pivot[4] + pivot[5]) / 2);
 	
 	//MAGNET FORCE CALL
   magnetForce(magNumPoints, objNumPoints, teslaData, magdVerts, 
      objdVerts, polarity, posiData.asBool(), offloadData.asBool());
 	
 	//double array back to Mpoints
 	for (i=0; i<objNumPoints; i++) {
 	   objVerts[i].x = objdVerts[i * 3 + 0];
 	   objVerts[i].y = objdVerts[i * 3 + 1];
 	   objVerts[i].z = objdVerts[i * 3 + 2];      
 	}

 	timer.endTimer(); printf("Runtime for threaded loop %f\n", timer.elapsedTime());
 	
   double objCenter[6] = {DBL_MAX, -DBL_MAX,DBL_MAX, -DBL_MAX,DBL_MAX, -DBL_MAX};
 	for (int i = 0; i < tempverts.length(); i++) {
      objCenter[0] = objVerts[i].x < objCenter[0] ? objVerts[i].x : objCenter[0];
      objCenter[1] = objVerts[i].x > objCenter[1] ? objVerts[i].x : objCenter[1];
      objCenter[2] = objVerts[i].y < objCenter[2] ? objVerts[i].y : objCenter[2];
      objCenter[3] = objVerts[i].y > objCenter[3] ? objVerts[i].y : objCenter[3];
      objCenter[4] = objVerts[i].z < objCenter[4] ? objVerts[i].z : objCenter[4];
      objCenter[5] = objVerts[i].z > objCenter[5] ? objVerts[i].z : objCenter[5];
   }
 	fprintf(stderr, "objcenter: %lf, %lf, %lf\n", (objCenter[0] + objCenter[1]) / 2, 
 	   (objCenter[2] + objCenter[3]) / 2, (objCenter[4] + objCenter[5]) / 2);
 	   
 	moveX = (objCenter[0] + objCenter[1]) / 2 - (pivot[0] + pivot[1]) / 2;
 	moveY = (objCenter[2] + objCenter[3]) / 2 - (pivot[2] + pivot[3]) / 2;
 	moveZ = (objCenter[4] + objCenter[5]) / 2 - (pivot[4] + pivot[5]) / 2;
 	
 	fprintf(stderr, "finalVector: (%lf, %lf, %lf)\n", moveX, moveY, moveZ);
 	
 	if (teslaData) {
 	   vecX.setFloat(moveX);
 	   vecY.setFloat(moveY);
 	   vecZ.setFloat(moveZ);
 	}
 	
	// write values back onto output using fast set method on iterator
	iter.setAllPositions(objVerts, MSpace::kWorld);

	if(failed) {
		printf("Closest point failed\n");
		return MStatus::kFailure;
	}

	return status;
}

// standard initialization procedures
//
MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");
	result = plugin.registerNode( "finalproject", finalproject::id, finalproject::creator, 
								  finalproject::initialize, MPxNode::kDeformerNode );

	return result;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus result;
	MFnPlugin plugin( obj );
	result = plugin.deregisterNode( finalproject::id );
	return result;
}
