/*
 *  r_group.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_parser_group.h"

ParserGroup::ParserGroup() {
    tag_name = "group";
    tag_short = false;
}

ParserGroup::~ParserGroup() {
    
}

int ParserGroup::tagInit(string init_string) {
    logLine("parser_group", LOG_DEBUG, "group inited with:" + init_string);
    
    if (init_string.size() == 0) {
        logLine("parser_group", LOG_DEBUG, "group init without group name");
        return -1;
    }
    
    current_group = init_string;
    
    return 0;
}

int ParserGroup::tagClose() {
    logLine("parser_group", LOG_DEBUG, "group tag closed");
    return 0;
}

int ParserGroup::tagParse(string line) {
    if (line.find_first_of("@") == 0) {
        string t_var, t_val;
        size_t e_pos = line.find_first_of("=");
        
        if (e_pos == string::npos) {
            logLine("parser_group", LOG_ERROR, "group: wrong definition");
            return -1;
        }
        
        t_var = line.substr(1, e_pos-1);
        t_val = line.substr(e_pos+1, line.size()-e_pos);
        
        trim(t_var);
        trim(t_val);

        // TODO: what variables we ever may need in ? 
        if (t_var.compare(0, 4, "user") == 0) {
            return 0;
        } else {
            logLine("parser_group", LOG_ERROR, "group: wrong variable: " + line);
            return -1;
        }
    } else {
        // as simple as it can be
        pushGroup(current_group, line);
        
        return 0;
    }
}

int ParserGroup::tagCheck() {
    return 0;
}
