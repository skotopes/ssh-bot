/*
 *  r_passfile.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 04.08.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_PASSFILE_H
#define R_PASSFILE_H

#include <iostream>
#include <vector>
#include "r_core.h"
#include "r_passwd.h"

using namespace std;

class Passfile : public Core {
public:
    Passfile();
    ~Passfile();
    int go(int argc, char **argv);

private:
    void usage();
    
    char *f_passwd;
};
#endif
