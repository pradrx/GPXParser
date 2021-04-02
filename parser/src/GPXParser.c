#include <stdlib.h>
#include "GPXParser.h"

/*
*  Code for parsing XML file was sourced from libXmlExample.c, provided by CIS*2750
*/

GPXdoc *parseGPXelement(GPXdoc *doc, xmlNode *cur_node) {
    GPXdoc *tempDoc = doc;

    xmlAttr *attr;

    for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);

        if (!(strcmp(attrName, "version"))) {
            double version = atof(cont);
            tempDoc->version = version;
        } else if (!(strcmp(attrName, "creator"))) {
            int len = strlen(cont) + 1;
            doc->creator = malloc(len);
            strcpy(tempDoc->creator, cont);
        }
    }

    strcpy(tempDoc->namespace, (char *)cur_node->ns->href);
    return tempDoc;
}

/**
 * Function to parse any Waypoint datatype given to it
 * Applicable to these tags: <wpt>, <rtept>, <trkpt>
 * Name value is stored in child nodes
 * Latitude and Longitude values stored in node->properties
 * Extra GPXData is stored in child nodes
 */

Waypoint *parseWPTelement(xmlNode *cur_node) {

    xmlAttr *attr;

    Waypoint *point = malloc(sizeof(Waypoint));
    point->name = NULL;

    point->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);

        if (!(strcmp(attrName, "lat"))) {
            point->latitude = atof(cont);
        } else if (!(strcmp(attrName, "lon"))) {
            point->longitude = atof(cont);
        }
    }

    xmlNode *child;

    for (child = cur_node->children; child != NULL; child = child->next) {

        if (child->type == XML_ELEMENT_NODE && !(strcmp((char *)child->name, "name"))) {
            int len = strlen((char *)child->children->content) + 1;
            point->name = malloc(len);
            strcpy(point->name, (char *)child->children->content);
        } 
        else if (child->type == XML_ELEMENT_NODE) {
            int len = strlen((char *)child->children->content) + 1;
            GPXData *extra_info = malloc(sizeof(GPXData) + (sizeof(char *) * len));
            strcpy(extra_info->name, (char *)child->name);
            strcpy(extra_info->value, (char *)child->children->content);
            insertBack(point->otherData, extra_info);
        }
    }

    if (point->name == NULL) {
        point->name = malloc(1);
        strcpy(point->name, "");
    }
    return point;
}

/**
 * Function to parse any Route datatype given to it
 * Applicable to these tags: <rte>
 * Name value is stored in child nodes
 * Waypoint values stored in child nodes (parsed with parseWPTelement)
 * Extra GPXData is stored in child nodes
 */

Route *parseRTEelement(xmlNode *cur_node) {

    Route *route =  malloc(sizeof(Route));
    route->name = NULL;

    route->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    route->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    xmlNode *child;

    for (child = cur_node->children; child != NULL; child = child->next) {
        if (child->type == XML_ELEMENT_NODE && !(strcmp((char *)child->name, "name"))) {
            int len = strlen((char *)child->children->content) + 1;
            route->name = malloc(len);
            strcpy(route->name, (char *)child->children->content);
        } else if (child->type == XML_ELEMENT_NODE && !(strcmp((char *)child->name, "rtept"))) {
            Waypoint *point = parseWPTelement(child);
            insertBack(route->waypoints, point);
        } else if (child->type == XML_ELEMENT_NODE) {
            int len = strlen((char *)child->children->content) + 1;
            GPXData *extra_info = malloc(sizeof(GPXData) + (sizeof(char *) * len));
            strcpy(extra_info->name, (char *)child->name);
            strcpy(extra_info->value, (char *)child->children->content);
            insertBack(route->otherData, extra_info);
        }
    }
    if (route->name == NULL) {
        route->name = malloc(1);
        strcpy(route->name, "");
    }
    return route;
}

/**
 * Function to parse any TrackSegment datatype given to it
 * Applicable to these tags: <trkseg>
 * Waypoint values stored in child nodes (parsed with parseWPTelement)
 */

TrackSegment *parseTRKSEGelement(xmlNode *cur_node) {

    TrackSegment *track_segment = malloc(sizeof(TrackSegment));

    track_segment->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    xmlNode *child;

    for (child = cur_node->children; child != NULL; child = child->next) {
        if (child->type == XML_ELEMENT_NODE && !(strcmp((char *)child->name, "trkpt"))) {
            Waypoint *point = parseWPTelement(child);
            insertBack(track_segment->waypoints, point);
        }
    }

    return track_segment;

}

/**
 * Function to parse any Track datatype given to it
 * Applicable to these tags: <trk>
 * Name value is stored in child nodes
 * Segment values stored in child nodes (parsed with parseTRKSEGelement)
 * Extra GPXData is stored in child nodes
 */

Track *parseTRKelement(xmlNode *cur_node) {

    Track *track = malloc(sizeof(Track));
    track->name = NULL;

    track->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    track->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    xmlNode *child;

    for (child = cur_node->children; child != NULL; child = child->next) {
        if (child->type == XML_ELEMENT_NODE && !(strcmp((char *)child->name, "name"))) {
            int len = strlen((char *)child->children->content) + 1;
            track->name = malloc(len);
            strcpy(track->name, (char *)child->children->content);
        } else if (child->type == XML_ELEMENT_NODE && !(strcmp((char *)child->name, "trkseg"))) {
            TrackSegment *track_segment = parseTRKSEGelement(child);
            insertBack(track->segments, track_segment);
        } else if (child->type == XML_ELEMENT_NODE) {
            int len = strlen((char *)child->children->content) + 1;
            GPXData *extra_info = malloc(sizeof(GPXData) + (sizeof(char *) * len));
            strcpy(extra_info->name, (char *)child->name);
            strcpy(extra_info->value, (char *)child->children->content);
            insertBack(track->otherData, extra_info);
        }
    }
    if (track->name == NULL) {
        track->name = malloc(1);
        strcpy(track->name, "");
    }
    return track;
}

/**
 * Function that recursively goes through the given XML doc given the root node
 * Goes to every tag in the xml structure
 * Parses the main tags <gpx>, <wpt>, <rte>, <trk>
 * Child tags are parsed through additional helper methods
 */

void parseXMLdoc(xmlNode *root_node, GPXdoc *doc) {
    xmlNode *cur_node = NULL;
    for (cur_node = root_node; cur_node != NULL; cur_node = cur_node->next) {
        if (!(strcmp((char *)cur_node->name, "gpx"))) {
            doc = parseGPXelement(doc, cur_node);
        } else if (!(strcmp((char *)cur_node->name, "wpt"))) {
            Waypoint *point = parseWPTelement(cur_node);
            insertBack(doc->waypoints, point);
        } else if (!(strcmp((char *)cur_node->name, "rte"))) {
            Route *route = parseRTEelement(cur_node);
            insertBack(doc->routes, route);
        } 
        else if (!(strcmp((char *)cur_node->name, "trk"))) {
            Track *track = parseTRKelement(cur_node);
            insertBack(doc->tracks, track);
        }
        
        parseXMLdoc(cur_node->children, doc);
    }
}

/**
 * Function to parse the GPXdoc with given file name
 * Makes use of helper functions to routinely parse every element in the XML doc
 */

GPXdoc *createGPXdoc(char *fileName) {
    GPXdoc *doc = malloc(sizeof(GPXdoc));

    xmlDoc *xmlDoc = xmlReadFile(fileName, NULL, 0);

    if (xmlDoc == NULL) {
        free(doc);
        return NULL;
    }
    
    xmlNode *root_element = xmlDocGetRootElement(xmlDoc);

    List *waypoint_list = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    List *route_list = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    List *track_list = initializeList(&trackToString, &deleteTrack, &compareTracks);


    doc->waypoints = waypoint_list;
    doc->routes = route_list;
    doc->tracks = track_list;

    parseXMLdoc(root_element, doc);

    xmlFreeDoc(xmlDoc);
    xmlCleanupParser();

    return doc;

}

char* GPXdocToString(GPXdoc* doc) {
    int len = strlen(doc->namespace) + 100 + strlen(doc->creator);

    char *retVal = malloc(len);
    sprintf(retVal, "GPXDOC | Namespace: %s, Version: %.1f, Creator: %s\n", doc->namespace, doc->version, doc->creator);
    return retVal;
}

void deleteGPXdoc(GPXdoc* doc) {
    if (doc != NULL) {
        if (doc->waypoints != NULL) {
            freeList(doc->waypoints);
        }
        if (doc->routes != NULL) {
            freeList(doc->routes);
        }
        if (doc->tracks != NULL) {
            freeList(doc->tracks);
        }
        free(doc->creator);
        free(doc);
    }
}

int getNumWaypoints(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    int num_waypoints = getLength(doc->waypoints);
    return num_waypoints;
}

int getNumRoutes(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    int num_routes = getLength(doc->routes);
    return num_routes;
}

int getNumTracks(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    int num_tracks = getLength(doc->tracks);
    return num_tracks;
}

int getNumSegments(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    Node *head = doc->tracks->head;
    Node *ptr;

    int num_segments = 0;

    for (ptr = head; ptr != NULL; ptr = ptr->next) {
        Track *track = (Track *)ptr->data;
        num_segments += getLength(track->segments);
    }
    return num_segments;
}

int getNumWaypointGPXData(Waypoint *point) {
    int num_gpx_data = 0;
    if (strcmp(point->name, "")) {
        num_gpx_data++;
    }
    num_gpx_data += getLength(point->otherData);
    return num_gpx_data;
}

int getNumRouteGPXData(Route *route) {
    int num_gpx_data = 0;
    if (strcmp(route->name, "")) {
        num_gpx_data++;
    }
    num_gpx_data += getLength(route->otherData);
    
    Node *head = route->waypoints->head;
    Node *ptr;

    for (ptr = head; ptr != NULL; ptr = ptr->next) {
        Waypoint *point = (Waypoint *)ptr->data;
        num_gpx_data += getNumWaypointGPXData(point);
    }

    return num_gpx_data;
}

int getNumTrackGPXData(Track *track) {
    int num_gpx_data = 0;
    if (strcmp(track->name, "")) {
        num_gpx_data++;
    }
    num_gpx_data += getLength(track->otherData);

    Node *head1 = track->segments->head;
    Node *ptr1;

    for (ptr1 = head1; ptr1 != NULL; ptr1 = ptr1->next) {
        TrackSegment *track_segment = (TrackSegment *)ptr1->data;
        Node *head2 = track_segment->waypoints->head;
        Node *ptr2;
        for (ptr2 = head2; ptr2 != NULL; ptr2 = ptr2->next) {
            Waypoint *point = (Waypoint *)ptr2->data;
            num_gpx_data += getNumWaypointGPXData(point);
        }
    }
    return num_gpx_data;
}

int getNumGPXData(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }

    int num_gpx_data = 0;

    Node *head1 = doc->waypoints->head;
    Node *ptr1;

    for (ptr1 = head1; ptr1 != NULL; ptr1 = ptr1->next) {
        Waypoint *point = (Waypoint *)ptr1->data;
        num_gpx_data += getNumWaypointGPXData(point);
    }

    Node *head2 = doc->routes->head;
    Node *ptr2;

    for (ptr2 = head2; ptr2 != NULL; ptr2 = ptr2->next) {
        Route *route = (Route *)ptr2->data;
        num_gpx_data += getNumRouteGPXData(route);
    }

    Node *head3 = doc->tracks->head;
    Node *ptr3;

    for (ptr3 = head3; ptr3 != NULL; ptr3 = ptr3->next) {
        Track *track = (Track *)ptr3->data;
        num_gpx_data += getNumTrackGPXData(track);
    }

    return num_gpx_data;    
}

Waypoint* getWaypoint(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return NULL;
    }
    Node *head = doc->waypoints->head;
    Node *ptr;

    for (ptr = head; ptr != NULL; ptr = ptr->next) {
        Waypoint *point = (Waypoint *)ptr->data;
        if (!(strcmp(point->name, name))) {
            return point;
        }
    }
    return NULL;
}

Track* getTrack(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return NULL;
    }
    Node *head = doc->tracks->head;
    Node *ptr;

    for (ptr = head; ptr != NULL; ptr = ptr->next) {
        Track *track = (Track *)ptr->data;
        if (!(strcmp(track->name, name))) {
            return track;
        }
    }
    return NULL;
}

Route* getRoute(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return NULL;
    }
    Node *head = doc->routes->head;
    Node *ptr;

    for (ptr = head; ptr != NULL; ptr = ptr->next) {
        Route *route = (Route *)ptr->data;
        if (!(strcmp(route->name, name))) {
            return route;
        }
    }
    return NULL;
}

void deleteWaypoint(void* data) {
    if (data != NULL) {
        Waypoint *point = (Waypoint *)data;
        free(point->name);
        freeList(point->otherData);
        free(point);
    }
}

char* waypointToString( void* data) {
    Waypoint *point = (Waypoint *)data;
    int len = 10000;
    char *retVal = malloc(len);
    sprintf(retVal, "WAYPOINT | Name: %s, Latitude: %f, Longitude: %f\n", point->name, point->latitude, point->longitude);
    char *other_data = toString(point->otherData);
    strcat(retVal, other_data);
    free(other_data);
    return retVal;
}

int compareWaypoints(const void *first, const void *second) {
    Waypoint *first_point = (Waypoint *)first;
    Waypoint *second_point = (Waypoint *)second;

    return strcmp((char*)first_point->name, (char*)second_point->name);
}

void deleteGpxData( void* data) {
    if (data != NULL) {
        GPXData *gpx_data = (GPXData *)data;
        free(gpx_data);
    }
}

char* gpxDataToString( void* data) {
    GPXData *gpx_data = (GPXData *)data;
    int len = 10000;
    char *retVal = malloc(len);
    sprintf(retVal, "GPXDATA | Name: %s, Value: %s\n", gpx_data->name, gpx_data->value);
    return retVal;
}

int compareGpxData(const void *first, const void *second) {
    GPXData *first_gpx_data = (GPXData *)first;
    GPXData *second_gpx_data = (GPXData *)second;

    return strcmp((char*)first_gpx_data->name, (char*)second_gpx_data->name);
}

void deleteRoute(void* data) {
    if (data != NULL) {
        Route *route = (Route *)data;
        free(route->name);
        freeList(route->waypoints);
        freeList(route->otherData);
        free(route);
    }
}

void dummyDeleteRoute(void *data) {
    if (data == NULL) {
        return;
    }
}

char* routeToString(void* data) {
    Route *route = (Route *)data;
    int len = 10000;
    char *retVal = malloc(len);
    sprintf(retVal, "ROUTE | Name: %s\n", route->name);
    char *waypoints = toString(route->waypoints);
    char *other_data = toString(route->otherData);
    strcat(retVal, waypoints);
    strcat(retVal, other_data);
    free(waypoints);
    free(other_data);
    return retVal;
}

int compareRoutes(const void *first, const void *second) {
    Route *first_route = (Route *)first;
    Route *second_route = (Route *)second;

    return strcmp((char*)first_route->name, (char*)second_route->name);
}

void deleteTrackSegment(void* data) {
    if (data != NULL) {
        TrackSegment *track_segment = (TrackSegment *)data;
        freeList(track_segment->waypoints);
        free(track_segment);
    }
}

char* trackSegmentToString(void* data) {
    TrackSegment *track_segment = (TrackSegment *)data;
    int len = 10000;
    char *retVal = malloc(len);
    sprintf(retVal, "TRACK SEGMENT\n");
    char *waypoints = toString(track_segment->waypoints);
    strcat(retVal, waypoints);
    free(waypoints);
    return retVal;
}

int compareTrackSegments(const void *first, const void *second) {
    return 0;
}

void deleteTrack(void* data) {
    if (data != NULL) {
        Track *track = (Track *)data;
        free(track->name);
        freeList(track->otherData);
        freeList(track->segments);
        free(track);
    }
}

void dummyDeleteTrack(void *data) {
    if (data == NULL) {
        return;
    }
}

char* trackToString(void* data) {
    Track *track = (Track *)data;
    int len = 10000;
    char *retVal = malloc(len);
    sprintf(retVal, "TRACK | Name: %s\n", track->name);
    char *track_segments = toString(track->segments);
    char *other_data = toString(track->otherData);
    strcat(retVal, track_segments);
    strcat(retVal, other_data);
    free(track_segments);
    free(other_data);
    return retVal;
}

int compareTracks(const void *first, const void *second) {
    Track *first_track = (Track *)first;
    Track *second_track = (Track *)second;
    return strcmp((char*)first_track->name, (char*)second_track->name);
}

/**
 * Assignment 2 Module 1 Functions
 */

/**
 * Helper function for validating a doc explicity against a schema file
 * Returns true if it passes the schema test
 * Returns false if arguments or null or it fails the schema test
 */

bool validDoc(xmlDocPtr doc, char *gpxSchemaFile) {
    if (gpxSchemaFile == NULL || !strcmp(gpxSchemaFile, "")) {
        return false;
    }

    xmlSchemaParserCtxtPtr ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaPtr schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    if (doc == NULL || schema == NULL) {
        if (schema != NULL) {
            xmlSchemaFree(schema);
        }
        return false;
    }

    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);

    if (xmlSchemaValidateDoc(valid_ctxt, doc) != 0) {
        xmlSchemaFree(schema);
        xmlSchemaFreeValidCtxt(valid_ctxt);
        return false;
    }

    xmlSchemaFree(schema);
    xmlSchemaFreeValidCtxt(valid_ctxt);

    return true;
}

/**
 * Main function for creating a validated GPXdoc struct
 * Returns a GPXdoc struct if it passes schema validation and arguments are not NULL 
 */

GPXdoc *createValidGPXdoc(char *fileName, char *gpxSchemaFile) {
    xmlDocPtr xmlDoc = xmlReadFile(fileName, NULL, 0);
    if (xmlDoc == NULL || validDoc(xmlDoc, gpxSchemaFile) == false) {
        xmlFreeDoc(xmlDoc);
        xmlCleanupParser();
        return NULL;
    }

    GPXdoc *doc = createGPXdoc(fileName);
    xmlFreeDoc(xmlDoc);
    return doc;
}

/**
 * Helper function to make waypoint tags for XML tree
 */

void makePointTags(Node *temp, xmlNodePtr parent_node, char *tag) {
    for (Node *point_head = temp; point_head != NULL; point_head = point_head->next) {
        Waypoint *point = (Waypoint *)point_head->data;
        xmlNodePtr waypoint_node = xmlNewChild(parent_node, NULL, BAD_CAST tag, NULL);
        char latitude[50];
        char longitude[50];
        sprintf(latitude, "%f", point->latitude);
        sprintf(longitude, "%f", point->longitude);
        xmlNewProp(waypoint_node, BAD_CAST "lat", BAD_CAST latitude);
        xmlNewProp(waypoint_node, BAD_CAST "lon", BAD_CAST longitude);

        if (strcmp(point->name, "")) {
            xmlNewChild(waypoint_node, NULL, BAD_CAST "name", BAD_CAST point->name);
        }

        for (Node *od_head = point->otherData->head; od_head != NULL; od_head = od_head->next) {
            GPXData *other_data = (GPXData *)od_head->data;
           xmlNewChild(waypoint_node, NULL, BAD_CAST other_data->name, BAD_CAST other_data->value);
        }
    }
}

/**
 * Helper function that creates an XML tree given a GPX doc
 */

xmlDocPtr createXMLdoc(GPXdoc *doc) {
    xmlDocPtr xmlDoc = xmlNewDoc(BAD_CAST "1.0");

    xmlNodePtr gpx_node = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(xmlDoc, gpx_node);

    char version[50];
    sprintf(version, "%.1f", doc->version);

    // GPX tag
    
    xmlNewProp(gpx_node, BAD_CAST "version", BAD_CAST version);
    xmlNewProp(gpx_node, BAD_CAST "creator", BAD_CAST doc->creator);

    xmlNsPtr ns = xmlNewNs(gpx_node, BAD_CAST doc->namespace, NULL);
    xmlSetNs(gpx_node, ns);

    makePointTags(doc->waypoints->head, gpx_node, "wpt");

    for (Node *route_head = doc->routes->head; route_head != NULL; route_head = route_head->next) {
        Route *route = (Route *)route_head->data;
        xmlNodePtr route_node = xmlNewChild(gpx_node, NULL, BAD_CAST "rte", NULL);
        if (strcmp(route->name, "")) {
            xmlNewChild(route_node, NULL, BAD_CAST "name", BAD_CAST route->name);
        }
        
        for (Node *od_head = route->otherData->head; od_head != NULL; od_head = od_head->next) {
            GPXData *other_data = (GPXData *)od_head->data;
            xmlNewChild(route_node, NULL, BAD_CAST other_data->name, BAD_CAST other_data->value);
        }

        makePointTags(route->waypoints->head, route_node, "rtept");
    }

    for (Node *track_head = doc->tracks->head; track_head != NULL; track_head = track_head->next) {
        Track *track = (Track *)track_head->data;
        xmlNodePtr track_node = xmlNewChild(gpx_node, NULL, BAD_CAST "trk", NULL);
        if (strcmp(track->name, "")) {
            xmlNewChild(track_node, NULL, BAD_CAST "name", BAD_CAST track->name);
        }

        for (Node *od_head = track->otherData->head; od_head != NULL; od_head = od_head->next) {
            GPXData *other_data = (GPXData *)od_head->data;
            xmlNewChild(track_node, NULL, BAD_CAST other_data->name, BAD_CAST other_data->value);
        }

        for (Node *seg_head = track->segments->head; seg_head != NULL; seg_head = seg_head->next) {
            TrackSegment *trkseg = (TrackSegment *)seg_head->data;
            xmlNodePtr seg_node = xmlNewChild(track_node, NULL, BAD_CAST "trkseg", NULL);
            makePointTags(trkseg->waypoints->head, seg_node, "trkpt");
        }
    }

    return xmlDoc;
}

/**
 * Main function that takes a GPXdoc struct and creates a corresponding XMl tree
 * The XML tree is saved to a file name that is passed in
 */

bool writeGPXdoc(GPXdoc *doc, char *fileName) {
    xmlDocPtr xmlDoc = NULL;

    if (doc == NULL || fileName == NULL || !(strcmp(fileName, ""))) {
        return false;
    }

    xmlDoc = createXMLdoc(doc);

    xmlSaveFormatFileEnc(fileName , xmlDoc, "UTF-8", 1);
    xmlFreeDoc(xmlDoc);
    return true;
}

/**
 * Helper function to validate a GPXData struct
 */

bool validateGPXData(GPXData *other_data) {
    if (other_data == NULL || !strcmp(other_data->name, "") || !strcmp(other_data->value, "")) {
        return false;
    }
    return true;
}

/**
 * Helper function to validate a Waypoint struct
 */

bool validateWaypoint(Waypoint *point) {
    if (point == NULL || point->name == NULL) {
        return false;
    }

    for (Node *od_head = point->otherData->head; od_head != NULL; od_head = od_head->next) {
        GPXData *other_data = (GPXData *)od_head->data;
        if (validateGPXData(other_data) == false) {
            return false;
        }
    }
    return true;
}

/**
 * Helper function to validate a Route struct
 */

bool validateRoute(Route *route) {
    if (route == NULL || route->name == NULL) {
        return false;
    }

    for (Node *point_head = route->waypoints->head; point_head != NULL; point_head = point_head->next) {
        Waypoint *point = (Waypoint *)point_head->data;
        if (validateWaypoint(point) == false) {
            return false;
        }
    }

    for (Node *od_head = route->otherData->head; od_head != NULL; od_head = od_head->next) {
        GPXData *other_data = (GPXData *)od_head->data;
        if (validateGPXData(other_data) == false) {
            return false;
        }
    }
    return true;
}

/**
 * Helper function to validate a Track struct
 */

bool validateTrack(Track *track) {
    if (track == NULL || track->name == NULL) {
        return false;
    }

    for (Node *seg_head = track->segments->head; seg_head != NULL; seg_head = seg_head->next) {
        TrackSegment *trkseg = (TrackSegment *)seg_head->data;
        for (Node *point_head = trkseg->waypoints->head; point_head != NULL; point_head = point_head->next) {
            Waypoint *point = (Waypoint *)point_head->data;
            if (validateWaypoint(point) == false) {
                return false;
            }
        }
    }

    for (Node *od_head = track->otherData->head; od_head != NULL; od_head = od_head->next) {
        GPXData *other_data = (GPXData *)od_head->data;
        if (validateGPXData(other_data) == false) {
            return false;
        }
    }
    return true;
}

/**
 * Main function to validate a GPXdoc struct
 */

bool validateGPXDoc(GPXdoc *doc, char *gpxSchemaFile) {
    if (doc == NULL) {
        return false;
    }

    if (!strcmp(doc->namespace, "")) {
        return false;
    }
    if (doc->creator == NULL || !strcmp(doc->creator, "")) {
        return false;
    }

    for (Node *point_head = doc->waypoints->head; point_head != NULL; point_head = point_head->next) {
        Waypoint *point = (Waypoint *)point_head->data;
        if (validateWaypoint(point) == false) {
            return false;
        }
    }

    for (Node *route_head = doc->routes->head; route_head != NULL; route_head = route_head->next) {
        Route *route = (Route *)route_head->data;
        if (validateRoute(route) == false) {
            return false;
        }
    }

    for (Node *track_head = doc->tracks->head; track_head != NULL; track_head = track_head->next) {
        Track *track = (Track *)track_head->data;
        if (validateTrack(track) == false) {
            return false;
        }
    }

    xmlDocPtr xmlDoc = createXMLdoc(doc);

    if (validDoc(xmlDoc, gpxSchemaFile) == false) {
        return false;
    }

    xmlFreeDoc(xmlDoc);

    return true;
}

/**
 * Assignment 2 Module 2 Functions
 */

/**
 * Main function to round any float to the nearest 10th place
 */

float round10(float len) {
    if (len < 0) {
        return len;
    }
    int length = (int)len;
    int difference = length % 10;

    int rounded;

    if (difference >= 5) {
        rounded = len - difference + 10;
    } else {
        rounded = len - difference;
    }
    return (float)rounded;
}

/**
 * Helper function that calculates the distance between two waypoints
 */

float haversine(Waypoint *point1, Waypoint *point2) {
    float radius = 6371000;

    if (point1 == NULL || point2 == NULL) {
        return 0;
    }

    float lat1 = point1->latitude;
    float lat2 = point2->latitude;
    float lon1 = point1->longitude;
    float lon2 = point2->longitude;

    float angle1 = lat1 * (M_PI / 180);
    float angle2 = lat2 * (M_PI / 180);

    float delta_lat = (lat2 - lat1) * (M_PI / 180);
    float delta_lon = (lon2 - lon1) * (M_PI / 180);

    float a = sin(delta_lat / 2) * sin(delta_lat / 2) + cos(angle1) * cos(angle2) * sin(delta_lon / 2) * sin(delta_lon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    float d = radius * c;
    return d;
}

/**
 * Main function that calculates the length of a route using Haversine formula
 */

float getRouteLen(const Route *rt) {
    if (rt == NULL|| rt->waypoints->head == NULL) {
        return 0;
    }
    float total_distance = 0;
    for (Node *point_head = rt->waypoints->head; point_head->next != NULL; point_head = point_head->next) {
        Waypoint *point1 = (Waypoint *)point_head->data;
        Waypoint *point2 = (Waypoint *)point_head->next->data;
        total_distance += haversine(point1, point2);
    }
    return total_distance;
}

/**
 * Main function that calculates the length of a track using Haversine formula
 */

float getTrackLen(const Track *tr) {
    if (tr == NULL) {
        return 0;
    }
    float total_distance = 0;
    for (Node *seg_head = tr->segments->head; seg_head != NULL; seg_head = seg_head->next) {
        TrackSegment *trkseg = (TrackSegment *)seg_head->data;
        for (Node *point_head = trkseg->waypoints->head; point_head->next != NULL; point_head = point_head->next) {
            Waypoint *point1 = (Waypoint *)point_head->data;
            Waypoint *point2 = (Waypoint *)point_head->next->data;

            total_distance += haversine(point1, point2);
        }
    }

    for (Node *seg_head = tr->segments->head; seg_head->next != NULL; seg_head = seg_head->next) {
        TrackSegment *trkseg1 = (TrackSegment *)seg_head->data;
        TrackSegment *trkseg2 = (TrackSegment *)seg_head->next->data;
        Waypoint *point1;
        Waypoint *point2;
        Node *point_head = trkseg1->waypoints->head;
        while (point_head->next != NULL) {
            point_head = point_head->next;
        }
        point1 = (Waypoint *)point_head->data;
        point2 = (Waypoint *)trkseg2->waypoints->head->data;

        total_distance += haversine(point1, point2);
    }
    return total_distance;
}

/**
 * Main function that returns the number of routes that are within a given delta of a given length
 */

int numRoutesWithLength(const GPXdoc *doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return 0;
    }

    int num_routes = 0;

    for (Node *route_head = doc->routes->head; route_head != NULL; route_head = route_head->next) {
        Route *route = (Route *)route_head->data;
        float route_len = getRouteLen(route);
        if (fabs(route_len - len) <= delta) {
            num_routes++;
        }
    }
    return num_routes;
}

/**
 * Main function that returns the number of tracks that are within a given delta of a given length
 */

int numTracksWithLength(const GPXdoc *doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return 0;
    }

    int num_tracks = 0;

    for (Node *track_head = doc->tracks->head; track_head != NULL; track_head = track_head->next) {
        Track *track = (Track *)track_head->data;
        float track_len = getTrackLen(track);
        if (fabs(track_len - len) <= delta) {
            num_tracks++;
        }
    }
    return num_tracks;
}

/**
 * Main function that returns whether or not a route is going in a loop
 */

bool isLoopRoute(const Route *rt, float delta) {
    if (rt == NULL || delta < 0) {
        return false;
    }

    if (getLength(rt->waypoints) < 4) {
        return false;
    }

    Waypoint *point1 = (Waypoint *)getFromFront(rt->waypoints);
    Waypoint *point2 = (Waypoint *)getFromBack(rt->waypoints);

    float distance = haversine(point1, point2);

    if (distance < delta) {
        return true;
    }

    return false;
}

/**
 * Main functions that return whether or not a track is going in a loop
 */

bool isLoopTrack(const Track *tr, float delta) {
    if (tr == NULL || delta < 0) {
        return false;
    }

    int total_points = 0;

    for (Node *seg_head = tr->segments->head; seg_head != NULL; seg_head = seg_head->next) {
        TrackSegment *trkseg = (TrackSegment *)seg_head->data;
        total_points += getLength(trkseg->waypoints);
    }

    if (total_points < 4) {
        return false;
    }


    TrackSegment *trkseg1 = getFromFront(tr->segments);
    TrackSegment *trkseg2 = getFromBack(tr->segments);

    Waypoint *point1 = getFromFront(trkseg1->waypoints);
    Waypoint *point2 = getFromBack(trkseg2->waypoints);

    float distance = haversine(point1, point2);

    if (distance < delta) {
        return true;
    }

    return false;
}

/**
 * Main function that returns a list of routes that are between two given points
 */

List *getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    if (doc == NULL) {
        return NULL;
    }

    List *route_list = initializeList(&routeToString, &dummyDeleteRoute, &compareRoutes);
    
    for (Node *route_head = doc->routes->head; route_head != NULL; route_head = route_head->next) {
        Route *route = (Route *)route_head->data;
        Waypoint *point1 = getFromFront(route->waypoints);
        Waypoint *point2 = getFromBack(route->waypoints);

        if (point1 != NULL && point2 != NULL && route->waypoints->length >= 2) {
            Waypoint source_point;
            Waypoint dest_point;
            source_point.latitude = sourceLat;
            source_point.longitude = sourceLong;
            dest_point.latitude = destLat;
            dest_point.longitude = destLong;

            float d1 = haversine(&source_point, point1);
            float d2 = haversine(&dest_point, point2);

            if (d1 <= delta && d2 <= delta) {
                insertBack(route_list, route);
            }
        }
    }

    if (route_list->length == 0) {
        return NULL;
    }

    return route_list;
}

/**
 * Main function that returns a list of tracks that are between two given points
 */

List *getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    if (doc == NULL) {
        return NULL;
    }
    List *track_list = initializeList(&routeToString, &dummyDeleteTrack, &compareRoutes);

    for (Node *track_head = doc->tracks->head; track_head != NULL; track_head = track_head->next) {
        Track *track = (Track *)track_head->data;
        TrackSegment *trkseg1 = getFromFront(track->segments);
        TrackSegment *trkseg2 = getFromBack(track->segments);

        Waypoint *point1 = getFromFront(trkseg1->waypoints);
        Waypoint *point2 = getFromBack(trkseg2->waypoints);

        if (point1 != NULL && point2 != NULL) {
            Waypoint source_point;
            Waypoint dest_point;
            source_point.latitude = sourceLat;
            source_point.longitude = sourceLong;
            dest_point.latitude = destLat;
            dest_point.longitude = destLong;

            float d1 = haversine(&source_point, point1);
            float d2 = haversine(&dest_point, point2);

            if (d1 <= delta && d2 <= delta) {
                insertBack(track_list, track);
            }
        }
    }

    if (track_list->length == 0) {
        return NULL;
    }

    return track_list;
}

/**
 * Assignment 2 Module 3 Functions
 */

/**
 * Main function that converts a Route struct to JSON
 */

char *routeToJSON(const Route *rt) {
    int len = 1024;
    char *ret_val = malloc(len);
    if (rt == NULL) {
        strcpy(ret_val, "{}");
        return ret_val;
    }
    char name[100];
    if (!strcmp(rt->name, "")) {
        strcpy(name, "None");
    } else {
        strcpy(name, rt->name);
    }
    int num_points = getLength(rt->waypoints);
    float route_length = round10(getRouteLen(rt));
    char loop_stat[10];
    if (isLoopRoute(rt, 10) == true) { // find what delta value is supposed to be
        strcpy(loop_stat, "true");
    } else {
        strcpy(loop_stat, "false");
    }

    sprintf(ret_val, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.1f,\"loop\":%s}", name, num_points, route_length, loop_stat);
    return ret_val;
}

/**
 * Main function that converts a Track struct to JSON
 */

char *trackToJSON(const Track *tr) {
    int len = 1024;
    char *ret_val = malloc(len);
    if (tr == NULL) {
        strcpy(ret_val, "{}");
        return ret_val;
    }
    char name[100];
    if (!strcmp(tr->name, "")) {
        strcpy(name, "None");
    } else {
        strcpy(name, tr->name);
    }
    float track_length = round10(getTrackLen(tr));
    char loop_stat[10];
    if (isLoopTrack(tr, 10) == true) {
        strcpy(loop_stat, "true");
    } else {
        strcpy(loop_stat, "false");
    }

    int num_points = 0;

    for (Node *head = tr->segments->head; head != NULL; head = head->next) {
        TrackSegment *trkseg = (TrackSegment *)head->data;
        num_points += getLength(trkseg->waypoints);
    }

    sprintf(ret_val, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.1f,\"loop\":%s}", name, num_points, track_length, loop_stat);
    return ret_val;
}

/**
 * Main function that converts a List struct that contains routes to JSON
 */

char *routeListToJSON(const List *routeList) {
    int len = 10000;
    char *ret_val = malloc(len);
    if (routeList == NULL || routeList->length == 0) {
        strcpy(ret_val, "[]");
        return ret_val;
    }
    strcpy(ret_val, "[");
    char *append_val;

    Node *route_ptr;
    for (route_ptr = routeList->head; route_ptr->next != NULL; route_ptr = route_ptr->next) {
        Route *route = (Route *)route_ptr->data;
        append_val = routeToJSON(route);
        strcat(ret_val, append_val);
        strcat(ret_val, ",");
        free(append_val);
    }
    append_val = routeToJSON((Route *)route_ptr->data);
    strcat(ret_val, append_val);
    strcat(ret_val, "]");
    free(append_val);
    return ret_val;
}

/**
 * Main function that converts a List struct that contains tracks to JSON
 */

char *trackListToJSON(const List *trackList) {
    int len = 10000;
    char *ret_val = malloc(len);
    if (trackList == NULL || trackList->length == 0) {
        strcpy(ret_val, "[]");
        return ret_val;
    }
    strcpy(ret_val, "[");
    char *append_val;

    Node *track_ptr;
    for (track_ptr = trackList->head; track_ptr->next != NULL; track_ptr = track_ptr->next) {
        Track *track = (Track *)track_ptr->data;
        append_val = trackToJSON(track);
        strcat(ret_val, append_val);
        strcat(ret_val, ",");
        free(append_val);
    }
    append_val = trackToJSON((Track *)track_ptr->data);
    strcat(ret_val, append_val);
    strcat(ret_val, "]");
    free(append_val);
    return ret_val;
}

/**
 * Main function that converts a GPXdoc struct to JSON
 */

char *GPXtoJSON(const GPXdoc *gpx) {
    int len = 10000;
    char *ret_val = malloc(len);
    if (gpx == NULL) {
        strcpy(ret_val, "{}");
        return ret_val;
    }

    float version = gpx->version;
    char *creator = gpx->creator;
    int num_waypoints = gpx->waypoints->length;
    int num_routes = gpx->routes->length;
    int num_tracks = gpx->tracks->length;

    sprintf(ret_val, "{\"version\":%0.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", version, creator, num_waypoints, num_routes, num_tracks);
    return ret_val;
}

/**
 * A2 Bonus Functions 
 */

/**
 * Main function that adds a Waypoint to a Route struct
 */

void addWaypoint(Route *rt, Waypoint *pt) {
    if (rt == NULL || pt == NULL) {
        return;
    }
    insertBack(rt->waypoints, pt);
}

/**
 * Main function that adds a Route to a GPXdoc struct
 */

void addRoute(GPXdoc *doc, Route *rt) {
    if (doc == NULL || rt == NULL) {
        return;
    }
    insertBack(doc->routes, rt);
}

/**
 * Main function that converts a JSON to a GPXdoc struct
 */

GPXdoc *JSONtoGPX(const char *gpxString) {
    if (gpxString == NULL) {
        return NULL;
    }
    GPXdoc *doc = malloc(sizeof(GPXdoc));

    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");
    
    if (gpxString == NULL) {
        return NULL;
    }

    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    doc->creator = malloc(1000);

    char *string = malloc(10000);
    strcpy(string, gpxString);

    char *delim = "{}\":,";
    
    char *ptr = strtok(string, delim);

    for (int i = 0; i < 3; i++) {
        ptr = strtok(NULL, delim);
        if (i == 0) {
            doc->version=atof(ptr);
        } else if (i == 2) {
            strcpy(doc->creator, ptr);
        }
    }

    free(string);

    return doc;
}

/**
 * Main function that converts a JSON to a Waypoint struct
 */

Waypoint *JSONtoWaypoint(const char* gpxString) {
    if (gpxString == NULL) {
        return NULL;
    }
    Waypoint *point = malloc(sizeof(Waypoint));

    point->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    point->name = malloc(1000);
    strcpy(point->name, "");

    char *string = malloc(10000);
    strcpy(string, gpxString);

    char *delim = "{}\":,";

    char *ptr = strtok(string, delim);

    for (int i = 0; i < 3; i++) {
        ptr = strtok(NULL, delim);
        if (i == 0) {
            point->latitude = atof(ptr);
        } else if (i == 2) {
            point->longitude = atof(ptr);
        }
    }
    free(string);
    return point;
}

/**
 * Main function that converts a JSON to a Route struct
 */

Route *JSONtoRoute(const char* gpxString) {
    if (gpxString == NULL) {
        return NULL;
    }
    Route *route = malloc(sizeof(Route));

    route->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    route->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    route->name = malloc(1000);

    char *string = malloc(10000);
    strcpy(string, gpxString);

    char *delim = "{}\":,";

    char *ptr = strtok(string, delim);
    ptr = strtok(NULL, delim);

    strcpy(route->name, ptr);

    free(string);
    return route;
}

/**
 * A3 Helper Functions
 * Transitioning C to JavaScript
 */

char *GPXFileToJSON(char *file_name) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    char *json = GPXtoJSON(doc);
    deleteGPXdoc(doc);
    return json;
}

char *GPXFileRoutes(char *file_name) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    char *route_list = routeListToJSON(doc->routes);
    deleteGPXdoc(doc);
    return route_list;
}

char *GPXFileTracks(char *file_name) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    char *track_list = trackListToJSON(doc->tracks);
    deleteGPXdoc(doc);
    return track_list;
}

char *GPXDataToJSON(GPXData *data) {
    char *ret_val = malloc(1000);
    char *name = data->name;
    char *value = data->value;

    if (data == NULL) {
        strcpy(ret_val, "{}");
        return ret_val;
    }

    sprintf(ret_val, "{\"name\":\"%s\", \"value\":\"%s\"}", name, value);
    return ret_val;
}

char *GPXDataListToJSON(List *otherData) {
    char *ret_val = malloc(10000);
    if (otherData == NULL || otherData->length == 0) {
        strcpy(ret_val, "[]");
        return ret_val;
    }
    strcpy(ret_val, "[");
    char *append_val;

    Node *data_ptr;
    for (data_ptr = otherData->head; data_ptr->next != NULL; data_ptr = data_ptr->next) {
        GPXData *data = (GPXData *)data_ptr->data;
        append_val = GPXDataToJSON(data);
        strcat(ret_val, append_val);
        strcat(ret_val, ",");
        free(append_val);
    }
    append_val = GPXDataToJSON((GPXData *)data_ptr->data);
    strcat(ret_val, append_val);
    strcat(ret_val, "]");
    free(append_val);
    return ret_val;
}

char *getOtherDataRoute(char *file_name, int route_idx) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    int i = 0;
    Node *node = doc->routes->head;
    while (i < route_idx) {
        node = node->next;
        i++;
    }
    Route *route = (Route *)node->data;
    char *data_json = GPXDataListToJSON(route->otherData);
    deleteGPXdoc(doc);
    return data_json;
}

char *getOtherDataTrack(char *file_name, int track_idx) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    int i = 0;
    Node *node = doc->tracks->head;
    while (i < track_idx) {
        node = node->next;
        i++;
    }
    Track *track = (Track *)node->data;
    char *data_json = GPXDataListToJSON(track->otherData);
    deleteGPXdoc(doc);
    return data_json;
}

int changeRouteName(char *file_name, char *new_name, int route_idx) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    int i = 0;
    Node *node = doc->routes->head;
    while (i < route_idx) {
        node = node->next;
        i++;
    }
    Route *route = (Route *)node->data;
    strcpy(route->name, new_name);
    writeGPXdoc(doc, file_name);
    deleteGPXdoc(doc);
    return 1;
}

int changeTrackName(char *file_name, char *new_name, int track_idx) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    int i = 0;
    Node *node = doc->tracks->head;
    while (i < track_idx) {
        node = node->next;
        i++;
    }
    Route *track = (Route *)node->data;
    strcpy(track->name, new_name);
    writeGPXdoc(doc, file_name);
    deleteGPXdoc(doc);
    return 1;
}

int createRoute(char *file_name, char *route_name) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    Route *route = malloc(sizeof(Route));

    route->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    route->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    route->name = malloc(100);
    strcpy(route->name, route_name);
    insertBack(doc->routes, route);
    writeGPXdoc(doc, file_name);
    deleteGPXdoc(doc);
    return 1;
}

int addWaypointToRoute(char *file_name, double latitude, double longitude) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    Waypoint *point = malloc(sizeof(Waypoint));

    point->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    point->latitude = latitude;
    point->longitude = longitude;
    point->name = malloc(100);
    strcpy(point->name, "");
    Route *route = getFromBack(doc->routes);
    addWaypoint(route, point);
    writeGPXdoc(doc, file_name);
    deleteGPXdoc(doc);
    return 1;
}

int createGPXFile(char *file_name, char *creator, double version) {
    GPXdoc *doc = malloc(sizeof(GPXdoc));

    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");

    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    doc->creator = malloc(1000);
    strcpy(doc->creator, creator);
    doc->version = version;
    writeGPXdoc(doc, file_name);
    deleteGPXdoc(doc);
    return 1;
}

char *findRoutesBetween(char *file_name, double lat1, double lon1, double lat2, double lon2, double delta) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    List *routes = getRoutesBetween(doc, lat1, lon1, lat2, lon2, delta);
    char *str = routeListToJSON(routes);
    deleteGPXdoc(doc);
    return str;
}

char *findTracksBetween(char *file_name, double lat1, double lon1, double lat2, double lon2, double delta) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    List *tracks = getTracksBetween(doc, lat1, lon1, lat2, lon2, delta);
    char *str = trackListToJSON(tracks);
    deleteGPXdoc(doc);
    return str;
}

int checkValidGPX(char *file_name) {
    GPXdoc *doc = createValidGPXdoc(file_name, "gpx.xsd");
    if (doc == NULL) {
        return 0;
    } else {
        return 1;
    }
}