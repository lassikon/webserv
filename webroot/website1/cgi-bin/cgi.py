#!/usr/bin/env python3
import sys
import os
import json
from urllib.parse import parse_qs

# Determine the request method
request_method = os.environ.get("REQUEST_METHOD", "GET")

# Initialize parsed_data
parsed_data = {}

# Handle GET method (query string)
if request_method == "GET":
    # Get query string data from the URL (after ?)
    query_string = os.environ.get("QUERY_STRING", "")
    parsed_data = parse_qs(query_string)

# Handle POST method (request body)
elif request_method == "POST":
    # Get content length
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))

    # Read the request body from stdin
    if content_length > 0:
        body = sys.stdin.read(content_length)
    else:
        body = ""

    # Parse the body based on content type
    content_type = os.environ.get("CONTENT_TYPE", "application/x-www-form-urlencoded")
    if content_type == "application/x-www-form-urlencoded":
        parsed_data = parse_qs(body)
    elif content_type == "application/json":
        parsed_data = json.loads(body)
    else:
        parsed_data = {"error": "Unsupported content type"}

# Get the "name" field from the parsed data (if it exists)
name = parsed_data.get('name', [''])[0]  # Default to empty string if not provided
title = parsed_data.get('title', [''])[0]  # Default to empty string if not provided

# Send the response headers
print("Content-Type: text/html\r\n\r\n")

# Send the response body
print("<html><body>")
print("<h1>CGI Script Response</h1>")

# Display a personalized message if the name was provided
if name:
    print(f"<p>Welcome, {name}!</p>")
elif name and title:
    print(f"<p>Welcome, {name}, {title} </p>")
else:
    print("<p>Welcome, Guest!</p>")

# Display the request method and received data
print(f"<p>Request Method: {request_method}</p>")
print(f"<p>Received Data: {parsed_data}</p>")

print("</body></html>")
