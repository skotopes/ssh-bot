/*
 *  r_core.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 20.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_CORE_H
#define R_CORE_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <pthread.h>
#include <sys/ioctl.h>

using namespace std;

enum {
    SCRIPT_COMMAND,
    SCRIPT_SCRIPT,
    SCRIPT_UPLOAD,
    SCRIPT_DOWNLOAD,
    SCRIPT_EXPECT,
    SCRIPT_EXITCODE,
    SCRIPT_ASKUSER
};

enum {
    LOG_ERROR,
    LOG_DEBUG,
    LOG_INFO
};

struct hosts_t {
    string ip_address;
    int port;
    string username;
    string password;
    map<string,string> extras;
};

struct groups_t {
    int gid;
    vector<string> hosts;
    map<string,string> extras;
};

struct command_t {
    int cid;
    bool important;
    bool local;
    string value;
};

struct script_t {
    vector<command_t> commands;
};

class Core  {
public:
    Core();
    virtual ~Core();

    // SuperGlobal definitions
    int addDefinition(char *arg);
    int processWithDef(string &stp);
    
    // Host map manipulators
    int pushHost(string hostname, int port, string username, string password);
    int pushHostExtra(string hostname, map<string,string> extras);
    int pullHost(string hostname, string &ip_address, int &port, string &username, string &password, string &enable_password);
    int pullHostExtra(string hostname, map<string,string> &extras);
    
    // Group map manipulators
    int pushGroup(string group_name, string host);
    int pushGroupExtra(string group_name, map<string,string> extra);
    int getGroups(vector<string> &groups_v);
    int pullGroupFull(string group_name, groups_t &group_hosts); // start point
    
    // Script manipulators 
    int pushScript(string group_name, vector<command_t> commands);
    int pullScript(string group_name, vector<command_t> &commands);
    
    // Logs and console i/o
    int logLine(string mod_name, int level, string line);
    int openLog(char *file_name);
    int coutLine(string line);
    int putBP(int line, string group, string host, string problem);
    
    // Helpers
    void trim( string& str);
    
    // task system
    static int tasks_total;
    static int tasks_compleate;
    static int timeout;
    // public stuff
    static string big_problem;
    static bool no_threads;
    static bool verbose;
    static bool debug;
    static bool stop;
    static bool finish;
    static bool cisco_mode;
    
private:
    // mutex work
    int lockStdout();
    int unlockStdout();

    // key makes
    string makeKey(string hostname, int port);

    // static private
    static bool core_inited;
    static pthread_mutex_t coutMutex;
    static int row,col;
    static ofstream log_file;

protected:  
    // I guess, you know what are you doing
    static map<string,hosts_t> hosts;
    static map<string,string> definitions;
    static map<string,groups_t> groups;
    static map<string,script_t> macros;
};

/*!
    @abstract   tab_base abstract class  
    @discussion basic tags manipulators and abstract class 
*/

class tags_base {
public:
    virtual ~tags_base();
    virtual int tagInit(string init_string) = 0;
    virtual int tagClose() = 0;
    virtual int tagParse(string line) = 0;
    virtual int tagCheck() = 0;
    
    // public tags definitions 
    string tag_name;
    bool tag_short;
};

extern char * private_key;

#endif
