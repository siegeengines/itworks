//user
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <pqxx/pqxx>
#include "Post.h"
using namespace std;
using namespace pqxx;
Post::Post(void){cout<<"constructed Post"<<endl;}
Post::~Post(void){cout<<"destructed Post"<<endl;}
Post::Post(string i,string pid,string uname,string ct,string ts,string v){
	content = ct;
    timestamp = ts;
    pUsername = uname;
    id = i;
    parent = pid;
    votes = v;
	cout<<"constructed Post"<<endl;
}
string Post::addPost(string post, string username, string parentID, connection & DB) {
    // first get the postID by querying largest postID and makeing it = largest +1 

    string postid;

	string sql = "SELECT max(postid) from public.post";
	nontransaction N (DB);
	result R(N.exec(sql));
	postid = to_string(stoi(R[0]["max"].c_str())+1);
	N.abort();
    // create timestamp
    if (parentID == "NULL"){
		sql = "INSERT INTO public.post (postid, parentid, postusername, postcontent, \
		postts, poststatus, votes) VALUES ('" + postid + "',NULL,'" + username + "','" +
		post + "',current_timestamp, true, 0)";
    }
	else{
		sql = "INSERT INTO public.post (postid, parentid, postusername, postcontent, \
		postts, poststatus, votes) VALUES ('" + postid + "','" + parentID + "','" + username + "','" +
		post + "',current_timestamp, true, 0)";
	}
    work W(DB);
    W.exec(sql);
    W.commit();  
    return "200";
}

string Post::deletePost(string postID, connection & DB){
    string sql = "DELETE FROM public.post WHERE postid = " + postID;
    work W(DB);
    W.exec(sql);
    W.commit();
    return "200";
}

string Post::flagPost(string postID, connection & DB) {
    string sql = "UPDATE public.post SET poststatus = false where postid = " + postID;
    work W(DB);
    W.exec(sql);
    W.commit();
    return "202";
}

string Post::voteOnPost(string postID,string vote, connection & DB) {
	string sql = "select votes from public.post where postid = "+ postID;
	string sql2 = "select postusername from public.post where postid = " + postID;
	nontransaction N (DB);
	result R(N.exec(sql2));
	string username = R[0]["postusername"].c_str();
	string sql3 = "select upvotes from public.reputation where username = '" + username + "'";
	string sql4 = "select downvotes from public.reputation where username = '" + username + "'";
	result X(N.exec(sql));
	string newvote;
	if (vote == "upvote"){
		result Y(N.exec(sql3));
		newvote = to_string(stoi(Y[0]["upvotes"].c_str())+1);
		sql2 = "UPDATE public.reputation SET upvotes = " + newvote + " where username = '" + username + "'";
		newvote = to_string(stoi(X[0]["votes"].c_str())+1);
	}
	else {
		result Y(N.exec(sql4));
		newvote = to_string(stoi(Y[0]["downvotes"].c_str())-1);
		sql2 = "UPDATE public.reputation SET downvotes = " + newvote + " where username = '" + username + "'";
		newvote = to_string(stoi(X[0]["votes"].c_str())-1);
	}
	N.abort();
	sql = "UPDATE public.post set votes = " + newvote + " where postid = " + postID; 
    work W(DB);
    W.exec(sql);
	W.exec(sql2);
    W.commit();
	return "201";
}

// return which data structure of Posts?
void Post::searchPosts(string keyword, vector <Post> & P,connection & DB) {
	// query posts that are relevant to the keyword
	vector<string> filter={"what","What","Do","do","how","How","When","when","that","this","it","i","I","me","you","a",
		"your","he","she","his","her","they","us","we","them","their","where","Where","for","Can","can","Could","could"};
    stringstream ss(keyword);
    string item;
    vector<string> tokens;
/*
	cout << keyword.substr(keyword.size()-1, keyword.size()) << "got this" << endl;
	if (keyword.substr(keyword.size()-1, keyword.size())== "?"){
		cout<< "? detected, removing"<<endl;
		keyword = keyword.substr(0, keyword.size()-2);
		
	}
*//*
	for(int i=0; i<tokens.size();i++){
		if (tokens[i] == "?"){
			cout << "? detected, removing " << *(tokens.begin()+i) << endl;
			tokens.erase(tokens.begin()+i);
		}
	}*/
	tokens.erase(remove(tokens.begin(), tokens.end(), "?"), tokens.end());
	char delim = ' ';
	while (getline(ss, item, delim)) {
        tokens.push_back(item);
		cout << item;
    }
	cout<<endl<<"end getline"<< endl;
	vector<string> searchword;
	/*
	for (int i = 0; i < tokens.size(); i++){
		for (int j = 0; j < filter.size(); j++){
			if (tokens[i] == filter[j]){
				cout <<"erased "<<tokens[i]<< endl;
				tokens.erase(tokens.begin()+i);
			}
		}
	}
*/
	
	vector<string>::iterator it;
	for (int i=0; i< filter.size();i++){
		it = find(tokens.begin(),tokens.end(),filter[i]);
		if(it!=tokens.end()){
			tokens.erase(it);
		}
	}
	string sql = "SELECT postid, parentid,postusername, postcontent,postts,votes FROM \
			public.post where poststatus = true AND (";
	for (vector<string>::iterator kw = tokens.begin(); kw != tokens.end(); kw++){
		cout<< *kw<<endl;
		sql+="postcontent like '%" + *kw + "%' OR ";
	}
	sql = sql.substr(0, sql.size()-3);
	sql += ")";
	cout<< sql << " what is actually being searched"<<endl;
	nontransaction N(DB);
    result R( N.exec( sql ));
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
		string postts = to_string(c["postts"]);
		postts = postts.substr(0, postts.size()-10);
 		Post post(to_string(c["postid"]),to_string(c["parentid"]),to_string(c["postusername"])
		,to_string(c["postcontent"]),postts,to_string(c["votes"]));
		P.push_back(post);
    }
}

void Post::dataDump(string & i,string & pid,string & uname,string & ct,string & ts,string & v){
	ct = content;
    ts = timestamp;
    uname = pUsername;
    i =id;
    pid = parent;
    v = votes;
}
