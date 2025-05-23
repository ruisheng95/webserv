#!/usr/bin/python3

import os
import sys
import cgi
import cgitb

startline = "HTTP/1.1 200 OK\r\n"
response_body = "<!DOCTYPE html>"
response_body += "<html>"
response_body += "<head></head>"
response_body += "<body><center><h1> Webpage from the cgi script for php files</h1></center></body>"
response_body += "<center><img src = \"../cgi/public_files/furina.jpg\" alt = \"furina picture\"><center>"
response_body += "</html>"
content_type = "text/html"

print(startline, end = "")
print("Content-Type: " + content_type + "\r\n", end="")
print("Content-Length: " + str(len(response_body)) + "\r\n", end="")
print("\r\n", end="")
print(response_body)