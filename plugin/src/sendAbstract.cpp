#include "sendAbstract.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MArgList.h>
#include <maya/MPoint.h>
#include <maya/MIOStream.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnAttribute.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MUuid.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MDistance.h>
#include <maya/MAngle.h>
#include <maya/MDataHandle.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>

#include "messaging.h"
#include "tweakHandler.h"
#include "hackPrint.h"
#include "testTypes.h"

#include "callbackHandler.h"
#include "mayaUtils.h"

SendAbstract::SendAbstract()
	:
	pMessaging(new Messaging("localhost", 8080)),
	pTweaksHandler(new TweakHandler())
{
}


SendAbstract::~SendAbstract()
{
}

void SendAbstract::traverseAllValidNodesForMesh(MDagPath& dagPath, std::function<void(MFnDependencyNode&)>& func)
{
	// first do the transforms node
	MFnDependencyNode transformNode(dagPath.transform());
	func(transformNode);

	// now the rest of the meshes history
	dagPath.extendToShape();
	MFnDependencyNode meshShapeNode(dagPath.node());

	traverseAllValidNodes(meshShapeNode, func);
}

void SendAbstract::traverseAllValidNodes(MFnDependencyNode & node, std::function<void(MFnDependencyNode&)>& func)
{
	if (MayaUtils::isValidNodeType(node.typeName()))
	{
		func(node);
	}

	// now lets see if it has a parent

	// If the inpuPolymesh is connected, we have history
	MStatus status;
	MPlug inMeshPlug;
	inMeshPlug = MayaUtils::getInPlug(node, status);

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);
		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];
		MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

		traverseAllValidNodes(upstreamNode, func);
	}
}

MStatus SendAbstract::getGenericNode(MFnDependencyNode & _inNode, json& _outNode)
{
	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	unsigned int numAttrib = _inNode.attributeCount();
	MStatus status;

	json nodeAttribs;

	for (unsigned int i = 0; i < numAttrib; i++)
	{
		MFnAttribute attrib(_inNode.attribute(i));

		MPlug plug = _inNode.findPlug(attrib.shortName().asChar(), &status);

		if (status != MStatus::kSuccess) continue;


		getAttribFromPlug(plug, nodeAttribs);
	}

	if (nodeAttribs.empty()) return MStatus::kFailure;

	// uuid
	// get the uuid of the node for sending broseph
	MUuid idObj = _inNode.uuid();

	_outNode["id"] = std::string(idObj.asString().asChar());
	_outNode["name"] = std::string(_inNode.name().asChar());
	_outNode["type"] = std::string(_inNode.typeName().asChar());
	_outNode["time"] = std::time(nullptr);
	_outNode["attribs"] = nodeAttribs;

	// use plugs to get node ordering
	_outNode["out"] = nullptr;
	_outNode["in"] = nullptr;

	MString inID;
	if (getIncomingID(_inNode, inID) == MStatus::kSuccess)
	{
		_outNode["in"] = std::string(inID.asChar());
	}

	MString outID;
	if (getOutgoingID(_inNode, outID) == MStatus::kSuccess)
	{
		_outNode["out"] = std::string(outID.asChar());
	}

	return MStatus::kSuccess;
}

MStatus SendAbstract::getIncomingID(MFnDependencyNode & _inNode, MString& _id)
{
	MFnDependencyNode upstreamNode;
	if (MayaUtils::getIncomingNodeObject(_inNode, upstreamNode) == MStatus::kSuccess)
	{
		_id = upstreamNode.uuid().asString();
		return MStatus::kSuccess;
	}

	return MStatus::kFailure;
}

MStatus SendAbstract::getOutgoingID(MFnDependencyNode & _inNode, MString& _id)
{
	MFnDependencyNode downStreamNode;
	if (MayaUtils::getOutgoingNodeObject(_inNode, downStreamNode) == MStatus::kSuccess)
	{
		_id = downStreamNode.uuid().asString();
		return MStatus::kSuccess;
	}

	return MStatus::kFailure;
}

MStatus SendAbstract::getAttribFromPlug(MPlug& _plug, json& _attribs)
{
	std::string attribName = _plug.partialName().asChar();

	if (_plug.isNull())
	{
		_attribs[attribName] = nullptr;
		return MStatus::kSuccess;
	}

	//hack stops crashing on plugs which have an index of -1
	if (attribName.find("-1") != std::string::npos)
	{
		return MStatus::kFailure;
	}

	if (_plug.isArray())
	{
		std::vector<json> plugArray;
		for (unsigned int i = 0; i < _plug.numElements(); i++)
		{
			// get the MPlug for the i'th array element
			MPlug elemPlug = _plug.elementByPhysicalIndex(i);

			json vals;
			if (getAttribFromPlug(elemPlug, vals) == MStatus::kSuccess)
			{
				plugArray.push_back(vals);
			}
		}
		_attribs[attribName] = plugArray;
		return MStatus::kSuccess;
	}

	if (_plug.isCompound())
	{
		// if the plug is a compound then it has a number of children plugs we need to grab
		unsigned int numChild = _plug.numChildren();
		std::vector<json> childrenPlugs(numChild);
		for (unsigned int i = 0; i < numChild; ++i)
		{
			MPlug childPlug = _plug.child(i);

			// get values
			// and store too
			json vals;
			if (getAttribFromPlug(childPlug, vals) == MStatus::kSuccess)
			{
				childrenPlugs.push_back(vals);
			}
		}
		_attribs[attribName] = childrenPlugs;
		return MStatus::kSuccess;
	}

	if (getNumericDataFromAttrib(_plug, _attribs) == MStatus::kSuccess) return MStatus::kSuccess;

	if (getTypeDataFromAttrib(_plug, _attribs) == MStatus::kSuccess) return MStatus::kSuccess;

	if (getUnitDataFromAttrib(_plug, _attribs) == MStatus::kSuccess) return MStatus::kSuccess;

	// this one is just stuff i know we can get
	if (getOtherDataFromAttrib(_plug, _attribs) == MStatus::kSuccess) return MStatus::kSuccess;

	MString error;
	error += "Couldn't find a match for plug: ";
	error += _plug.name();
	error += " type: ";
	error += _plug.attribute().apiTypeStr();
	HackPrint::print(error);

	return MStatus::kFailure;
}

MStatus SendAbstract::getNumericDataFromAttrib(MPlug& _plug, json& _attribs)
{
	std::string attribName = _plug.partialName().asChar();
	MObject attribute = _plug.attribute();

	if (attribute.hasFn(MFn::kNumericAttribute))
	{
		MFnNumericAttribute fnAttrib(attribute);

		switch (fnAttrib.unitType())
		{
			case MFnNumericData::kInvalid:
			{
				_attribs[attribName] = "invalid";
				return MStatus::kSuccess;
			}
			case MFnNumericData::kBoolean:
			{
				bool value;
				_plug.getValue(value);
				_attribs[attribName] = value;
				return MStatus::kSuccess;
			}
			case MFnNumericData::kShort:
			{
				short value;
				_plug.getValue(value);
				_attribs[attribName] = value;
				return MStatus::kSuccess;
			}
			case MFnNumericData::kInt:
			{
				int value;
				_plug.getValue(value);
				_attribs[attribName] = value;
				return MStatus::kSuccess;
			}
			case MFnNumericData::kFloat:
			{
				float value;
				_plug.getValue(value);
				_attribs[attribName] = value;
				return MStatus::kSuccess;
			}
			case MFnNumericData::kDouble:
			{
				double value;
				_plug.getValue(value);
				_attribs[attribName] = value;
				return MStatus::kSuccess;
			}
			case MFnNumericData::k3Float:
			{
				MVector float3;
				_plug.child(0).getValue(float3.x);
				_plug.child(1).getValue(float3.y);
				_plug.child(2).getValue(float3.z);

				_attribs[attribName] = "3float";
				return MStatus::kSuccess;
			}
			case MFnNumericData::kByte:
			case MFnNumericData::kChar:
			{
				// bit hacky
				char value;
				_plug.getValue(value);
				_attribs[attribName] = value;
				return MStatus::kSuccess;
			}
			case MFnNumericData::k2Short:
			{
				_attribs[attribName] = "2short";
				return MStatus::kSuccess;
			}
			case MFnNumericData::k3Short:
			{
				_attribs[attribName] = "3short";
				return MStatus::kSuccess;
			}
			case MFnNumericData::k2Int:
			{
				_attribs[attribName] = "2int";
				return MStatus::kSuccess;
			}
			case MFnNumericData::k3Int:
			{
				_attribs[attribName] = "3int";
				return MStatus::kSuccess;
			}
			case MFnNumericData::k2Float:
			{
				_attribs[attribName] = "2float";
				return MStatus::kSuccess;
			}
			case MFnNumericData::k2Double:
			{
				_attribs[attribName] = "2double";
				return MStatus::kSuccess;
			}
			case MFnNumericData::k3Double:
			{
				_attribs[attribName] = "3double";
				return MStatus::kSuccess;
			}
			case MFnNumericData::k4Double:
			{
				_attribs[attribName] = "4double";
				return MStatus::kSuccess;
			}
			case MFnNumericData::kAddr:
			{
				_attribs[attribName] = "address";
				return MStatus::kSuccess;
			}
		}
	}

	return MStatus::kFailure;
}

MStatus SendAbstract::getTypeDataFromAttrib(MPlug& _plug, json& _attribs)
{
	std::string attribName = _plug.partialName().asChar();
	MObject attribute = _plug.attribute();

	// do the same for typed attribs
	if (attribute.hasFn(MFn::kTypedAttribute))
	{
		MFnTypedAttribute fnAttrib(attribute);

		switch (fnAttrib.attrType())
		{
			case MFnData::kInvalid:
			{
				_attribs[attribName] = "invalid";
				return MStatus::kSuccess;
			}
			case MFnData::kString:
			{
				MString value;
				_plug.getValue(value);
				_attribs[attribName] = value.asChar();
				return MStatus::kSuccess;
			}
			case MFnData::kMatrix:
			{
				MDataHandle data = _plug.asMDataHandle();
				MMatrix matrix = data.asMatrix();
				std::vector<double> matData;
				for (int i = 0; i < 4; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						matData.push_back(matrix[i][j]);
					}
				}
				_attribs[attribName] = matData;
				_plug.destructHandle(data);
				return MStatus::kSuccess;
			}
			case MFnData::kNumeric:
			{
				_attribs[attribName] = "number";
				return MStatus::kSuccess;
			}
			case MFnData::kPlugin:
			{
				_attribs[attribName] = "plugin";
				return MStatus::kSuccess;
			}
			case MFnData::kPluginGeometry:
			{
				_attribs[attribName] = "pluginGeo";
				return MStatus::kSuccess;
			}
			case MFnData::kStringArray:
			{
				_attribs[attribName] = "stringArray";
				return MStatus::kSuccess;
			}
			case MFnData::kDoubleArray:
			{
				_attribs[attribName] = "doubleArray";
				return MStatus::kSuccess;
			}
			case MFnData::kIntArray:
			{
				_attribs[attribName] = "intArray";
				return MStatus::kSuccess;
			}
			case MFnData::kPointArray:
			{
				_attribs[attribName] = "pointArray";
				return MStatus::kSuccess;
			}
			case MFnData::kVectorArray:
			{
				_attribs[attribName] = "vecArray";
				return MStatus::kSuccess;
			}
			case MFnData::kComponentList:
			{
				_attribs[attribName] = "complist";
				return MStatus::kSuccess;
			}
			case MFnData::kMesh:
			{
				_attribs[attribName] = "mesh";
				return MStatus::kSuccess;
			}
			case MFnData::kLattice:
			{
				_attribs[attribName] = "lattice";
				return MStatus::kSuccess;
			}
			case MFnData::kNurbsCurve:
			{
				_attribs[attribName] = "nurbscurve";
				return MStatus::kSuccess;
			}
			case MFnData::kNurbsSurface:
			{
				_attribs[attribName] = "nurbSurface";
				return MStatus::kSuccess;
			}
			case MFnData::kSphere:
			{
				_attribs[attribName] = "sphere";
				return MStatus::kSuccess;
			}
			case MFnData::kDynArrayAttrs:
			{
				_attribs[attribName] = "dynAttri";
				return MStatus::kSuccess;
			}
			case MFnData::kDynSweptGeometry:
			{
				_attribs[attribName] = "sweptGeo";
				return MStatus::kSuccess;
			}
			case MFnData::kSubdSurface:
			{
				_attribs[attribName] = "subdSurface";
				return MStatus::kSuccess;
			}
			case MFnData::kNObject:
			{
				_attribs[attribName] = "Object";
				return MStatus::kSuccess;
			}
			case MFnData::kNId:
			{
				_attribs[attribName] = "ID";
				return MStatus::kSuccess;
			}
		}
	}

	return MStatus::kFailure;
}

MStatus SendAbstract::getUnitDataFromAttrib(MPlug& _plug, json& _attribs)
{
	std::string attribName = _plug.partialName().asChar();
	MObject attribute = _plug.attribute();

	// do the same for typed attribs
	if (attribute.hasFn(MFn::kUnitAttribute))
	{
		MFnUnitAttribute fnAttrib(attribute);

		switch (fnAttrib.unitType())
		{
			case MFnUnitAttribute::kInvalid:
			{
				_attribs[attribName] = "invalid";
				return MStatus::kSuccess;
			}
			case MFnUnitAttribute::kAngle:
			{
				MAngle value;
				_plug.getValue(value);
				_attribs[attribName] = value.value();
				return MStatus::kSuccess;
			}
			case MFnUnitAttribute::kDistance:
			{
				MDistance value;
				_plug.getValue(value);
				_attribs[attribName] = value.value();
				return MStatus::kSuccess;
			}
			case MFnUnitAttribute::kTime:
			{
				_attribs[attribName] = "time";
				return MStatus::kSuccess;
			}
		}
	}

	return MStatus::kFailure;
}

MStatus SendAbstract::getOtherDataFromAttrib(MPlug& _plug, json& _attribs)
{
	std::string attribName = _plug.partialName().asChar();
	MObject attribute = _plug.attribute();

	if (attribute.hasFn(MFn::kDoubleLinearAttribute))
	{
		double value;
		_plug.getValue(value);
		_attribs[attribName] = value;
		return MStatus::kSuccess;
	}

	if (attribute.hasFn(MFn::kFloatLinearAttribute))
	{
		float value;
		_plug.getValue(value);
		_attribs[attribName] = value;
		return MStatus::kSuccess;
	}

	if (attribute.hasFn(MFn::kEnumAttribute))
	{
		int value;
		_plug.getValue(value);
		_attribs[attribName] = value;
		return MStatus::kSuccess;
	}

	if (attribute.hasFn(MFn::kMessageAttribute))
	{
		MString value;
		_plug.getValue(value);
		_attribs[attribName] = value.asChar();
		return MStatus::kSuccess;
	}

	return MStatus::kFailure;
}