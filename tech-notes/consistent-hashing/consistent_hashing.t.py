#!/usr/bin/env python3

import unittest
from consistent_hashing import ConsistentHashtable

class TestConsistentHashtable(unittest.TestCase):
    def setUp(self):
        pass

    def testPutAndGet(self):
        ht = ConsistentHashtable(3, 2)
        self.assertEqual(None,  ht.get("some_key"))
        self.assertEqual(False, ht.has_content("some_key"))
        
        content = "this is good"
        key = ht.put(content)
        self.assertEqual(True, ht.has_content(content))
        self.assertEqual(content, ht.get(key))
        return

if __name__ == '__main__':
    unittest.main()
