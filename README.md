# c_webserver
Web-server application that allows persistent connections

TCP Webserver

This project submission contains a .c file that can be compiled to run a TCP Webserver.

---ser.c---
Once compiled an executable server is created. No arguments are passed.

Execution is done by the following way.
Example: ./ser

HTTP Methods Implemented:

1. GET
This method is used to fetch webpages from the server and display them on the browser. The request
is of the form of:
	
	GET / HTTP/1.1
	Host: localhost:8097
	Connection: keep-alive
	Upgrade-Insecure-Requests: 1
	User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.89 Safari/537.36
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
	Accept-Encoding: gzip, deflate, sdch
	Accept-Language: en-US,en;q=0.8

2. POST
This method is used to post a file to the server. The request is of the form of:

	POST / HTTP/1.1
	Host: localhost:8097
	User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:49.0) Gecko/20100101 Firefox/49.0
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
	Accept-Language: en-US,en;q=0.5
	Accept-Encoding: gzip, deflate
	Content-Type: application/json
	Connection: keep-alive
	Content-Length: 0

3. All other HTTP commands return a 501 Not Implemented reply.

Error types Handled:

The following error types have been handled:

	1. 400 Bad Request
		This error takes care of the cases where there is an invalid method or an invalid HTTP version
		in the request being handled. If the method is not one of GET, POST, PUT, HEAD, PROPFIND etc
		then the server returns the 400 Bad Request status and displays a html webpage saying that the
		method type in not correct.

		Similarly is the HTTP version is any version other than 1.0 or 1.1, the server responds with a
		400 Bad Request reply.

	2. 404 Not Found
		This error takes care of the case when the file requested for does not exist. If file asked for
		by the request does not exist then it will return the 404 not found error. Other than replying 
		with the requisite error header this also prompts a web page that notifies the user that the 
		requested file does not exist in webserver

	3. 501 Not Implemented
		This error takes care of all the methods that haven't been supported, these include checking the
		following methods:
			a. PUT
			b. DELETE
			C. HEAD
			d. PATCH
			e. OPTIONS
			f. PROPFIND
			g. COPY
			h. MOVE

		If the request parses any of these methods a 501 Not Implemented header is sent, along with a web
		page notifying the user that the method they asked for has not been implemented.

	4. 500 Internal Server Error
		This error is a generic error that is prompted if any other error occurs. It has been implemented
		to be shown when the format type of the file requested is not supported.

	5. Port number error
		If the port number falls in the reserved zone that is below, 1024 the server will not start

	6. ws.conf not found
		If the configuration file is not found the server does not start
