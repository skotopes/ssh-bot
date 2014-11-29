/*
 *  r_script.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_parser_host.h"

ParserHost::ParserHost() {
    tag_name = "host";
    tag_short = false;
}

ParserHost::~ParserHost() {
    
}

int ParserHost::tagInit(string init_string) {
    // Can not imagine what for we can use init string here
    if (init_string.size()!=0) {
        logLine("parser_host", LOG_ERROR, "host inited with: "+ init_string + " but it shouldn`t");
        return -1;
    }
    return 0;
}

int ParserHost::tagClose() {
    logLine("parser_host", LOG_DEBUG, "host tag closed");
    return 0;
}

int ParserHost::tagParse(string line) {
    if (line.find_first_of("@") == 0) {
        string t_var, t_val;
        size_t e_pos = line.find_first_of("=");
        
        if (e_pos == string::npos) {
            logLine("parser_host", LOG_ERROR, "host: wrong definition");
            return -1;
        }

        t_var = line.substr(1, e_pos-1);
        t_val = line.substr(e_pos+1, line.size()-e_pos);

        trim(t_var);
        trim(t_val);
        
        if (t_var.compare(0, 4, "user") == 0) {
            default_user = t_val;
            return 0;
        } else if (t_var.compare(0, 4, "pass") == 0) {
            default_pass = t_val;
            return 0;
        } else {
            logLine("parser_host", LOG_ERROR, "host: wrong variable: " + line);
            return -1;
        }
    } else {
        int port = 22;
        string username = default_user, password = default_pass, hostname;
        size_t at_pos = line.find_first_of("@");
        size_t dl_pos = line.find_first_of(":");
//        size_t ws_pos = line.find_first_of(" \t");
        
        if ( at_pos != string::npos) {
            hostname = line.substr(at_pos+1, line.size()-at_pos);
            
            if ( dl_pos != string::npos && dl_pos < at_pos) {
                username = line.substr(0, dl_pos);
                password = line.substr(dl_pos+1, at_pos-dl_pos-1);
            } else {
                username = line.substr(0, at_pos);
            }
            
        } else {
            hostname = line;
        }
        
        logLine("parse_host", LOG_DEBUG, "h: " + hostname + " u: " + username + " p: " + password);
        
        if (username.size() == 0 || password.size() == 0) {
            logLine("parser_host", LOG_ERROR, "Empty pass or username for host: " + hostname);
            return -1;
        }

        pushHost(hostname, port, username, password);
        pushGroup((string)"DEFAULT", hostname);
        // pushHostExtra(hostname, <#map extras#>);
        return 0;
    }
}

int ParserHost::tagCheck() {
    return 0;
}
