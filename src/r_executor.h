/*
 *  r_executor.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 23.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_EXECUTOR_H
#define R_EXECUTOR_H

#include <iostream>
#include <vector>
#include <libssh/libssh.h>
#include "r_core.h"
#include "r_worker.h"

using namespace std;

class Executor : public Core {
public:
    Executor(int thread_slots=10);
    virtual ~Executor();
    int execute();
    int validate();
    
private:
    static void *workerThread(void *data);
    
    static int free_slots;
    pthread_attr_t attr;

    vector<string> groups_v;
    vector<Worker*> pool_v;
    vector<pthread_t> threads;
};

#endif
