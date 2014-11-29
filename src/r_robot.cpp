/*
 *  r_processor.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.04.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_robot.h"

char * private_key = NULL;

/*!
 @method     robot
 @abstract   constructor
 @discussion class constructor
 */

Robot::Robot() {
    verbose = false;
}

/*!
 @method     ~robot
 @abstract   destructor
 @discussion class destructor
 */

Robot::~Robot() {

}

/*!
 @method     go
 @abstract   run robot
 @discussion check args, vars and go
 */

int Robot::go(int argc, char **argv) {
    int c;
    f_macros = NULL;
    f_passwd = NULL;
    f_passwd_pass = NULL;
    f_log = NULL;
    cisco_mode = false;
    
    while ((c = getopt (argc, argv, "hm:p:D:l:t:vsk:P:C")) != -1)
        switch (c) {
            case 'v':
                if (verbose) {
                    debug = true;
                }
                verbose = true;
                break;
            case 's':
                no_threads = true;
                break;                
            case 't':
                timeout = atoi(optarg);
                if (timeout < 0) {
                    coutLine("Time out can`t be less then 0 seconds\r\n");
                    usage();
                    return 255;
                }
                break;                                
            case 'm':
                f_macros = optarg;
                break;
            case 'p':
                f_passwd = optarg;
                break;
            case 'P':
                f_passwd_pass = optarg;
            case 'k':
                private_key = optarg;
                break;
            case 'D':
                addDefinition(optarg);
                break;
            case 'l':
                f_log = optarg;
                break;
            case 'C':
                cisco_mode = true;
                cout << "WARNING!! Cisco mode is highly experimental. WARNING!!" << endl;
                break;
            case 'h':
            default:
                usage();
                return 255;
        }
    
    if (f_macros == NULL) {
        usage();
        return 255;
    }
    
    if (parseAll() < 0) {
        coutLine("Execution failed: unable to parse, exiting\r\n");
        return 254;
    }

    if (startProgress() < 0) {
        coutLine("Execution failed: unable to start progress thread\r\n");
        return 253;
    }    
    
    if (executeAll() < 0){
        coutLine("Execution failed: error while executing scripts, exiting\r\n");
        return 252;
    }
    
    // set exit flag for all helpers threads
    finish = true;

    if (waitProgress() < 0) {
        coutLine("Execution failed, whait for thread failed\r\n");
        return 251;
    }
    
    // Clean exit from previous part amy be not so clean as should
    if (stop) {
        coutLine("\r\n\r\n");
        coutLine(big_problem);
        coutLine("Houston, We've Got a Problem\r\n");
        return 250;
    } else {
        coutLine("That's All Folks!\r\n");
        return 0;
    }
}

/*!
    @method     progressThread()
    @abstract   progress thread
    @discussion progress bar and informer
*/

void *Robot::progressThread(void *data) {
    Robot *rb = (Robot*)data;
    struct winsize w;
    char end_sign[] = "/-\\|";
    int max_bar_size=0, es=0;
    
    rb->logLine("robot", LOG_INFO, "progress thread started");
        
    do {
        sleep(1);
        // TODO each time new one, do we realy need it? 
        stringstream out(stringstream::in | stringstream::out);

        // a little bit info about terminal
        ioctl(0, TIOCGWINSZ, &w);

        out << "Tasks: [";
        
        max_bar_size = w.ws_col-30;
        
        if (rb->tasks_total == 0) {
            out << "NO TASKS]\r";
            
            rb->coutLine(out.str());
            
            continue;
        }
        
        for (int i=0; i < max_bar_size; i++) {
            if (i < (max_bar_size * rb->tasks_compleate / rb->tasks_total) ) {
                out << "=";
            } else {
                out << ".";
            }
        }
        
        es++;
        if (es == 4) es = 0;

        out << "] "<< rb->tasks_compleate << "/" << rb->tasks_total;
        
        if (debug) {
            out << "\r\n";
        } else {
            out << " " << end_sign[es] << "\r";
        }
        
        rb->coutLine(out.str());

    } while (!rb->finish);

    // small trick just in case if we need it
    rb->coutLine("\r\n"); 

    pthread_exit(NULL);
}

/*!
    @method     startProgress()
    @abstract   start progress thread
    @discussion starting progress thread
*/

int Robot::startProgress() {
    pthread_attr_init(&progress_attr);
    pthread_attr_setdetachstate(&progress_attr, PTHREAD_CREATE_JOINABLE);

    int ec = pthread_create(&progress_thread, &progress_attr, progressThread, (void*)this);

    if (ec) {
        logLine("robot", LOG_ERROR, "progress thread creation error: " + ec);
        return -1;
    }

    return 0;
}

int Robot::waitProgress() {
    pthread_join(progress_thread, NULL);
    return 0;
}

/*!
    @method     parseAll()
    @abstract   parse macros and password file
    @discussion 2 objects: parser and passwd for macroses and passwords
*/

int Robot::parseAll() {
    if (f_log != NULL) {
        if (openLog(f_log) < 0) {
            coutLine("Unable to open log file");
            return -1;
        }
    } 
    
    if (f_passwd != NULL) {
        string pp;

        if (f_passwd_pass == NULL) {
            coutLine("Passphrase for password file please: ");
            cin >> pp;
        }
        else {
            pp = f_passwd_pass;
        }

        Passwd pwd(pp);
        int ec = pwd.read(f_passwd);
        if (ec < 0) {
            logLine("robot", LOG_ERROR, "unable to read passfile, exiting");
            return -1;
        }
    }
    
    Parser prs;

    if (prs.parseFile(f_macros) < 0) {
        coutLine("Unable to parse macros file\r\n");
        return -1;
    }
    
    return 0;
}

/*!
    @method     executeAll()
    @abstract   execute macroses
    @discussion executing macroses, see executor class for additional info
*/

int Robot::executeAll() {
    Executor exc;
    
    if (exc.validate() < 0) {
        return -1;
    }
    
    if (exc.execute() < 0) {
        return -2;
    }
    
    return 0;
}

/*!
 @method     usage
 @abstract   robot usage
 @discussion small help for robot, not thread safe
 */

void Robot::usage() {
    cout << "At least macros file expected." << endl
        << "Usage ./robot [args]" << endl
        << "Possible args:"<< endl
        << "-m [macros_file] \t\t Macros file" << endl
        << "-p [passwd_file] \t\t Encrypted password file" << endl
        << "-P [password]\t\t\t Password file password (INSECURE!!)" << endl
        << "-D [VAR=VAL] \t\t\t Global variable definition" << endl
        << "-l [log_file] \t\t\t Master log" << endl
        << "-t [sec] \t\t\t Timeout for shell operations(must be greater then 0)" << endl
        << "-k path_to_secret_key_file" << endl
        << "-s \t\t\t\t Switch OFF multiple thread" << endl
        << "-v \t\t\t\t Increase verbosity (-vv for debug level)" << endl;
}

/*!
    @abstract   enter point
*/

int main(int argc, char *argv[]){   
    Robot *rbt = new Robot;
    return rbt->go(argc, argv);
}
