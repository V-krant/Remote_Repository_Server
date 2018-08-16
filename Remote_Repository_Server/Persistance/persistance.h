#pragma once
///////////////////////////////////////////////////////////////////////////////
// persistance.h - Provides storing and restoring of database to an XML file //
// ver 1.1																	 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018			 //
///////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides a Persistance class which provides functions to
* - Store database to an XML file
* - Restore database from an XML file
* - Merge an existing database with the one provided by an XML file
*
* Public Interface:
* -----------------
* - void xmlStore(RepoCore<T>& RC, string filespec1, string filespec2);
* - RepoCore<T> xmlRestore(string filespec1, string filespec2);
* - void xmlMerge(RepoCore<T>& RC, string filespec1, string filespec2);
* - void storeDatabase(DbCore<T> db, string filespec);
* - DbCore<T> restoreDatabase(string filespec);
*
* Required Files:
* ---------------
* persistance.h
* DbCore.h
* XmlDocument.h, XmlDocument.cpp
* repocore.h
*
* Maintenance History:
* --------------------
* ver 1.1
* - Added functionality to store and restore all the repository maps
*
* - first release
*/

#include <iostream>
#include <iomanip>
#include <functional>
#include <memory>
#include <fstream>
#include "../DBCore/dbcore.h"
#include "../XmlDocument/XmlDocument.h"
#include "../Payload/IPayLoad.h"
#include "../RepoCore/repocore.h"

using namespace std;
using namespace NoSqlDb;
using namespace XmlProcessing;
using namespace Repository;

/////////////////////////////////////////////////////////////////////
// Persistance class
// - provides mechanism to store and restore database from an XML file

template<typename T>
class Persistance {
public:
	void xmlStore(RepoCore<T>& RC, string filespec1, string filespec2);
	RepoCore<T> xmlRestore(string filespec1, string filespec2);
	void xmlMerge(RepoCore<T>& RC, string filespec1, string filespec2);
	void storeDatabase(DbCore<T> db, string filespec);
	DbCore<T> restoreDatabase(string filespec);
private:
	using Sptr = shared_ptr<AbstractXmlElement>;
	string fileSpecToString(string path);
	DbElement<T> restoreHelper(vector<Sptr> pValueChildren, DbElement<T> elem);
	void restoreDependants(XmlDocument& newXDoc, RepoCore<T>& newRepo);
};

//----< Store database and all other maps to an XML file >----------------------------------
template<typename T>
void Persistance<T>::xmlStore(RepoCore<T>& RC, string filespec1,string filespec2) {
	Sptr sMap = makeTaggedElement("Maps");
	Sptr sDocElem = makeDocElement(sMap);
	Sptr sDb = makeTaggedElement("StatusMap");
	sMap->addChild(sDb);
	XmlDocument xDoc(sDocElem);
	for (auto item : RC.statusMap()){					// Store Status Map to an xml file
		Sptr sRecord = makeTaggedElement("statusRecord");
		sDb->addChild(sRecord);
		Sptr sKey = makeTaggedElement("fileName", item.first);
		sRecord->addChild(sKey);
		if (item.second == true) {
			Sptr sValue = makeTaggedElement("status", "true");
			sRecord->addChild(sValue);}
		else {
			Sptr sValue = makeTaggedElement("status", "false");
			sRecord->addChild(sValue);}
	}
	Sptr vDb = makeTaggedElement("VersionMap");
	sMap->addChild(vDb);
	for (auto item : RC.versionMap()){					// Store Version Map to an xml file
		Sptr vRecord = makeTaggedElement("versionRecord");
		vDb->addChild(vRecord);
		Sptr vKey = makeTaggedElement("fileName", item.first);
		vRecord->addChild(vKey);
		Sptr vValue = makeTaggedElement("version", to_string(item.second));
		vRecord->addChild(vValue);}
	Sptr dDb = makeTaggedElement("DependantsMap");
	sMap->addChild(dDb);
	for (auto item : RC.dependantsMap()){				// Store Dependants Map to an xml file
		Sptr dRecord = makeTaggedElement("dependantRecord");
		dDb->addChild(dRecord);
		Sptr dKey = makeTaggedElement("fileName", item.first);
		dRecord->addChild(dKey);	
		if (item.second.size() > 0) {
			Sptr dValue = makeTaggedElement("Parents");
			dRecord->addChild(dValue);
			for (string parent : item.second) {
				Sptr dChild = makeTaggedElement("Parent", parent);
				dValue->addChild(dChild);}
		}
	}
	string Xml = xDoc.toString();
	//cout << Xml << endl;
	ofstream out(filespec2);
	out << Xml << endl;
	out.close();
	storeDatabase(RC.database(), filespec1);
}

//----< Restore dependants map from an XML file >----------------------------------
template<typename T>
void Persistance<T>::restoreDependants(XmlDocument& newXDoc, RepoCore<T>& newRepo) {
	std::vector<Sptr> records3 = newXDoc.descendents("dependantRecord").select();
	for (auto pRecord : records3)
	{
		string key;
		string version;
		vector<Sptr> pChildren = pRecord->children();
		for (auto pChild : pChildren)
		{
			if (pChild->tag() == "fileName" &&  pChild->children().size() > 0)
				key = pChild->children()[0]->value();
			if (pChild->tag() == "Parents") {
				std::vector<Sptr> pChildValues = pChild->children();
				for (auto pchildrenValue : pChildValues) {
					if (pchildrenValue->tag() == "Parent" &&  pchildrenValue->children().size() > 0)
						newRepo.dependantsMap()[key].push_back(pchildrenValue->children()[0]->value());
				}
			}
		}
	}
}

//----< Restore a database and all other maps from an XML file >----------------------------------
template<typename T>
RepoCore<T> Persistance<T>::xmlRestore(string filespec1, string filespec2) {
	string newSDoc = fileSpecToString(filespec2);
	RepoCore<T> newRepo;
	DbCore<T> newdb;
	using Key = std::string;
	XmlDocument newXDoc(newSDoc, XmlDocument::str);
	std::vector<Sptr> records1 = newXDoc.descendents("statusRecord").select();
	for (auto pRecord : records1)					// Restore Status Map from an xml file
	{
		Key key;
		string status;
		vector<Sptr> pChildren = pRecord->children();
		for (auto pChild : pChildren)
		{
			if (pChild->tag() == "fileName" && pChild->children().size() > 0)
				key = pChild->children()[0]->value();
			else if(pChild->children().size() > 0)
				status = pChild->children()[0]->value();
		}
		if (status == "true")
			newRepo.statusMap()[key] = true;
		else
			newRepo.statusMap()[key] = false;
	}
	std::vector<Sptr> records2 = newXDoc.descendents("versionRecord").select();
	for (auto pRecord : records2)					// Restore Version Map from an xml file
	{
		Key key;
		string version;
		vector<Sptr> pChildren = pRecord->children();
		for (auto pChild : pChildren)
		{
			if (pChild->tag() == "fileName" && pChild->children().size() > 0)
				key = pChild->children()[0]->value();
			else if(pChild->children().size() > 0)
				version = pChild->children()[0]->value();
		}
		newRepo.versionMap()[key] = stoi(version);
	}
	restoreDependants(newXDoc, newRepo);			// Restore Dependants Map from an xml file
	
	newRepo.database(restoreDatabase(filespec1));
	return newRepo;
}

//----< Restore elements from an XML file to an existing file >----------------------------------
template<typename T>
void Persistance<T>::xmlMerge(RepoCore<T>& RC, string filespec1, string filespec2) {
	RepoCore<T> tempRepo = xmlRestore(filespec1, filespec2);
	vector<string> tempKeys = tempRepo.database().keys();
	for (string tempKey : tempKeys) {
		if (!RC.database().contains(tempKey)) {
			db[tempKey] = tempDb[tempKey];
		}
	}
}

//----< Restore a database to an XML file >----------------------------------
template<typename T>
void Persistance<T>::storeDatabase(DbCore<T> db, string filespec)
{
	Sptr pDb = makeTaggedElement("db");
	Sptr pDocElem = makeDocElement(pDb);
	XmlDocument xDoc(pDocElem);

	for (auto item : db)
	{
		Sptr pRecord = makeTaggedElement("dbRecord");
		pDb->addChild(pRecord);
		Sptr pKey = makeTaggedElement("key", item.first);
		pRecord->addChild(pKey);

		Sptr pValue = makeTaggedElement("value");
		pRecord->addChild(pValue);
		Sptr pDateTime = makeTaggedElement("datetime", item.second.dateTime());
		pValue->addChild(pDateTime);
		Sptr pName = makeTaggedElement("name", item.second.name());
		pValue->addChild(pName);
		Sptr pDescrip = makeTaggedElement("description", item.second.descrip());
		pValue->addChild(pDescrip);
		Sptr pStatus = makeTaggedElement("status", item.second.status());
		pValue->addChild(pStatus);
		Sptr pPayload = makeTaggedElement("payload");
		pValue->addChild(pPayload);
		item.second.payLoad().storePayload(pPayload, item.second.payLoad());
		if (item.second.children().size() > 0) {
			Sptr pChildren = makeTaggedElement("children");
			pValue->addChild(pChildren);
			for (string child : item.second.children()) {
				Sptr pChild = makeTaggedElement("child", child);
				pChildren->addChild(pChild);
			}
		}
	}
	string Xml = xDoc.toString();
	//cout << Xml << endl;
	ofstream out(filespec);
	out << Xml << endl;
	out.close();
}

//----< Restore a database from an XML file >----------------------------------
template<typename T>
DbCore<T> Persistance<T>::restoreDatabase(string filespec)
{
	string newSDoc = fileSpecToString(filespec);
	DbCore<T> newdb;
	using Key = std::string;
	XmlDocument newXDoc(newSDoc, XmlDocument::str);
	std::vector<Sptr> records = newXDoc.descendents("dbRecord").select();
	for (auto pRecord : records)
	{
		Key key;
		DbElement<T> elem;
		vector<Sptr> pChildren = pRecord->children();
		for (auto pChild : pChildren)
		{

			if (pChild->tag() == "key" && pChild->children().size() > 0)
			{
				key = pChild->children()[0]->value();
			}
			else
			{	
				vector<Sptr> pValueChildren = pChild->children();
				elem = restoreHelper(pValueChildren, elem);

			}
		}
		newdb[key] = elem;
	}
	return newdb;
}

//----< Convert XML file to a string >----------------------------------
template<typename T>
string Persistance<T>::fileSpecToString(string path) {
	title("Restoring database from Xml fileSpec: " + path);
	string xmlString;
	try
	{
		XmlDocument newXDoc(path, XmlDocument::file);
		//std::cout << newXDoc.toString();
		xmlString = newXDoc.toString();
	}
	catch (std::exception& ex)
	{
		std::cout << "\n\n  " << ex.what();
	}
	return xmlString;
}

//----< Helper function to restore database >----------------------------------
template<typename T>
DbElement<T> Persistance<T>::restoreHelper(vector<Sptr> pValueChildren, DbElement<T> elem) {
	for (auto pValueChild : pValueChildren)
	{
		if (pValueChild->tag() == "name" && pValueChild->children().size() > 0)
		{
			// get value of TextElement child
			elem.name(pValueChild->children()[0]->value());
		}
		if (pValueChild->tag() == "description" && pValueChild->children().size() > 0)
		{
			// get value of TextElement child
			elem.descrip(pValueChild->children()[0]->value());
		}
		if (pValueChild->tag() == "datetime" && pValueChild->children().size() > 0)
		{
			// get value of TextElement child
			elem.dateTime(pValueChild->children()[0]->value());
		}
		if (pValueChild->tag() == "status" && pValueChild->children().size() > 0)
		{
			// get value of TextElement child
			elem.status(pValueChild->children()[0]->value());
		}
		if (pValueChild->tag() == "payload" && pValueChild->children().size() > 0)
		{
			elem.payLoad()= elem.payLoad().restorePayload(pValueChild);
			// get value of TextElement child
		}
		if (pValueChild->tag() == "children") {
			std::vector<Sptr> pChildValues = pValueChild->children();
			for (auto pchildrenValue : pChildValues) {
					// get value of TextElement child
				if(pchildrenValue->children().size() > 0)
					elem.children().push_back(pchildrenValue->children()[0]->value());
			}
		}
	}
	return elem;
}
