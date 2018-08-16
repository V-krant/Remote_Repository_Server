/////////////////////////////////////////////////////////////////////
// DbCore.h - Implements NoSql database			                   //
// ver 1.0                                                         //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018 //
/////////////////////////////////////////////////////////////////////

#include "DbCore.h"
#include "../Payload/PayLoad.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include "../Utilities/TestUtilities/TestUtilities.h"
#include <iostream>
#include <iomanip>
#include <functional>

using namespace NoSqlDb;

//----< reduce the number of characters to type >----------------------

auto putLine = [](size_t n = 1, std::ostream& out = std::cout)
{
	Utilities::putline(n, out);
};

///////////////////////////////////////////////////////////////////////
// DbProvider class
// - provides mechanism to share a test database between test functions
//   without explicitly passing as a function argument.

class DbProvider
{
public:
	DbCore<Payload>& db() { return db_; }
private:
	static DbCore<Payload> db_;
};

DbCore<Payload> DbProvider::db_;

///////////////////////////////////////////////////////////////////////
// test functions

//----< Creating a DbElment in database >----------------------------------
bool testDbElem()
{
	std::cout << "\n  Creating a db element with key \"Fawcett\":";

	// create database to hold std::string payload

	DbCore<Payload> db;
	DbProvider dbp;
	dbp.db() = db;

	// create some demo elements and insert into db

	DbElement<Payload> demoElem;

	demoElem.name("Jim");
	demoElem.descrip("Instructor for CSE687");
	demoElem.dateTime(DateTime().makeTime(2005, 5, 20, 5, 4, 20));
	Payload P1;
	P1.filepath("../Folder1");
	P1.categories().push_back("Media");
	P1.categories().push_back("Video");
	demoElem.payLoad(P1);

	if (demoElem.name() != "Jim")
		return false;
	if (demoElem.descrip() != "Instructor for CSE687")
		return false;

	showHeader();
	showElem(demoElem);

	db["Fawcett"] = demoElem;
	dbp.db() = db;
	putLine();
	return true;
}

//----< helper function to add element in database >----------------------------------
void addElements(DbCore<Payload>& db) {
	DbElement<Payload> demoElem;
	demoElem.name("Ammar");
	demoElem.dateTime(DateTime().makeTime(2010, 5, 15, 15, 45, 30));
	demoElem.descrip("TA for CSE687");
	Payload P1;
	P1.filepath("../Folder2");
	P1.categories().push_back("Media");
	P1.categories().push_back("Music");
	demoElem.payLoad(P1);
	db["Salman"] = demoElem;

	demoElem.name("Jianan");
	demoElem.dateTime(DateTime().makeTime(2017, 5, 15, 15, 45, 30));
	Payload P2;
	P2.filepath("../Folder3");
	P2.categories().push_back("Document");
	P2.categories().push_back("Report");
	P2.categories().push_back("Pdf");
	demoElem.payLoad(P2);
	db["Sun"] = demoElem;

	demoElem.name("Nikhil");
	demoElem.dateTime(DateTime().makeTime(2015, 5, 15, 15, 45, 30));
	Payload P3;
	P3.filepath("../Folder4");
	P3.categories().push_back("Document");
	P3.categories().push_back("Presentation");
	demoElem.payLoad(P3);
	db["Prashar"] = demoElem;

	demoElem.name("Pranjul");
	demoElem.dateTime(DateTime().makeTime(2007, 3, 15, 15, 45, 30));
	Payload P4;
	P4.filepath("../Folder5");
	P4.categories().push_back("Media");
	P4.categories().push_back("Image");
	P4.categories().push_back("JPEG");
	demoElem.payLoad(P4);
	db["Arora"] = demoElem;

	demoElem.name("Akash");
	demoElem.dateTime(DateTime().makeTime(2013, 3, 15, 15, 45, 30));
	Payload P5;
	P5.filepath("../Folder6");
	P5.categories().push_back("Document");
	demoElem.payLoad(P5);
	db["Anjanappa"] = demoElem;
}

//----< helper function to add children to a DbElement >----------------------------------
void addChildren(DbCore<Payload>& db) {
	db["Fawcett"].children().push_back("Salman");
	db["Fawcett"].children().push_back("Sun");
	db["Fawcett"].children().push_back("Prashar");
	db["Fawcett"].children().push_back("Arora");
	db["Fawcett"].children().push_back("Anjanappa");

	showHeader();
	showElem(db["Fawcett"]);

	db["Salman"].children().push_back("Sun");
	db["Salman"].children().push_back("Prashar");
	db["Salman"].children().push_back("Arora");
	db["Salman"].children().push_back("Anjanappa");

	// display the results

	putLine();
	std::cout << "\n  showing all the database elements:";
	showDb(db);
	putLine();

	std::cout << "\n  database keys are: ";
	showKeys(db);
	putLine();
}

//----< Create a database by adding DbElments >----------------------------------
bool testDbCore()
{
	DbProvider dbp;
	DbCore<Payload> db = dbp.db();
	addElements(db);


	std::cout << "\n  after adding elements with keys: ";
	DbCore<Payload>::Keys keys = db.keys();
	showKeys(db);
	putLine();

	std::cout << "\n  make all the new elements children of element with key \"Fawcett\"";
	addChildren(db);
	dbp.db() = db;
	return true;
}
//----< test stub >----------------------------------------------------

#ifdef TEST_DBCORE

using namespace Utilities;

int main()
{
	Utilities::Title("Testing DbCore - He said, she said database");
	putLine();

	TestExecutive ex;

	// define test structures with test function and message

	TestExecutive::TestStr ts1{ testDbElem, "Create DbElement<std::string>" };
	TestExecutive::TestStr ts2{ testDbCore, "Create DbCore<std::string>" };


	// register test structures with TestExecutive instance, ex

	ex.registerTest(ts1);
	ex.registerTest(ts2);

	// run tests

	bool result = ex.doTests();
	if (result == true)
		std::cout << "\n  all tests passed";
	else
		std::cout << "\n  at least one test failed";

	putLine(2);
	getchar();
	return 0;
}
#endif
