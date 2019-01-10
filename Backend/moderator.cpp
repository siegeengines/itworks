#include <iostream>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "User.h"
#include "Post.h"
#include "Moderator.h"
using namespace std;
using namespace pqxx;
Moderator::Moderator(void){cout<<"constructed Moderator"<<endl;}
Moderator::~Moderator(void){cout<<"destructed Moderator"<<endl;}

void Moderator::getFlagPosts(vector <Post> & P,connection& DB){
	string sql = "select postid, parentid,postusername,postcontent,postts,votes from public.post where poststatus = false";
	nontransaction N (DB);

    result R( N.exec( sql ));
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
		Post post(to_string(c["postid"]),to_string(c["parentid"]),to_string(c["postusername"])
		,to_string(c["postcontent"]),to_string(c["postts"]),to_string(c["votes"]));
		P.push_back(post);
	}
}

string Moderator::action(string postID, string choice, connection& DB){
	if (choice == "unflag"){
		string sql = "UPDATE public.post SET poststatus = true where postid = " + postID;
    		work W(DB);
    		W.exec(sql);
    		W.commit();
	}
	else if(choice == "remove"){
		Post post;
		post.deletePost(postID, DB);
	}
	return "200";
}
