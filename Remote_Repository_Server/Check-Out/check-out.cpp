///////////////////////////////////////////////////////////////////////////
// check-out.h - Implements file checkout functionality for the database //
// ver 1.0																 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018		 //
///////////////////////////////////////////////////////////////////////////

#include "check-out.h"
#include<iostream>
#include "../Payload/PayLoad.h"
#include "../Persistance/persistance.h"

using namespace checkOut;

//----< test stub >----------------------------------------------------
#ifdef TEST_CHECKOUT

int main() {
	RepoCore<Payload> R1;
	Persistance<Payload> P1;
	R1 = P1.xmlRestore("../database.xml", "../StorageInfo.xml");
	std::cout << "\n\n Checking Out file 'Repository::repocore.h.1' along with its dependencies to path '../Client' :\n ";
	CheckOut<Payload> CO1(R1);
	CO1.clientPath("../Client/");
	CO1.fileSpec("Repository::repocore.h");
	CO1.version(1);
	CO1.getFile();
	getchar();
	return 0;
}
#endif