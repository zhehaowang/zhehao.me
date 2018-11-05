#!/usr/bin/env python3

import os
import hashlib
import bisect

class ConsistentHashtable():
    def __init__(self, node_cnt, tag_cnt):
        self.nodes = dict()
        self.node_list = []

        for node in range(node_cnt):
            for tag in range(tag_cnt):
                node_key = self.hash('node' + str(node) + '_' + str(tag))
                self.nodes[node_key] = dict()
                bisect.insort_left(self.node_list, node_key)
        return

    def locate_key(self, key):
        node_idx = bisect.bisect_left(self.node_list, key)
        if node_idx == len(self.node_list):
            node_idx = 0
        return self.node_list[node_idx]

    def put(self, content):
        key = self.hash(content)
        self.nodes[self.locate_key(key)][key] = content
        return key

    def get(self, key):
        node_idx = bisect.bisect_left(self.node_list, key)
        if key in self.nodes[self.locate_key(key)]:
            return self.nodes[self.locate_key(key)][key]
        else:
            return None

    def has_content(self, content):
        return False if self.get(self.hash(content)) == None else True

    def hash(self, content):
        message = hashlib.sha256()
        if type(content) == str:
            message.update(content.encode('utf-8'))
        else:
            # Non-string content is not currently supported
            message.update(content)
        return message.hexdigest()

    def add_node(self, tag_cnt):
        return

    def remove_node(self, node_idx):
        return

if __name__ == "__main__":
    ht = ConsistentHashtable(3, 2)
    content = "rekt"
    key = ht.put(content)
    print(ht.has_content(content))
    print(ht.get(key))