import os

def gen_from_pattern(stack, pattern, words, numbers, symbols, result):
    if len(pattern) == 0:
        result.append(stack)
        return
    if pattern[0] == "w":
        for w in words:
            gen_from_pattern(stack + w, pattern[1:], words, numbers, symbols, result)
    if pattern[0] == "n":
        for w in numbers:
            gen_from_pattern(stack + w, pattern[1:], words, numbers, symbols, result)
    if pattern[0] == "s":
        for w in symbols:
            gen_from_pattern(stack + w, pattern[1:], words, numbers, symbols, result)
    return

def gen_dictionary(pattern, outfile):
    words = []
    with open("core.txt", "r") as rfile:
        for line in rfile:
            words.append(line.strip())
    numbers = []
    with open("year.txt", "r") as rfile:
        for line in rfile:
            numbers.append(line.strip())
    symbols = []
    with open("symbols.txt", "r") as rfile:
        for line in rfile:
            symbols.append(line.strip())
    
    # with open(outfile, "w") as wfile:
    #     for w in words:
    #         for n in numbers:
    #             for s in symbols:
    #                 wfile.write(w + s + n + '\n')
    stack = ""
    result = []
    gen_from_pattern(stack, pattern, words, numbers, symbols, result)

    with open(outfile, "a") as wfile:
        for entry in result:
            wfile.write(entry + '\n')
    return

outfile = "out.log"
if os.path.isfile(outfile):
    os.remove(outfile)

gen_dictionary("w", outfile)
gen_dictionary("sw", outfile)
gen_dictionary("ws", outfile)
gen_dictionary("wn", outfile)
gen_dictionary("nw", outfile)
gen_dictionary("wsn", outfile)
gen_dictionary("swn", outfile)
gen_dictionary("wns", outfile)
gen_dictionary("wssn", outfile)
gen_dictionary("wsns", outfile)
gen_dictionary("nws", outfile)