/////////////////////////////////////////////////////////////////////////
// repocore.h - Core of the repository								   //
// ver 1.0															   //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018	   //
/////////////////////////////////////////////////////////////////////////

#include<iostream>
#include "repocore.h"
#include "../Payload/PayLoad.h"
#include "../Persistance/persistance.h"

using namespace Repository;

//----< test stub >----------------------------------------------------
#ifdef TEST_REPOCORE

int main() {
	RepoCore<Payload> R1;
	Persistance<Payload> P1;
	R1 = P1.xmlRestore("../database.xml", "../StorageInfo.xml");
	showDb(R1.database());
	for (auto elem : R1.dependantsMap()) {
		cout << "\nkey = " << elem.first;
		for (auto d : elem.second) {
			cout << "\nDependant : " << d;
		}
	}
	for (auto elem : R1.versionMap()) {
		cout << "\nkey = " << elem.first;
		cout << " Version : " << elem.second;
	}
	for (auto elem : R1.statusMap()) {
		cout << "\nkey = " << elem.first;
		if(elem.second==true)
			cout << " Status : " << "Close";
		else
			cout << " Status : " << "Open";
	}
}
#endif