///////////////////////////////////////////////////////////////////////////////
// persistance.h - Provides storing and restoring of database to an XML file //
// ver 1.1																	 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018			 //
///////////////////////////////////////////////////////////////////////////////

#include "persistance.h"
#include "../Payload/PayLoad.h"
#include "../RepoCore/repocore.h"

//----< test stub >----------------------------------------------------

#ifdef TEST_PERSISTANCE
int main()
{
	std::cout << "\n  Restoring database from an XML file at location \"../database.xml\" :";
	RepoCore<Payload> rc;
	Persistance<Payload> P1;
	rc = P1.xmlRestore("../database.xml","../StorageInfo.xml");
	showDb(rc.database());
	std::cout << "\n  Restoring and augmenting database from an XML file at location \"../database.xml\" :";
	P1.xmlMerge(rc, "../database.xml", "../StorageInfo.xml");
	showDb(rc.database());
	std::cout << "\n  Storing database to an XML file at location \"../database.xml\" :";
	P1.xmlStore(rc,"../database.xml", "../StorageInfo.xml");
	getchar();
	return 0;
}
#endif
