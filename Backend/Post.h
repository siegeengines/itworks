#ifndef POST_H
#define POST_H
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <pqxx/pqxx>
using namespace std;
using namespace pqxx;
class Post {
    public:
		string addPost(string,string,string,connection&);
		string deletePost(string,connection&);
		string flagPost(string,connection&);
		string voteOnPost(string,string,connection&);
		void searchPosts(string, vector <Post> &,connection &);
		void dataDump(string&,string&,string&,string&,string&,string&);
		Post();
		Post(string,string,string,string,string,string);
		~Post();
    private:
        string content;
        string timestamp;
        string pUsername;
        bool status;
        string id;
        string parent;
        string votes;
};
#endif
