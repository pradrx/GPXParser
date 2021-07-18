# GPX Parser

### A school project made to allow for the easy handling of GPX files.

This web application was put together in the course CIS2750 - Software Systems Development and Integration solely by me. The project is mainly composed of 3 seperate parts.

The first is the GPX parser which is fully written in C as to allow extremely fast handling of these files. LIBXML2 was used to
interpret the XML files and then individual functions were made to seperate the underlying structure of the file into seperate objects, such as Routes, Tracks, and Waypoints.

The second component is the GUI, which is a web interface in this case made with JavaScript and jQuery. Express.js was used to handle endpoints for transfering data such as user files and the data in the GPX files. Node.js was used to run the server locally.
Bootstrap was used for majority of the visuals including the scroll down bars, as well as many of the button designs.

The third component is the mySQL database connected to the website. Unfortunately, this feature has been deprecated as it was connected to our school's proxy server and contained some private information, so I am not able to upload that to this repository publicly. It is planned to move this to a local mySQL database or possibly a noSQL database so that users can store their files indefintely and would not need to reupload them every time they ran the program again.

### Setup
-----
<strong>It is very likely that there are a few bugs left in this edition of the GPX Parser. If you find any major issues, please email me at rajendranpradhapan@gmail.com with the exact error and I will try my best to resolve it ASAP!</strong>

Setting up the website requires a few prerequisities. These are listed below:

1. GCC (C compiler)
2. libxml2-dev installed (to handle XML files)
3. NodeJS
4. Node Package Manager (should be installed with NodeJS)


After all this is completed, we can now start the process of hosting the server.

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


### Future Additions/Changes
-----
- Connecting again to a online SQL database hosted by myself
- Implement user login to allow stored files for individual people
- Clean up user interface
