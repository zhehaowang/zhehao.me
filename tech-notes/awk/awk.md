# Awk notes

grep, sed family of tools, especially good for tabular / csv files

Examples
```sh
# 1. General
awk 'PATTERN { ACTION }' data1 data2
# default PATTERN is all.
# default ACTION is print entire line.

awk '{print}' data1
awk '{print $0}' data1
# $0 is the whole line

awk '$3 > 0 { print $1 , $2 * $3 }' data1
# comma puts a space in

# 2. Built-in variables
# NF: number of fields
awk '{ print NF }' data2

awk '{ print $NF }' data2 # give me the last column

# NR: number of row
awk '{ print NR }' data2
awk '{ print "processing line", NR, "content", $0 }' data2
# arbitrary strings

# 3. math: + - * / ^(power)

# 4. printf: formatted print
awk '{ printf("line %s product is %6.2f\n", NR, $2 * $3) }' data1

# 5. pattern
awk '$2 * $3 > 1.0 { print NR }' data2
awk '$1 == "a" { print $0 }' data1
awk '/gar.*/' data2            # regexp
awk 'NR == 1 || /gar.*/' data2 # or

# 6. begin and end
awk 'BEGIN { print "Begin" } { print $0 } END { print "End" }' data1
# begin: before start running awk, do ACTION.
awk 'END { print }' data1
# print last line

# 7. user defined variables
awk '{r = r + 1} END {printf("found %d number of rows\n", r)}' data2
# r here is automatically initialized to 0

# 8. awk programs
awk -f example.awk data1
awk -f example2.awk data1

# 9. concatenate strings
awk '{f = f $1 " "} END {printf("%s\n", f)}' data2

# 10. control flow statements
awk '$1 > 0.0 {r = r + 1} END {if (r > 0) { printf("matching line count %d\n", r); printf("we good\n"); } else printf("no matching line") }' data2
# if-else statement with a block
awk '{ i= 1; while (i < 3) {print $1; i = i + 1;} }' data1
# while loop
awk '{ arr[NR] = $0 } END {i = NR; while (i > 0) { print arr[i]; i = i - 1 } }' data1
# reverse a file using an array

# 11. different separator
awk -F";" '{ i= 1; while (i < 3) {print $1; i = i + 1;} }' data3
```
