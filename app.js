'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname + '/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/', function (req, res) {
    res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css', function (req, res) {
    //Feel free to change the contents of style.css to prettify your Web app
    res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js', function (req, res) {
    fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
        const minimizedContents = JavaScriptObfuscator.obfuscate(contents, { compact: true, controlFlowFlattening: true });
        res.contentType('application/javascript');
        res.send(minimizedContents._obfuscatedCode);
    });
});

//Respond to POST requests that upload files to uploads/ directory


//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function (req, res) {
    fs.stat('uploads/' + req.params.name, function (err, stat) {
        if (err == null) {
            res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
        } else {
            console.log('Error in file downloading route: ' + err);
            res.send('');
        }
    });
});

//******************** Your code goes here ******************** 

//Sample endpoint

var libGPXParser = ffi.Library('./libgpxparser.so', {
    'GPXFileToJSON': ['string', ['string']],
    'GPXFileRoutes': ['string', ['string']],
    'GPXFileTracks': ['string', ['string']],
    'getOtherDataRoute': ['string', ['string', 'int']],
    'getOtherDataTrack': ['string', ['string', 'int']],
    'changeRouteName': ['int', ['string', 'string', 'int']],
    'changeTrackName': ['int', ['string', 'string', 'int']],
    'createRoute': ['int', ['string', 'string']],
    'addWaypointToRoute': ['int', ['string', 'double', 'double']],
    'createGPXFile': ['int', ['string', 'string', 'double']],
    'findRoutesBetween': ['string', ['string', 'double', 'double', 'double', 'double', 'double']],
    'findTracksBetween': ['string', ['string', 'double', 'double', 'double', 'double', 'double']],
    'checkValidGPX': ['int', ['string']]
});

app.post('/upload', async function (req, res) {
    if (!req.files) {
        return res.status(400).json({message: "No file uploaded!"});
    }

    let uploadFile = req.files.file;

    if (uploadFile.name.substring(uploadFile.name.length - 4, uploadFile.name.length) !== '.gpx') {
        return res.status(400).json({message: "Incorrect file format!"});
    }

    var fileNames = await listDir();
    if (fileNames.indexOf(uploadFile.name) > -1) {
        return res.status(400).json({message: "File already exists!"});
    }

    // Use the mv() method to place the file somewhere on your server
    uploadFile.mv('uploads/' + uploadFile.name, function (err) {
        if (err) {
            return res.status(500).send(err);
        }

        res.redirect('/');
    });
});

app.get('/getFiles', async function (req, res) {
    var fileNames = await listDir();
    var pathNames = [];

    for (var i = 0; i < fileNames.length; i++) {
        pathNames.push('./uploads/' + fileNames[i]);
    }

    var GPXInfo = await getGPXInfo(pathNames);

    res.send({
        fileNames: fileNames,
        pathNames: pathNames,
        GPXInfo: GPXInfo
    });
});

app.get('/fillViewButton', async function (req, res) {
    var fileNames = await listDir();

    res.send({
        fileNames: fileNames
    });
});

app.get('/viewPanel', async function (req, res) {
    var fileName = req.query.fileName;

    var GPXRoutes = await getRoutes(fileName);
    var GPXTracks = await getTracks(fileName);

    res.send({
        routeList: GPXRoutes,
        trackList: GPXTracks
    });
})

app.get('/otherDataTable', async function (req, res) {
    var fileName = req.query.fileName;
    var index = req.query.index;
    var GPXData;

    if (req.query.type === 'Route') {
        GPXData = await getRouteOtherData(fileName, index);
    } else {
        GPXData = await getTrackOtherData(fileName, index);
    }

    res.send({
        GPXData: GPXData
    });

});

app.get('/renameComponent', async function (req, res) {
    var fileName = req.query.fileName;
    var index = req.query.index;

    if (req.query.type == 'Route') {
        await changeRoute(fileName, req.query.newName, index);
    } else {
        await changeTrack(fileName, req.query.newName, index);
    }

    res.send({
        res: 'good'
    })

});

app.get('/addRoute', async function (req, res) {
    var pathName = req.query.pathName;
    var waypoints = req.query.waypoints;
    var routeName = req.query.routeName;

    await addRoute(pathName, routeName);
    
    if (typeof waypoints !== 'undefined') {
        for (var i = 0; i < waypoints.length; i = i + 2) {
            var latitude = waypoints[i];
            var longitude = waypoints[i + 1];
            await addWaypoint(pathName, latitude, longitude);
        }
    }

    res.send({
        res: 'good'
    })
});

app.get('/createGPX', async function (req, res) {
    var pathName = req.query.pathName;
    var version = req.query.version;
    var creator = req.query.creator;
    var fileName = req.query.fileName;

    var fileNames = await listDir();

    var response;

    if (fileNames.indexOf(fileName) > -1) {
        response = "bad";
    } else {
        await createNewGPX(pathName, creator, version);
        response = "good";
    }
    
    res.send({
        res: response
    })
});

app.get('/findPath', async function (req, res) {
    var lat1 = req.query.lat1;
    var lon1 = req.query.lon1;
    var lat2 = req.query.lat2;
    var lon2 = req.query.lon2;
    var delta = req.query.delta;

    var fileNames = await listDir();
    var pathNames = [];

    for (var i = 0; i < fileNames.length; i++) {
        pathNames.push('./uploads/' + fileNames[i]);
    }

    var routes = [];
    var tracks = [];

    for (var i = 0; i < pathNames.length; i++) {
        var route = await findRoutes(pathNames[i], lat1, lon1, lat2, lon2, delta);
        if (route.length > 0) {
            routes.push(route); 
        }

        var track = await findTracks(pathNames[i], lat1, lon1, lat2, lon2, delta)
        if (track.length > 0) {
            tracks.push(track);
        }
    }

    res.send({
        routeList: routes,
        trackList: tracks
    });
});

async function listDir() {
    try {
        var fileNames = await fs.promises.readdir('./uploads');
        return fileNames;
    } catch (err) {
        console.error('Error occured while reading directory!', err);
    }
}

async function getGPXInfo(fileNames) {
    var GPXInfo = [];
    for (var i = 0; i < fileNames.length; i++) {
        GPXInfo.push(JSON.parse(libGPXParser.GPXFileToJSON(fileNames[i])));
    }
    return GPXInfo;
}

async function getRoutes(fileName) {
    var pathName = './uploads/' + fileName;
    var GPXRoutes = JSON.parse(libGPXParser.GPXFileRoutes(pathName));
    return GPXRoutes;
}

async function getTracks(fileName) {
    var pathName = './uploads/' + fileName;
    var GPXTracks = JSON.parse(libGPXParser.GPXFileTracks(pathName));
    return GPXTracks;
}

async function getRouteOtherData(fileName, index) {
    var GPXData = JSON.parse(libGPXParser.getOtherDataRoute(fileName, index).replace(/(\r\n|\n|\r)/gm, ""));
    return GPXData;
}

async function getTrackOtherData(fileName, index) {
    var GPXData = JSON.parse(libGPXParser.getOtherDataTrack(fileName, index).replace(/(\r\n|\n|\r)/gm, ""));
    return GPXData;
}

async function changeRoute(fileName, newName, index) {
    libGPXParser.changeRouteName(fileName, newName, index);
}

async function changeTrack(fileName, newName, index) {
    libGPXParser.changeTrackName(fileName, newName, index);
}

async function addRoute(pathName, routeName) {
    libGPXParser.createRoute(pathName, routeName);
}

async function addWaypoint(pathName, latitude, longitude) {
    libGPXParser.addWaypointToRoute(pathName, latitude, longitude);
}

async function createNewGPX(file_name, creator, version) {
    libGPXParser.createGPXFile(file_name, creator, version);
}

async function findRoutes(fileName, lat1, lon1, lat2, lon2, delta) {
    var routes = JSON.parse(libGPXParser.findRoutesBetween(fileName, lat1, lon1, lat2, lon2, delta));
    return routes;
}

async function findTracks(fileName, lat1, lon1, lat2, lon2, delta) {
    var tracks = JSON.parse(libGPXParser.findTracksBetween(fileName, lat1, lon1, lat2, lon2, delta));
    return tracks;
}

async function isValid(fileName) {
    var val = parseInt(libGPXParser.checkValidGPX(fileName));
    return val;
}

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
