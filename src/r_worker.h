/*
 *  r_worker.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 27.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_WORKER_H
#define R_WORKER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "r_core.h"
#include "ssh.h"

using namespace std;

class Worker : public Core {
public:
    Worker(string host, string group);
    virtual ~Worker();
    int run();
    
private:
    // main worker methods
    int executeScripts();

    // ssh helpers and additional stuff
    int prepareShell();
    int doCommand(string command, bool ps);
    int doWait(string command);
    int doWaitExit(string command);
    int localCommand(string command);
    int localWait(string command);
    int localWaitExit(string command);
    int openConnection();
    int uploadFile(string command);
    int downloadFile(string command);
    int readBuffer();
    
    // host info and extras
    map<string, string> extras;
    string groupname;
    string hostname;
    string ip_address;
    string username;
    string password;
    string key;
    string enable_password;
    int port;
    bool use_key;

    //channels and sub systems
    ssh::Session * session;
    ssh::Shell * shell;
    ssh::SFTP * scp;

    // variables
    const char* err_msg;
    string z_buff;
    int exc;
    bool do_set_ps;
};

#endif
