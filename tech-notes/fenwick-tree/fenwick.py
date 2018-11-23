# Fenwick tree, or binary index binary tree
# Motivation:
#   Given an array [a_0...a_n], we want to support two operations,
#   getSum(0..i)
#   update(i, x)
#
# While a naive array impl would allow update in O(1) and getSum in O(n),
# and an array storing the value of sum till index i would allow update
# in O(n) and getSum in O(1), Fenwick tree would allow both in O(logN)

class FenwickTree():
    def __init__(self, array):
        self.data = [0 for i in range(len(array) + 1)]
        for i in range(len(array)):
            self.inc(i, array[i])
        # print(self.data)
        return

    def getSum(self, idx):
        total = 0
        idx += 1
        while idx > 0:
            total += self.data[idx]
            idx -= idx & (-idx)
        return total

    def inc(self, idx, x):
        idx += 1
        while idx < len(self.data):
            self.data[idx] += x
            idx += idx & (-idx)
        return

class FenwickTreeGetAndUpdateRange():
    def __init__(self, array):
        self.data = [0 for i in range(len(array) + 1)]
        for i in range(len(array)):
            self.incRange(i, i, array[i])
        print(self.data)
        return

    def get(self, idx):
        total = 0
        idx += 1
        while idx > 0:
            total += self.data[idx]
            idx -= idx & (-idx)
        return total

    def incRange(self, start, end, x):
        idx = start + 1
        while idx < len(self.data):
            self.data[idx] += x
            idx += idx & (-idx)

        idx = end + 2
        while idx < len(self.data):
            self.data[idx] -= x
            idx += idx & (-idx)
        return

def getSumAndUpdate():
    array = [1, 5, 3, 2, 4]
    fenwick = FenwickTree(array)
    
    print(fenwick.getSum(0))
    print(fenwick.getSum(1))
    print(fenwick.getSum(2))
    print(fenwick.getSum(3))
    print(fenwick.getSum(4))

    fenwick.inc(1, 2)
    print("")
    print(fenwick.getSum(0))
    print(fenwick.getSum(1))
    print(fenwick.getSum(2))
    print(fenwick.getSum(3))
    print(fenwick.getSum(4))

def getAndUpdateRange():
    array = [1, 5, 3, 2, 4]
    fenwick = FenwickTreeGetAndUpdateRange(array)

    print(fenwick.get(0))
    print(fenwick.get(1))
    print(fenwick.get(2))
    print(fenwick.get(3))
    print(fenwick.get(4))

    fenwick.incRange(1, 3, 2)
    print("")
    print(fenwick.get(0))
    print(fenwick.get(1))
    print(fenwick.get(2))
    print(fenwick.get(3))
    print(fenwick.get(4))
    return

if __name__ == "__main__":
    getAndUpdateRange()
