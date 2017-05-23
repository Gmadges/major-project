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

	// query means your asking for the settings values
	syn.addFlag("-q", "-query");

	// settings we handle
	// cant set the current mesh
	syn.addFlag("-cm", "-currentMesh");
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
		setResult(getSettings(parser));
	}
	else
	{
		setSettings(parser);
	}

	return status;
}

MStringArray SettingsCmd::getSettings(MArgDatabase& parser)
{
	MStringArray values;

	if (parser.isFlagSet("-cm"))
	{
		values.append("currentMesh");
		values.append(DataStore::getInstance().getCurrentRegisteredMesh().c_str());
	}

	if (parser.isFlagSet("-fm"))
	{
		values.append("fullMesh");
		MString tmp;
		tmp += DataStore::getInstance().getFullMeshRequest();
		values.append(tmp);
	}

	return values;
}

void SettingsCmd::setSettings(MArgDatabase& parser)
{
	if (parser.isFlagSet("-fm"))
	{
		bool tmp;
		parser.getFlagArgument("-p", 0, tmp);
		DataStore::getInstance().setFullMeshRequest(tmp);
	}
}
