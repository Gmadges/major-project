#include "settingsCmd.h"

#include <maya/MStringArray.h>

#include "dataStore.h"
#include "callbackHandler.h"

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
	syn.addFlag("-ui", "-updateInterval", MSyntax::kDouble);

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
	MString fullMeshString;
	fullMeshString += DataStore::getInstance().getFullMeshRequest();
	values.append(fullMeshString);

	values.append("updateInterval");
	MString updateIntString;
	updateIntString += DataStore::getInstance().getUpdateInterval();
	values.append(updateIntString);

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

	if (parser.isFlagSet("-ui"))
	{
		double tmp;
		parser.getFlagArgument("-ui", 0, tmp);
		DataStore::getInstance().setUpdateInterval(tmp);
		CallbackHandler::getInstance().startTimerCallback(true);
	}
}
