#!/usr/bin/python3

import os
import sys
import cgi
import cgitb

#enable error handling in the browser for debugging
cgitb.enable()

# while True:
# 	pass

public_files_dir = "./cgi/public_files/"

#get request method
#os is a std lib thing that can help get stuff abt the operating system
request_method = os.environ.get("REQUEST_METHOD")
route = os.environ.get("ROUTE")

form = cgi.FieldStorage()

if request_method == "POST":
	if "file" in form:

		#file item is like a struct which contains information about the file
		file_item = form["file"]
		file_name = file_item.filename
		file_path_and_name = public_files_dir + file_name

		#create the dir if it doesnt exist
		if not os.path.exists(public_files_dir):
			os.makedirs(public_files_dir)
		
		#open file to write stuff
		#wb stands for writing mode : write binary (to take into acc files wif no words like pics)
		#include file item struct theres a "FILE" object, which we can call read() to get the file contents
		file_data = b''
		file_data = file_item.file.read()

		with open(file_path_and_name, "wb") as newfile:
			newfile.write(file_data)

		#status msg
		msg = "Succesfully uploaded file!"
		print("HTTP/1.1 200 OK\r\n", end="")
		
	else:
		msg = "Failed to upload file"
		print("HTTP/1.1 500 Internal Server Error\r\n", end="")
	response_body = "<!DOCTYPE html>"
	response_body += "<html>"
	response_body += "<head></head>"
	response_body += "<body><center><h1>" + msg + "</h1></center></body>"
	response_body += "</html>"

	print("Content-Type: text/html\r\n", end="")
	print("Connection: keep-alive\r\n", end="")
	print("Content-Length: " + str(len(response_body)) + "\r\n", end="")
	print("\r\n", end="")
	print(response_body)

elif request_method == "GET":

	route_parts = route.rsplit("/")
	#check on this ltr
	if len(route_parts) > 3:

		file_name = route_parts[3]
		#sys.stderr.write(f"Error: {file_name}\n")

		if os.path.exists(public_files_dir + file_name):

			file_name_parts = file_name.split(".")
			file_extension = file_name_parts[1]

			with open(public_files_dir + file_name, "rb") as infile:
				response_body = infile.read()


			if(file_extension == "txt"):
				content_type = "text/plain"
				response_body = response_body.decode('utf-8')
			elif file_extension == "html":
				content_type = "text/html"
				response_body = response_body.decode('utf-8')
			elif file_extension == "css":
				content_type = "text/css"
				response_body = response_body.decode('utf-8')
			elif file_extension == "js":
				content_type = "application/javascript"
				response_body = response_body.decode('utf-8')
			elif file_extension == "json":
				content_type = "application/json"
				response_body = response_body.decode('utf-8')
			elif file_extension == "jpg" or file_extension == "jpeg":
				content_type = "image/jpeg"
			elif file_extension == "png":
				content_type = "image/png"
			elif file_extension == "gif":
				content_type = "image/gif"
			else:
				content_type = "text/plain"
			startline = "HTTP/1.1 200 OK\r\n"

		else:
			startline = "HTTP/1.1 404 Not found\r\n"
			response_body = "<!DOCTYPE html>"
			response_body += "<html>"
			response_body += "<head></head>"
			response_body += "<body><center><h1> Error 404: Not found</h1></center></body>"
			response_body += "</html>"
			content_type = "text/html"
	else:
		startline = "HTTP/1.1 403 Forbidden\r\n"
		response_body = "<!DOCTYPE html>"
		response_body += "<html>"
		response_body += "<head></head>"
		response_body += "<body><center><h1> Error 403: Forbidden</h1></center></body>"
		response_body += "</html>"
		content_type = "text/html"

	print(startline, end = "")
	print("Content-Type: " + content_type + "\r\n", end="")
	#print("Connection: keep-alive\r\n", end="")
	if content_type in ("image/jpeg", "image/png", "image/gif"):
		print("Content-Length: " + str(os.path.getsize(public_files_dir + file_name)) + "\r\n", end="")
		print("\r\n", end="", flush=True) #need to flush out first cuz the buffer write ft below converts all the output to binary
		sys.stdout.buffer.write(response_body)
	else:
		print("Content-Length: " + str(len(response_body)) + "\r\n", end="")
		print("\r\n", end="")
		print(response_body)

	if is_fd_open(1):
		os.close(1)

elif request_method == "DELETE":
	# print("HTTP/1.1 200 OK\r\n", end="")
	# print("abc", public_files_dir + form["filename"].value)
	if "filename" in form and os.path.isfile(public_files_dir + form["filename"].value) and os.access(public_files_dir + form["filename"].value, os.W_OK):
		os.remove(public_files_dir + form["filename"].value)
		msg = "Succesfully deleted file!"
		print("HTTP/1.1 200 OK\r\n", end="")
	else:
		msg = "Failed to delete file"
		print("HTTP/1.1 500 Internal Server Error\r\n", end="")
	response_body = "<!DOCTYPE html>"
	response_body += "<html>"
	response_body += "<head></head>"
	response_body += "<body><center><h1>"+msg+"</h1></center></body>"
	response_body += "</html>"
	content_type = "text/html"
	print("Content-Type: " + content_type + "\r\n", end="")
	print("Connection: keep-alive\r\n", end="")
	print("Content-Length: " + str(len(response_body)) + "\r\n", end="")
	print("\r\n", end="")
	print(response_body)
