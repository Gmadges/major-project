#include "settingsCmd.h"

#include <maya/MStringArray.h>

#include "dataStore.h"

SettingsCmd::SettingsCmd()
{
}

SettingsCmd::~SettingsCmd()
{
}

void* SettingsCmd::creator()
{
	return new SettingsCmd;
}

MSyntax SettingsCmd::newSyntax()
{
	MSyntax syn;

	// query returns all our current settings in a string array
	syn.addFlag("-q", "-query");

	// settings we can change here
	syn.addFlag("-fm", "-fullMesh", MSyntax::kBoolean);

	return syn;
}

MStatus SettingsCmd::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	if (parser.isFlagSet("-q"))
	{
		setResult(getSettings());
	}
	else
	{
		setSettings(parser);
	}

	return status;
}

MStringArray SettingsCmd::getSettings()
{
	MStringArray values;

	// current mesh
	values.append("currentMesh");
	values.append(DataStore::getInstance().getCurrentRegisteredMesh().c_str());

	// fullmesh enabled
	values.append("fullMesh");
	MString tmp;
	tmp += DataStore::getInstance().getFullMeshRequest();
	values.append(tmp);

	return values;
}

void SettingsCmd::setSettings(MArgDatabase& parser)
{
	if (parser.isFlagSet("-fm"))
	{
		bool tmp;
		parser.getFlagArgument("-fm", 0, tmp);
		DataStore::getInstance().setFullMeshRequest(tmp);
	}
}
