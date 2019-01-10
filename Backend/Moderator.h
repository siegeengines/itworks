#ifndef MODERATOR_H
#define MODERATOR_H
#include <iostream>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "User.h"
#include "Post.h"
using namespace std;
using namespace pqxx;
class Moderator : public User{
    public:
	Moderator();
	~Moderator();
	void getFlagPosts(vector <Post> &, connection &);
	string action(string, string,connection &);
};
#endif
