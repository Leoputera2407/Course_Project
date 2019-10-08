TRUNCATE TABLE project1.artist CASCADE;
TRUNCATE TABLE project1.song CASCADE;
TRUNCATE TABLE project1.token CASCADE;

\copy project1.artist FROM '/home/cs143/data/artist.csv' WITH DELIMITER ',' QUOTE '"' ENCODING 'utf8' CSV;
\copy project1.song FROM '/home/cs143/data/song.csv' WITH DELIMITER ',' QUOTE '"' ENCODING 'utf8' CSV;
\copy project1.token FROM '/home/cs143/data/token.csv' WITH DELIMITER ',' QUOTE '"' ENCODING 'utf8' CSV;

-- load idf table.
INSERT INTO project1.idf (token, idf)
    SELECT token, SUM(1) as idf FROM project1.token
    WHERE count > 0 -- just a precaution
    GROUP BY token;

-- compute tfidf
INSERT INTO project1.tfidf (song_id, token, score)
    SELECT
        token.song_id AS song_id,
        token.token AS token,
        token.count::float * LOG( (SELECT COUNT(song_id)FROM project1.song) / idf.idf::float) AS score
    FROM project1.token
    INNER JOIN project1.idf ON idf.token = token.token;

