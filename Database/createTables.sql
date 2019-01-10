CREATE TABLE public.User (
	username varchar PRIMARY KEY,
	f_name varchar,
	l_name varchar,
	email varchar,
	password varchar,
	creationTS timestamp,
	);

CREATE TABLE public.Reputation (
	username varchar,
	upVotes int,
	downVotes int,
	PRIMARY KEY(username),
	FOREIGN KEY (username) REFERENCES public.User(username)
	);

CREATE TABLE public.Post (
	postID int,
	parentID int,
	postUsername varchar,
	postContent varchar,
	postTS timestamp,
	postStatus Boolean,
	votes int,
	PRIMARY KEY(postID),
	FOREIGN KEY (parentID) REFERENCES public.Post(postID),
	FOREIGN KEY (postUsername) REFERENCES public.User(username)
	);

CREATE TABLE public.Tag (
	postID int,
	tag varchar,
	PRIMARY KEY (postID, tag)
	FOREIGN KEY(postID) REFERENCES public.Post(postID)
	);

CREATE TABLE public.Moderator (
	mID int,
	password varchar
	PRIMARY KEY (mID)
	)