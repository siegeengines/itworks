#ifndef USER_H
#define USER_H
#include <iostream>
#include <string>
#include <pqxx/pqxx>
using namespace std;
using namespace pqxx;
class User {
    public:
		string createAccount(string,string,string,string,string,connection&);
		string login(string,string,connection&);
		string deleteUser(string,connection&);
		string updatePassword(string,string,connection&);
		bool usernameExist(string,connection&);
		bool passwordInvalid(string password);
		bool emailInvalid(string username);
		void profile(string,string&,string&,string&,string&,connection&);
		User();
		~User();
    private:
		string username;
		string fname;
		string lname;
		string email;
		string password;
		int reputation;         
};
#endif
