#!/usr/bin/env python3

from locust import HttpLocust, TaskSet

def get_dickens(l):
    req_string = '/dickens-dombey-622.txt'
    l.client.get(req_string)

class UserBehaviour(TaskSet):
    tasks = {get_dickens:1}
    def on_start(self):
        pass

    def on_stop(self):
        pass


class WebsiteUser(HttpLocust):
    task_set = UserBehaviour
    min_wait = 5000
    max_wait = 9000


