# TCP-Proxy-Server
A TCP proxy server that returns the landing page of a given URL. 

### Overview
Uses TCP sockets to connect a client to their desired HTTP website. The proxy server will form a GET statement and retrieve the landing page of the website. It will check the given response code to ensure the landing page was returned correctly. If it was, it stores the website name into an array, stores the landing page into a file with the timestamp as the file name, and the website URL with the timestamp into a `list.txt` file that stores the cache locally across program uses. The cache has a maximum size of 6 websites, and if the user enters a URL that is already in the cache, it will return the contents of the matching file, rather than performing another GET request. If the website is not returned correctly, it will forward the error message to the user, and wll not save the URL into the cache. Example files are provided to start the program with a full cache. This program was developed in a Linux Environment.

### To Run
There is a makefile included. Run the `make` command to compile the code. This will generate an executable for the client and the server, named `client` and `server` respectively. Run each of these with the same port number as a command line argument.
  
  *Note, server is hardcoded to run a specific server. This will likely need to be changed per usage.

### To Use
On the client side, the user will be prompted to input a HTTP website URL. Once entered, the client will be returned either the landing page, or the given error from the server. The client can continue to enter in URLs until they enter the `quit` command to end the loop.
