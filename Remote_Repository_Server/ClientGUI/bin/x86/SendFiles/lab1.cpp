
#include <iostream>
using namespace std;

int generateRandomNo(){
return rand()%1000;
}

int main(){
int data[1000];
	for(int i=0;i<1000;i++){
	data[i]=generateRandomNo();
		cout<<data[i]<<"  ";
	}
	return 0;
}