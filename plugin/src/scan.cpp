#include "scan.h"

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

#include "messaging.h"
#include "tweakHandler.h"
#include "hackPrint.h"

Scan::Scan()
	:
	pMessaging(new Messaging("localhost", 8080)),
	pTweaksHandler(new TweakHandler())
{
}

Scan::~Scan()
{
}

void* Scan::creator()
{
	return new Scan;
}

MSyntax Scan::newSyntax() 
{

	MSyntax syn;

	syn.addFlag("-a", "-address", MSyntax::kString);
	syn.addFlag("-p", "-port", MSyntax::kUnsigned);

	return syn;
}

MStatus	Scan::doIt(const MArgList& args)
{
	MStatus status;

	// reset socket
	MString addr;
	int port;
	status = getArgs(args, addr, port);
	if (status != MStatus::kSuccess)
	{
		HackPrint::print("no input values specified");
		return status;
	}

	pMessaging->resetSocket(std::string(addr.asChar()), port);

	// filter to only meshes
	MItDag dagIterator(MItDag::kDepthFirst, MFn::kMesh, &status);

	if (!status) {
		status.perror("MItDag constructor");
		return status;
	}

	for (; !dagIterator.isDone(); dagIterator.next()) {

		MDagPath dagPath;

		status = dagIterator.getPath(dagPath);
		if (!status) {
			status.perror("MItDag::getPath");
			continue;
		}
		MFnDagNode dagNode(dagPath, &status);
		if (!status) {
			status.perror("MFnDagNode constructor");
			continue;
		}

		// check for tweaks
		if (pTweaksHandler->hasTweaks(dagPath))
		{
			MObject tweakNode;
			HackPrint::print("we got tweaks");
			if (pTweaksHandler->createPolyTweakNode(dagPath, tweakNode) == MStatus::kSuccess)
			{
				HackPrint::print("created a node ting");
				dagPath.extendToShape();
				if (pTweaksHandler->connectTweakNodes(tweakNode, dagPath.node()) == MStatus::kSuccess)
				{
					HackPrint::print("connected");
				}
			}
		}

		// turn tweaks into a node before sending

		if (sendMesh(dagPath) != MStatus::kSuccess) return MStatus::kFailure;
	}
	return MS::kSuccess;
}

MStatus Scan::sendMesh(MDagPath & meshDAGPath)
{
	MStatus status;
	GenericMesh meshMessage;
	std::vector<GenericNode> nodeList;

	meshDAGPath.extendToShape();
	MObject meshNodeShape = meshDAGPath.node();
	MFnDependencyNode depNodeFn(meshNodeShape);

	MFnMesh mesh(meshNodeShape, &status);

	if (!status) 
	{
		status.perror("MFnMesh:constructor");
		return status;
	}

	MFnDependencyNode transformNode(meshDAGPath.transform());
	GenericNode transNode;
	status = getGenericNode(transformNode, transNode);
	if (status != MStatus::kSuccess) return status;
	nodeList.push_back(transNode);

	traverseHistory(depNodeFn, nodeList);

	// should have atleast 3 nodes for a mesh
	// transform, shape and mesh
	if (nodeList.size() < 3) return MStatus::kFailure;

	meshMessage.setMeshName(std::string(transformNode.name().asChar()));
	meshMessage.setRequestType(SCENE_UPDATE);

	// hardcode for now
	meshMessage.setMeshType(CUBE);

	meshMessage.setNodes(nodeList);

	HackPrint::print("sending " + meshMessage.getMeshName());
	if (pMessaging->sendUpdate(meshMessage))
	{
		HackPrint::print("mesh sent succesfully");
		return MStatus::kSuccess;
	}

	HackPrint::print("unable to send");
	return MStatus::kFailure;
}

MStatus Scan::getArgs(const MArgList& args, MString& address, int& port)
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	// get the command line arguments that were specified
	if (parser.isFlagSet("-p"))
	{
		parser.getFlagArgument("-p", 0, port);
	}
	else
	{
		status = MStatus::kFailure;
	}

	if (parser.isFlagSet("-a"))
	{
		parser.getFlagArgument("-a", 0, address);
	}
	else
	{
		status = MStatus::kFailure;
	}

	return status;
}

void Scan::traverseHistory(MFnDependencyNode & node, std::vector<GenericNode>& nodeList)
{
	HackPrint::print(node.name());
	HackPrint::print(node.typeName());
	
	if (node.typeName() == MString("transform") ||
		node.typeName() == MString("mesh") ||
		node.typeName() == MString("polyTweak") ||
		//node.typeName() == MString("polySplitRing") ||
		node.typeName() == MString("polyCube"))
	{
		GenericNode genNode;
		if (getGenericNode(node, genNode) == MStatus::kSuccess)
		{
			nodeList.push_back(genNode);
		}
	}

	// now lets see if it has a parent

	// If the inpuPolymesh is connected, we have history
	MStatus status;
	MPlug inMeshPlug;
	inMeshPlug = node.findPlug("inputPolymesh", &status);
	
	// if it doesnt have that plug try this one
	if (status != MStatus::kSuccess)
	{
		inMeshPlug = node.findPlug("inMesh");
	}

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);
		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];
		MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

		traverseHistory(upstreamNode, nodeList);
	}
}

MStatus Scan::getGenericNode(MFnDependencyNode & _inNode, GenericNode& _outNode)
{
	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	unsigned int numAttrib = _inNode.attributeCount();
	MStatus status;

	attribMap nodeAttribs;

	for (unsigned int i = 0; i < numAttrib; i++)
	{
		MFnAttribute attrib(_inNode.attribute(i));

		MPlug plug = _inNode.findPlug(attrib.shortName().asChar(), &status);
		
		if (status != MStatus::kSuccess) continue;

		attribMap values;
		if (getAttribFromPlug(plug, values) == MStatus::kSuccess)
		{
			nodeAttribs.insert(values.begin(), values.end());
		}
	}

	if (nodeAttribs.empty()) return MStatus::kFailure;

	_outNode.setNodeName(std::string(_inNode.name().asChar()));
	_outNode.setNodeType(_inNode.typeName().asChar());
	_outNode.setAttribs(nodeAttribs);

	return MStatus::kSuccess;
}

MStatus Scan::getAttribFromPlug(MPlug& _plug, attribMap& _attribs)
{
	std::string attribName = _plug.partialName().asChar();

	if (_plug.isNull())
	{
		_attribs.insert(attribType(attribName, msgpack::object()));
		return MStatus::kSuccess;
	}

	//hack stops crashing on plugs which have an index of -1
	if (attribName.find("-1") != std::string::npos)
	{
		return MStatus::kFailure;
	}

	// check for tweak plug, thats special
	if (attribName.compare("tk") == 0)
	{
		std::vector<double> tweaks;
		pTweaksHandler->getTweaksArrayfromPlug(_plug, tweaks);
		_attribs.insert(attribType(attribName, msgpack::object(tweaks)));
		return MStatus::kSuccess;
	}

	if (_plug.isArray())
	{
		for (unsigned int i = 0; i < _plug.numElements(); i++)
		{
			// get the MPlug for the i'th array element
			MPlug elemPlug = _plug.elementByPhysicalIndex(i);

			attribMap values;
			if (getAttribFromPlug(elemPlug, values) == MStatus::kSuccess)
			{
				_attribs.insert(values.begin(), values.end());
			}
		}

		// need to ad somethinf with a bit more info
		_attribs.insert(attribType(attribName, msgpack::object()));
		return MStatus::kSuccess;
	}

	if (_plug.isCompound())
	{
		// if the plug is a compound then it has a number of children plugs we need to grab
		unsigned int numChild = _plug.numChildren();

		for(unsigned int i = 0; i < numChild; ++i)
		{
			MPlug childPlug = _plug.child(i);

			// get values
			// and store too
			attribMap values;
			if (getAttribFromPlug(childPlug, values) == MStatus::kSuccess)
			{
				_attribs.insert(values.begin(), values.end());
			}
		}

		// should put something about this
		_attribs.insert(attribType(attribName, msgpack::object()));
		return MStatus::kSuccess;
	}

    //MString value;
    float fValue;
    if (_plug.getValue(fValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(fValue)));
		return MStatus::kSuccess;
    }

    double dValue;
    if (_plug.getValue(dValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(dValue)));
		return MStatus::kSuccess;
    }

    MString sValue;
    if (_plug.getValue(sValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(sValue.asChar())));
		return MStatus::kSuccess;
    }

    int iValue;
    if (_plug.getValue(iValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(iValue)));
		return MStatus::kSuccess;
    }

    bool bValue;
    if (_plug.getValue(bValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(bValue)));
		return MStatus::kSuccess;
    }

	//// TODO more maya data types
	//MAngle MAngleValue();
	//if (_plug.getValue(bValue) == MStatus::kSuccess)
	//{
	//	_attribs.insert(attribType(attribName, msgpack::object(bValue)));
	//	return MStatus::kSuccess;
	//}

	//MDistance DistenceValue();
	//if (_plug.getValue(bValue) == MStatus::kSuccess)
	//{
	//	_attribs.insert(attribType(attribName, msgpack::object(bValue)));
	//	return MStatus::kSuccess;
	//}


	//MTime TimeValue();
	//if (_plug.getValue(bValue) == MStatus::kSuccess)
	//{
	//	_attribs.insert(attribType(attribName, msgpack::object(bValue)));
	//	return MStatus::kSuccess;
	//}

	//// TODO look at handling data and objects possibly.

	return MStatus::kFailure;
}