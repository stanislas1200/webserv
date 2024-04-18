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

# Split the POST data to extract the filename
filename = ""
for line in post_data.splitlines():
    if line.startswith("Content-Disposition:"):
        filename = line.split("filename=")[-1].strip('"')
        break

# Output the extracted filename
if filename:
    print(f"Uploaded filename: {filename}")
else:
    print("Error: No file uploaded.")
