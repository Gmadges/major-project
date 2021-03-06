#include "tweakHandler.h"

#include "maya/MPlugArray.h"
#include "maya/MFnNumericData.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MObjectArray.h"
#include "maya/MFloatVectorArray.h"

#include "hackPrint.h"
#include <exception>

TweakHandler::TweakHandler()
{
}


TweakHandler::~TweakHandler()
{
}

MFloatVector TweakHandler::getFloat3FromPlug(MPlug& plug)
{
	MFloatVector data;
	MObject object;
	plug.getValue(object);
	MFnNumericData numDataFn(object);
	numDataFn.getData(data[0], data[1], data[2]);
	return data;
}

bool TweakHandler::hasTweaks(MDagPath& meshDAGPath)
{
	meshDAGPath.extendToShape();
	MFnDependencyNode depNodeFn(meshDAGPath.node());

	MStatus status;
	MPlug tweakPlug = depNodeFn.findPlug("pnts");
	if (!tweakPlug.isNull())
	{
		if (!tweakPlug.isArray()) return false;

		MPlug tweak;
		int i;
		int numElements = tweakPlug.numElements();

		for (i = 0; i < numElements; i++)
		{
			tweak = tweakPlug.elementByPhysicalIndex(i, &status);
			if (status == MS::kSuccess && !tweak.isNull())
			{
				// get the values from plug
				MFloatVector tweakData = getFloat3FromPlug(tweak);

				if (0 != tweakData.x ||
					0 != tweakData.y ||
					0 != tweakData.z)
				{
					return true;
				}
			}
		}
	}

	return false;
}

MStatus TweakHandler::createPolyTweakNode(MDagPath& meshDAGPath, MObject& tweakNode)
{
	MStatus status = MS::kSuccess;

	// Clear tweak undo information (to be rebuilt)
	//
	MIntArray fTweakIndexArray;
	MFloatVectorArray fTweakVectorArray;

	// Extract the tweaks and place them into a polyTweak node. This polyTweak node
	// will be placed ahead of the modifier node to maintain the order of operations.
	// Special care must be taken into recreating the tweaks:
	//
	//		1) Copy tweak info (including connections!)
	//		2) Remove tweak info from both meshNode and a duplicate meshNode (if applicable)
	//		3) Cache tweak info for undo operations
	//

	// Declare our function sets
	//
	MFnDependencyNode depNodeFn;

	// Declare our attributes and plugs
	//
	MPlug	meshTweakPlug;
	MPlug	upstreamTweakPlug;
	MObject tweakNodeTweakAttr;

	// Declare our tweak processing variables
	//
	MPlug				tweak;
	MPlug				tweakChild;
	MObject				tweakData;
	MObjectArray		tweakDataArray;

	MIntArray			tweakSrcConnectionCountArray;
	MPlugArray			tweakSrcConnectionPlugArray;
	MIntArray			tweakDstConnectionCountArray;
	MPlugArray			tweakDstConnectionPlugArray;

	MPlugArray			tempPlugArray;

	// Create the tweak node and get its attributes
	tweakNode = fDGModifier.createNode("polyTweak", &status);
	depNodeFn.setObject(tweakNode);
	tweakNodeTweakAttr = depNodeFn.attribute("tweak");

	if (status != MStatus::kSuccess) return status;

	meshDAGPath.extendToShape();
	depNodeFn.setObject(meshDAGPath.node());
	meshTweakPlug = depNodeFn.findPlug("pnts");

	if (!meshTweakPlug.isArray()) return MStatus::kFailure;

	unsigned numElements = meshTweakPlug.numElements();

	// Gather meshTweakPlug data
	for (unsigned int i = 0; i < numElements; i++)
	{
		// MPlug::numElements() only returns the number of physical elements
		// in the array plug. Thus we must use elementByPhysical index when using
		// the index i.
		tweak = meshTweakPlug.elementByPhysicalIndex(i);

		// If the method fails, the element is NULL. Only append the index
		// if it is a valid plug.
		if (!tweak.isNull())
		{
			// Cache the logical index of this element plug
			unsigned logicalIndex = tweak.logicalIndex();

			// Collect tweak data and cache the indices and float vectors
			tweak.getValue(tweakData);
			tweakDataArray.append(tweakData);
			fTweakIndexArray.append(logicalIndex);
			MFloatVector tweakVector = getFloat3FromPlug(tweak);
			fTweakVectorArray.append(tweakVector);

			// Collect tweak connection data
			//
			// Parse down to the deepest level of the plug tree and check
			// for connections - look at the child nodes of the element plugs.
			// If any connections are found, record the connection and disconnect
			// it.
			//

			if (!tweak.isCompound()) return MStatus::kFailure;

			unsigned numChildren = tweak.numChildren();
			for (unsigned int j = 0; j < numChildren; j++)
			{
				tweakChild = tweak.child(j);
				if (tweakChild.isConnected())
				{
					// Get all connections with this plug as source, if they exist
					//
					tempPlugArray.clear();
					if (tweakChild.connectedTo(tempPlugArray, false, true))
					{
						unsigned numSrcConnections = tempPlugArray.length();
						tweakSrcConnectionCountArray.append(numSrcConnections);

						for (unsigned int k = 0; k < numSrcConnections; k++)
						{
							tweakSrcConnectionPlugArray.append(tempPlugArray[k]);
							fDGModifier.disconnect(tweakChild, tempPlugArray[k]);
						}
					}
					else
					{
						tweakSrcConnectionCountArray.append(0);
					}

					// Get the connection with this plug as destination, if it exists
					//
					tempPlugArray.clear();
					if (tweakChild.connectedTo(tempPlugArray, true, false))
					{
		
						if (tempPlugArray.length() != 1) return MStatus::kFailure;

						tweakDstConnectionCountArray.append(1);
						tweakDstConnectionPlugArray.append(tempPlugArray[0]);
						fDGModifier.disconnect(tempPlugArray[0], tweakChild);
					}
					else
					{
						tweakDstConnectionCountArray.append(0);
					}
				}
				else
				{
					tweakSrcConnectionCountArray.append(0);
					tweakDstConnectionCountArray.append(0);
				}
			}
		}
	}

	// Apply meshTweakPlug data to our polyTweak node
	//
	MPlug polyTweakPlug(tweakNode, tweakNodeTweakAttr);
	unsigned numTweaks = fTweakIndexArray.length();
	int srcOffset = 0;
	int dstOffset = 0;

	for (unsigned int i = 0; i < numTweaks; i++)
	{
		// Apply tweak data
		//
		tweak = polyTweakPlug.elementByLogicalIndex(fTweakIndexArray[i]);
		tweak.setValue(tweakDataArray[i]);

		if (!tweak.isCompound()) return MStatus::kFailure;

		unsigned numChildren = tweak.numChildren();
		for (unsigned int j = 0; j < numChildren; j++)
		{
			tweakChild = tweak.child(j);

			// Apply tweak source connection data
			if (0 < tweakSrcConnectionCountArray[i*numChildren + j])
			{
				for (unsigned int k = 0;
				k < (unsigned int)tweakSrcConnectionCountArray[i*numChildren + j];
					k++)
				{
					fDGModifier.connect(tweakChild,
						tweakSrcConnectionPlugArray[srcOffset]);
					srcOffset++;
				}
			}

			// Apply tweak destination connection data
			//
			if (0 < tweakDstConnectionCountArray[i*numChildren + j])
			{
				fDGModifier.connect(tweakDstConnectionPlugArray[dstOffset],
					tweakChild);
				dstOffset++;
			}
		}
	}

	// Now, set the tweak values on the meshNode(s) to zero (History dependent)
	//
	MFnNumericData numDataFn;
	MObject nullVector;

	// CLEAR TWEAKS ON THE MESH

	// Create a NULL vector (0,0,0) using MFnNumericData to pass into the plug
	numDataFn.create(MFnNumericData::k3Float);
	numDataFn.setData(0, 0, 0);
	nullVector = numDataFn.object();

	for (unsigned int i = 0; i < numTweaks; i++)
	{
		// Access using logical indices since they are the only plugs guaranteed
		// to hold tweak data.
		//
		tweak = meshTweakPlug.elementByLogicalIndex(fTweakIndexArray[i]);
		tweak.setValue(nullVector);
	}

	// Only have to clear the tweaks off the duplicate mesh if we do not have history
	// and we want history.
	depNodeFn.setObject(meshDAGPath.node());
	upstreamTweakPlug = depNodeFn.findPlug("pnts");

	if (!upstreamTweakPlug.isNull())
	{
		for (unsigned int i = 0; i < numTweaks; i++)
		{
			tweak = meshTweakPlug.elementByLogicalIndex(fTweakIndexArray[i]);
			tweak.setValue(nullVector);
		}
	}

	// now do everything we said
	status = fDGModifier.doIt();
	return status;
}

MStatus TweakHandler::connectTweakNodes(MObject& tweakNode, MObject& meshNode)
{
	MFnDependencyNode tweakDepNode;
	tweakDepNode.setObject(tweakNode);

	MFnDependencyNode meshDepNode;
	meshDepNode.setObject(meshNode);

	MStatus status;
	MPlug inMeshPlug;
	inMeshPlug = meshDepNode.findPlug("inputPolymesh", &status);

	// if it doesnt have that plug try this one
	if (status != MStatus::kSuccess)
	{
		inMeshPlug = meshDepNode.findPlug("inMesh");
	}

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];
		MPlug tweakDestPlug(tweakNode, tweakDepNode.attribute("inputPolymesh"));
		MPlug tweakSrcPlug(tweakNode, tweakDepNode.attribute("output"));
		MPlug modifierDestPlug(meshNode, meshDepNode.attribute("inMesh", &status));
		
		// got all our plugs, lets connect things
		// dicsonnect
		MStatus status = fDGModifier.disconnect(tempPlugArray[0], modifierDestPlug);
		if (status != MStatus::kSuccess) return status;

		// connect ot parent
		status = fDGModifier.connect(tempPlugArray[0], tweakDestPlug);
		if (status != MStatus::kSuccess) return status;

		// connect to child
		status = fDGModifier.connect(tweakSrcPlug, modifierDestPlug);
		if (status != MStatus::kSuccess) return status;

		// do the connecting
		status = fDGModifier.doIt();
		return status;
	}

	return MStatus::kFailure;
}

MStatus TweakHandler::setTweakPlugFromArray(MPlug& _plug, std::vector<json>& tweakArray)
{
	MStatus status = MStatus::kFailure;

	// gonna do some hacky string manip to find out data from plug names
	// in the future something better should be done. on the sending end i should
	// also store indices in the message

	for (auto& itr : tweakArray)
	{
		for (auto& tweak : json::iterator_wrapper(itr))
		{
			unsigned int indice = 0;
			double x = 0.0;
			double y = 0.0;
			double z = 0.0;

			// find tweak indici
			try
			{
				size_t first = tweak.key().find("[") + 1;
				size_t last = tweak.key().find("]");
				std::string indString = tweak.key().substr(first, last - first);
				indice = std::stoi(indString);
			}
			catch (std::exception& e)
			{
				HackPrint::print(e.what());
				continue;
			}

			// iterate json array within tweak
			for (auto& it : tweak.value())
			{
				if (it.is_null()) continue;

				for (auto& tweakVals : json::iterator_wrapper(it))
				{
					// finx x y z
					std::string name(tweakVals.key().substr(tweakVals.key().size() - 1, 1));

					if (name.compare("x") == 0)
					{
						x = tweakVals.value();
					}

					if (name.compare("y") == 0)
					{
						y = tweakVals.value();
					}

					if (name.compare("z") == 0)
					{
						z = tweakVals.value();
					}
				}
			}

			// apply
			MPlug tweakPlug = _plug.elementByLogicalIndex(indice, &status);
			
			if (status != MStatus::kSuccess)
			{
				continue;
			}

			// mel hack
			MFnDependencyNode tweakNode = _plug.node();
			MString cmd = "setAttr \"";
			cmd += tweakNode.name();
			cmd += ".tk[";
			cmd += indice;
			cmd += "] \" - type \"double3\" ";
			cmd += x;
			cmd += " ";
			cmd += y;
			cmd += " ";
			cmd += z;
			//HackPrint::print(cmd);
			status = MGlobal::executeCommand(cmd);
		}
	}

	return status;
}

MStatus TweakHandler::setPointPlugFromArray(MPlug& _plug, std::vector<json>& pointArray)
{
	MStatus status = MStatus::kFailure;

	// gonna do some hacky string manip to find out data from plug names
	// in the future something better should be done. on the sending end i should
	// also store indices in the message

	for (auto& itr : pointArray)
	{
		for (auto& tweak : json::iterator_wrapper(itr))
		{
			unsigned int indice = 0;
			double x = 0.0;
			double y = 0.0;
			double z = 0.0;

			// find tweak indici
			try
			{
				size_t first = tweak.key().find("[") + 1;
				size_t last = tweak.key().find("]");
				std::string indString = tweak.key().substr(first, last - first);
				indice = std::stoi(indString);
			}
			catch (std::exception& e)
			{
				HackPrint::print(e.what());
				continue;
			}

			// iterate json array within tweak
			for (auto& it : tweak.value())
			{
				if (it.is_null()) continue;

				for (auto& tweakVals : json::iterator_wrapper(it))
				{
					if (tweakVals.value().is_null()) continue;
					
					// finx x y z
					std::string name(tweakVals.key().substr(tweakVals.key().size() - 1, 1));

					if (name.compare("x") == 0)
					{
						x = tweakVals.value().get<double>();
					}

					if (name.compare("y") == 0)
					{
						y = tweakVals.value().get<double>();
					}

					if (name.compare("z") == 0)
					{
						z = tweakVals.value().get<double>();
					}
				}
			}

			// apply
			MPlug tweakPlug = _plug.elementByLogicalIndex(indice, &status);

			if (status != MStatus::kSuccess)
			{
				continue;
			}

			// mel hack
			MFnDependencyNode tweakNode = _plug.node();
			MString cmd = "setAttr \"";
			cmd += tweakNode.name();
			cmd += ".pt[";
			cmd += indice;
			cmd += "] \" - type \"double3\" ";
			cmd += x;
			cmd += " ";
			cmd += y;
			cmd += " ";
			cmd += z;
			HackPrint::print(cmd);
			status = MGlobal::executeCommand(cmd);
		}
	}

	return status;
}

