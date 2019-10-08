DROP SCHEMA IF EXISTS project1 CASCADE;
CREATE SCHEMA IF NOT EXISTS project1;

CREATE TABLE project1.artist (
	artist_id INTEGER PRIMARY KEY,
	artist_name VARCHAR(255)
);


CREATE TABLE project1.token (
	song_id INTEGER,
	token VARCHAR(255),
	count INTEGER,
	PRIMARY KEY (song_id, token)
);

CREATE TABLE project1.song (
	song_id INTEGER PRIMARY KEY,
	artist_id INTEGER,
	song_name VARCHAR(255),
	page_link VARCHAR(1000), 
	FOREIGN KEY (artist_id) REFERENCES project1.artist (artist_id) 
);

CREATE TABLE project1.tfidf (
	song_id INTEGER,
	token VARCHAR(255),
	score FLOAT,
	--PRIMARY KEY(song_id, token)
    PRIMARY KEY(token, song_id)
	/* NOTE: i disagree. PRIMARY KEY should have the other order. (token, song_id)
	to facilitate faster searches. */
);

-- first create idf table.
CREATE TABLE IF NOT EXISTS project1.idf (
    token VARCHAR(255),
    idf INTEGER,
    PRIMARY KEY(token)
);

