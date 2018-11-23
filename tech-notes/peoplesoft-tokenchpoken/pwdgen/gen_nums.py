import os

def write_num(start, end):
    with open("year.txt", 'a') as wfile:
        for num in range(start, end + 1):
            wfile.write(str(num) + '\n')

if os.path.isfile("year.txt"):
    os.remove("year.txt")
write_num(1980, 2020)
write_num(0, 100)
