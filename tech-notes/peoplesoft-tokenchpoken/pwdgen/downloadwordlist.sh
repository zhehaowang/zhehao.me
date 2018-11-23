#!/bin/bash

# wordlist without duplicates for spell checking
# EN/HU/DE words, many with suffixes, most common ones are at the beginning

# If the wget links are bad (Google doesn't likes when people use direct links in wget from Drive):
    # Dictionary-wordlist-small-2015-12-03.7z.001
    # Dictionary-wordlist-small-2015-12-03.7z.002
# mail me if you have any advise or question

mkdir tempforwordlist
cd tempforwordlist

printf '\n\n----------\n\nDownloading\n\n'
    useragent="Mozilla/5.0 (X11; Linux x86_64; rv:45.0) Gecko/20100101 Firefox/45.0"

    publicurl=$(wget -q --user-agent="${useragent}" --save-cookies cookies.txt "https://goo.gl/u3AycN" -O - | perl -pe 's/href/\nXXXXXC/g; s/">/\n/g' | grep XXXXXC | grep confirm | perl -pe 's/XXXXXC="/https:\/\/docs.google.com/g; s/&amp;/&/g' | tail -1)
    wget --user-agent="${useragent}" --load-cookies cookies.txt "$publicurl" -O Dictionary-wordlist-small-2015-12-03.7z.001
    rm cookies.txt

    publicurl=$(wget -q --user-agent="${useragent}" --save-cookies cookies.txt "https://goo.gl/MF9zkw;" -O - | perl -pe 's/href/\nXXXXXC/g; s/">/\n/g' | grep XXXXXC | grep confirm | perl -pe 's/XXXXXC="/https:\/\/docs.google.com/g; s/&amp;/&/g' | tail -1)
    wget --user-agent="${useragent}" --load-cookies cookies.txt "$publicurl" -O Dictionary-wordlist-small-2015-12-03.7z.002
    rm cookies.txt

printf '\n\n----------\n\nCreating checksum\n\n'
    cksum *.0* | tee -a cksums.txt
    ls -lah *.0*

printf '\n\n----------\n\nExtracting...\n\n'
    7z x Dictionary-wordlist-small-2015-12-03.7z.001
    ls -lah Dictionary-wordlist-small-2015-12-03

