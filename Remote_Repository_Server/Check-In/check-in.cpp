///////////////////////////////////////////////////////////////////////////
// check-in.cpp - Implements file checkin functionality for the database //
// ver 1.0																 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018		 //
///////////////////////////////////////////////////////////////////////////

#include "check-in.h"
#include<iostream>
#include "../Payload/PayLoad.h"

using namespace checkIn;

//----< test stub >----------------------------------------------------
#ifdef TEST_CHECKIN

int main() {
	std::cout << "\n Checking In file 'NoSqlDb::dbcore.h' with close status : ";
	RepoCore<Payload> R1;
	CheckIn<Payload> C1(R1);
	C1.fileSpec("NoSqlDb::dbcore.h");
	C1.clientPath("../DBCore/dbcore.h");
	DbElement<Payload> demoElem;
	demoElem.dateTime(DateTime().now());
	demoElem.descrip("Heart of Database");
	demoElem.status("Close");
	Payload P1;
	P1.categories().push_back("static lib");
	P1.categories().push_back("header");
	demoElem.payLoad(P1);
	C1.metadata(demoElem);
	C1.storeFile();
	showDb(R1.database());
	getchar();
	return 0;
}
#endif