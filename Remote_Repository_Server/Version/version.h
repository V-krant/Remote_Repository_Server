#pragma once
/////////////////////////////////////////////////////////////////////////
// version.h - Implements versioning of a file						   //
// ver 1.0															   //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	   //
/////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* - This package provides a class Version which maintains the versionMap
* - and the statusMap of the repository. It facilitates versioning and 
* - setting the status of a file
*
* Public Interface:
* -----------------
* - Version(RepoCore<T>* repo);
* - string& filespec() ;
* - string filespec() const ;
* - void filespec(const string& filespec) ;
* - void CheckStatus();
* - int AssignVersion();
* - void setStatus(DbElement<T>& metadata, string key);
*
* Required Files:
* ---------------
* version.h, version.cpp
* repocore.h
*
* Maintenance History:
* --------------------
* - first release
*/


#include<iostream>
#include<string>
#include "../RepoCore/repocore.h"

using namespace std;
using namespace Repository;

namespace Versioning {

	/////////////////////////////////////////////////////////////////////
	// Version class
	// - maintains statusMap and versionMap of repository
	template<typename T>
	class Version {
	public:
		Version(RepoCore<T>* repo) {
			repo_ = repo;
		}

		string& filespec() { return filespec_; }
		string filespec() const { return filespec_; }
		void filespec(const string& filespec) { filespec_ = filespec; }

		bool& ciStatus() { return ciStatus_; }
		bool ciStatus() const { return ciStatus_; }
		void ciStatus(const bool& ciStatus) { ciStatus_ = ciStatus; }

		void CheckStatus();
		int AssignVersion();
		void setStatus(DbElement<T>& metadata, string key);

	private:
		RepoCore<T> *repo_;
		string filespec_;
		bool ciStatus_ = false;
	};

	//----< uses statusMap to check the current status of a file  >----------------------------------
	template<typename T>
	void Version<T>::CheckStatus() {
		unordered_map<string, bool>::const_iterator found = repo_->statusMap().find(filespec_);
		if (found == repo_->statusMap().end()) {
			repo_->statusMap()[filespec_] = false;
			repo_->versionMap()[filespec_] = 1;
		}
		else if(found->second == true){
			ciStatus_ = true;
		}
	}

	

	//----< uses versionMap to assign a version number to file  >----------------------------------
	template<typename T>
	int Version<T>::AssignVersion() {
		if (ciStatus_ == false) {
			return repo_->versionMap()[filespec_];
		}
		else {
			repo_->versionMap()[filespec_]++;
			return repo_->versionMap()[filespec_];
			
		}
	}

	//----< sets the status of the file based on user requirements and file dependencies  >----------------------------------
	template<typename T>
	void Version<T>::setStatus(DbElement<T>& metadata, string key) {
		if(metadata.status()=="Open")
			repo_->statusMap()[filespec_] = false;
		else if(metadata.children().size()>0){
			for (auto child : metadata.children()) {
				size_t pos = child.find_last_of(".");
				int c_ver = stoi(child.substr(pos + 1, child.length()));
				child.erase(pos, child.length());
				if (c_ver < repo_->versionMap()[child]) {
					repo_->statusMap()[filespec_] = true;
				}
				else {
					unordered_map<string, bool>::const_iterator found = repo_->statusMap().find(child);
					if (found == repo_->statusMap().end()) {
						repo_->statusMap()[filespec_] = false;
						metadata.status("Open");
						break;
					}
					else if (found->second == true) {
						repo_->statusMap()[filespec_] = true;
					}
					else {
						repo_->statusMap()[filespec_] = false;
						metadata.status("Open");
						break;
					}
				}
			}
		}
		else
			repo_->statusMap()[filespec_] = true;
		for (auto dependant : repo_->dependantsMap()[key]) {
			string temp = dependant.substr(0, dependant.find_last_of("."));
			filespec_ = temp;
			repo_->database()[dependant].status("Close");
			setStatus(repo_->database()[dependant],dependant);
		}
	}
}
