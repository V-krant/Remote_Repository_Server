/////////////////////////////////////////////////////////////////////////
// version.h - Implements versioning of a file						   //
// ver 1.0															   //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	   //
/////////////////////////////////////////////////////////////////////////

#include "version.h"
#include<iostream>
#include "../Payload/PayLoad.h"
#include "../Persistance/persistance.h"

using namespace Versioning;

//----< test stub >----------------------------------------------------
#ifdef TEST_CHECKOUT

int main() {
	RepoCore<Payload> R1;
	Persistance<Payload> P1;
	R1 = P1.xmlRestore("../database.xml", "../StorageInfo.xml");
	std::cout << "\n\n Versioning file 'Repository::repocore.h' :\n ";
	Version<Payload> VS(&R1);
	VS.filespec("Repository::repocore.h");
	VS.CheckStatus();
	string vn = to_string(VS.AssignVersion());
	VS.setStatus(R1.database()["Repository::repocore.h.1"], "Repository::repocore.h." + vn);
	getchar();
	return 0;
}
#endif