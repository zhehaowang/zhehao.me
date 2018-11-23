#!/usr/bin/env python3

import xml.etree.ElementTree
import pprint
import argparse
import requests as pyrequests

def parse_xml(file_name, verbose = False):
    """
    Converts IE-11 exported NetworkData.xml to structured requests containing method, url, headers, cookies, and post data.
    The structured request can later be replayed with requests module.
    @param  file_name The xml file name to read from
    @param  verbose   Print intermediate debug info
    @return Dict {'url': {'url', 'cookies', 'headers', 'data'}}
    """
    e = xml.etree.ElementTree.parse(file_name).getroot()
    requests = dict()
    for entries in e.findall('entries'):
        for entry in entries.findall('entry'):
            for request in entry.findall('request'):
                url = request.find('url').text
                method = request.find('method').text

                headers = request.find('headers')
                result_headers = dict()
                for header in headers:
                    if header.find('name').text == 'Cookie':
                        # we'll supply cookie separately
                        continue
                    if not header.find('name').text or header.find('name').text in result_headers:
                        print("header null or existing")
                    result_headers[header.find('name').text] = header.find('value').text

                cookies = request.find('cookies')
                result_cookies = dict()
                for cookie in cookies:
                    if not cookie.find('name').text or cookie.find('name').text in result_headers:
                        print("cookie null or existing")
                    result_cookies[cookie.find('name').text] = cookie.find('value').text

                requests[url] = {"url": url, "cookies": result_cookies, "headers": result_headers, "method": method}

                post = request.find('postData')
                if post:
                    postData = post.find('text').text.split('&')
                    # unused, mimeType expected to be set up in header
                    mimeType = post.find('mimeType').text
                    requests[url]["data"] = {}
                    for entry in postData:
                        parts = entry.split('=')
                        requests[url]["data"][parts[0]] = parts[1]
    return requests

"""
def request_to_curl(request):
    # NOT IMPLEMENTED
    if not 'url' in request:
        print('malformed request')
        return
    cmd = 'curl \'' + request['url'] + '\' '
    if 'headers' in request:
        for header in request['headers']:
            continue
    if 'cookies' in request:
        for cookie in request['cookies']:
            continue
    return
"""

def send_request(request, overrides, verbose = False):
    """
    Given a request dictionary, apply the cookies / url get parameters override and replay the request
    """
    url = request['url']
    headers = request['headers']
    cookies = request['cookies']
    
    if "cookies" in overrides:
        cookie_overrides = overrides["cookies"]
        for key in cookie_overrides:
            request["cookies"][key] = overrides["cookies"][key]
            print('overriding (cookies:' + key + ')')
    
    if verbose:
        pp = pprint.PrettyPrinter(indent = 2)
        pp.pprint(requests[url])

    if "url" in overrides:
        url_overrides = overrides["url"]
        for key in url_overrides:
            idx = url.find(key + "=")
            if idx >= 0:
                url = url[:idx] + key + "=" + url_overrides[key] + url[idx + len(key) + 1 + len(url_overrides[key]):]
            print('overriding (url:' + key + ')')
    
    if request['method'] == 'GET':
        response = pyrequests.get(url, headers = headers, cookies = cookies, timeout = 5.0)
        return response.text
    elif request['method'] == 'POST':
        data = request["data"] if "data" in request else None
        response = pyrequests.post(url, data = data, headers = headers, cookies = cookies, timeout = 5.0)
        return response.text
    return ""

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'Fake tokens to mypd')
    parser.add_argument('-f', '--xmlfile', type = str, default = 'traffic_pay_slip_page.xml',
                        help = 'The xml file to read requests from')
    parser.add_argument('-t', '--pstoken', type = str, default = "stub",
                        help = 'The fake token to override with')
    parser.add_argument('-e', '--emplid', type = str, default = "103369",
                        help = 'The emplid to override with')
    parser.add_argument('-v', '--verbose', type = bool, default = False,
                        help = 'Verbose mode')

    args = parser.parse_args()

    file_name = args.xmlfile
    requests = parse_xml(file_name, args.verbose)

    overrides = {}
    if args.pstoken:
        overrides["cookies"] = {"PS_TOKEN": args.pstoken}
    if args.emplid:
        overrides["url"] = {"EMPLID": args.emplid}

    menu = []
    cnt = 0
    for url in requests:
        menu.append((cnt, requests[url]['method'], url))
        cnt += 1

    menu_str = "Select one to replay with token override:\n"
    for entry in menu:
        menu_str += "  " + str(entry[0]) + ". " + entry[1] + ":" + entry[2] + "\n"
    print(menu_str)

    num = input("Choose a request: ")
    response = send_request(requests[menu[int(num)][2]], overrides, args.verbose)
    with open("out.log", "w") as wfile:
        wfile.write(response)
    print("Done. Result written to out.log")