/*
 *  r_parser.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 16.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_parser.h"

/*!
    @method     parser()
    @abstract   class constructor
    @discussion create ifstream, push additional parsers into tag vector
*/

Parser::Parser() {
    in_file = new ifstream();
    in_section = false;

    // enabling additional parsing engines
    tags.push_back(new ParserInclude);
    tags.push_back(new ParserScript);
    tags.push_back(new ParserGroup);
    tags.push_back(new ParserHost);
}

/*!
    @method     ~parser()
    @abstract   destructor
    @discussion object destructor
*/

Parser::~Parser() {
    in_file->close();
}

/*!
    @method     parseFile(char * file_name)
    @abstract   parse file
    @discussion open file and parse basic syntax
*/

int Parser::parseFile(char * file_name) {
    char buff[65535];
    string line;
    int line_n = 0;

    in_file->open(file_name);
    
    if (!in_file->is_open())
        return -1;
    
    while (!in_file->eof()) {
        in_file->getline(buff,65535);
        line_n ++;
        line = buff;
        
        // strip comments
        size_t cm_pos = line.find_first_of("#");
        if (cm_pos != line.size()-1){
            line = line.substr(0, cm_pos);
        }
        
        // strip whitespaces
        trim(line);
        
        // proceed if not zero size
        if (line.length() !=0) {
            if(!parseBuff(line, line_n)) return -2;
        }
    }
    
    return 0;
}

/*!
    @method     parseBuff(string line, int line_n)
    @abstract   parse line
    @discussion parse line, feed tags parsers with content if we need
*/

bool Parser::parseBuff(string line, int line_n) {
    vector<tags_base*>::iterator it;

    // Additional syntax parser only if we inside of section
    if (in_section) {
        for (it=tags.begin(); it<tags.end(); it++) {
            if (section_name == (*it)->tag_name) {
                // Short tags must be closed with ";" 
                if ((*it)->tag_short) {
                    return false;
                }
                
                if (line.compare(0, 1, "}") == 0) {
                    if ((*it)->tagClose() == 0) {
                        in_section = false;
                    }

                    return true;
                }
                
                if ((*it)->tagParse(line) != 0) {
                    return false;
                } else {
                    return true;
                }
            }
        }
        
        // compiller is a little bit stupid, we got warning if not return value here
        logLine("parser", LOG_ERROR, "We never should reach this point");
        return false;
    } else {
        // If string is tag definition
        if (line.find_first_of("@") == 0) {
            // Last character in string should be group "{" TODO: should we change it ? 
            if (line.compare(line.size()-1, 1, "{") != 0 && line.compare(line.size()-1, 1, ";") != 0) {
                logLine("parser", LOG_ERROR, "error at line " + line_n + (string)": \"{\" or \";\" expected");
                return false;
            }
            
            // Iterating throw the tag vector
            for (it=tags.begin(); it<tags.end(); it++) { 
                if (line.compare(1, (*it)->tag_name.size(), (*it)->tag_name) == 0) {
                    string tmp;

                    if (!(*it)->tag_short) {
                        in_section = true;
                        section_name = (*it)->tag_name;
                    }
                    
                    // If we got additional tag init string
                    if ((line.size() - (*it)->tag_name.size() - 2) > 0) {
                        tmp = line.substr((*it)->tag_name.size()+1, line.size() - (*it)->tag_name.size() - 2);
                        if (tmp.compare(0, 1, " ") != 0) {
                            logLine("parser", LOG_ERROR, "Parser> error at line " + line_n + (string)": @tag definition error");
                            return false;
                        }
                        trim(tmp);
                    }
                    
                    // Initing tag with tmp string(size may be 0)
                    if ((*it)->tagInit(tmp) != 0) {
                        logLine("parser", LOG_ERROR, "unable to init tag " + (*it)->tag_name + ": with " + tmp);
                        return false;
                    } else {
                        return true;
                    }
                }
            }
            logLine("parser", LOG_ERROR, "error at line " + line_n + (string)": wrong tag");
            return false;
        } else {
            logLine("parser", LOG_ERROR, "error at line " + line_n + (string)": @tag expected");
            return false;
        }
    }
}
