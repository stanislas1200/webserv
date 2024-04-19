#!/Library/Developer/CommandLineTools/usr/bin/python3
#!/bin/python3

import os
from urllib.parse import parse_qs
# print("Content-Type: text/plain\r\n\r\n")  # HTTP header

query_string = os.environ.get("QUERY_STRING", 0)
d = parse_qs(query_string)

# print(d["name"][0])

if (d["name"]):
    try:
        with open(f'./upload/{d["name"][0]}','rb') as fo: 
            print(fo.read())
    except Exception as e:
        print("Content-type:text/html\r\n\r\n")
        print('<br>Exception :')
        print(e)
