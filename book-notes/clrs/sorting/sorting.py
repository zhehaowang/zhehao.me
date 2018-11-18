#!/usr/bin/env python3

import os
import sys

def selection_sort(list_in):
    # time complexity:  Theta(n^2)
    # space complexity: O(1) in-place, O(n) make-a-copy
    list_out = list(list_in)

    for i in range(len(list_out) - 1):
        # loop invariant: after index i,
        #  * list_out[0:i + 1] is sorted
        #  * list_out[0:i + 1] contains the (i + 1)-smallest items from list_in
        minimum_idx = i
        for j in range(i + 1, len(list_out)):
            if list_out[minimum_idx] > list_out[j]:
                minimum_idx = j
        list_out[i], list_out[minimum_idx] = list_out[minimum_idx], list_out[i]

    return list_out

def insertion_sort(list_in):
    # time complexity:  O(n^2), Omega(n)
    # space complexity: O(1) in-place, O(n) make-a-copy
    list_out = list(list_in)

    for i in range(1, len(list_out)):
        # loop invariant: after index i,
        #  * list_out[0:i + 1] is sorted
        #  * list_out[0:i + 1] contains the same elements as list_in[0:i + 1]
        for j in range(i, 0, -1):
            if list_out[j] < list_out[j - 1]:
                list_out[j], list_out[j - 1] = list_out[j - 1], list_out[j]
            else:
                break
    return list_out

def bubble_sort(list_in):
    # time complexity:  Theta(n^2)
    # space complexity: O(1) in-place, O(n) make-a-copy
    list_out = list(list_in)

    for i in range(1, len(list_out)):
        # loop invariant: after index i,
        #  * list[len(list) - i:] is sorted
        #  * list[len(list) - i:] contains the largest elements in list_in
        for j in range(1, len(list_out) - i + 1):
            if list_out[j] < list_out[j - 1]:
                list_out[j], list_out[j - 1] = list_out[j - 1], list_out[j]
    return list_out

def merge_sort(list_in):
    # time complexity: O(n logn)
    # space complexity: O(n)
    def helper(list_in, start, end, temp):
        # sort list_in[start:end), using temp as pre-aollocated temporary buffer
        # space
        if end - start < 2:
            return
        else:
            mid = (start + end) // 2
            # divide
            helper(list_in, start, mid, temp)
            helper(list_in, mid, end, temp)

            # conquer and combine
            i = start
            j = mid
            k = 0
            while i < mid and j < end:
                # loop invariant: temp[k] contains the k smallest elements from
                # left and right combined
                if list_in[i] < list_in[j]:
                    temp[k] = list_in[i]
                    i += 1
                else:
                    temp[k] = list_in[j]
                    j += 1
                k += 1

            # this can be replaced with putting an \infty at the end of both
            # left and right subarrays
            if i < mid:
                temp[k:end - start] = list_in[i:mid]
            elif j < end:
                temp[k:end - start] = list_in[j:end]

            list_in[start:end] = temp[0:end - start]
            return
        return

    temp    = list(list_in)
    to_sort = list(list_in)
    helper(to_sort, 0, len(list_in), temp)
    return to_sort

def quick_sort(list_in):
    return

def heap_sort(list_in):
    return