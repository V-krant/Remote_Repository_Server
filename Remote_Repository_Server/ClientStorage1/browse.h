#pragma once
///////////////////////////////////////////////////////////////////////
// browse.h - Implements browsing functionality for the database	 //
// ver 1.0															 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	 //
///////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* - This package provides a class Browse which provides the user an interface
* - to query the database with different attributes. It also provides a 
* - function to display full text of a selected file.
*
* Public Interface
* -------------------
* - Query<T> browseMetaData(string name, DateTime startTime, DateTime endTime, string descrip, string payload);
* - void browseFile(string key);
* - Query<T> browseFiles(string key);
* - void browseDpndncsOf(string key);
* - void openFile(string key);
* - Query<T> andQueries(vector<Query<T>> queries);
* - Query<T> unionQueries(vector<Query<T>> queries);
*
* Required Files:
* ---------------
* browse.h, browse.cpp
* query.h
* repocore.h
* Process.h
* DateTime.h, DateTime.cpp
*
* Maintenance History:
* --------------------
* - first release
*/

#include <iostream>
#include <string>
//#include "../Process/Process.h"
#include "../Query/query.h"
#include "../DateTime/DateTime.h"
#include "../RepoCore/repocore.h"

using namespace Repository;
//using namespace Application;

namespace Browsing {
	
	template<typename T>
	class Browse {
	public:
		Browse(RepoCore<T>& repo) {
			repo_ = &repo;
		}
		Query<T> browseMetaData(string name, string version, string payload, string children, string noDependants);
		void browseFile(string key);
		Query<T> browseFiles(string key);
		void browseDpndncsOf(string key);
		//void openFile(string key);
		Query<T> andQueries(vector<Query<T>> queries);
		Query<T> unionQueries(vector<Query<T>> queries);
		void browseVersion(Query<T> * Q, string version);
		void browseNoDependants(Query<T> * Q);

	private:
		RepoCore<T>* repo_;
		vector<string> keys_;
	};

	//----< query the database using condition class and metadata>----------------------------------
	template<typename T>
	Query<T> Browse<T>::browseMetaData(string name, string version, string payload,string children, string noDependants) {
		Condition CO;
		CO.name(name);
		CO.children(children);
		CO.payload(payload);
		Query<T> Q(repo_->database());
		Q.select(CO);
		if (version != "")
			browseVersion(&Q, version);
		if (noDependants == "true")
			browseNoDependants(&Q);
		showDb(Q.db());
		return Q;
	}

	template<typename T>
	void Browse<T>::browseVersion(Query<T> * Q,string version) {
		DbCore<T> tempDb = Q->db();
		for (auto elem : tempDb.dbStore()) {
			size_t pos=elem.first.find_last_of('.');
			std::string vn;
				vn = elem.first.substr(pos+1, elem.first.length()-pos);
			if (vn != version)
			Q->db().removeElement(elem.first);
		}
	}

	template<typename T>
	void Browse<T>::browseNoDependants(Query<T> * Q) {
		for (auto elem : repo_->dependantsMap()) {
			if (elem.second.size() != 0)
				Q->db().removeElement(elem.first);
		}
	}

	//----< query the databse using a key value >----------------------------------
	template<typename T>
	void Browse<T>::browseFile(string key) {
		Query<T> Q1(repo_->database());
		Q1.valueOfKey(key);
	}

	//----< query the databse using a regex for key values >----------------------------------
	template<typename T>
	Query<T> Browse<T>::browseFiles(string key) {
		Condition C1;
		C1.key(key);
		Query<T> Q2(repo_->database());
		Q2.select(C1);
		showDb(Q2.db());
		return Q2;
	}

	//----< query the database for children of a given key >----------------------------------
	template<typename T>
	void Browse<T>::browseDpndncsOf(string key) {
		Query<T> Q3(repo_.database());
		Q1.childrenOfKey(key);
	}

	//----< To AND the result of two or more queries >----------------------------------
	template<typename T>
	Query<T> Browse<T>::andQueries(vector<Query<T>> queries){
		Query<T> qtemp = queries.back();
		queries.pop_back();
		for (auto query : queries) {
			qtemp.CommonWith(query);
		}
		showDb(qtemp.db());
		return qtemp;
	}

	//----< To OR the result of two or more queries >----------------------------------
	template<typename T>
	Query<T> Browse<T>::unionQueries(vector<Query<T>> queries) {
		Query<T> qtemp = queries.back();
		queries.pop_back();
		for (auto query : queries) {
			qtemp.UnionWith(query);
		}
		showDb(qtemp.db());
		return qtemp;
	}

	//----< To open a file using notepad.exe >----------------------------------
	//template<typename T>
	//void Browse<T>::openFile(string key) {
	//	Process p;
	//	p.title("Opening file with key");
	//	string appPath = "c:/windows/system32/notepad.exe";
	//	p.application(appPath);
	//	std::string fpath = repo_->database()[key].payLoad().filepath();
	//	string cmdLine = "/A " + fpath;
	//	p.commandLine(cmdLine);

	//	cout << "\n  starting process: \"" << appPath << "\"";
	//	cout << "\n  with this cmdlne: \"" << cmdLine << "\"";
	//	p.create();
	//}
}