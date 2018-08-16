#pragma once
/////////////////////////////////////////////////////////////////////////
// repocore.h - Core of the repository								   //
// ver 1.0															   //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	   //
/////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* - This package provides a class Repository which maintains the database
* - and other required data structures. It stores a version map, a status map,
* - a database object and a dependencies map.
* 
* Public Interface:
* -----------------
* - DbCore<T>& database();
* - DbCore<T> database() const ;
* - void database(const DbCore<T>& database);
*
* - StatusMap& statusMap() ;
* - StatusMap statusMap() const ;
* - void statusMap(const StatusMap& statusMap) ;
*		
* - VersionMap& versionMap() ;
* - VersionMap versionMap() const ;
* - void versionMap(const VersionMap& versionMap) ;
*
* - DependantsMap& dependantsMap() ;
* - DependantsMap dependantsMap() const ;
* - void dependantsMap(const DependantsMap& dependantsMap;
*
* Required Files:
* ---------------
* repocore.h, repocore.cpp
* dbcore.h
*
* Maintenance History:
* --------------------
* - first release
*/

#include <string>
#include <iostream>
#include "../DBCore/dbcore.h"

using namespace std;
using namespace NoSqlDb;

namespace Repository
{
	//////////////////////////////////////////////////////////////////////////////////////
	// RepoCore class
	// - maintains all the maps and database consisting of all the files in the repository

	template<typename T>
	class RepoCore {
	public:
		using StatusMap = std::unordered_map<string, bool>;
		using VersionMap = std::unordered_map<string, int>;
		using DependantsMap = std::unordered_map<string, vector<string>>;

		DbCore<T>& database() { return database_; }
		DbCore<T> database() const { return database_; }
		void database(const DbCore<T>& database) { database_ = database; }

		StatusMap& statusMap() { return statusMap_; }
		StatusMap statusMap() const { return statusMap_; }
		void statusMap(const StatusMap& statusMap) { statusMap_ = statusMap; }
		
		VersionMap& versionMap() { return versionMap_; }
		VersionMap versionMap() const { return versionMap_; }
		void versionMap(const VersionMap& versionMap) { versionMap_ = versionMap; }

		DependantsMap& dependantsMap() { return dependantsMap_; }
		DependantsMap dependantsMap() const { return dependantsMap_; }
		void dependantsMap(const DependantsMap& dependantsMap) { dependantsMap_ = dependantsMap; }

	private:
		
		 DbCore<T> database_;
		 StatusMap statusMap_;
		 VersionMap versionMap_;
		 DependantsMap dependantsMap_;
	};
}
