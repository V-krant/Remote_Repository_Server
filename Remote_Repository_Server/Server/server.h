#pragma once
///////////////////////////////////////////////////////////////////////
// server.h - Repository server that processes incoming messages	 //
// ver 1.0                                                           //
// Vikant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018    //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  Package contains one class, Server, that contains a Message-Passing Communication
*  facility. It processes each message by invoking an installed callable object
*  defined by the message's command key.
*
*  Message handling runs on a child thread, so the Server main thread is free to do
*  any necessary background processing (none, so far).
*
*  Provides a static repocore object to be used for checkin,checkout,etc.
*
* Public Interface:
* ------------------
* - Server(MsgPassingCommunication::EndPoint ep, const std::string& name);
* - void start();
* - void stop();
* - void addMsgProc(Key key, ServerProc proc);
* - void processMessages();
* - void postMessage(MsgPassingCommunication::Message msg);
* - MsgPassingCommunication::Message getMessage();
* - static Dirs getDirs(const SearchPath& path = storageRoot);
* - static Files getFiles(const SearchPath& path = storageRoot);
* - std::string getPath(std::string fileName);
*
*  Required Files:
* -----------------
*  ServerPrototype.h, ServerPrototype.cpp
*  Comm.h, Comm.cpp, IComm.h
*  Message.h, Message.cpp
*  FileSystem.h, FileSystem.cpp
*  Utilities.h
*
*  Maintenance History:
* ----------------------
*  ver 1.0 : 3/27/2018
*  - first release
*/
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include "../CppCommWithFileXfer/Message/Message.h"
#include "../CppCommWithFileXfer/MsgPassingComm/Comm.h"
#include "../DBCore/dbcore.h"
#include "../Query/query.h"
#include "../Check-In/check-in.h"
#include "../Check-Out/check-out.h"
#include "../RepoCore/repocore.h"
#include "../Payload/PayLoad.h"
#include "../Browse/browse.h"
#include"../Persistance/persistance.h"
#include <windows.h>
#include <tchar.h>

using namespace NoSqlDb;
using namespace Repository;
using namespace checkIn;
using namespace checkOut;
using namespace Browsing;

namespace RepoServer
{
	using File = std::string;
	using Files = std::vector<File>;
	using Dir = std::string;
	using Dirs = std::vector<Dir>;
	using SearchPath = std::string;
	using Key = std::string;
	using Msg = MsgPassingCommunication::Message;
	using ServerProc = std::function<Msg(Msg)>;
	using MsgDispatcher = std::unordered_map<Key, ServerProc>;

	const SearchPath storageRoot = "../Storage";  // root for all server file storage
	const MsgPassingCommunication::EndPoint serverEndPoint("localhost", 8080);  // listening endpoint

//////////////////////////////////////////////////////
//	Server class provides a receiver thread and a static repository 
//	object for handling incoming fequests
	class Server
	{
	public:
		Server(MsgPassingCommunication::EndPoint ep, const std::string& name);
		void start();
		void stop();
		void addMsgProc(Key key, ServerProc proc);
		void processMessages();
		void postMessage(MsgPassingCommunication::Message msg);
		MsgPassingCommunication::Message getMessage();
		static Dirs getDirs(const SearchPath& path = storageRoot);
		static Files getFiles(const SearchPath& path = storageRoot);

		static RepoCore<Payload>& repocore() { return RC_; }
	private:
		void processMessageHelper1(Msg msg, Msg reply);
		void processMessageHelper2(Msg msg, Msg reply);
		MsgPassingCommunication::Comm comm_;
		MsgDispatcher dispatcher_;
		std::thread msgProcThrd_;
		static RepoCore<Payload> RC_;

	};
	RepoCore<Payload> Server::RC_;
	//----< initialize server endpoint and give server a name >----------

	inline Server::Server(MsgPassingCommunication::EndPoint ep, const std::string& name)
		: comm_(ep, name) {}

	//----< start server's instance of Comm >----------------------------

	inline void Server::start()
	{
		comm_.start();
	}
	//----< stop Comm instance >-----------------------------------------

	inline void Server::stop()
	{
		if (msgProcThrd_.joinable())
			msgProcThrd_.join();
		comm_.stop();
	}
	//----< pass message to Comm for sending >---------------------------

	inline void Server::postMessage(MsgPassingCommunication::Message msg)
	{
		try
		{
			comm_.postMessage(msg);
		}
		catch (const std::exception&)
		{
			std::cout << "\nUnable to send message...\n";
		}
	}
	//----< get message from Comm >--------------------------------------

	inline MsgPassingCommunication::Message Server::getMessage()
	{
		Msg msg;
		try
		{
			msg = comm_.getMessage();
			
		}
		catch (const std::exception&)
		{
			std::cout << "\nUnable to receive message...\n";
		}
		return msg;
	}
	//----< add ServerProc callable object to server's dispatcher >------

	inline void Server::addMsgProc(Key key, ServerProc proc)
	{
		dispatcher_[key] = proc;
	}

	//----< helper function for processing incoming messages >----------
	void Server::processMessageHelper1(Msg msg, Msg reply) {
		if (msg.containsKey("dependants")) {
			if (msg.value("dependants") == "true") {
				for (auto filename : FileSystem::Directory::getFiles("../SendFiles")) {
					reply.attribute("file", filename);
					postMessage(reply);
					msg.show();
					reply.show();
				}
			}
			else {
				try
				{
					postMessage(reply);
				}
				catch (const std::exception&)
				{
					std::cout << "\nUnable to send message...\n";
				}
				reply.show();
			}
		}
		else {
			try
			{
				postMessage(reply);
			}
			catch (const std::exception&)
			{
				std::cout << "\nUnable to send message...\n";
			}
			reply.show();
		}
	}

	//----< helper function for processing incoming messages >----------
	void Server::processMessageHelper2(Msg msg, Msg reply) {
		if (reply.containsKey("count")) {
			int count = stoi(reply.value("count"));

			while (count > 0) {

				msg.attribute("parentDir", reply.value("dir" + std::to_string(count)));
				msg.attribute("path", reply.value("path") + "/" + reply.value("dir" + std::to_string(count)));
				try
				{
					postMessage(msg);
				}
				catch (const std::exception&)
				{
					std::cout << "\nUnable to send message...\n";
				}
				count--;
			}
			reply.command("sentStorage");
			try
			{
				postMessage(reply);
			}
			catch (const std::exception&)
			{
				std::cout << "\nUnable to send message...\n";
			}

		}

		if (reply.containsKey("file")) {
			reply.attribute("fileName", reply.file());
			reply.command("sentFile");
			reply.remove("file");
			try
			{
				postMessage(reply);
			}
			catch (const std::exception&)
			{
				std::cout << "\nUnable to send message...\n";
			}
			reply.show();
		}
	}

	//----< start processing messages on child thread >------------------

	inline void Server::processMessages()
	{
		auto proc = [&]()
		{
			if (dispatcher_.size() == 0)
			{
				std::cout << "\n  no server procs to call";
				return;
			}
			while (true)
			{
				Msg msg = getMessage();
				if (!msg.containsKey("file")) {
					std::cout << "\n  received message: " << msg.command() << " from " << msg.from().toString();
					if (msg.containsKey("verbose"))
					{
						std::cout << "\n";
						msg.show();
					}
					if (msg.command() == "serverQuit")
						break;
					Msg reply;
					try
					{
						reply = dispatcher_[msg.command()](msg);
					}
					catch (const std::exception&)
					{
						std::cout << "\nUnable to call a server proc...\n";
					}
					if (msg.to().port != msg.from().port)  // avoid infinite message loop
					{
						if (msg.containsKey("connect")) {
							reply.attribute("connect", "ConnectToClient");
						}
						processMessageHelper1(msg, reply);
						processMessageHelper2(msg, reply);
					}
					else
						std::cout << "\n  server attempting to post to self";
				}
			}
			std::cout << "\n  server message processing thread is shutting down";
		};
		std::thread t(proc);
		std::cout << "\n  starting server thread to process messages";
		msgProcThrd_ = std::move(t);
	}
}