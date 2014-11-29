/*
 *  r_host.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_PARSER_GROUP_H
#define R_PARSER_GROUP_H

#include <iostream>
#include "r_core.h"

using namespace std;

class ParserGroup: public tags_base, public Core {
public:
    ParserGroup();
    ~ParserGroup();
    
    int tagInit(string init_string);
    int tagClose();
    int tagParse(string line);
    int tagCheck();
    
private:
    string current_group;
};

#endif
