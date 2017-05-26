#ifndef SETTINGSCMD_H
#define SETTINGSCMD_H

#include <maya/MPxCommand.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

class SettingsCmd : public MPxCommand
{
public:
	SettingsCmd();
	~SettingsCmd();

	static void* creator();
	static MSyntax newSyntax();
	virtual MStatus doIt(const MArgList&) override;

private:
	MStringArray getSettings();
	void setSettings(MArgDatabase& parser);
};

#endif
