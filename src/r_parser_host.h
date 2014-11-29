/*
 *  r_host.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_PARSER_HOST_H
#define R_PARSER_HOST_H

#include <iostream>
#include "r_core.h"

using namespace std;

class ParserHost: public tags_base, public Core {
public:
    ParserHost();
    ~ParserHost();
    
    int tagInit(string init_string);
    int tagClose();
    int tagParse(string line);
    int tagCheck();

private:
    string default_user;
    string default_pass;
};

#endif
