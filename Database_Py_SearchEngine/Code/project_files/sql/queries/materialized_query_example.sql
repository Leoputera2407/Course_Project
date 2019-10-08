-- how we would use materialized views.
DROP MATERIALIZED VIEW IF EXISTS tempcache;
CREATE MATERIALIZED VIEW tempcache AS
WITH song_matches AS (
    SELECT SUM(tfidf.score) as score, tfidf.song_id FROM project1.tfidf
    WHERE tfidf.token IN ('hey', 'mama')
    GROUP BY tfidf.song_id
    HAVING SUM(1) = 2 -- 2 distinct tokens 
    ORDER BY score DESC
)
SELECT song.song_name, artist.artist_name, song_matches.score
    FROM project1.song
    INNER JOIN song_matches ON song_matches.song_id = song.song_id
    INNER JOIN project1.artist ON artist.artist_id = song.artist_id
    ORDER BY song_matches.score DESC;
