#pragma once
/////////////////////////////////////////////////////////////////////
// DbCore.h - Implements NoSql database			                   //
// ver 1.1                                                         //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018 //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides three classes:
* - DbCore implements core NoSql database operations, but does not
*   provide editing, querying, or persisting.
* - DbElement provides the value part of our key-value database.
*   It contains fields for name, description, date, child collection,
*    status and a payload field of the template type.
*
* The package also provides functions for displaying:
* - set of all database keys
* - database elements
* - all records in the database
*
* Public Interface
* -------------------
* - std::string& name();
* - std::string name() const;
* - void name(const std::string& name) ;
*
* - std::string& descrip() ;
* - std::string descrip() const ;
* - void descrip(const std::string& name) ;
*
* - DateTime& dateTime() ;
* - DateTime dateTime() const
* - void dateTime(const DateTime& dateTime) ;
*
* - Children& children() ;
* - Children children() const ;
* - void children(const Children& children) ;
*
* - T& payLoad();
* - T payLoad() const ;
* - void payLoad(const T& payLoad) ;
*		
* - std::string& status() ;
* - std::string status() const ;
* - void status(const std::string& status) ;
*
* - void deleteKey(Key key);
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* Utilities.h, Utilities.cpp
*
* Maintenance History:
* --------------------
* ver 1.1
* - Added a status field in DbElement
*
* - first release
*/

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "../DateTime/DateTime.h"

namespace NoSqlDb
{
	/////////////////////////////////////////////////////////////////////
	// DbElement class
	// - provides the value part of a NoSql key-value database

	template<typename T>
	class DbElement
	{
	public:
		using Key = std::string;
		using Children = std::vector<Key>;

		// methods to get and set DbElement fields

		std::string& name() { return name_; }
		std::string name() const { return name_; }
		void name(const std::string& name) { name_ = name; }

		std::string& descrip() { return descrip_; }
		std::string descrip() const { return descrip_; }
		void descrip(const std::string& name) { descrip_ = name; }

		DateTime& dateTime() { return dateTime_; }
		DateTime dateTime() const { return dateTime_; }
		void dateTime(const DateTime& dateTime) { dateTime_ = dateTime; }

		Children& children() { return children_; }
		Children children() const { return children_; }
		void children(const Children& children) { children_ = children; }

		T& payLoad() { return payLoad_; }
		T payLoad() const { return payLoad_; }
		void payLoad(const T& payLoad) { payLoad_ = payLoad; }
		
		std::string& status() { return status_; }
		std::string status() const { return status_; }
		void status(const std::string& status) { status_ = status; }

		void deleteKey(Key key);

	private:
		std::string name_;
		std::string descrip_;
		DateTime dateTime_;
		Children children_;
		std::string status_ = "Open";
		T payLoad_;
	};

	//----< delete a key from children of DbElement >----------------------------------
	template<typename T>
	void DbElement<T>::deleteKey(Key key) {
		std::vector<std::string>::iterator it1 = find(children_.begin(), children_.end(), key);
		if (it1 != children_.end()) {
			children_.erase(it1);
		}
	}

	/////////////////////////////////////////////////////////////////////
	// DbCore class
	// - provides core NoSql db operations
	// - does not provide editing, querying, or persistance operations

	template <typename T>
	class DbCore
	{
	public:
		using Key = std::string;
		using Keys = std::vector<Key>;
		using Children = Keys;
		using DbStore = std::unordered_map<Key, DbElement<T>>;
		using iterator = typename DbStore::iterator;
		
		// methods to access database elements

		Keys keys();
		bool contains(const Key& key);
		size_t size();
		void throwOnIndexNotFound(bool doThrow) { doThrow_ = doThrow; }
		DbElement<T>& operator[](const Key& key);
		DbElement<T> operator[](const Key& key) const;
		void deleteElement(Key key);
		void removeElement(Key key);
		typename iterator begin() { return dbStore_.begin(); }
		typename iterator end() { return dbStore_.end(); }

		// methods to get and set the private database hash-map storage

		DbStore& dbStore() { return dbStore_; }
		DbStore dbStore() const { return dbStore_; }
		void dbStore(const DbStore& dbStore) { dbStore_ = dbStore; }

	private:
		DbStore dbStore_;
		DbStore Trash_;
		bool doThrow_ = false;
	};

	/////////////////////////////////////////////////////////////////////
	// DbCore<T> methods

	//----< delete a db element >----------------------------------
	template<typename T>
	void DbCore<T>::deleteElement(Key key) {
		Trash_[key] = dbStore_[key];
		dbStore_.erase(key);
		for (auto it = dbStore_.begin(); it != dbStore_.end(); ++it) {
			it->second.deleteKey(key);
		}
		std::cout << std::endl;
	}

	//----< remove a single element from database >----------------------------------
	template<typename T>
	void DbCore<T>::removeElement(Key key) {
		if (contains(key))
			dbStore_.erase(key);
	}

	//----< does db contain this key? >----------------------------------

	template<typename T>
	bool DbCore<T>::contains(const Key& key)
	{
		iterator iter = dbStore_.find(key);
		return iter != dbStore_.end();
	}
	//----< returns current key set for db >-----------------------------

	template<typename T>
	typename DbCore<T>::Keys DbCore<T>::keys()
	{
		DbCore<T>::Keys dbKeys;
		DbStore& dbs = dbStore();
		size_t size = dbs.size();
		dbKeys.reserve(size);
		for (auto item : dbs)
		{
			dbKeys.push_back(item.first);
		}
		return dbKeys;
	}

	//----< return number of db elements >-------------------------------

	template<typename T>
	size_t DbCore<T>::size()
	{
		return dbStore_.size();
	}

	//----< extracts value from db with key >----------------------------
	/*
	*  - indexes non-const db objects
	*  - In order to create a key-value pair in the database like this:
	*      db[newKey] = newDbElement
	*    we need to index with the new key and assign a default element.
	*    That will be replaced by newDbElement when we return from operator[]
	*  - However, if we want to index without creating new elements, we need
	*    to throw an exception if the key does not exist in the database.
	*  - The behavior we get is determined by doThrow_.  If false we create
	*    a new element, if true, we throw. Creating new elements is the default
	*    behavior.
	*/
	template<typename T>
	DbElement<T>& DbCore<T>::operator[](const Key& key)
	{
		if (!contains(key))
		{
			if (doThrow_)
				throw(std::exception("key does not exist in db"));
			else
				return (dbStore_[key] = DbElement<T>());
		}
		return dbStore_[key];
	}

	//----< extracts value from db with key >----------------------------
	/*
	*  - indexes const db objects
	*/
	template<typename T>
	DbElement<T> DbCore<T>::operator[](const Key& key) const
	{
		if (!contains(key))
		{
			throw(std::exception("key does not exist in db"));
		}
		return dbStore_[key];
	}

	/////////////////////////////////////////////////////////////////////
	// display functions

	//----< display database key set >-----------------------------------

	template<typename T>
	void showKeys(DbCore<T>& db, std::ostream& out = std::cout)
	{
		out << "\n  ";
		for (auto key : db.keys())
		{
			out << key << " ";
		}
	}
	//----< show record header items >-----------------------------------

	inline void showHeader(std::ostream& out = std::cout)
	{
		out << "\n  ";
		out << std::setw(30) << std::left << "Key";
		out << std::setw(30) << std::left << "DateTime";
		out << std::setw(18) << std::left << "Name";
		out << std::setw(30) << std::left << "Description";
		out << std::setw(15) << std::left << "Status";
		out << std::setw(40) << std::left << "Payload";
		out << "\n  ";
		out << std::setw(30) << std::left << "------------------------";
		out << std::setw(30) << std::left << "------------------------";
		out << std::setw(18) << std::left << "----------";
		out << std::setw(30) << std::left << "-----------------------";
		out << std::setw(15) << std::left << "--------";
		out << std::setw(40) << std::left << "-----------------------------------------------------";
	}
	//----< display DbElements >-----------------------------------------

	template<typename T>
	void showElem( DbElement<T>& el, std::ostream& out = std::cout)
	{
	
			out << std::setw(30) << std::left << std::string(el.dateTime());
			out << std::setw(18) << std::left << el.name();
			out << std::setw(30) << std::left << el.descrip();
			out << std::setw(15) << std::left << el.status();
			out <<std::left << el.payLoad();
			typename DbElement<T>::Children children = el.children();
			if (children.size() > 0)
			{
				out << "\n    child keys: ";
				for (auto key : children)
				{
					out << " " << key;
				}
		}
	}
	//----< display all records in database >----------------------------

	template<typename T>
	void showDb( DbCore<T>& db, std::ostream& out = std::cout)
	{
		showHeader(out);
		typename DbCore<T>::DbStore dbs = db.dbStore();
		for (auto item : dbs)
		{
			out << "\n  ";
			out << std::setw(30) << std::left << item.first;
			showElem(item.second, out);
		}
	}
}
