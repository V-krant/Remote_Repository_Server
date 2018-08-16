/////////////////////////////////////////////////////////////////////////
// server.cpp - Repository server that processes incoming messages	   //
// ver 1.0                                                             //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018     //
/////////////////////////////////////////////////////////////////////////

#include "server.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <chrono>

namespace MsgPassComm = MsgPassingCommunication;

using namespace RepoServer;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;

//----< get files from a given path >----------
Files Server::getFiles(const RepoServer::SearchPath& path)
{
	return Directory::getFiles(path);
}

//----< get directories from a given path >----------
Dirs Server::getDirs(const RepoServer::SearchPath& path)
{
	return Directory::getDirectories(path);
}

//----< display message and a template element >----------
template<typename T>
void show(const T& t, const std::string& msg)
{
	std::cout << "\n  " << msg.c_str();
	for (auto item : t)
	{
		std::cout << "\n    " << item.c_str();
	}
}

//----< reply a dummy message to the sender >----------
std::function<Msg(Msg)> echo = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	return reply;
};

//----< callable object to get files from a directory >----------
std::function<Msg(Msg)> getFiles = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getFiles");
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		std::string key;
		if (path != ".") {
			searchPath = searchPath + "\\" + path;

		}
		Files files = Server::getFiles(searchPath);
		size_t count = 0;
		for (auto item : files)
		{
			std::string countStr = Utilities::Converter<size_t>::toString(++count);
			reply.attribute("file" + countStr, item);
			if (msg.value("namespaces") != "")
				reply.attribute("key" + countStr, msg.value("namespaces") + "::" + item);
			else
				reply.attribute("key" + countStr, item);
		}
		reply.attribute("filescount", Utilities::Converter<size_t>::toString(count));
	}
	else
	{
		std::cout << "\n  getFiles message did not define a path attribute";
	}
	return reply;
};

//----< callable object to get directories inside a directory >----------
std::function<Msg(Msg)> getDirs = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getDirs");
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		if (path != ".")
			searchPath = searchPath + "\\" + path;
		Files dirs = Server::getDirs(searchPath);
		size_t count = 0;
		for (auto item : dirs)
		{
			if (item != ".." && item != ".")
			{
				std::string countStr = Utilities::Converter<size_t>::toString(++count);
				reply.attribute("dir" + countStr, item);
			}
		}
	}
	else
	{
		std::cout << "\n  getDirs message did not define a path attribute";
	}
	return reply;
};

//----< callable object to copy a file to client >----------
std::function<Msg(Msg)> sendFile = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("sending");
	std::string path;
	if (msg.containsKey("path"))
		path = msg.value("path");
	else {
		path = Server::repocore().database()[msg.value("filename")].payLoad().filepath();
	}
	if (path != "" && path != "absent")
	{
		if (path != ".") {
			std::size_t pos = path.find_last_of("/");
			std::string fileName = path.substr(pos + 1);
			FileSystem::File::copy(path, "../SendFiles/" + fileName);
			reply.file(fileName);
		}
	}
	else {
		reply.attribute("fileName", "false");
		reply.command("sentFile");
	}
	return reply;
};

//----< convert string to a vector of strings >----------
std::vector<std::string> strToVector(std::string str) {
	vector<std::string> vecStr;
	std::string delimiter = " ";
	size_t s_pos = 0;
	size_t pos = 0;
	std::string token;
	while ((pos = str.find(delimiter, s_pos)) != string::npos) {
		token = str.substr(s_pos, pos - s_pos);
		vecStr.push_back(token);
		s_pos = pos + delimiter.length();
	}
	return vecStr;
}

//----< callable object to checkin a file in repository >----------
std::function<Msg(Msg)> checkInFile = [](Msg msg) {
	checkIn::CheckIn<Payload> C1(Server::repocore());
	C1.fileSpec(msg.value("filespec"));
	C1.clientPath("../SaveFiles/" + msg.value("filename"));
	DbElement<Payload> Elem;
	Elem.dateTime(DateTime().now());
	Elem.descrip(msg.value("description"));
	if (msg.value("children") != "") {
		vector<std::string> children = strToVector(msg.value("children"));
		for (auto child : children)
			Elem.children().push_back(child);
	}
	Elem.status(msg.value("status"));
	Payload P1;
	if (msg.value("category") != "") {
		vector<std::string> categories = strToVector(msg.value("category"));
		for (auto cat : categories)
			P1.categories().push_back(cat);
	}
	Elem.payLoad(P1);
	C1.metadata(Elem);
	bool flag = C1.storeFile();					// Checkin file to database
	Msg reply;
	if (flag == false)
		reply.attribute("success", "false");
	else
		reply.attribute("success", "true");
	reply.attribute("filename", msg.value("filename"));
	NoSqlDb::showDb(Server::repocore().database());
	reply.to(msg.from());
	reply.from(msg.to());
	if (Server::repocore().statusMap()[msg.value("filespec")])
		reply.attribute("status", "Close");
	else
		reply.attribute("status", "Open");
	reply.command("checkedIn");
	Persistance<Payload> P;
	P.xmlStore(Server::repocore(), "../Database.xml", "../StorageInfo.xml");
	return reply;
};

//----< callable object to checkout a file from repository >----------
std::function<Msg(Msg)> checkOutFile = [](Msg msg) {

	CheckOut<Payload> CO1(Server::repocore());
	CO1.clientPath("../SendFiles/");
	try
	{
		CO1.version(std::stoi(msg.value("version")));
	}
	catch (const std::exception&)
	{
		msg.attribute("filekey", msg.value("filekey") + "." + msg.value("version"));
	}
	CO1.fileSpec(msg.value("filekey"));
	CO1.copyDependencies(msg.value("dependants"));
	CO1.getFile();

	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("checkingOut");
	size_t pos = (msg.value("filekey").find_last_of("::"));
	if (pos < msg.value("filekey").size())
		reply.file(msg.value("filekey").substr(pos + 1, msg.value("filekey").length() - pos - 1));					// Send file to client
	else
		reply.file(msg.value("filekey"));
	if (FileSystem::File::exists("../SendFiles/" + reply.value("file")))
		reply.attribute("checkedOut", "true");
	else
		reply.attribute("checkedOut", "false");
	return reply;
};

//----< callable object to get metadata of a repository file >----------
std::function<Msg(Msg)> getMetaData = [](Msg msg) {
	std::string key = msg.value("filename");
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("putMetadata");
	if (Server::repocore().database().contains(key))
	{
		reply.attribute("name", Server::repocore().database()[key].name());
		reply.attribute("description", Server::repocore().database()[key].descrip());
		reply.attribute("datetime", Server::repocore().database()[key].dateTime());
		std::vector<std::string> categories = Server::repocore().database()[key].payLoad().categories();
		std::string cat = "";
		for (auto c : categories)
			cat = cat + " " + c;
		reply.attribute("category", cat);
		reply.attribute("filepath", Server::repocore().database()[key].payLoad().filepath());
		reply.attribute("status", Server::repocore().database()[key].status());
		std::string children = "";
		for (auto c : Server::repocore().database()[key].children())
			children = children + " " + c;
		reply.attribute("children", children);
		reply.attribute("success", "true");
	}
	else
		reply.attribute("success", "false");
	return reply;
};

//----< get repo directory structure >----------
std::function<Msg(Msg)> getRepoStorage = [](Msg msg) {

	Files dirs = Server::getDirs(msg.value("path"));
	Files files = Server::getFiles(msg.value("path"));
	Msg reply;

	reply.to(msg.from());
	reply.from(msg.to());
	size_t count = 0;
	for (auto item : dirs)
	{
		if (item != ".." && item != ".")
		{
			std::string countStr = Utilities::Converter<size_t>::toString(++count);
			reply.attribute("dir" + countStr, item);
		}
	}
	reply.attribute("count", std::to_string(count));
	count = 0;
	for (auto item : files)
	{
		std::string countStr = Utilities::Converter<size_t>::toString(++count);
		reply.attribute("file" + countStr, item);
	}
	reply.attribute("parentDir", msg.value("parentDir"));
	reply.attribute("path", msg.value("path"));
	reply.command("addRepoStorage");
	return reply;
};

//----< clear SendFiles directory >----------
std::function<Msg(Msg)> clearSendDir = [](Msg msg) {
	for (auto file : FileSystem::Directory::getFiles("../SendFiles"))
		FileSystem::File::remove("../SendFiles/" + file);
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	return reply;
};

//----< search database for given queries >----------
std::function<Msg(Msg)> searchDatabase = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getFiles");
	Browse<Payload> B1(Server::repocore());
	if (msg.value("querytype") == "single") {				// single query present
		Query<Payload> Q1 = B1.browseMetaData(msg.value("filename1"), msg.value("version1"), msg.value("category1"), msg.value("dependencies1"), msg.value("nodependants1"));
		size_t count = 0;
		for (auto elem : Q1.db()) {
			count++;
			reply.attribute("file" + to_string(count), elem.second.name());
			reply.attribute("key" + to_string(count), elem.first);
		}
		reply.attribute("filescount", to_string(count));
	}
	else if (msg.value("querytype") == "union") {				// union query
		vector<Query<Payload>>Q1;
		size_t querySize = std::stoi(msg.value("count"));
		while (querySize > 0) {
			Q1.push_back(B1.browseMetaData(msg.value("filename" + to_string(querySize)), msg.value("version" + to_string(querySize)), msg.value("category" + to_string(querySize)), msg.value("dependencies" + to_string(querySize)), msg.value("nodependants" + to_string(querySize))));
			querySize--;
		}
		Query<Payload>unionQ = B1.unionQueries(Q1);
		size_t count = 0;
		for (auto elem : unionQ.db()) {
			count++;
			reply.attribute("file" + to_string(count), elem.second.name());
			reply.attribute("key" + to_string(count), elem.first);
		}
		reply.attribute("filescount", to_string(count));
	}
	else {														// and query
		vector<Query<Payload>>Q1;
		size_t querySize = std::stoi(msg.value("count"));
		while (querySize > 0) {
			Q1.push_back(B1.browseMetaData(msg.value("filename" + to_string(querySize)), msg.value("version" + to_string(querySize)), msg.value("category" + to_string(querySize)), msg.value("dependencies" + to_string(querySize)), msg.value("nodependants" + to_string(querySize))));
			querySize--;
		}
		Query<Payload>andQ = B1.andQueries(Q1);
		size_t count = 0;
		for (auto elem : andQ.db()) {
			count++;
			reply.attribute("file" + to_string(count), elem.second.name());
			reply.attribute("key" + to_string(count), elem.first);
		}
		reply.attribute("filescount", to_string(count));
	}
	return reply;
};

//----< main() function of Server.cpp >----------
int main()
{
	std::cout << "\n ***Starting Server on port 8080***";
	std::cout << "\n ==================================";
	std::cout << "\n";

	//StaticLogger<1>::attach(&std::cout);
	//StaticLogger<1>::start();

	Server server(serverEndPoint, "Repository Server");		//Start server on port 8080
	server.start();
	if (FileSystem::File::exists("../Database.xml") && FileSystem::File::exists("../StorageInfo.xml")) {
		Persistance<Payload> P;
		Server::repocore() = P.xmlRestore("../Database.xml", "../StorageInfo.xml");
	}
	server.addMsgProc("echo", echo);
	server.addMsgProc("getFiles", getFiles);
	server.addMsgProc("getDirs", getDirs);
	server.addMsgProc("serverQuit", echo);
	server.addMsgProc("sendFile", sendFile);				// Add callable objects to dictionary
	server.addMsgProc("checkIn", checkInFile);
	server.addMsgProc("checkOutFile", checkOutFile);
	server.addMsgProc("getMetaData", getMetaData);
	server.addMsgProc("getRepoStorage", getRepoStorage);
	server.addMsgProc("clearSendDir", clearSendDir);
	server.addMsgProc("searchDatabase", searchDatabase);
	server.processMessages();								// Start receiver and sender thread

	std::cin.get();
	std::cout << "\n";

	Msg msg(serverEndPoint, serverEndPoint);
	msg.command("serverQuit");
	server.postMessage(msg);
	server.stop();
	return 0;
}

