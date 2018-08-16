///////////////////////////////////////////////////////////////////////
// browse.cpp - Implements browsing functionality for the database	 //
// ver 1.0															 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	 //
///////////////////////////////////////////////////////////////////////

#include "browse.h"
#include<iostream>
#include "../Payload/PayLoad.h"
#include "../Persistance/persistance.h"

using namespace Browsing;

//----< test stub >----------------------------------------------------
#ifdef TEST_BROWSE

int main() {
	cout << "\n\n Displaying package descriptions of files with keys matching regex 'repo' : \n";
	RepoCore<Payload> R1;
	Persistance<Payload> P1;
	R1 = P1.xmlRestore("../database.xml", "../StorageInfo.xml");
	Browse<Payload> B1(R1);
	Query<Payload> Q1 = B1.browseFiles("repo");
	cout << "\n\n Displaying full text of file with key 'Repository::repocore.h.1' : \n";
	B1.openFile("Repository::repocore.h.1");
	getchar();
	return 0;
}
#endif