#pragma once
/////////////////////////////////////////////////////////////////////////
// check-in.h - Implements file checkin functionality for the database //
// ver 1.0															   //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	   //
/////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* - This package provides a class CheckIn which implements checkin
* - operations on a file such as versioning, coying file to the 
* - repository storage and adding file to database.
*
* Public Interface
* -------------------
* - CheckIn(RepoCore<T>& repo);
*
* - DbElement<T>& metadata();
* - DbElement<T> metadata() const;
* - void metadata(const DbElement<T>& metadata) ;

* - string& fileSpec() ;
* - string fileSpec() const ;
* - void fileSpec(const string& fileSpec) ;

* - string& clientPath() ;
* - string clientPath() const;
* - void clientPath(const string& clientPath) ;

* - void storeFile();
*
* Required Files:
* ---------------
* check-in.h, check-in.cpp
* version.h
* repocore.h
* FileSystem.h
*
* Maintenance History:
* --------------------
* - first release
*/

#include<iostream>
#include<string>
#include<unordered_set>
#include "../RepoCore/repocore.h"
#include "../Version/version.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"

using namespace std;
using namespace Repository;
using namespace Versioning;
using namespace FileSystem;

namespace checkIn 
{
	/////////////////////////////////////////////////////////////////////
	// CheckIn class
	// - provides functionality for checking in a file to repository
	template<typename T>
	class CheckIn {
	public:
		CheckIn(RepoCore<T>& repo) {
			repo_ = &repo;
		}

		DbElement<T>& metadata() { return metadata_; }
		DbElement<T> metadata() const { return metadata_; }
		void metadata(const DbElement<T>& metadata) { metadata_ = metadata; }

		string& fileSpec() { return fileSpec_; }
		string fileSpec() const { return fileSpec_; }
		void fileSpec(const string& fileSpec) { fileSpec_ = fileSpec; }

		string& clientPath() { return clientPath_; }
		string clientPath() const { return clientPath_; }
		void clientPath(const string& clientPath) { clientPath_ = clientPath; }
		
		bool checkCyclicDependancy(string Key);
		bool storeFile();

	private:
		DbElement<T> metadata_;
		string fileSpec_;
		string clientPath_;
		RepoCore<T>* repo_;
		vector<string> folders_;
		string fileKey_;
		string vn_;
		void editMetaData();
		void storeHelper();
		void updateDependant();
	};

	template<typename T>
	bool CheckIn<T>::checkCyclicDependancy(string Key) {
		bool flag = true;
		if (find(repo_->database()[Key].children().begin(), repo_->database()[Key].children().end(), fileKey_) != repo_->database()[Key].children().end())
			return false;
		for (auto child : repo_->database()[Key].children())
			flag = checkCyclicDependancy(child);
		return flag;
	}

	//----< call functions for versioning, storing and copying of a given file >----------------------------------
	template<typename T>
	bool CheckIn<T>::storeFile() {
		editMetaData();
		bool flag = true;
		
		if (repo_->statusMap().find(fileSpec_) != repo_->statusMap().end()) {
			if (repo_->statusMap()[fileSpec_] == false) {
				fileKey_ = fileSpec_ + "." + to_string(repo_->versionMap()[fileSpec_]);
				for (auto child : metadata_.children())
					flag = checkCyclicDependancy(child);
				if (flag == false)
					return false;
			}
		}
		Version<T> VS(repo_);
		VS.filespec(fileSpec_);
		VS.CheckStatus();
		vn_ = to_string(VS.AssignVersion());
		storeHelper();
		repo_->database()[fileSpec_ + "." + vn_] = metadata_;
		updateDependant();
		VS.setStatus(metadata_, fileSpec_ + "." + vn_);
		repo_->database()[fileSpec_ + "." + vn_] = metadata_;
		return true;
	}

	//----< edit metadata attributes using a given filespec by user >----------------------------------
	template<typename T>
	void CheckIn<T>::editMetaData() {
		string delimiter = "::";
		size_t s_pos = 0;
		size_t pos = 0;
		std::string token;
		while ((pos = fileSpec_.find(delimiter,s_pos)) != string::npos) {
			token = fileSpec_.substr(s_pos, pos-s_pos);
			folders_.push_back(token);
			s_pos = pos + delimiter.length();
		}
		token = fileSpec_.substr(s_pos,fileSpec_.length());
		metadata_.name() = token;
		unordered_set<string> s(folders_.begin(), folders_.end());
		folders_.assign(s.begin(), s.end());
		fileSpec_.clear();
		for (auto folder : folders_) {
			fileSpec_ = fileSpec_+folder + "::";
		}
		fileSpec_ = fileSpec_ + token;
	}

	//----< update the dependants map for a given file >----------------------------------
	template<typename T>
	void CheckIn<T>::updateDependant() {
		if (metadata_.status() == "Close") {
			for (auto child : metadata_.children()) {
				repo_->dependantsMap()[child].push_back(fileSpec_ + "." + vn_);
			}
		}
	}

	//----< helper function to copy file to the repository storage >----------------------------------
	template<typename T>
	void CheckIn<T>::storeHelper() {
		Directory DI;
		string path = "../Storage";
		bool dirPresent = DI.exists(path);
		if (!dirPresent) 
			DI.create(path);
		dirPresent = false;
		while(folders_.size()!=0){
			for (auto folder : folders_) {
				dirPresent = DI.exists(path + "/" + folder);
				if (dirPresent) {
					path = path + "/" + folder;
					while(find(folders_.begin(), folders_.end(), folder)!=folders_.end())
					folders_.erase(find(folders_.begin(), folders_.end(), folder));
					break;
				}
			}
			if (!dirPresent)
				break;
		}
		for (auto folder : folders_) {
			DI.create(path + "/" + folder);
			path = path + "/" + folder;
		}
		folders_.clear();
		string scrPath = path + "/" + metadata_.name() + "." + vn_;
		metadata_.payLoad().editPayload(scrPath);
		FileSystem::File F(metadata_.name() + "." + vn_);
		F.copy(clientPath_, scrPath);
	}
}