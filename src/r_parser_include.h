/*
 *  r_parser_include.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 28.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_PARSER_INCLUDE_H
#define R_PARSER_INCLUDE_H

#include <iostream>
#include "r_core.h"
#include "r_parser.h"

using namespace std;

class ParserInclude: public tags_base, public Core {
public:
    ParserInclude();
    ~ParserInclude();
    
    int tagInit(string init_string);
    int tagClose();
    int tagParse(string line);
    int tagCheck();
    
private:

};

#endif
