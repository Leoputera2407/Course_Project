#!/usr/bin/python3

from flask import Flask, render_template, request


import search
import math
import urllib
#import urlparse
import urllib.parse as urlparse
from urllib.parse import urlencode

application = app = Flask(__name__)
app.debug = True
PAGE_SIZE = 20 # number of results in a single page.
num_records = None



"""
helper function that returns full href of page starting from new_page_from.
"""
def create_button_href(url, new_page_from):
    #Update url parts
    url_parts = list(urlparse.urlparse(url))
    query = dict(urlparse.parse_qsl(url_parts[4]))
    query.update({'page_from' : new_page_from})
    url_parts[4]= urlencode(query)
    return urlparse.urlunparse(url_parts)

@app.route('/search', methods=["GET"])
def dosearch():
    global PAGE_SIZE, num_records
    query = request.args['query']
    qtype = request.args['query_type']
    page_from = int(request.args.get('page_from', default=1))

    search_results, returned_table_size = search.search(query, qtype, page_from)
    if returned_table_size is not None:
        print("updating numrecords to :", returned_table_size)
        num_records = returned_table_size
    total_pages = math.ceil(num_records / PAGE_SIZE)
    page_prev = None
    page_next = None
    if page_from is 1:
        if total_pages is 1:
            page_prev = None
            page_next = None
        else:
            page_next = 2
    else:
        if total_pages == page_from:
             print("totalpages is page from")
             page_prev = page_from - 1
             page_next = None
        else:
            page_prev = page_from - 1
            page_next = page_from + 1

    return render_template('results.html',
        query=query,
        #total_results=len(search_results),
        total_results=num_records,
        page_prev='' if page_prev is None else create_button_href(request.url, page_prev),
        page_next='' if page_next is None else create_button_href(request.url, page_next),
        current_page=page_from,
        total_pages=total_pages,
        search_results=search_results)

@app.route("/")
def index():
    if request.method == "GET":
        return render_template('index.html')

if __name__ == "__main__":
    app.run(host='0.0.0.0')
