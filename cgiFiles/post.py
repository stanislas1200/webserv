#!/usr/bin/python3

import sys
import os

# Print the Content-Type header required for CGI scripts
print("Content-Type: text/plain\r\n\r\n")

# Read the Content-Length header to determine the length of the POST data
content_length = int(os.environ.get("CONTENT_LENGTH", 0))

if (content_length == 0):
	print("No POST data received.")
	sys.exit()

# Read the POST data from stdin
post_data = sys.stdin.read(content_length)

print(f'Content_length: {content_length}')
print("Body: ")
print(post_data)
