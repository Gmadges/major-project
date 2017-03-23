#include "mayaUtils.h"

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