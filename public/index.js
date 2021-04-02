// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(async function () {
    // On page-load AJAX Example
    // jQuery.ajax({
    //     type: 'get',            //Request type
    //     dataType: 'json',       //Data type - we will use JSON for almost everything 
    //     url: '/endpoint1',   //The server endpoint we are connecting to
    //     data: {
    //         data1: "Value 1",
    //         data2:1234.56
    //     },
    //     success: function (data) {
    //         /*  Do something with returned object
    //             Note that what we get is an object, not a string, 
    //             so we do not need to parse it on the server.
    //             JavaScript really does handle JSONs seamlessly
    //         */
    //         jQuery('#blah').html("On page load, received string '"+data.somethingElse+"' from server");
    //         //We write the object to the console to show that the request was successful
    //         console.log(data); 

    //     },
    //     fail: function(error) {
    //         // Non-200 return, do something with error
    //         $('#blah').html("On page load, received error from server");
    //         console.log(error); 
    //     }
    // });

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/getFiles',
        success: function (data) {
            for (var i = 0; i < data.fileNames.length; i++) {
                var anchor = '<a href="' + data.pathNames[i] + '" download ">' + data.fileNames[i] + '</a>';
                $('#filetable').append('<tr><td>' + anchor + '</td><td>' + data.GPXInfo[i].version + '</td><td>' + data.GPXInfo[i].creator + '</td><td>' + data.GPXInfo[i].numWaypoints + '</td><td>' + data.GPXInfo[i].numRoutes + '</td><td>' + data.GPXInfo[i].numTracks + '</td></tr>');
            }
        },
        fail: function (error) {
            console.log(error);
        }
    });

    await $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/fillViewButton',
        success: function (data) {
            for (var i = 0; i < data.fileNames.length; i++) {
                var anchor = '<a class="dropdown-item" href="#">' + data.fileNames[i] + '</a>';
                $('.dropdown-menu').append(anchor);
            }
        },
        fail: function (error) {
            console.log(error);
        }
    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible

    $('#uploadForm').submit(function (e) {
        e.preventDefault();
        const formData = new FormData();
        formData.append('file', $('#uploadFile')[0].files[0]);

        $.ajax({
            type: 'post',
            encType: 'multipart/form-data',
            processData: false,
            contentType: false,
            url: '/upload',
            data: formData,
            success: function () {
                location.reload();
            },
            error: ({ responseJSON: { message } }) => {
                alert(message);
            }
        });
    });

    $('.dropdown-item').click(function (e) {
        e.preventDefault();
        var content = e.target.innerText;

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/viewPanel',
            data: {
                fileName: content,
            },
            success: function (data) {
                $('#viewtable tbody').empty();
                $('#componenttable tbody').empty();
                $('#routenameform .form-group label').text("No Component Selected");

                for (var i = 0; i < data.routeList.length; i++) {
                    var string = '<tr><td>' + 'Route ' + (i + 1) + '</td><td>' + data.routeList[i].name + '</td><td>' + data.routeList[i].numPoints + '</td><td>' + data.routeList[i].len + '</td><td>' + data.routeList[i].loop + '</td></tr>';
                    $('#viewtable tbody').append(string);
                }

                for (var i = 0; i < data.trackList.length; i++) {
                    var string = '<tr><td>' + 'Track ' + (i + 1) + '</td><td>' + data.trackList[i].name + '</td><td>' + data.trackList[i].numPoints + '</td><td>' + data.trackList[i].len + '</td><td>' + data.trackList[i].loop + '</td></tr>';
                    $('#viewtable tbody').append(string);
                }

                $('.selectedfile h5').html('selected file: ' + content);
            },
            fail: function (error) {
                console.log(error);
            }
        });

    });

    // $('#viewtable tbody tr td:nth-child(1)').click(function (e) {
    //     var content = e.target.innerText;
    //     console.log(content);
    // })

    $(document).on('click', '#viewtable tbody tr td:nth-child(1)', function (e) {
        var content = e.target.innerText;
        var arr = content.split(" ");
        var type = arr[0];
        var index = parseInt(arr[1], 10) - 1;

        var fileString = $('.selectedfile h5').text();
        var temp = fileString.split(" ");
        var fileName = './uploads/' + temp[temp.length - 1];

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/otherDataTable',
            data: {
                type: type,
                index: index,
                fileName: fileName
            },
            success: function (data) {
                $('#componenttable tbody').empty();
                $('#routenameform .form-group label').text('Change ' + type + ' ' + (parseInt(index) + 1) + ' name:');
                for (var i = 0; i < data.GPXData.length; i++) {
                    var string = '<tr><td>' + data.GPXData[i].name + '</td><td>' + data.GPXData[i].value + '</td></tr>';
                    $('#componenttable tbody').append(string);
                }
            },
            fail: function (error) {
                console.log(error);
            }
        });

    });

    $('#routenameform').submit(function (e) {
        var name = $('#entryBox').val();
        var text = $('#routenameform .form-group label').text();
        text = text.split(' ');
        var type = text[1];
        var index = parseInt(text[2]) - 1;
        e.preventDefault();

        var fileString = $('.selectedfile h5').text();
        var temp = fileString.split(" ");
        var fileName = temp[temp.length - 1];
        if (text[0] === 'No') {
            alert("No component selected!");
        } else {
            var pathName = './uploads/' + temp[temp.length - 1];

            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/renameComponent',
                data: {
                    type: type,
                    index: index,
                    fileName: pathName,
                    newName: name
                },
                success: function (data) {
                    $.ajax({
                        type: 'get',
                        dataType: 'json',
                        url: '/viewPanel',
                        data: {
                            fileName: fileName,
                        },
                        success: function (data) {
                            $('#viewtable tbody').empty();

                            for (var i = 0; i < data.routeList.length; i++) {
                                var string = '<tr><td>' + 'Route ' + (i + 1) + '</td><td>' + data.routeList[i].name + '</td><td>' + data.routeList[i].numPoints + '</td><td>' + data.routeList[i].len + '</td><td>' + data.routeList[i].loop + '</td></tr>';
                                $('#viewtable tbody').append(string);
                            }

                            for (var i = 0; i < data.trackList.length; i++) {
                                var string = '<tr><td>' + 'Track ' + (i + 1) + '</td><td>' + data.trackList[i].name + '</td><td>' + data.trackList[i].numPoints + '</td><td>' + data.trackList[i].len + '</td><td>' + data.trackList[i].loop + '</td></tr>';
                                $('#viewtable tbody').append(string);
                            }

                            $('.selectedfile h5').html('selected file: ' + fileName);
                        },
                        fail: function (error) {
                            console.log(error);
                        }
                    });
                },
                fail: function (error) {
                    console.log(error);
                }
            });
        }

    });

    $('#getroutedetailsform').submit(function (e) {
        var latitude = $('#waypointLatBox').val();
        var longitude = $('#waypointLonBox').val();

        if (latitude < -90.0 || latitude > 90.0) {
            alert("Invalid Latitude Value!");
        } else if (longitude < -180.0 || longitude > 180.0) {
            alert("Invalid Longitude Value!");
        } else if (isNaN(latitude) || latitude === '') {
            alert("Invalid Latitude Value!");
        } else if (isNaN(longitude) || longitude === '') {
            alert("Invalid Longitude Value!");
        } else {
            latitude = parseFloat(latitude).toFixed(6);
            longitude = parseFloat(longitude).toFixed(6);
            $('#waypointtable tbody').append('<tr><td>' + latitude + '</td><td>' + longitude + '</td><tr>');
        }

        e.preventDefault();
    });

    $('#addrouteform').submit(function (e) {
        var waypoints = [];
        var routeName = $('#nameBox').val();

        $('#waypointtable tbody>tr').each(function () {
            $('td', this).each(function () {
                waypoints.push($(this).text());
            });
        });

        var fileString = $('.selectedfile h5').text();
        var temp = fileString.split(" ");
        var fileName = temp[temp.length - 1];
        var pathName = './uploads/' + temp[temp.length - 1];

        e.preventDefault();

        if (fileName === 'selected') {
            alert('No file selected!');
        } else {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/addRoute',
                data: {
                    waypoints: waypoints,
                    pathName: pathName,
                    routeName, routeName
                },
                success: function (data) {
                    $.ajax({
                        type: 'get',
                        dataType: 'json',
                        url: '/viewPanel',
                        data: {
                            fileName: fileName,
                        },
                        success: function (data) {
                            $('#viewtable tbody').empty();

                            for (var i = 0; i < data.routeList.length; i++) {
                                var string = '<tr><td>' + 'Route ' + (i + 1) + '</td><td>' + data.routeList[i].name + '</td><td>' + data.routeList[i].numPoints + '</td><td>' + data.routeList[i].len + '</td><td>' + data.routeList[i].loop + '</td></tr>';
                                $('#viewtable tbody').append(string);
                            }

                            for (var i = 0; i < data.trackList.length; i++) {
                                var string = '<tr><td>' + 'Track ' + (i + 1) + '</td><td>' + data.trackList[i].name + '</td><td>' + data.trackList[i].numPoints + '</td><td>' + data.trackList[i].len + '</td><td>' + data.trackList[i].loop + '</td></tr>';
                                $('#viewtable tbody').append(string);
                            }

                            $('.selectedfile h5').html('selected file: ' + fileName);
                        },
                        fail: function (error) {
                            console.log(error);
                        }
                    });
                },
                fail: function (error) {
                    console.log(error);
                }
            });
        }

    });

    $('#creategpxform').submit(async function (e) {
        //var routeName = $('#nameBox').val();
        var fileName = $('#fileNameBox').val();
        var version = $('#versionBox').val();
        var creator = $('#creatorBox').val();

        var pathName = './uploads/' + fileName;

        e.preventDefault();

        if (fileName.length < 5 || fileName.substring(fileName.length - 4, fileName.length) !== '.gpx') {
            alert('Improper File Name!');
        } else if (parseFloat(version) !== 1.1) {
            alert('Improper Version!');
        } else {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/createGPX',
                data: {
                    fileName: fileName,
                    pathName: pathName,
                    version: version,
                    creator: creator
                },
                success: function (data) {
                    if (data.res === "good") {
                        $.ajax({
                            type: 'get',
                            dataType: 'json',
                            url: '/getFiles',
                            success: function (data) {
                                $('#filetable tbody').empty();
                                for (var i = 0; i < data.fileNames.length; i++) {
                                    var anchor = '<a href="' + data.pathNames[i] + '" download ">' + data.fileNames[i] + '</a>';
                                    $('#filetable').append('<tr><td>' + anchor + '</td><td>' + data.GPXInfo[i].version + '</td><td>' + data.GPXInfo[i].creator + '</td><td>' + data.GPXInfo[i].numWaypoints + '</td><td>' + data.GPXInfo[i].numRoutes + '</td><td>' + data.GPXInfo[i].numTracks + '</td></tr>');
                                }
                            },
                            fail: function (error) {
                                console.log(error);
                            }
                        });
                    } else {
                        alert("File already exists!");
                    }
                },
                fail: function (error) {
                    console.log(error);
                }
            });
        }

    });

    $('#findpathform').submit(function (e) {
        var lat1 = $('#lat1Box').val();
        var lon1 = $('#lon1Box').val();
        var lat2 = $('#lat2Box').val();
        var lon2 = $('#lon2Box').val();
        var delta = $('#deltaBox').val();

        lat1 = parseFloat(lat1).toFixed(6);
        lon1 = parseFloat(lon1).toFixed(6);
        lat2 = parseFloat(lat2).toFixed(6);
        lon2 = parseFloat(lon2).toFixed(6);
        delta = parseFloat(delta);

        e.preventDefault();

        if (lat1 < -90.0 || lat1 > 90.0) {
            alert("Invalid Latitude 1 Value!");
        } else if (lon1 < -180.0 || lon1 > 180.0) {
            alert("Invalid Longitude 1 Value!");
        } else if (isNaN(lat1) || lat1 === '') {
            alert("Invalid Latitude 1 Value!");
        } else if (isNaN(lon1) || lon1 === '') {
            alert("Invalid Longitude 1 Value!");
        } else if (lat2 < -90.0 || lat2 > 90.0) {
            alert("Invalid Latitude 2 Value!");
        } else if (lon2 < -180.0 || lon2 > 180.0) {
            alert("Invalid Longitude 2 Value!");
        } else if (isNaN(lat2) || lat2 === '') {
            alert("Invalid Latitude 2 Value!");
        } else if (isNaN(lon2) || lon2 === '') {
            alert("Invalid Longitude 2 Value!");
        } else if (delta < 0) {
            alert("Invalid delta value!");
        } else {

            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/findPath',
                data: {
                    lat1: lat1,
                    lon1: lon1,
                    lat2: lat2,
                    lon2: lon2,
                    delta: delta
                },
                success: function (data) {
                    $('#pathstable tbody').empty();

                    var routeCounter = 1;
                    var trackCounter = 1;

                    // for (var i = 0; i < data.routeList.length; i++) {
                    //     var string = '<tr><td>' + 'Route ' + (i + 1) + '</td><td>' + data.routeList[i].name + '</td><td>' + data.routeList[i].numPoints + '</td><td>' + data.routeList[i].len + '</td><td>' + data.routeList[i].loop + '</td></tr>';
                    //     $('#pathstable tbody').append(string);
                    // }

                    // for (var i = 0; i < data.trackList.length; i++) {
                    //     var string = '<tr><td>' + 'Track ' + (i + 1) + '</td><td>' + data.trackList[i].name + '</td><td>' + data.trackList[i].numPoints + '</td><td>' + data.trackList[i].len + '</td><td>' + data.trackList[i].loop + '</td></tr>';
                    //     $('#pathstable tbody').append(string);
                    // }

                    for (var i = 0; i < data.routeList.length; i++) {
                        for (var j = 0; j < data.routeList[i].length; j++) {
                            var string = '<tr><td>' + 'Route ' + routeCounter + '</td><td>' + data.routeList[i][j].name + '</td><td>' + data.routeList[i][j].numPoints + '</td><td>' + data.routeList[i][j].len + '</td><td>' + data.routeList[i][j].loop + '</td></tr>';
                            $('#pathstable tbody').append(string);
                            routeCounter++;
                        }
                    }

                    for (var i = 0; i < data.trackList.length; i++) {
                        for (var j = 0; j < data.trackList[i].length; j++) {
                            var string = '<tr><td>' + 'Track ' + trackCounter + '</td><td>' + data.trackList[i][j].name + '</td><td>' + data.trackList[i][j].numPoints + '</td><td>' + data.trackList[i][j].len + '</td><td>' + data.trackList[i][j].loop + '</td></tr>';
                            $('#pathstable tbody').append(string);
                            trackCounter++;
                        }
                    }
                },
                fail: function (error) {
                    console.log(error);
                }
            });
        }
    });




});
