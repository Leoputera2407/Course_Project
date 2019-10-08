#!/usr/bin/python3
import psycopg2
import re
import string
import sys

_PUNCTUATION = frozenset(string.punctuation)

def _remove_punc(token):
    """Removes punctuation from start/end of token."""
    i = 0
    j = len(token) - 1
    idone = False
    jdone = False
    while i <= j and not (idone and jdone):
        if token[i] in _PUNCTUATION and not idone:
            i += 1
        else:
            idone = True
        if token[j] in _PUNCTUATION and not jdone:
            j -= 1
        else:
            jdone = True
    return "" if i > j else token[i:(j+1)]

def _get_tokens(query):
    rewritten_query = []
    tokens = re.split('[ \n\r]+', query)
    for token in tokens:
        cleaned_token = _remove_punc(token)
        if cleaned_token:
            if "'" in cleaned_token:
                cleaned_token = cleaned_token.replace("'", "''")
            rewritten_query.append(cleaned_token)
    return rewritten_query


def get_pg_connection():
    return psycopg2.connect(host="localhost", database="searchengine",user="cs143", password="cs143" )

AND_QUERY="""
WITH song_matches AS (
    SELECT SUM(tfidf.score) as score, tfidf.song_id FROM project1.tfidf
    WHERE tfidf.token IN %s
    GROUP BY tfidf.song_id
    HAVING SUM(1) = %s
    ORDER BY SUM(tfidf.score) DESC
)
SELECT s.song_name, a.artist_name, s.page_link
    FROM song_matches sm
    INNER JOIN project1.song s ON s.song_id = sm.song_id
    INNER JOIN project1.artist a ON a.artist_id = s.artist_id
    ORDER BY sm.score DESC
"""


OR_QUERY="""
WITH song_matches AS (
    SELECT SUM(tfidf.score) as score, tfidf.song_id FROM project1.tfidf
    WHERE tfidf.token IN %s
    GROUP BY tfidf.song_id
    ORDER BY SUM(tfidf.score) DESC
)
SELECT s.song_name, a.artist_name, s.page_link
    FROM song_matches sm
    INNER JOIN project1.song s ON s.song_id = sm.song_id
    INNER JOIN project1.artist a ON a.artist_id = s.artist_id
    ORDER BY sm.score DESC
"""

CREATE_QUERY_CACHE="""
CREATE MATERIALIZED VIEW query_cache AS
(
{}
);

"""

DROP_QUERY_CACHE="""
DROP MATERIALIZED VIEW IF EXISTS query_cache;
"""

QUERY_CACHE="""
SELECT song_name, artist_name, page_link
FROM query_cache
LIMIT 20
OFFSET %s;
"""

QUERY_CACHE_TOTAL_RECORDS="""
SELECT count(*) FROM query_cache;
"""




is_first_query = True
cached_query_hash = None


"""
function arguments:
    - query: string of search. e.g. 'I killed a man'
    - query_type: one of 'and' or 'or'
    - page_no: pagination number from 1 to N

returns: (rows, n_total_results)
where rows is the array of records returned, n_total_results is number of records 

n_total_results will be an integer for the first query. For subsequent identical queries,
it will be None.

Returns 20 rows (or less) per page.

"""

def search(query, query_type, page_no):
    global AND_QUERY, OR_QUERY, DROP_QUERY_CACHE, CREATE_QUERY_CACHE, QUERY_CACHE, QUERY_CACHE_TOTAL_RECORDS, is_first_query, cached_query_hash
    n_total_results = None
    rewritten_query = _get_tokens(query)
    if query_type.lower() != "and" and query_type.lower() != 'or':
        print("invalid query_type!")
        return []
    try:
        conn = get_pg_connection()
    except:
        print("cannot connect to postgres. Error!")
        return []
    rows = []
    cursor = None
    need_commit = False
    try:
        cursor = conn.cursor()
        query_hash = query_type.lower() + str(sorted(set(rewritten_query)))
        print("query_hash: ", query_hash, " cached query hash: ", cached_query_hash)
        if cached_query_hash is None or cached_query_hash != query_hash:
            # evict and change cache.
            need_commit = True
            print("trying drop")
            cursor.execute(DROP_QUERY_CACHE)
            print("drop succeeded.")

            if query_type.lower() == "and":
                cursor.execute(CREATE_QUERY_CACHE.format(AND_QUERY),
                    (tuple(rewritten_query), len(rewritten_query),)
                )
            else: # it's an or.
                cursor.execute(CREATE_QUERY_CACHE.format(OR_QUERY),
                    (tuple(rewritten_query),)
                )
            print("changing cached_query_hash from:", cached_query_hash, " to ", query_hash)
            cached_query_hash = query_hash
        print("is_first_query: ", is_first_query, " need_commit:" , need_commit)
        if is_first_query or need_commit:
            # then return the cached table size. 
            cursor.execute(QUERY_CACHE_TOTAL_RECORDS)
            n_total_results = list(cursor.fetchone())[0]
            is_first_query = False

        # then, query the materialized view.
        offset = (page_no - 1) * 20
        if offset < 0:
            offset = 0
        cursor.execute(QUERY_CACHE, (offset, ))
        rows = cursor.fetchall()
    except Exception as e:
        print("cursor error", e)
        pass
    finally: 
        if cursor is not None:
            cursor.close()
        if need_commit:
            conn.commit()
        conn.close()
    return rows, n_total_results


if __name__ == "__main__":
    if len(sys.argv) > 2:
        rows, n_total_results = search(' '.join(sys.argv[2:]), sys.argv[1].lower(), 1)
        print(rows)
    else:
        print("USAGE: python3 search.py [or|and] term1 term2 ...")
