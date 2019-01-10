#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
//#include <pthread.h>
#include <string>
#include <iostream>
#include <vector>
#include <pqxx/pqxx>
#include "User.h"
#include "Post.h"
#include "Moderator.h"
#define PORT 8080
using namespace pqxx;
static int new_socket;
vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}
int main(int argc, char const *argv[])
{
    int server_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    string hello = "Hello from server";
//	pthread_t connections[10000];
//	int threadNumber(0);
    connection DB("dbname = itworks user = postgres password = itworks1! \
	hostaddr = 127.0.0.1 port = 5432");
	if (DB.is_open()) {
	cout << "Opened database successfully: " << DB.dbname() << endl;}
	vector <Post> postCollection;
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
        sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	if (listen(server_fd, 3) < 0)
	{        
		perror("listen");
        exit(EXIT_FAILURE);
	}
	cout << "Thread No: " << pthread_self() << endl;
    char test[1024];
    bzero(test, 1025);
    while(true){   
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
            (socklen_t*)&addrlen))<0){
       		perror("accept");
       		exit(EXIT_FAILURE);
		}else{
        	bzero(test, 1025);
        	string msg;
        	valread = read(new_socket, test, 1024);
			cout << valread << endl;
			if (valread){
        		string tester (test);
        		cout << tester << std::endl;
        		vector<string> data = split(test,',');
				if (data[0] == "create"){
					User user;
					msg = user.createAccount(data[1],data[2],data[3],data[4],data[5],DB);
					send(new_socket, msg.c_str(),strlen(msg.c_str()),0);
				}
				else if (data[0] == "login"){
					User user;
					msg = user.login(data[1],data[2],DB);
					send(new_socket, msg.c_str(),strlen(msg.c_str()),0);
				}
				else if (data[0] == "delete"){
					User user;
					msg = user.deleteUser(data[1],DB);
					send (new_socket,msg.c_str(),strlen(msg.c_str()),0); 
					cout<<msg<<endl;
				}
				else if (data[0] == "question"){
					Post post;
					post.searchPosts(data[1],postCollection,DB);
					string content;
					string timestamp;
					string pUsername;
					string id;
					string parentid;
					string votes;
					if (postCollection.empty()){
						msg = "305";
						send(new_socket, msg.c_str(),strlen(msg.c_str()),0);
					}
					else{
						for (vector<Post>::iterator p= postCollection.begin();p!=postCollection.end();p++){
							(*p).dataDump(id,parentid,pUsername,content,timestamp,votes);
							msg += id+ ',' + parentid + ',' + pUsername+ ',' +content+ ',' +timestamp+ ',' 
								+votes+ '\n';
						}
						msg = msg.substr(0, msg.size()-1);
						cout << msg << endl;
						send (new_socket,msg.c_str(),strlen(msg.c_str()),0);
						postCollection.clear();
					}
				}
				else if (data[0]== "solution"){
					Post post;
					msg = post.addPost(data[1],data[2],data[3],DB);
					send(new_socket, msg.c_str(),strlen(msg.c_str()),0);
				}
				else if (data[0]=="p_solution"){
					Post post;
					msg = post.addPost(data[1],data[2],"NULL",DB);
					send (new_socket, msg.c_str(),strlen(msg.c_str()),0);
				}
				else if (data[0]=="vote"){
					Post post;
					if (data[2]!="flag"){
						msg = post.voteOnPost(data[1],data[2],DB);
					}
					else{
						msg = post.flagPost(data[1],DB);
					}
					send (new_socket, msg.c_str(),strlen(msg.c_str()),0);
	
				}
				else if (data[0]=="reputation"){
					User user;
					string fname;
					string lname;
					string email;
					string reputation;
					user.profile(data[1],fname,lname,email,reputation,DB);
					msg = data[1] + "," + fname + "," + lname + "," + email + "," +reputation;
					send (new_socket, msg.c_str(),strlen(msg.c_str()),0);
				}
				else if (data[0]=="moderator"){
					Moderator mod;
					mod.getFlagPosts(postCollection,DB);
					string content;
					string timestamp;
					string pUsername;
					string id;
					string parentid;
					string votes;
					cout << data[0]<<endl;
					if (postCollection.empty()){
						msg = "301";
						send(new_socket, msg.c_str(),strlen(msg.c_str()),0);
					}
					else{
						for (vector<Post>::iterator p= postCollection.begin();p!=postCollection.end();p++){
							(*p).dataDump(id,parentid,pUsername,content,timestamp,votes);
							msg += id+ ',' + parentid + ',' + pUsername+ ',' +content+ ',' +timestamp+ ',' 
								+votes+ '\n';
						}
						msg = msg.substr(0, msg.size()-1);
						cout << msg << endl;
						send (new_socket,msg.c_str(),strlen(msg.c_str()),0);
						postCollection.clear();
					}
				}
				else if (data[0]=="moderate"){
					Moderator mod;
					msg = mod.action(data[1],data[2],DB);
					send (new_socket,msg.c_str(),strlen(msg.c_str()),0);
				}

   			}
		}
	}
    cout << "\nClosing thread and conn" << endl;
    close(new_socket);
	DB.disconnect();
	cout<< "DB disconnected" << endl;
    return 0;
}




