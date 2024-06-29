# GPX Parser

This web application was put together in the course CIS2750 - Software Systems Development and Integration nicknamed "Angel of Death". It is a
library for reading and writing GPX files connected to a JavaScript frontend through an FFI.

### Setup
-----
Setting up the website requires a few prerequisities. These are listed below:

1. GCC
2. libxml2-dev
3. NodeJS
4. Node Package Manager

First make your way to the root folder of the project and run the command:

````
npm install
````

This will install all of the dependencies listed in our package.json folder, the most important of them being nodemon, express, and ffi-napi.

Next, you will need to open a terminal in the parser directory and create the object file that will contain our compiled C functions for our parser. To do this, run the command:

````
make parser
````

This will create a file called libgpxparser.so in the root directory which our JavaScript code will constantly reference to use our C parser functions.

Once this is done, the server should be able to be hosted. To launch the server, run this command in the root folder:

````
npm run dev (PORT)
````

PORT can be any port number, 3000 is typically used for convention in the development stages.

The server should now be live, and can be accessed through any web browser by connecting to the link "localhost:3000".
