#ifndef PAYLOAD_H
#define PAYLOAD_H
///////////////////////////////////////////////////////////////////////
// PayLoad.h - application defined payload                           //
// ver 1.0                                                           //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018   //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  This package provides a single class, PayLoad:
*  - holds a payload string and vector of categories
*  - provides means to set and access those values
*  - provides methods used by different classes to store restore payload
*  - to an xml file and modify payload values
*  - provides a show function to display PayLoad specific information
*  - PayLoad processing is very simple, so this package contains only
*    a header file, making it easy to use in other packages, e.g.,
*    just include the PayLoad.h header.

* - Payload provides a struct containing a string and a vector of
*   strings and is used to set the payLoad of DbElement.
*
* Public Interface:
* -----------------
* - string filepath() ;
* - void filepath(const string& filePath)  ;
*
* - vector<string>& categories() ;
* - vector<string> categories() const ;
* - void categories(const vector<string>& categories) ;
*
* - void editPayload(string path);
* - void storePayload(Sptr pPayload, Payload P);
* - static Payload restorePayload(Sptr pPayload);
* - bool match(string category);
*
*  Required Files:
*  ---------------
*    PayLoad.h, PayLoad.cpp - application defined package
*    DbCore.h, DbCore.cpp
*    XmlDocument.h
*
*  Maintenance History:
*  --------------------
*  - first release
*
*/

#include <string>
#include <vector>
#include <iostream>
#include<regex>

#include "../XmlDocument/XmlDocument.h"
#include "../DbCore/DbCore.h"
#include "IPayLoad.h"


///////////////////////////////////////////////////////////////////////
// PayLoad class provides:
// - a std::string value which, in Project #2, will hold a file path
// - a vector of string categories, which for Project #2, will be 
//   Repository categories

namespace NoSqlDb
{
	using namespace std;
	class Payload : public IPayLoad<Payload>
	{
	public:
		Payload() = default;
		using Xml = string;
		using Sptr = shared_ptr<XmlProcessing::AbstractXmlElement>;

		string filepath() { return filePath_; }
		void filepath(const string& filePath) { filePath_ = filePath; }

		vector<string>& categories() { return categories_; }
		vector<string> categories() const { return categories_; }
		void categories(const vector<string>& categories) { categories_ = categories; }

		void editPayload(string path);
		void storePayload(Sptr pPayload, Payload P);
		static Payload restorePayload(Sptr pPayload);
		bool match(string category);

	private:
		string filePath_;
		vector<string> categories_;
		friend ostream &  operator<<(ostream & stream, Payload P);
	};

	//----< overload output operator to display payload >---------------------------------------------
	ostream &  operator<<(ostream & stream, Payload P) {
		stream << "FilePath: " << P.filepath();
		if (P.categories().size() > 0)
		{
			stream << " Categories:";
			for (auto cat : P.categories())
			{
				stream << " " << cat;
			}
		}
		return stream;
	}

	//----< to edit the filepath in payload >---------------------------------------------
	void Payload::editPayload(string path) {
		filePath_ = path;
	}

	//----< to persist payload to an xml file >---------------------------------------------
	void Payload::storePayload(Sptr pPayload, Payload P) {
		Sptr pPayloadChild = makeTaggedElement("filepath", P.filepath());
		pPayload->addChild(pPayloadChild);
		for (string child : P.categories()) {
			Sptr pPChild = makeTaggedElement("category", child);
			pPayload->addChild(pPChild);
		}
	}

	//----< Restore payload values to an object of Payload class >---------------------------------------------
	Payload Payload::restorePayload(Sptr pPayload) {
		Payload P;
		vector<Sptr> pChildValues = pPayload->children();
		for (auto pchildrenValue : pChildValues) {
			if (pchildrenValue->tag() == "filepath" && pchildrenValue->children().size()>0)
				P.filepath(pchildrenValue->children()[0]->value());
			if (pchildrenValue->tag() == "category" && pchildrenValue->children().size()>0)
			{
				P.categories().push_back(pchildrenValue->children()[0]->value());
			}
		}
		return P;
	}

	//----< Search the payload categories matching a given category >----------------------------------
	bool Payload::match(string category) {
		regex c(category);
		for (string cat : categories_) {
			if (regex_search(cat, c))
				return true;
		}
		return false;
	}
}
#endif
