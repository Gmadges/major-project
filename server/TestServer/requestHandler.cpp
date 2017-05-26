#include "requestHandler.h"

#include "database.h"
#include "userInfo.h"

RequestHandler::RequestHandler(std::shared_ptr<Database> _db, std::shared_ptr<UserInfo> _user)
	:
	pDB(_db),
	pUserInfo(_user)
{
}


RequestHandler::~RequestHandler()
{
}

json RequestHandler::requestMesh(json& _request)
{
	if (_request.count("id") > 0)
	{
		return pDB->getMesh(_request["id"].get<std::string>());
	}

	return json();
}

json RequestHandler::requestMeshUpdates(json& _request)
{
	json response;

	if (_request.count("id") > 0)
	{
		if (_request.count("uid") > 0)
		{
			std::vector<json> edits = pDB->getMeshEdits(_request["id"].get<std::string>());

			if(edits.empty()) return response;

			// first iterate back to find if any changes have occured.
			std::string userId = _request["uid"].get<std::string>();
			time_t lastReq = pUserInfo->getUsersLastUpdate(userId);

			if (lastReq == 0) return response;

			response["edits"] = std::vector<json>();

			for (auto& edit : edits)
			{
				// now spot which changes are not by the user
				if (edit["db_time"].get<time_t>() > lastReq)
				{
					if (edit["uid"].get<std::string>().compare(userId) != 0)
					{
						// we have a change that isnt ours
						if (_request["fullMesh"] == true)
						{
							// if full mesh is there we send the whole mesh currently and leave it at that.
							json mesh = pDB->getMesh(_request["id"].get<std::string>());
							response["edits"] = mesh["nodes"];
							break;
						}
						else
						{
							response["edits"].push_back(edit);
						}
					}
				}
			}
		}
	}

	return response;
}
