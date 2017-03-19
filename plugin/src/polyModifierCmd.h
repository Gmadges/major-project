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

#ifndef _polyModifierCmd
#define _polyModifierCmd

// General Includes
//
#include <maya/MIntArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MDagPath.h>
#include <maya/MDGModifier.h>
#include <maya/MDagModifier.h>
#include <maya/MPlug.h>

// Proxies
//
#include <maya/MPxCommand.h>


class polyModifierCmd : public MPxCommand
{
public:

                        	polyModifierCmd();
	virtual					~polyModifierCmd();

// Restrict access to derived classes only
//
protected:

	////////////////////////////////////
	// polyModifierCmd Initialization //
	////////////////////////////////////

	// Target polyMesh to modify
	//
	void							setMeshNode( MDagPath mesh );
	MDagPath						getMeshNode() const;

	///////////////////////////////
	// polyModifierCmd Execution //
	///////////////////////////////

	MStatus							doModifyPoly(MObject& node);

	MDGModifier			fDGModifier;
	MDagModifier		fDagModifier;

private:

	//////////////////////////////////////////////
	// polyModifierCmd Internal Processing Data //
	//////////////////////////////////////////////

	// This structure is used to maintain the data vital to the modifyPoly method.
	// It is necessary to simplify parameter passing between the methods used inside
	// modifyPoly (specifically inside connectNodes()). The diagram below dictates
	// the naming style used:
	//
	// NOTE: modifierNode is intentionally left out of this structure since it
	//		 is given protected access to derived classes.
	//
	// Before:
	//
	// (upstreamNode) *src -> dest* (meshNode)
	//
	// After:
	//
	// (upstreamNode) *src -> dest* (modifierNode) *src -> dest* (meshNode)
	//
	struct modifyPolyData
	{
		MObject	meshNodeTransform;
		MObject	meshNodeShape;
		MPlug	meshNodeDestPlug;
		MObject	meshNodeDestAttr;

		MObject	upstreamNodeTransform;
		MObject	upstreamNodeShape;
		MPlug	upstreamNodeSrcPlug;
		MObject	upstreamNodeSrcAttr;

		MObject	modifierNodeSrcAttr;
		MObject	modifierNodeDestAttr;

		MObject	tweakNode;
		MObject tweakNodeSrcAttr;
		MObject tweakNodeDestAttr;
	};

	//////////////////////////////////////
	// polyModifierCmd Internal Methods //
	//////////////////////////////////////

	bool					isCommandDataValid();
	void					collectNodeState();

	// Modifier node methods
	//
	MStatus					createModifierNode( MObject& modifierNode );

	// Node processing methods (need to be executed in this order)
	//
	MStatus					processMeshNode( modifyPolyData& data );
	MStatus					processUpstreamNode( modifyPolyData& data );
	MStatus					processModifierNode( MObject modifierNode,
												 modifyPolyData& data );
	MStatus					processTweaks( modifyPolyData& data );

	// Node connection method
	//
	MStatus					connectNodes( MObject modifierNode );

	// Mesh caching methods - Only used in the directModifier case
	//
	MStatus					cacheMeshData();
	MStatus					cacheMeshTweaks();

	// Undo methods
	//
	MStatus					undoCachedMesh();
	MStatus					undoTweakProcessing();
	MStatus					undoDirectModifier();

	/////////////////////////////////////
	// polyModifierCmd Utility Methods //
	/////////////////////////////////////

	MStatus					getFloat3PlugValue( MPlug plug, MFloatVector& value );
	MStatus					getFloat3asMObject( MFloatVector value, MObject& object );

	//////////////////////////
	// polyModifierCmd Data //
	//////////////////////////

	// polyMesh
	//
	bool				fDagPathInitialized;
	MDagPath			fDagPath;
	MDagPath			fDuplicateDagPath;

	// Node State Information
	//
	bool				fHasHistory;
	bool				fHasTweaks;
	bool				fHasRecordHistory;

	// Cached Tweak Data (for undo)
	//
	MIntArray			fTweakIndexArray;
	MFloatVectorArray	fTweakVectorArray;

	// Cached Mesh Data (for undo in the 'No History'/'History turned off' case)
	//
	MObject				fMeshData;
};

//
// Inlines
//

// polyMesh
//
inline void polyModifierCmd::setMeshNode( MDagPath mesh )
{
	fDagPath = mesh;
	fDagPathInitialized = true;
}

inline MDagPath polyModifierCmd::getMeshNode() const
{
	return fDagPath;
}

#endif
