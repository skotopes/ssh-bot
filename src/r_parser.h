/*
 *  r_parser.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 16.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_PARSER_H
#define R_PARSER_H

#include <fstream>
#include <iostream>
#include <vector>

#include "r_core.h"
#include "r_parser_include.h"
#include "r_parser_script.h"
#include "r_parser_host.h"
#include "r_parser_group.h"

using namespace std;

class Parser: public Core {
public:
    Parser();
    ~Parser();
    int parseFile(char * file_name);

private:
    bool parseBuff(string line, int line_n);

    ifstream *in_file;

    bool in_section;
    vector<tags_base*> tags;
    string section_name;
};


#endif
