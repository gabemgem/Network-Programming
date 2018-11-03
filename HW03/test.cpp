#include <string>
#include <cstring>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
	vector<int> ar = {1,2,3,4,5,6,7,8,9,10};
	vector<int>::iterator i = ar.begin();
	vector<int> toDelete = {1, 5, 7};

	advance(i, toDelete[0]);
	cout<<"Pos after "<<toDelete[0]<<": "<<*i<<"\n";
	i = ar.erase(i);
	advance(i, toDelete[1]-toDelete[0]);
	cout<<"Pos after "<<toDelete[1]<<": "<<*i<<"\n";
	i = ar.erase(i);
	advance(i, toDelete[2]-toDelete[1]);
	cout<<"Pos after "<<toDelete[2]<<": "<<*i<<"\n";
	i = ar.erase(i);

	for(int a = 0; a<ar.size(); ++a) {
		cout<<ar[a]<<", ";
	}
	cout<<"\n";

	/*
	string temp(argv[1], strlen(argv[1]));
	int dash = temp.find_first_of('-');
	string comm = temp.substr(0, dash);
	string comm2 = temp.substr(dash+1);
	cout<<comm<<"\n"<<comm2<<"\n";
	
	/*
	char* temp = strtok(argv[1], "-");
	while(temp!=NULL) {
		cout<<"1: "<<temp<<"\n";
		temp = strtok(NULL, "-");
	}
	*/

	return 0;
}