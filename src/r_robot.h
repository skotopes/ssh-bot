/*
 *  r_processor.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.04.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_ROBOT_H
#define R_ROBOT_H

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <sys/ioctl.h>
#include "r_core.h"
#include "r_parser.h"
#include "r_executor.h"
#include "r_passwd.h"

using namespace std;

class Robot: public Core {
public:
    Robot();
    virtual ~Robot();

    int go(int argc, char **argv);
    
private:
    // private progress thread
    static void *progressThread(void *data);
    int startProgress();
    int waitProgress();
    
    // private other
    int parseAll();
    int executeAll();
    void usage();

    // private progress thread vars
    pthread_t progress_thread;
    pthread_attr_t progress_attr;

    // private other vars
    char *f_macros;
    char *f_passwd;
    char *f_passwd_pass;
    char *f_log;
};

#endif
