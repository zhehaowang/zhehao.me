#!/usr/bin/env python3

import unittest
import random

from sorting import selection_sort, insertion_sort, merge_sort, bubble_sort

class TestSorting(unittest.TestCase):
    def setUp(self):
        self.funcs = [
            {'call': insertion_sort, 'name': 'insertion sort'},
            {'call': selection_sort, 'name': 'selection sort'},
            {'call': merge_sort, 'name': 'merge sort'},
            {'call': bubble_sort, 'name': 'bubble sort'},
        ]
        return

    def test_sort_empty_list(self):
        list_in = []
        for item in self.funcs:
            list_out = item['call'](list_in)
            self.assertEqual([], list_out, item['name'] + ' expects returning empty list')

    def test_sort_ordered_list(self):
        list_in = [-3, 0, 2, 56, 70]
        expected_list_out = sorted(list_in)
        for item in self.funcs:
            list_out = item['call'](list_in)
            self.assertEqual(expected_list_out, list_out, item['name'] + ' returns wrong result')

    def test_sort_reverse_ordered_list(self):
        list_in = [7, 3, -9]
        expected_list_out = sorted(list_in)
        for item in self.funcs:
            list_out = item['call'](list_in)
            self.assertEqual(expected_list_out, list_out, item['name'] + ' returns wrong result')

    def test_sort_regular_operation(self):
        list_in = [random.random() for e in range(100)]
        expected_list_out = sorted(list_in)
        for item in self.funcs:
            list_out = item['call'](list_in)
            self.assertEqual(expected_list_out, list_out, item['name'] + ' returns wrong result')

if __name__ == '__main__':
    unittest.main()
