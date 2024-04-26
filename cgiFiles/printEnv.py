#!/usr/bin/python3

import os

print("Content-Type: text/plain\r\n\r\n")  # HTTP header

# Print environment variables
for key, value in os.environ.items():
    print(f"<p>{key}: {value}</p>")
