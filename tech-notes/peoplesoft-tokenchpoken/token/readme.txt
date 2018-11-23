### Credit: erpscan

https://erpscan.com/press-center/blog/peoplesoft-security-part-3-peoplesoft-sso-tokenchpoken-attack/

### Original readme

TokenChpoken v0.5 beta 
Oracle PS_TOKEN cracker

Alexey Tyurin - a.tyurin at erpscan.com
ERPScan Research Group - http://www.erpscan.com


TokenChpoken is a special toolkit for attacking Oracle PeopleSoft PS_TOKEN.

Here is an example of how to use the toolkit.

0) Get a PS_TOKEN from a target PeopleSoft server. 

1) Parse the PS_TOKEN extract all the important values. Just feed the PS_TOKEN to the following script of the toolkit: 


python parse.py -c PS_TOKEN_COOKIE_HERE

Output example:
SHA-1 hash from the token: e36a2b956e0466aebb4bb506da78538f2ecd4f99

Information from the token: 
Little endian
PTWEBSERVER - user name 
ENG - lang code 
PSFT_HR - node name
2015-07-01-08.06.46.109641 - creation time

salt.txt is saved
-------------


2) Perform a bruteforce attack. 

We should take several of the results received by parse.py out to the second script - brute.py, where: 

-s SHA-1 – a hash value of the PS_TOKEN (“SHA-1 hash from the token”)
-d – a path to a dictionary for bruteforcing
-f – a path to a file with a decompressed value of the PS_TOKEN (the file is created by parse.py and it is called “salt.txt” by default)

Example:
python brute.py  -s e36a2b956e0466aebb4bb506da78538f2ecd4f99-d dictionary.txt -f salt.txt 

3) After we get a correct node password by bruteforcing, we can create a new PS_TOKEN cookie.

Again, we should take several of the results received by parse.py and brute.py to the last script - generate.py, where:
 -e 0 or 1 - little or big endian encoding will be used in the new PS_TOKEN (from parse.py)
 -u - username for the new PS_TOKEN (you can set any username that exists in the victim PS server)
 -l - language (from parse.py)
 -p - node password (from brute.py)
 -n - node name (from parse.py, or you can set another one (if you know what you’re doing))
 -d - date and time without milliseconds (they are hardcoded). Should be close to the time on the victim server.
 
Example:
python generate.py -e 0 -u PS -l ENG -p password -n PSFT_HR -d 2015-07-01-08.06.46.

4) You can set the new PS_TOKEN cookie in your browser and then log into the target PS server.

Starting from this version I’ve introduced a full support of big endian PS_TOKENs. However, there haven’t been much testing and thus it’s a beta, so just in case, fell free to contact me via email. 

Be an ethical hacker.
Enjoy!

Alexey Tyurin - a.tyurin@erpscan.com


Information from the token: 
Big endian
103369 - user name 
ENG - lang code 
HRPROD - node name
2018-02-12-12.45.12.295613 - creation time
salt.txt is saved

