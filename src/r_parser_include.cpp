/*
 *  r_parser_include.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 28.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_parser_include.h"

/*!
    @method     parser_include()
    @abstract   class constructor
    @discussion set tag name and tag type
*/

ParserInclude::ParserInclude() {
    tag_name = "include";
    tag_short = true;
}

/*!
 @method     ~parser_include()
 @abstract   class destructor
 @discussion do nothing
 */

ParserInclude::~ParserInclude() {
    
}

int ParserInclude::tagInit(string init_string) {
    logLine("parser_include", LOG_DEBUG, "included" + init_string);
    
    Parser include_prs;
    
    if (include_prs.parseFile((char*)init_string.c_str()) < 0) {
        logLine("parser_include", LOG_DEBUG, "error parsing file" + init_string);
        return -1;
    }
    
    return 0;
}

int ParserInclude::tagClose() {
    logLine("parser_include", LOG_DEBUG, "tag closed");
    return 0;
}

int ParserInclude::tagParse(string line) {
    logLine("parser_include", LOG_DEBUG, "tag include?" + line);
    return 0;
}

int ParserInclude::tagCheck() {
    return 0;
}
