/*
 *  r_core.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 20.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_core.h"

// some core static stuff
int Core::tasks_total = 0;
int Core::tasks_compleate = 0;
int Core::timeout = 600;

string Core::big_problem = "";
bool Core::cisco_mode = false;
bool Core::no_threads = false;
bool Core::verbose = false;
bool Core::debug = false;
bool Core::stop = false;
bool Core::finish = false;
bool Core::core_inited = false;

map<string,string> Core::definitions;
map<string,hosts_t> Core::hosts;
map<string,groups_t> Core::groups;
map<string,script_t> Core::macros;
pthread_mutex_t Core::coutMutex;
ofstream Core::log_file;

/*!
    @method     core()
    @abstract   class constructor
    @discussion class constructor, group and mutex initializer
*/

Core::Core() {
    if (!core_inited) {
        // Adding default group
        groups_t * tmp_group = &groups["DEFAULT"];
        tmp_group->gid = 0;
        // Initing log mutex
        pthread_mutex_init(&coutMutex, NULL);
        // Fine? isn`t it?
        core_inited = true;
    }
}

/*!
    @method     ~core()
    @abstract   class destructor
    @discussion do nothing
*/

Core::~Core() {
    
}

/*!
    @method     pushHost(string hostname, int port, string username, string password)
    @abstract   adding host to host map
    @discussion adding host to host map, or overwrites it
*/

std::string Core::makeKey(string hostname, int port) {
    std::stringstream sstm;

    if (hostname.find(':') == string::npos) {
        sstm << hostname << ":" << port;
        return sstm.str();
    } else {
	return hostname;
    }
}

int Core::pushHost(string hostname, int port, string username, string password) {
    hosts_t * tmp_host;
    std::string key;
    
    key = makeKey(hostname, port);

    //tmp_host = &hosts[hostname];
    tmp_host = &hosts[key];
    tmp_host->ip_address = hostname;
    tmp_host->port = port;
    tmp_host->username = username;
    tmp_host->password = password;

    return 0;
}

int Core::pushHostExtra(string hostname, map<string,string> extras) {
    hosts_t * tmp_host = &hosts[hostname];
    tmp_host->extras = extras;

    return 0;
}

int Core::pullHost(string hostname, string &ip_address, int& port, string &username, string &password, string &enable_password) {
    map<string,hosts_t>::iterator it;
    std::string key;

    key = makeKey(hostname, 22);
    it = hosts.find(key);
    
    if (it == hosts.end()) {
        return -1;
    }

    hosts_t * tmp_host = &(*it).second;
    ip_address = tmp_host->ip_address;
    port = tmp_host->port;
    username = tmp_host->username;
    password = tmp_host->password;

    if (!cisco_mode) return 0;

    key = makeKey(hostname, 0);
    it = hosts.find(key);
    if (it == hosts.end()) {
        return 0; // may be we don't need enable password
    }

    tmp_host = &(*it).second;
    enable_password = tmp_host->password;

    return 0;
}

int Core::pullHostExtra(string hostname, map<string,string> &extras) {
    map<string,hosts_t>::iterator it;
    hosts_t * tmp_host;
    
    it = hosts.find(hostname);
    
    if (it == hosts.end()) {
        return -1;
    }
    
    tmp_host = &(*it).second;
    extras = tmp_host->extras;

    return 0;
}

int Core::pushGroup(string group_name, string host) {
    groups_t * tmp_group = &groups[group_name];
    tmp_group->hosts.push_back(host);
    
    return 0;
}

int Core::pushGroupExtra(string group_name, map<string,string> extra) {
    groups_t * tmp_group = &groups[group_name];
    tmp_group->extras = extra;
    
    return 0;
}

/*!
    @method     getGroups(vector<string> &groups_v)
    @abstract   get groups from map
    @discussion get groups name from map. 
*/

int Core::getGroups(vector<string> &groups_v) {
    map<string,groups_t>::iterator it;
    
    for (it=groups.begin(); it != groups.end(); it++) {
        groups_v.push_back((*it).first);
    }
    
    return 0;
}

int Core::pullGroupFull(string group_name, groups_t &group_hosts) {
    map<string,groups_t>::iterator it;
    
    it = groups.find(group_name);
    
    if (it == groups.end()) {
        return -1;
    }
    
    group_hosts = (*it).second;
    
    return 0;
}

int Core::pushScript(string group_name, vector<command_t> commands) {
    script_t *script_tmp = &macros[group_name];
    script_tmp->commands = commands;

    return 0;
}

int Core::pullScript(string group_name, vector<command_t> &commands) {
    static map<string,script_t>::iterator it;
    
    it = macros.find(group_name);
    
    if (it == macros.end()) {
        return -1;
    } else {
        commands = (*it).second.commands;
        return 0;
    }
}

int Core::openLog(char *file_name) {
    log_file.open(file_name, ios::app);
    
    if (!log_file.is_open()){
        return -1;
    }
    
    return 0;
}

int Core::logLine(string mod_name, int level, string line) {
    stringstream output;
    struct winsize w;
    
    switch (level) {
        case LOG_INFO:
            output << "[I] ";
            break;
        case LOG_DEBUG:
            output << "[D] ";
            break;
        case LOG_ERROR:
            output << "[E] ";
            break;
        default:
            return -1;
            break;
    }
    
    // a little bit info about terminal
    ioctl(0, TIOCGWINSZ, &w);
    
    trim(line);
    output << mod_name << ": " << line;
    
    if (log_file.is_open())
        log_file << output.str() << endl;
    
    switch (level) {
        case LOG_INFO:
            if (!verbose) return 0;
            break;
        case LOG_DEBUG:
            if (!debug) return 0;
            break;
        case LOG_ERROR:
            break;
        default:
            return -1;
            break;
    }
    
    int ss_size = output.str().size();
    int sc_size = w.ws_col;
    
    if (ss_size < sc_size) {
        for (int i=0; i<(sc_size - ss_size);i++) {
            output << " ";
        }
    }
    
    lockStdout();
    cout << output.str() << endl;
    unlockStdout();
    
    return 0;
}

int Core::coutLine(string line) {
    lockStdout();
    cout << line << flush;
    unlockStdout();
    
    return 0;
}

int Core::putBP(int line, string group, string host, string problem) {
    stringstream p;

    p << "Error at line: "<< line << " group: "<< group 
    << " host:" << host << " problem: "<< problem << endl; 
    
    lockStdout();
    big_problem += p.str();
    unlockStdout();
    
    return 0;
}

int Core::addDefinition(char *arg) {
    string key, val;
    
    key = arg;
    trim(key);
    
    size_t eq_pos = key.find("=");
    if (eq_pos == string::npos) {
        return -1; // no equal in definition
    }
    
    val = key.substr(eq_pos+1, key.size());
    key = key.substr(0, eq_pos);
    
    logLine("core", LOG_DEBUG, "global definition: " + key + ":" + val);
        
    if ((key.size() == 0) || (val.size() == 0)) {
        return -2;
    }    

    definitions[key] = val;
    return 0;
}

int Core::processWithDef(string &stp) {
    map<string,string>::iterator d_it;
    
    while (true) {
        string v_name;
        size_t in_p, out_p;

        in_p = stp.find("{{");
        
        if (in_p == string::npos) {
            // we do not need to proceed string without template tags
            return 0;
        }
        
        in_p += 2; // offset from tag begining
        
        out_p = stp.find("}}", in_p);
        
        if (out_p == string::npos) {
            // probably not properly closed tag, but we do not care
            return 0;
        }
        
        v_name = stp.substr(in_p, out_p-in_p);
        
        trim(v_name);
        
        if (v_name.size() == 0) {
            logLine("core", LOG_ERROR, "template without variable");
            return -1;
        }
        
        d_it = definitions.find(v_name);
        
        if (d_it == definitions.end()) {
            logLine("core", LOG_ERROR, "got template definition without predefined value: " + v_name);
            return -2;
        }

        // finaly replacing template with value
        stp.replace(in_p-2, out_p-in_p+4, (*d_it).second);
        logLine("core", LOG_DEBUG, stp);
    }
    
    // nice isn`t? 
    return 0;
}


void Core::trim( string& str) {
    size_t startpos = str.find_first_not_of(" \t");
    size_t endpos = str.find_last_not_of(" \t");
    
    if(( string::npos == startpos ) || ( string::npos == endpos)) {
        str ="";
    } else {
        str = str.substr( startpos, endpos-startpos+1 );
    }
}

int Core::lockStdout() {
    pthread_mutex_lock(&coutMutex);
    return 0;
}

int Core::unlockStdout() {
    pthread_mutex_unlock(&coutMutex);
    return 0;
}

/*!
    @method     ~tags_base
    @abstract   virtual destructor
    @discussion virtual destructor for tag_base abstract class
*/

tags_base::~tags_base() {
    
}
