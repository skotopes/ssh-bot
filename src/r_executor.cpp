/*
 *  r_executor.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 23.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_executor.h"

// Static definitions

int Executor::free_slots;

/*!
    @method     executor()
    @abstract   class constructor
    @discussion class constructor, nothing else
*/

Executor::Executor(int thread_slots) {
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (thread_slots <= 0) {
        logLine("executor", LOG_ERROR, "wrong slots number");
    } else {
        free_slots = thread_slots;
    }
}

/*!
 @method     ~executor()
 @abstract   class destructor
 @discussion class destructor, nothing else
 */

Executor::~Executor() {
    
}

/*!
    @method     execute()
    @abstract   start execution
    @discussion group validation, worker allocation 
*/

int Executor::execute() {    
    vector<string>::iterator g_it;
    
    for (g_it = groups_v.begin(); g_it < groups_v.end(); g_it++) {
        groups_t gt;
        vector<string>::iterator h_it;
        vector<Worker*>::iterator s_it;
        
        // pulling group info
        if (pullGroupFull((*g_it), gt) < 0 ) return -1;
        
        
        // allocating workers
        for (h_it = gt.hosts.begin(); h_it < gt.hosts.end(); h_it++) {
            pool_v.push_back(new Worker((*h_it),(*g_it)));
        }

        // starting all allocated workers
        logLine("executor", LOG_DEBUG, "starting threads");
        for (s_it = pool_v.begin(); s_it < pool_v.end(); s_it++ ) {
            if (no_threads) {
                // workers already prepared for job, we only need to start them
                (*s_it)->run();
            } else {
                // allocate thread and start it
                pthread_t thread;
                
                int ec = pthread_create(&thread, &attr, Executor::workerThread, (void*)(*s_it));
                
                if (ec) {
                    logLine("executor", LOG_ERROR, "thread creation error: " + ec);
                    return -1;
                }
                
                threads.push_back(thread);                
            }
        }
        
        if (!no_threads) {
            vector<pthread_t>::iterator tits;
            for (tits = threads.begin(); tits < threads.end(); tits++) {
                pthread_join((*tits), NULL);
            }
        }

        logLine("executor", LOG_DEBUG, "threads execution finished, shutting down workers");

        // free memory by hands, cause we realy need it
        for (s_it = pool_v.begin(); s_it < pool_v.end(); s_it++ ) {
            delete(*s_it);
        }

        pool_v.clear();
        threads.clear();
    }

    return 0;
}

int Executor::validate() {
    // Let`s get all groups that available
    if (getGroups(groups_v) < 0 || groups_v.size() == 0) {
        logLine("executor", LOG_ERROR, "No groups found");
        return -1;
    }
    
    // iterate and check
    vector<string>::iterator g_it;    
    for (g_it = groups_v.begin(); g_it < groups_v.end(); g_it++) {
        groups_t gt;
        vector<string>::iterator h_it;
        
        // pulling group info
        if (pullGroupFull((*g_it), gt) < 0 ) {
            logLine("executor", LOG_ERROR, "Unable to pull group info");
            return -2;
        }

        // let`s check that all hosts is present
        for (h_it = gt.hosts.begin(); h_it < gt.hosts.end(); h_it++) {
            int pt;
            string ip, un, ps, ep;

            if (pullHost((*h_it), ip, pt, un, ps, ep) < 0) {
                logLine("executor", LOG_ERROR, "Host not defined " + (*h_it));
                return -2;
            }

            if (pt < 1 || pt > 65535) {
                logLine("executor", LOG_ERROR, (*h_it) + " Port is not in range");
                return -2;
            }

            if (un.length() == 0 || ps.length() == 0 ) {
                logLine("executor", LOG_ERROR, (*h_it) + " Username or password not defined");
                return -3;
            }
            
            logLine("executor", LOG_DEBUG, (*h_it) + " is fine");
        }
        
        // Get scripts for group
        vector<command_t> c_t;
        if (pullScript((*g_it), c_t) < 0 && (*g_it).compare("DEFAULT") != 0) {
            logLine("executor", LOG_ERROR, (*g_it) + " Unable to get scripts for this group");
            return -4;
        }
        
        // command validator
        vector<command_t>::iterator c_it;
        for (c_it = c_t.begin(); c_it < c_t.end(); c_it++) {
            command_t *com = &(*c_it);
            switch (com->cid) {
                case SCRIPT_ASKUSER:
                    if (com->local) {
                        logLine("executor", LOG_ERROR, "this command cannot be local");
                        return -5;
                    }
                    break;
                case SCRIPT_SCRIPT:
                    break;
                case SCRIPT_COMMAND:
                    break;
                case SCRIPT_EXITCODE:
                    if (com->value.find_first_not_of("01234567890") != string::npos) {
                        logLine("executor", LOG_ERROR, "wrong exitcode definition");
                        return -5;
                    }
                    break;
                case SCRIPT_EXPECT:
                    break;
                case SCRIPT_UPLOAD:
                    if (com->local) {
                        logLine("executor", LOG_ERROR, "this command cannot be local");
                        return -5;
                    }
                    break;
                case SCRIPT_DOWNLOAD:
                    if (com->local) {
                        logLine("executor", LOG_ERROR, "this command cannot be local");
                        return -5;
                    }
                    break;
                default:
                    logLine("executor", LOG_ERROR, "falidation failed: unimplemented command");
                    return -5;
                    break;
            }
        }
        
        tasks_total += c_t.size() * gt.hosts.size();
    }

    return 0;
}

void *Executor::workerThread(void *data) {
    Worker *w = (Worker*) data;
    w->run();
    pthread_exit(NULL);
}
