#!/usr/bin/python3

import os
import sys
from urllib.parse import parse_qs

query_string = os.environ.get("QUERY_STRING", 0)
d = parse_qs(query_string)

# print(d["name"][0])

if (d["name"]):
    try:
        with open(f'./upload/{d["name"][0]}','rb') as fo: 
            data = fo.read()
            sys.stdout.buffer.write("Content-type:application/octet-stream\r\n\r\n".encode())  # HTTP header
            sys.stdout.buffer.write(data)
    except Exception as e:
        print("Content-type:text/html\r\n\r\n")
        print('<br>Exception :')
        print(e)
        exit(1)
