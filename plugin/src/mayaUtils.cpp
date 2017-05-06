#include "mayaUtils.h"

#include <maya/MSelectionList.h>
#include <maya/MUuid.h>

MayaUtils::MayaUtils()
{
}


MayaUtils::~MayaUtils()
{
}


bool MayaUtils::isValidNodeType(MString& _type)
{
	return (_type == MString("transform") ||
			_type == MString("mesh") ||
			_type == MString("polyTweak") ||
			//_type == MString("polySplitRing") ||
			_type == MString("polyCube"));
}

MStatus MayaUtils::getNodeObjectFromUUID(MString& uuid, MObject& _node)
{
	// get node
	MSelectionList sList;
	MUuid ID(uuid);
	sList.add(ID);
	return sList.getDependNode(0, _node);
}

bool MayaUtils::doesItExist(std::string& _id)
{
	MStatus status;
	MSelectionList selList;
	MUuid id(_id.c_str());
	status = selList.add(id);

	return (status == MStatus::kSuccess);
}



