#pragma once
///////////////////////////////////////////////////////////////////////////
// check-out.h - Implements file checkout functionality for the database //
// ver 1.0																 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018		 //
///////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* - This package provides a class CheckOut which implements
* - file restoration along with its dependencies to the client storage.
*
* Public Interface
* -------------------
* - CheckIn(RepoCore<T>& repo);
*
* - string& fileSpec() ;
* - string fileSpec() const ;
* - void fileSpec(const string& fileSpec) ;

* - int& version() ;
* - int version() const  const ;
* - void version(const int& version) ;

* - string& clientPath() ;
* - string clientPath() const;
* - void clientPath(const string& clientPath) ;
*
* - void getFile();
*
* Required Files:
* ---------------
* check-out.h, check-out.cpp
* version.h
* repocore.h
* FileSystem.h
*
* Maintenance History:
* --------------------
* - first release
*/

#include <iostream>
#include <string>
#include "../RepoCore/repocore.h"
#include "../Version/version.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"

using namespace std;
using namespace Repository;
using namespace Versioning;
using namespace FileSystem;

namespace checkOut
{
	///////////////////////////////////////////////////////////////////////////
	// CheckOut class
	// - Implements file checkout functionality for a given file and clientpath

	template<typename T>
	class CheckOut {
	public:
		CheckOut(RepoCore<T>& repo) {
			repo_ = &repo;
		}
		void getFile();

		string& fileSpec() { return fileSpec_; }
		string fileSpec() const { return fileSpec_; }
		void fileSpec(const string& fileSpec) { fileSpec_ = fileSpec; }

		string& clientPath() { return clientPath_; }
		string clientPath() const { return clientPath_; }
		void clientPath(const string& clientPath) { clientPath_ = clientPath; }

		int& version() { return version_; }
		int version() const { return version_; }
		void version(const int& version) { version_ = version; }

		void copyDependencies(const string& copyDependencies) { copyDependencies_ = copyDependencies; }

	private:
		RepoCore<T>* repo_;
		string fileSpec_;
		string clientPath_;
		string clientFileName_;
		string repoPath_ = "../Storage/";
		int version_ = 0;
		string fileName_;
		string copyDependencies_ = "false";
		void getRepoPath();
		void copyFiles();
	};

	//----< copy file from repository to client storage  >----------------------------------
	template<typename T>
	void CheckOut<T>::getFile() {
		unordered_map<string, bool>::const_iterator found = repo_->statusMap().find(fileSpec_);
		if (found != repo_->statusMap().end()) {
			if (version_ > repo_->versionMap()[fileSpec_]) {
				cout << "\n  The given file is not present in the Repository\n";
				return;
			}
			else {
				getRepoPath();
				copyFiles();
			}
		}
		else {
			cout << "\n  The given file is not present in the Repository\n";
			return;
		}
	}

	//----< helper function to extract repository path >----------------------------------
	template<typename T>
	void CheckOut<T>::getRepoPath() {
		string delimiter = "::";
		size_t s_pos = 0;
		size_t pos = 0;
		std::string token;
		while ((pos = fileSpec_.find(delimiter, s_pos)) != string::npos) {
			token = fileSpec_.substr(s_pos, pos - s_pos);
			repoPath_ = repoPath_ + token + "/";
			s_pos = pos + delimiter.length();
		}
		token = fileSpec_.substr(s_pos, fileSpec_.length());
		clientFileName_ =  token;
		if (version_ == 0) {
			fileName_ = token + "." + to_string(repo_->versionMap()[fileSpec_]);
			version_ = repo_->versionMap()[fileSpec_];
		}
		else
			fileName_ = token + "." + to_string(version_);
		repoPath_ = repoPath_ + fileName_;
	}

	//----< helper class to copy file along with its dependencies using File class>----------------------------------
	template<typename T>
	void CheckOut<T>::copyFiles() {
		FileSystem::File::copy(repoPath_, clientPath_+clientFileName_);
		if (copyDependencies_ == "false")
			return;
		for (auto child : repo_->database()[fileSpec_+"."+to_string(version_)].children()) {
			size_t pos = child.find_last_of(".");
			fileSpec_ = child.substr(0,pos);
			version_ = stoi(child.substr(pos+1, child.length()));
			repoPath_ = "../Storage/";
			getFile();
		}
	}
}