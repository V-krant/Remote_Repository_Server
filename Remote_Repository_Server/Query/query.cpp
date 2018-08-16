///////////////////////////////////////////////////////////////////////
// query.h - Implements different quering functions for the database //
// ver 1.0															 //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	 //
///////////////////////////////////////////////////////////////////////

#include "query.h"
#include "../Persistance/persistance.h"
#include "../Payload/PayLoad.h"

//----< Test individual query functions >----------------------------------
void singleQueryTest(DbCore<Payload> db) {
	std::cout << "\n  Query to display the value of key \"Bhopatrao\":";
	Query <Payload>Q1(db);
	Q1.valueOfKey("Bhopatrao");
	std::cout << "\n  Query to display the children of key \"Fawcett\":";
	Q1.childrenOfKey("Fawcett");
	std::cout << "\n  Query to display set of all keys matching regex \"[a]|[o]\":";
	Condition C1;
	C1.key("[a]|[o]");
	Q1.select(C1);
	showDb(Q1.db());
	std::cout << "\n  Query to display all keys matching regex \"[a]\" for name and \"[TA]\" for description in their metadata section:";
	Condition C2;
	C2.name("[a]");
	C2.descrip("[TA]");
	Query<Payload> Q2(db);
	Q2.select(C2);
	showDb(Q2.db());
	std::cout << "\n  Query to display all keys written within time-date \"00:00:00 1st Jan 2005 \" and \"00:00:00 1st Jan 2015\":";
	Condition C3;
	C3.startTime(DateTime().makeTime(2005, 1, 1, 0, 0, 0));
	C3.endTime(DateTime().makeTime(2015, 1, 1, 0, 0, 0));
	Query<Payload>Q3(db);
	Q3.select(C3);
	showDb(Q3.db());
	std::cout << "\n  Query to display all keys written from time-date \"00:00:00 1st Jan 2010 \":";
	Condition C4;
	C4.startTime(DateTime().makeTime(2010, 1, 1, 0, 0, 0));
	Query<Payload>Q4(db);
	Q4.select(C4);
	showDb(Q4.db());
}

//----< Test multiple query functions >----------------------------------
void multipleQueryTest(DbCore<Payload> db) {
	std::cout << "\n  Initial query for description matching regex \"[TA]\" :";
	Condition C5;
	C5.descrip("[TA]");
	Query<Payload> Q5(db);
	Q5.select(C5);
	showDb(Q5.db());
	std::cout << "\n  Nested query for time-date starting  \"00:00:00 1st Jan 2010\" on previous query :";
	Condition C6;
	C6.startTime(DateTime().makeTime(2010, 1, 1, 0, 0, 0));
	Q5.select(C6);
	showDb(Q5.db());
	std::cout << "\n  A query to display all keys matching regex \"an\" for name :";
	Condition C7;
	C7.name("an");
	Query<Payload> Q7(db);
	Q7.select(C7);
	showDb(Q7.db());
	Q7.UnionWith(Q5);
	std::cout << "\n Union of sets of keys of the previous two queries: ";
	showDb(Q7.db());
	std::cout << "\n  A query on the union to display all keys of category \"Media\" :";
	Condition C8;
	C8.payload("Media");
	Q7.select(C8);
	showDb(Q7.db());
}
//----< test stub >----------------------------------------------------

#ifdef TEST_QUERY
int main()
{
std::cout << "\n  Restoring database from an XML file at location \"../database.xml\" :";
RepoCore<Payload> rc;
Persistance<Payload> P1;
rc = P1.xmlRestore("../database.xml","../RepoInfo.xml");
showDb(rc.database());
singleQueryTest(rc.database());
multipleQueryTest(rc.database());
getchar();
return 0;
}
#endif