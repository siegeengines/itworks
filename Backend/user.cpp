//user
#include <iostream>
#include <string>
#include <pqxx/pqxx>
#include "User.h"
using namespace std;
using namespace pqxx;
User::User(void){cout<<"constructed User"<<endl;}
User::~User(void){cout<<"destructed User"<<endl;}

bool User::usernameExist(string username, connection & DB) {
    bool exists;
    /* Create SQL statement */
    string sql = "select exists(select 1 from public.user where username='"+ username +"')";
	string sql2 = "select exists(select 1 from public.moderator where mid='"+ username +"')";
    /* Create a non-transactional object. */
    nontransaction N(DB);

    /* Execute SQL query */
	
    result R( N.exec( sql ));
	result X(N.exec(sql2));
	if (R[0]["exists"].as(exists)){
		return true;	
	}
	if (X[0]["exists"].as(exists)){
		return true;	
	}
    return false;
}

string User::login(string username, string password, connection & DB) {
    bool exists;
	if (!usernameExist(username,DB)){
	return "302";
}
    string sql = "select exists(select 1 from public.user where username = '" + username + "' and password = '" + password + "')";
    string sql2 = "select exists(select 1 from public.moderator where mid = '" + username + "' and password = '" + password + "')";
	
    /* Create a non-transactional object. */
    nontransaction N(DB);
      
    /* Execute SQL query */
    result R( N.exec( sql ));
    result X(N.exec(sql2));

    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        if (c[0].as(exists))
            return "200";
    }
    for (result::const_iterator c = X.begin(); c != X.end(); ++c) {
        if (c[0].as(exists))
            return "201";
    }
    return "303";
}

string User::deleteUser(string username, connection & DB) {

    // deleter user
	string sql1 = "DELETE FROM public.user WHERE username = '" + username + "'";
    string sql2 = "UPDATE public.post SET postusername = 'anon' WHERE postusername = '" + username + "'";
    string sql3 = "DELETE FROM public.reputation WHERE username = '" + username +"'";
    
    work W(DB);
    W.exec(sql3);
    W.exec(sql2);
    W.exec(sql1);
    W.commit();
    
    return "200";  
}

string User::updatePassword(string username, string password, connection & DB) {
    string sql = "UPDATE public.user SET password = '" + password + "' WHERE username = '" + username+"'";
    
    work W(DB);
    W.exec(sql);
    W.commit();
    
    return "200";
}

string User::createAccount(string username, string fname, string lname, string email, string password ,connection & DB) {
    if (usernameExist(username, DB))
        return "301"; // 202 = username already exists


    //send query to create account im DB
	string sql = "INSERT INTO public.user (username, f_name, l_name, email, password,creationts)  \
                  VALUES ('" + username + "','" + fname + "','" + lname + "','" + email + "','" + password +  "',NULL)";
    /* Create a transactional object. */
    work W(DB);
    /* Execute SQL query */
    W.exec( sql );
	sql = "INSERT INTO public.reputation (username, upvotes, downvotes) VALUES ('" + username +"',0,0)";
	W.exec( sql );
    W.commit();

    
    return "200";
}


void User::profile(string uname,string& fname,string& lname,string& email,string& reputation,connection & DB){
	string sql = "select username, f_name, l_name, email from public.user where username = '" + uname + "'";
	nontransaction N(DB);
	result R( N.exec( sql ));
	fname = R[0]["f_name"].c_str();
	lname = R[0]["l_name"].c_str();
	email = R[0]["email"].c_str();
	sql = "select upvotes, downvotes from public.reputation where username = '" + uname + "'";
	result X( N.exec( sql ));
	reputation = to_string(stoi(X[0]["upvotes"].c_str())+stoi(X[0]["downvotes"].c_str()));
}

