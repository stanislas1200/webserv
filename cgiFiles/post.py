#!/usr/bin/env python3
# !/Library/Developer/CommandLineTools/usr/bin/python3

import sys
import os

# Print the Content-Type header required for CGI scripts
print("Content-Type: text/plain\r\n")

# Read the Content-Length header to determine the length of the POST data
content_length = int(os.environ.get("CONTENT_LENGTH", 0))

# Read the POST data from stdin
post_data = sys.stdin.read(content_length)

print(post_data)
