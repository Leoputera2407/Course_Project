WITH song_matches AS (
    SELECT SUM(tfidf.score) as score, tfidf.song_id FROM project1.tfidf
    WHERE tfidf.token IN ('could', 'ever', 'feel', 'this', 'real') 
    GROUP BY tfidf.song_id
    HAVING SUM(1) = 5 -- 5 distinct tokens - i.e. we have "could ever feel this real".
    ORDER BY score DESC
    LIMIT 10
)
SELECT song.song_name, artist.artist_name, song_matches.score
    FROM project1.song
    INNER JOIN song_matches ON song_matches.song_id = song.song_id
    INNER JOIN project1.artist ON artist.artist_id = song.artist_id
    ORDER BY song_matches.score DESC
