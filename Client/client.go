package main

import (
	"fmt"
	"html/template"
	"net/http"
	"net"
	"time"
	"strings"
	"sort"
	"strconv"
)

// check errors
func check(err error) {
	if err != nil {
		fmt.Println("check err\n")
		fmt.Println(err)
	}
}

// takes a message string and sends it to the server
func tellServer(message string) string {
	// connect to server
	conn, err := net.Dial("tcp", "www.itworks.ga:8080") // cpp server
	defer conn.Close()
	check(err)
	// send message to connection
	fmt.Fprintf(conn, message)
	fmt.Println("send server message: ", message)
	// get response
	response := make([]byte, 1024)
	n, err1 := conn.Read(response)
	check(err1)
	response_message := string(response[:n])
	fmt.Println("server response: ", response_message)
	return response_message
}

// handler for making a new account
func createAccount(w http.ResponseWriter, r *http.Request) {
	userCookie, _ := r.Cookie("SessionID")
	if userCookie != nil {
		http.Redirect(w, r, "/main", 302)
		return
	}
	if r.Method == "GET" {
		t, _ := template.ParseFiles("create_account.html")
		t.Execute(w, nil)
	} else if r.Method == "POST" {
		r.ParseForm()		
		username := r.PostFormValue("username")
		fname := r.PostFormValue("fname")
		lname := r.PostFormValue("lname")
		email := r.PostFormValue("email")
		password := r.PostFormValue("password")
		if username == "" || fname == "" || lname == "" || email == "" || password == "" {
			fmt.Fprintf(w, "One or multiple fields left blank")
		}	
		// compose message to client, of request, username, name, email & password
		client_msg := "create," + username + "," + fname + "," + lname + "," + email + "," + password	
		// send server message
		response := tellServer(client_msg)
		// check servers response
		if response == "200" { // e.g. all OK
			http.Redirect(w, r, "/login", 302)
			return
		} else if response == "301" { 
			fmt.Fprintf(w, "Username already exists")
		}
	}
}

// login handler
func login(w http.ResponseWriter, r *http.Request) {
	userCookie, _ := r.Cookie("SessionID")
	if userCookie != nil {
		http.Redirect(w, r, "/main", 302)
		return
	}
	if r.Method == "GET" {
		t, _ := template.ParseFiles("login.html")
		t.Execute(w, nil)
	} else if r.Method == "POST" {
		r.ParseForm()
		username := r.PostFormValue("username")
		password := r.PostFormValue("password")	

		if username == "" || password == "" {
			fmt.Fprintf(w, "Either username or password field blank")
			http.Redirect(w, r, "/main", 302)
			return
		}		
		client_msg := "login," + username +"," + password
		response := tellServer(client_msg)
		if response == "302" {
			fmt.Fprintf(w, "User does not exist")
		} else if response == "303" {
			fmt.Fprintf(w, "Incorrect password")
		} else if response == "200" {
			cookieValue := r.PostFormValue("username")
			expire := time.Now().Add(1 * time.Hour)
			userCookie := http.Cookie{Name: "SessionID", Value: cookieValue, Expires: expire}
			http.SetCookie(w, &userCookie)
			http.Redirect(w, r, "/main", 302)
		} else if response == "201" { // e.g. user is a moderator
			cookieValue := r.PostFormValue("username")
			expire := time.Now().Add(1 * time.Hour)
			userCookie := http.Cookie{Name: "SessionID", Value: cookieValue, Expires: expire}
			http.SetCookie(w, &userCookie)
			http.Redirect(w, r, "/moderate", 302)
		}
	}
}

func moderate(w http.ResponseWriter, r *http.Request) {
	// send server "moderator"
	// receive string of flagged posts or 301 e.g. no data
	// send server "moderate,postID,unflag/remove"
	// receive "200"
	userCookie, _ := r.Cookie("SessionID")
	if userCookie == nil {
		http.Redirect(w, r, "/login", 302)
		return
	}
	if r.Method == "GET" {
		response := tellServer("moderator")
		if response == "301" {
			t, _ := template.ParseFiles("moderate.html")
			t.Execute(w, nil)
			return
		}
		posts := decodeString(response)
		
		t := template.New("some_name")
		t, _ = template.ParseFiles("moderate.html")
		t.Execute(w, posts)

	} else if r.Method == "POST" {
		r.ParseForm()
		choice := r.Form.Get("choice")
		client_msg := "moderate," + choice
		response := tellServer(client_msg)
		fmt.Println("reponse in post")
		if response == "200" {
			http.Redirect(w, r, "/moderate", 302)
		} else {
			fmt.Fprintf(w, "error")
		}
	}
}

// handler for deleting user account
func deleteAccount(w http.ResponseWriter, r *http.Request) {
	// send server "delete,username"
	// receive 200 (e.g. OK)
	userCookie, _ := r.Cookie("SessionID")
	if userCookie == nil {
		http.Redirect(w, r, "/login", 302)
		return
	}
	if r.Method == "GET" {
		t, _ := template.ParseFiles("delete.html")
		t.Execute(w, nil)
	} else if r.Method == "POST" {
		fmt.Println("in delete function")
		// expire users cookie and remove them from User list, redirect to login page
		username := userCookie.Value
		cookieValue := username
		expire := time.Now()
		userCookie := http.Cookie{Name: "SessionID", Value: cookieValue, Expires: expire}
		http.SetCookie(w, &userCookie)
		// tell server to delete username
		client_msg := "delete," + username
		fmt.Println("waiting for response")
		response := tellServer(client_msg)
		if response == "200" {
			http.Redirect(w, r, "/login", 302)
		}	
	}
}


// handler for logging out... no server interfacing 
func logout(w http.ResponseWriter, r *http.Request) {
	userCookie, _ := r.Cookie("SessionID")
	if userCookie == nil {
		http.Redirect(w, r, "/login", 302)
		return
	}
	if r.Method == "GET" {
		t, _ := template.ParseFiles("logout.html")
		t.Execute(w, nil)
	} else if r.Method == "POST" {
		// expire cookie & redirect them back to login page
		username := userCookie.Value
		cookieValue := username
		expire := time.Now()
		userCookie := http.Cookie{Name: "SessionID", Value: cookieValue, Expires: expire}
		http.SetCookie(w, &userCookie)
		http.Redirect(w, r, "/login", 302)
	}
}

type post struct {
	IDu string
	IDd string
	IDf string
	IDr string
	IDuf string
	ParentID string
	Username string
	Content string
	Timestamp string
	Votes string
} 

func decodeString(posts string) []post {
	// posts are formated so that each line contains "id,parentId,username,content,ts,votes\n..."
	// posts string can not end in /n or error index out of range will occur
	var post_list []post
	posts_tokens := strings.Split(posts, "\n")
	for _, p := range posts_tokens {
		var curr_post post
		post_tokens := strings.Split(p, ",")
		curr_post.IDu = post_tokens[0] + ",upvote"
		curr_post.IDd = post_tokens[0] + ",downvote"
		curr_post.IDf = post_tokens[0] + ",flag"
		curr_post.IDuf = post_tokens[0] + ",unflag"
		curr_post.IDr = post_tokens[0] + ",remove"
		curr_post.ParentID = post_tokens[1]
		curr_post.Username = post_tokens[2]
		curr_post.Content = post_tokens[3]
		curr_post.Timestamp = post_tokens[4]
		curr_post.Votes = post_tokens[5]

		post_list = append(post_list, curr_post)
	}

	sort.SliceStable(post_list, func(i, j int) bool {
		a, _ := strconv.Atoi(post_list[i].Votes)
		b, _ := strconv.Atoi(post_list[j].Votes)
  		return a > b
	})

	return post_list
}

func home(w http.ResponseWriter, r *http.Request) {
	userCookie, _ := r.Cookie("SessionID")
	if userCookie == nil {
		http.Redirect(w, r, "/login", 302)
		return
	}
	if r.Method == "GET" {
		t, _ := template.ParseFiles("main.html")
		t.Execute(w, nil)
	}
	if r.Method == http.MethodPost {
		r.ParseForm()
		choice1 := r.Form.Get("choice1") //upvote, downvote, flag
		choice := r.Form.Get("choice") // ask question, submit solution
		input := r.PostFormValue("input") // question / solution input
		username := userCookie.Value
		var client_msg string
		if choice == "solution" {
			client_msg = "p_solution," + input + "," + username
		} else if choice == "question" {
			client_msg = "question," + input + "," + username
		} else {
			client_msg = "vote," + choice1
			fmt.Println(client_msg)
		}
		response := tellServer(client_msg)
		if response == "305" {
			fmt.Fprintf(w, "there are no results")
		} else if response == "200" {
			fmt.Fprintf(w, "post has been added!")
		} else if response == "201" {
			fmt.Fprintf(w, "vote sucessfully submitted")
		} else if response == "202" {
			fmt.Fprintf(w, "post sucessfully flagged")
		} else {
			posts := decodeString(response)
			t := template.New("some_name")
			t, _ = template.ParseFiles("main.html")
			t.Execute(w, posts)
		}
	}
}

type user struct {
	Username string
	Fname string
	Lname string
	Email string
	Reputation string
} 

// helper function for profile handler
func generateUser(str_user string) user {
	// given in format: username,fname,lname,email,reputation
	var curr_user user
	user_tokens := strings.Split(str_user, ",")
	curr_user.Username = user_tokens[0]
	curr_user.Fname = user_tokens[1]
	curr_user.Lname = user_tokens[2]
	curr_user.Email = user_tokens[3]
	curr_user.Reputation = user_tokens[4]
	fmt.Println(curr_user)
	return curr_user

}

// profile handler, gives user profile information including reputation
func profile(w http.ResponseWriter, r *http.Request) {
	userCookie, _ := r.Cookie("SessionID")
	if userCookie == nil {
		http.Redirect(w, r, "/login", 302)
		return
	}
	if r.Method == "GET" {
		username := userCookie.Value
		client_msg := "reputation," + username
		response := tellServer(client_msg)
		if response == "306" {
			fmt.Fprintf(w, "error")
		} else {
			var users []user
			users = append(users, generateUser(response))
			t := template.New("some_name")
			t, _ = template.ParseFiles("profile.html")
			t.Execute(w, users)
		}
	}
}

func main() {

	// posts := decodeString("1,2,lisa,content,10:11,20\n3,4,john,content2,1:13,2")
	// for _, post := range posts {
	// 	fmt.Println(post)
	// }


	http.HandleFunc("/main", home)
	http.HandleFunc("/create", createAccount)
	http.HandleFunc("/login", login)
	http.HandleFunc("/moderate", moderate)
	http.HandleFunc("/profile", profile)
	http.HandleFunc("/", login)
	http.HandleFunc("/delete", deleteAccount)
	http.HandleFunc("/logout", logout)
	http.ListenAndServe(":8081", nil)

}