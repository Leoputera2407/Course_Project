#### SongSearch

We implemented a simple python webserver that queries our postgres database for song lyrics.

You could input the song title, artist name or even snippets of the lyrics and we'll return possible songs that match what you inputted.

##### How to run

You must have postgresSQL and it's running in your computer (`CREATE {target_database}`)

`cd code/project_files/sql`

`psql -f load.sql -d {target_database}`

`cd ..\SearchEngine`

`python wsgq.py`

`cd SearchEngine`

`python search engine.py`

