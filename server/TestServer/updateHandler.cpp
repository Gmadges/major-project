#include "updateHandler.h"

#include "testTypes.h"
#include "database.h"
#include <thread>

UpdateHandler::UpdateHandler(std::shared_ptr<Database> _db)
	:
	pDB(_db)
{
}


UpdateHandler::~UpdateHandler()
{
}

bool UpdateHandler::registerMesh(json& _request)
{
	if (_request.count("mesh") > 0)
	{
		return pDB->putMesh(_request["mesh"]);
	}
	return false;
}

bool UpdateHandler::updateMesh(json& _request)
{
	if (_request.count("mesh") > 0)
	{
		// fire off to do comparison
		// do on another thread might be long
		std::thread job(&UpdateHandler::updateAndStoreMesh, this, _request["mesh"], _request["uid"]);
		job.detach();

		return true;
	}

	return false;
}

void UpdateHandler::updateAndStoreMesh(json _mesh, std::string userID)
{
	try 
	{
		// super basic right now
		// replaces any nodes with the new versions of them selves.
		json meshAndEdit = pDB->getMeshWithEdits(_mesh["id"].get<std::string>());
		json currentMesh = meshAndEdit["mesh"];
		std::vector<json> edits = meshAndEdit["edits"];
		std::vector<json> nodeList = currentMesh["nodes"];

		for (auto& newNode : _mesh["nodes"])
		{
			if (newNode["edit"] == EditType::ADD)
			{
				insertNodeIntoList(newNode, nodeList);
			}
			else if (newNode["edit"] == EditType::EDIT)
			{
				editNodeInList(newNode, nodeList);
			}
			else if (newNode["edit"] == EditType::DEL)
			{
				removeNodefromList(newNode, nodeList);
			}
		}

		currentMesh["nodes"] = nodeList;
		edits.push_back(_mesh);
		edits.back()["uid"] = userID;

		meshAndEdit["mesh"] = currentMesh;
		meshAndEdit["edits"] = edits;

		pDB->putMeshWithEdits(meshAndEdit);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

void UpdateHandler::insertNodeIntoList(json& _node, std::vector<json>& _nodeList)
{
	std::string inID; 
	std::string outID; 
	
	std::string ID = _node["id"];

	// find the "in" node
	if (!_node["in"].is_null())
	{
		inID = _node["in"].get<std::string>();
	}

	if (!_node["out"].is_null())
	{
		outID = _node["out"].get<std::string>();
	}

	_nodeList.push_back(_node);
	
	if (inID.empty() && outID.empty())
	{
		return;
	}

	for (unsigned int i = 0; i < _nodeList.size(); ++i)
	{
		// TODO should probable check the original values instead of overwriting

		if (inID.compare(_nodeList[i]["id"]) == 0)
		{
			_nodeList[i]["out"] = ID;
		}

		if (outID.compare(_nodeList[i]["id"]) == 0)
		{
			_nodeList[i]["in"] = ID;
		}
	}
}

void UpdateHandler::removeNodefromList(json& _node, std::vector<json>& _nodeList)
{
	std::string id = _node["id"];
	std::string inID;
	std::string outID;

	// brute force find our node because we need its in's and outs
	for (unsigned int i = 0; i < _nodeList.size(); ++i)
	{
		if (id.compare(_nodeList[i]["id"]) == 0)
		{
			std::cout << "del : " << id << std::endl;
			if (_nodeList[i]["in"] != nullptr)
			{
				inID = _nodeList[i]["in"].get<std::string>();
			}

			if (_nodeList[i]["out"] != nullptr)
			{
				outID = _nodeList[i]["out"].get<std::string>();
			}

			_nodeList.erase(_nodeList.begin() + i);
			break;
		}
	}

	for (unsigned int i = 0; i < _nodeList.size(); ++i)
	{
		if (_nodeList[i]["in"] != nullptr)
		{
			if (outID.compare(_nodeList[i]["in"].get<std::string>()) == 0)
			{
				_nodeList[i]["in"] = inID;
			}
		}

		if (_nodeList[i]["out"] != nullptr)
		{
			if (inID.compare(_nodeList[i]["out"].get<std::string>()) == 0)
			{
				_nodeList[i]["out"] = outID;
			}
		}
	}
}

void UpdateHandler::editNodeInList(json& _node, std::vector<json>& _nodeList)
{
	std::string id = _node["id"];

	for (unsigned int i = 0; i < _nodeList.size(); ++i)
	{
		if (id.compare(_nodeList[i]["id"]) == 0)
		{
			std::cout << "edit : " << id << std::endl;

			//json difference = json::diff(_nodeList[i], _node);
			//std::cout << difference.dump(4);

			// this difference is a "patch" that can be used to update the old node. we can manipulate this to figure out what should be changed.

			_nodeList[i] = _node;
		}
	}
}
