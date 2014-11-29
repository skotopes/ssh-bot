/*
 *  r_script.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_parser_script.h"

ParserScript::ParserScript() {
    tag_name = "script";
    tag_short = false;
}

ParserScript::~ParserScript() {

}

int ParserScript::tagInit(string init_string) {
    logLine("parser_script", LOG_DEBUG, "script inited with:" + init_string);
    
    if (init_string.size()==0) {
        current_group = "DEFAULT";
    } else {
        if (init_string.compare(0, 7, "DEFAULT")==0) {
            return -1;
        }
        current_group = init_string;        
    }

    return 0;
}

int ParserScript::tagClose() {
    logLine("parser_script", LOG_DEBUG, "script tag closed:");
    pushScript(current_group, tmp_cmd);
    tmp_cmd.erase(tmp_cmd.begin(),tmp_cmd.end());
    return 0;
}

int ParserScript::tagParse(string line) {
    command_t tmp;

    tmp.important = true;
    tmp.local = false;
    
    if (line.compare(0, 1, "-") == 0) {
        // this line is less important then other
        tmp.important = false;
        line.erase(0, 1);
    }

    if (line.compare(0, 1, "!") == 0) {
        // this line is for local execution only
        tmp.local = true;
        line.erase(0, 1);
    }
    
    if (line.compare(0, 8, "command ") == 0) {
        tmp.cid = SCRIPT_COMMAND;
        tmp.value = line.substr(8, line.size()-8);
        trim(tmp.value);
    } else if (line.compare(0, 7, "script ") == 0) {
        tmp.cid = SCRIPT_SCRIPT;
        tmp.value = line.substr(7, line.size()-7);
        trim(tmp.value);
    } else if (line.compare(0, 7, "upload ") == 0) {
        tmp.cid = SCRIPT_UPLOAD;
        tmp.value = line.substr(7, line.size()-7);
        trim(tmp.value);
    } else if (line.compare(0, 9, "download ") == 0) {
        tmp.cid = SCRIPT_DOWNLOAD;
        tmp.value = line.substr(9, line.size()-9);
        trim(tmp.value);
    } else if (line.compare(0, 7, "expect ") == 0) {
        tmp.cid = SCRIPT_EXPECT;
        tmp.value = line.substr(7, line.size()-7);
        trim(tmp.value);
    } else if (line.compare(0, 9, "exitcode ") == 0) {
        tmp.cid = SCRIPT_EXITCODE;
        tmp.value = line.substr(9, line.size()-9);
        trim(tmp.value);
    } else if (line.compare(0, 10, "inputuser ") == 0) {
        tmp.cid = SCRIPT_ASKUSER;
        tmp.value = line.substr(10, line.size()-10);
        trim(tmp.value);
    } else {
        logLine("parser_script", LOG_ERROR, "wrong command " + line);
        return -1;
    }

    if (processWithDef(tmp.value) < 0){
        logLine("parser", LOG_ERROR, "error in definition");
        return -1;
    }
    
    tmp_cmd.push_back(tmp);
    
    return 0;    
}

int ParserScript::tagCheck() {
    return 0;
}
