#pragma once
///////////////////////////////////////////////////////////////////////
// query.h - Implements different quering functions for the database //
// ver 1.0															 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	 //
///////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides two classes:
* - Condition class provides mechanism to set different parameters to 
*   query the database. An object of this class is passed to Query class
*   to query these parameters.
* - Query class provides a function to query a Condition class object as
*   well individual functions to query a single parameter. It also provides
*	functions to AND/OR multiple queries.
*   The class also provides functions for displaying:
* - set of all keys passed by a query
* - database elements passed by a query
*
* Public Interface:
* ------------------
* Condition Class
* - std::string& name();
* - std::string name() const;
* - void name(const std::string& name);

* - std::string& descrip();
* - std::string descrip() const;
* - void descrip(const std::string& name);

* - DateTime& startTime() ;
* - DateTime startTime() const ;
* - void startTime(const DateTime& startTime) ;

* - DateTime& endTime() ;
* - DateTime endTime() const ;
* - void endTime(const DateTime& endTime) ;

* - Children& children() ;
* - Children children() const ;
* - void children(const Children& children) ;

* - Key& key() ;
* - Key key() const ;
* - void key(const Key& key) ;

* - std::string& payload() ;
* - std::string payload() const ;
* - void payload(const std::string& payload) ;
*
* Query Class
* - DbCore<T>& db() { return db_; }
* - DbCore<T> db() const { return db_; }
* - void db(const DbCore<T>& db) { db_ = db; }
*
* - Query(const DbCore<T>& db) : db_(db) {}
* - Query<T> select(const Condition& cond);
* - void selectName(std::string name);
* - void selectDescrip(std::string descrip);
* - void selectDateTime_1(DateTime start_time);
* - void selectDateTime_2(DateTime start_time, DateTime end_time );
* - void selectPayload(std::string category);
* - void selectKey(std::string key);
* - void valueOfKey(std::string key);
* - void childrenOfKey(std::string key);
* - Query<T> UnionWith(Query<T> Q);
* - Query<T> CommonWith(Query<T> Q);
* - void keys();
*
* Required Files:
* ---------------
* query.h
* DbCore.h
* DateTime.h, DateTime.cpp
*
* Maintenance History:
* --------------------
* - first release
*/

#include <regex>
#include <iostream>
#include <iomanip>
#include <functional>
#include"../DBCore/dbcore.h"
#include "../DateTime/DateTime.h"

using namespace NoSqlDb;

/////////////////////////////////////////////////////////////////////
// Condition class
// - provides parameters for quering a database

class Condition
{
public:
	using Key = std::string;
	//using Children = std::vector<Key>;

	std::string& name() { return name_; }
	std::string name() const { return name_; }
	void name(const std::string& name) { name_ = name; }

	std::string& descrip() { return descrip_; }
	std::string descrip() const { return descrip_; }
	void descrip(const std::string& name) { descrip_ = name; }

	DateTime& startTime() { return startTime_; }
	DateTime startTime() const { return startTime_; }
	void startTime(const DateTime& startTime) { startTime_ = startTime; }

	DateTime& endTime() { return endTime_; }
	DateTime endTime() const { return endTime_; }
	void endTime(const DateTime& endTime) { endTime_ = endTime; }

	std::string& children() { return children_; }
	std::string children() const { return children_; }
	void children(const std::string& children) { children_ = children; }

	Key& key() { return key_; }
	Key key() const { return key_; }
	void key(const Key& key) { key_ = key; }

	std::string& payload() { return payload_; }
	std::string payload() const { return payload_; }
	void payload(const std::string& payload) { payload_ = payload; }

private:
	std::string name_;
	std::string descrip_;
	DateTime startTime_;
	DateTime endTime_;
	Key key_;
	std::string children_;
	std::string payload_;
};

/////////////////////////////////////////////////////////////////////
// Query class
// - provides functionality for different type of queries on database

template <typename T>
class Query
{
public:
	using Key = std::string;
	using Keys = std::vector<Key>;
	using Children = Keys;
	using DbStore = std::unordered_map<Key, DbElement<T>>;

	DbCore<T>& db() { return db_; }
	DbCore<T> db() const { return db_; }
	void db(const DbCore<T>& db) { db_ = db; }

	Query(const DbCore<T>& db) : db_(db) {}
	Query<T> select(const Condition& cond);
	void selectName(std::string name);
	void selectDescrip(std::string descrip);
	void selectDateTime_1(DateTime start_time);
	void selectDateTime_2(DateTime start_time, DateTime end_time );
	void selectPayload(std::string category);
	void selectKey(std::string key);
	void selectKeyInChildren(std::string key);
	void valueOfKey(std::string key);
	void childrenOfKey(std::string key);
	Query<T> UnionWith(Query<T> Q);
	Query<T> CommonWith(Query<T> Q);
	void keys();

private:
	DbCore<T> db_;
		Keys keys_;
		bool newQuery = true;
};

//----< store keys in a vector from the database >----------------------------------
template<typename T>
void Query<T>::keys()
{
	Query<T>::Keys dbKeys;
	DbStore& dbs = db_.dbStore();
	size_t size = dbs.size();
	dbKeys.reserve(size);
	for (auto item : dbs)
	{
		keys_.push_back(item.first);
	}
}

//----< Display the value of given key >----------------------------------
template<typename T>
void Query<T>::valueOfKey(std:: string key) {
	std::cout << "\n\n  The given query returned the following results:\n";
	showHeader();
	showElem(db_[key]);
}

//----< Display the children of given key >----------------------------------
template<typename T>
void Query<T>::childrenOfKey(std::string key) {
	typename DbElement<T>::Children children = db_[key].children();
	std::cout << "\n\n  The given query returned the following results:\n";
	if (children.size() > 0)
	{
		std::cout << "\n    child keys: ";
		for (auto key : children)
		{
			std::cout << " " << key;
		}
	}
}

//----< Display the set of keys from a specified time-date >----------------------------------
template<typename T>
void Query<T>::selectDateTime_1(DateTime start_time) {
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		if (start_time > elem.second.dateTime()) {
			db_.removeElement(elem.first);
		}
	}
}

//----< Display the set of keys between a specified time-date >----------------------------------
template<typename T>
void Query<T>::selectDateTime_2(DateTime start_time, DateTime end_time) {
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		if (start_time > elem.second.dateTime() || end_time < elem.second.dateTime()) {
			db_.removeElement(elem.first);
		}
	}
}

//----< Search the database for names matching a given regex >----------------------------------
template<typename T>
void Query<T>::selectName(std::string name) {
	std::regex n(name);
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		bool match = regex_search(elem.second.name(), n);
		if (!match) {
			db_.removeElement(elem.first);
		}
	}
}

//----< Search the database for description matching a given regex >----------------------------------
template<typename T>
void Query<T>::selectDescrip(std::string descrip) {
	std::regex n(descrip);
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		bool match = regex_search(elem.second.descrip(), n);
		if (!match) {
			db_.removeElement(elem.first);
		}
	}
}

//----< Search the database for payload matching a given regex >----------------------------------
template<typename T>
void Query<T>::selectPayload(std::string category) {
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		bool match = elem.second.payLoad().match(category);
		if (!match) {
			db_.removeElement(elem.first);
		}
	}
}



//----< Search the database for keys matching a given regex >----------------------------------
template<typename T>
void Query<T>::selectKey(std::string key) {
	std::regex n(key);
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		bool match = regex_search(elem.first, n);
		if (!match) {
			db_.removeElement(elem.first);
		}
	}
}

//----< Search the database for children matching a given regex >----------------------------------
template<typename T>
void Query<T>::selectKeyInChildren(std::string key) {
	std::regex n(key);
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		bool match = false;
		for (auto child : elem.second.children())
				match = regex_search(child, n);
		if (!match) {
			db_.removeElement(elem.first);
		}
	}
}

//----< Call different search functions to query a given condition >----------------------------------
template<typename T>
Query<T> Query<T>::select(const Condition& cond)
{
	if (newQuery == true)
		keys();
	if (cond.name() != "") {
		selectName(cond.name());
	}
	if (cond.descrip() != "") {
		selectDescrip(cond.descrip());
	}
	if (cond.key() != "") {
		selectKey(cond.key());
	}
	if (cond.children() != "") {
		selectKeyInChildren(cond.children());
	}
	if (cond.startTime() < DateTime().now() && cond.endTime()<DateTime().now()) {
		selectDateTime_2(cond.startTime(), cond.endTime());
	}
	else if (cond.startTime() < DateTime().now()) {
		selectDateTime_1(cond.startTime());
	}
	if (cond.payload() != "") {
		selectPayload(cond.payload());
	}
	newQuery = false;
	return *this;
}

//----< AND two queries results to a single query >----------------------------------
template<typename T>
Query<T> Query<T>::CommonWith(Query<T> Q) {
	DbCore<T> temp_Db = db_;
	for (auto elem : temp_Db.dbStore()) {
		if (!Q.db().contains(elem.first)) {
			db_.removeElement(elem.first);
		}
	}
	return *this;
}

//----< OR two queries results to a single query >----------------------------------
template<typename T>
Query<T> Query<T>::UnionWith(Query<T> Q) {
	DbCore<T> temp_Db = Q.db();
	for (auto elem : temp_Db.dbStore()) {
		if (!db_.contains(elem.first)) {
			db_[elem.first] = elem.second;
		}
	}
	return *this;
}
