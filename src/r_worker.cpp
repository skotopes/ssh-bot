/*
 *  r_worker.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 27.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_worker.h"

/*!
 @method     worker(string host, string group)
 @abstract   constructor
 @discussion class constructor, ne7ssh initialization, host info and blablabla
 */

Worker::Worker(string host, string group):
    session(nullptr), shell(nullptr), scp(nullptr)
{
    use_key = false;
    do_set_ps = false;
    hostname = host;
    groupname = group;
}

/*!
    @method     ~worker()
    @abstract   class destructor
    @discussion closing shell channel if it still is open
*/

Worker::~Worker() {
    if (scp) {
        delete scp; scp = 0;
    }
    if (shell) {
        delete shell; shell = 0;
    }
    if (session) {
        delete session; session = 0;
    }
}

/*!
 @method     run()
 @abstract   main worker loop
 @discussion open connection and execute macros
 */

int Worker::run() {
    pullHostExtra(hostname, extras);
    pullHost(hostname, ip_address, port, username, password, enable_password);
    
    if (stop) {
        return 0;
    }
    
    logLine((string)"worker", LOG_INFO, (string)"started for " + hostname);

    if (openConnection() < 0) {
        return -1;
    }
    
    if (executeScripts() < 0) {
        stop = true;
        return -2;
    }
    
    logLine((string)"worker", LOG_INFO, (string)"finished for " + hostname);
    return 0;
}

/*!
 @method     executeScripts()
 @abstract   executing scripts
 @discussion fetching scripts for group and execute them
 */

int Worker::executeScripts() {
    vector<command_t> commands;
    vector<command_t>::iterator c_it;
    
    if (pullScript(groupname, commands) < 0) {
        logLine("worker", LOG_ERROR, hostname+" Unable to pull script for " + groupname);
        return -1;
    }
    
    if (prepareShell() < 0) {
        logLine("worker", LOG_ERROR, hostname+" Unable to setup shell");
        return -2;
    }

    int cn = 0;
    for (c_it = commands.begin(); c_it < commands.end(); c_it++) {
        bool wps = true;
        command_t *t = &(*c_it);
        
        // PS do we need to wait
        c_it++;
        if ((*c_it).cid == SCRIPT_EXPECT) {
            wps = false;
        }
        c_it--;
        
        // emergency stop
        if (stop) {
            logLine("worker", LOG_INFO, hostname+" Aborting execution");
            return 0;
        }
        
        // execute command
        cn ++;
        int err_code;
        switch (t->cid) {
        case SCRIPT_ASKUSER:
            break;
        case SCRIPT_SCRIPT:
            break;
        case SCRIPT_COMMAND:
            if (t->local)
                err_code = localCommand(t->value);
            else
                err_code = doCommand(t->value, wps);

            if ( err_code < 0 && t->important) {
                putBP(cn, groupname, hostname, "unable to execute command");
                return -4;
            }
            break;
        case SCRIPT_EXITCODE:
            if (t->local)
                err_code = localWaitExit(t->value);
            else
                err_code = doWaitExit(t->value);

            if ( err_code < 0 && t->important) {
                putBP(cn, groupname, hostname, "exitcode mismatch");
                return -4;
            }
            break;
        case SCRIPT_EXPECT:
            if (t->local)
                err_code = localWait(t->value);
            else
                err_code = doWait(t->value);

            if ( err_code < 0 && t->important) {
                putBP(cn, groupname, hostname, "expect failed");
                return -4;
            }
            break;
        case SCRIPT_UPLOAD:
            if (uploadFile(t->value) < 0 && t->important) {
                putBP(cn, groupname, hostname, "unable to upload file");
                return -4;
            }
            break;
        case SCRIPT_DOWNLOAD:
            if (downloadFile(t->value) < 0 && t->important) {
                putBP(cn, groupname, hostname, "unable to download file");
                return -4;
            }
            break;
        default:
            logLine("worker", LOG_ERROR, hostname+" unimplemented command");
            return -3;
            break;
        }
        
        tasks_compleate++;
    }
    
    return 0;
}

/*!
 @method     prepareShell()
 @abstract   prepare remote shell for work
 @discussion setting PS and checking that we able to work with this shell
 */

int Worker::prepareShell() {
    if (cisco_mode) {
        shell->send("enable\n");
        if (!shell->waitFor("assword:", 10)) {
            logLine("worker", LOG_ERROR, "prepareShell failed!");
            return -1;
        }
        shell->send(enable_password.c_str());
        shell->send("\n");
        if (!shell->waitFor("#", 10)) {
            logLine("worker", LOG_ERROR, "prepareShell failed! 2");
            return -1;
        }

        return  0;
    }

    // default PS for shell
    shell->send("unset HISTFILE; LANG=C ;PS1='R7S: '\n");
    
    if (!shell->waitFor("R7S: ", 10)) {
        logLine("worker", LOG_ERROR, "prepareShell failed!");
        return -1;
    }

    return 0;
}

/*!
 @method     doCommand(string command)
 @abstract   executing command
 @discussion executing command and waiting for PS, then reading buffer
 */

int Worker::doCommand(string command, bool ps) {
    command += "\n";
    string waitfor;
    
    shell->send(command.c_str());

    if (ps) {
        if (do_set_ps) {
            prepareShell();
            do_set_ps = false;
        }

        if (cisco_mode)
            waitfor = "#";
        else
            waitfor = "R7S: ";

        if (!shell->waitFor(waitfor.c_str(), timeout)) {
            logLine("worker", LOG_ERROR, hostname+" shell PS whait failed for: "+command);
            
            readBuffer();
            return -1;
        }

        logLine("worker", LOG_ERROR, "finished"+command);
        
        readBuffer();
    } else {
        if (command.find("su") != string::npos) do_set_ps = true;
    }

    return 0;
}

/*!
 @method     doWait(string command)
 @abstract   expecting some string
 @discussion expecting some string and reading buffer
 */

int Worker::doWait(string command) {
    if (!shell->waitFor(command.c_str(), 10)) {
        logLine("worker", LOG_ERROR, hostname+" expect fail for: " + command);
        return -1;
    }
    
    logLine("worker", LOG_DEBUG, hostname+" expect success");
    readBuffer();
    
    return 0;
}

/*!
 @method     doWaitExit(string command)
 @abstract   expect some exitcode
 @discussion hack to get exit code from remote shell
 */

int Worker::doWaitExit(string command) {        
    shell->send("echo \"exit:$?\"\n");
    string ec = "exit:";
    ec += command;
    
    // And now for exit code
    if (!shell->waitFor(ec.c_str(), 10)) {
        logLine("worker", LOG_ERROR, hostname+" exit code fail");
        readBuffer();
        return -1;
    }

    logLine("worker", LOG_DEBUG, hostname+" exit code success");

    readBuffer();
    
    return 0;
}

/*!
 @method     localCommand(string command)
 @abstract   executing some command localy
 @discussion fork and execute command, pushing stdout to pipe
 */

int Worker::localCommand(string command) {
    int p_stdout[2],p_exitcode[2];
    
    // create pipes
    pipe(p_stdout);
    pipe(p_exitcode);
    
    // Behavior as in ne7ssh, send will clear input buffer and blablabla
    z_buff.clear();
    exc = 0;
    
    int fpid = fork();
    
    if (fpid == 0) {
        // some where in kenya
        FILE *ex_c;
        int ec;
        
        ex_c = fdopen(p_exitcode[1], "w");
        close(p_exitcode[0]);
        
        dup2(p_stdout[1], STDOUT_FILENO);
        close(p_stdout[0]);
        
        close(STDERR_FILENO);
        
        // execute command here
        ec = system(command.c_str());
        
        fputc(ec, ex_c);
        fclose(ex_c);
        
        exit(0);
    } else if (fpid > 0){
        // lions and tigers
        int c;
        FILE *out_s, *exc_s;
        
        out_s = fdopen (p_stdout[0], "r");
        close(p_stdout[1]);
        
        exc_s = fdopen (p_exitcode[0], "r");
        close(p_exitcode[1]);
        
        while ((c = fgetc (out_s)) != EOF){
            z_buff += (char)c;
        }
        
        while ((c = fgetc (exc_s)) != EOF) {
            exc = c;
        }
        
        fclose (out_s);
    } else {
        logLine("worker", LOG_DEBUG, hostname+": unable to create fork");
        return -1;
    }
    
    logLine("worker", LOG_DEBUG, hostname+": "+z_buff);
    
    return 0;
}

/*!
 @method     localWait(string command)
 @abstract   expecting some string in stdout
 @discussion same as doWait but for local commands
 */

int Worker::localWait(string command) {
    if (z_buff.find(command,0) == string::npos) {
        logLine("worker", LOG_DEBUG, "expect failed for: " + command);
        return -1;
    }
    
    return 0;
}

/*!
 @method     localWaitExit(string command)
 @abstract   expecting some exitcode
 @discussion same as doWaitExit
 */

int Worker::localWaitExit(string command) {
    stringstream s;
    
    int t_ec = atoi(command.c_str());
    
    if (exc != t_ec) {
        s << hostname << " exit code missmatch: " << exc << " != " << t_ec;
        logLine("worker", LOG_DEBUG, s.str());
        return -1;
    }
    
    return 0;
}

/*!
 @method     openConnection()
 @abstract   open connection(private)
 @discussion opening connection and assigning channel number
 */

int Worker::openConnection() {
    if (private_key) {
        session->connectWithKey(ip_address.c_str(), port, username.c_str(), private_key);
    } else {
        session->connectWithPassword(ip_address.c_str(), port, username.c_str(), password.c_str(), true, 30);
    }
    
    if (!shell) {
        err_msg = session->getError();
        if ( err_msg != NULL) {
            logLine("worker", LOG_ERROR, hostname+" Connection for shell failed with last error: " + (string)err_msg);
        } else {
            logLine("worker", LOG_ERROR, hostname+" Connection for shell failed, and we don`t know why");
        }
        
        return -1;
    }
    
    return 0;
}

/*!
 @method     upload_file
 @abstract   Uupload file with sftp
 @discussion Uploading file with sftp and checking result
 */

int Worker::uploadFile(string command) {
    string local_path = command.substr(0, command.find(" ")); trim(local_path);
    string remote_path = command.substr(command.find(" "), command.length() - command.find(" ")); trim(remote_path);

    logLine("netssh", LOG_INFO, hostname+" uploading: \"" + local_path + "\" to \"" + remote_path + "\"");
    if (!scp) {
        scp = session->openSCP();
        if (!scp) {
            err_msg = scp->getError();
            if ( err_msg != NULL) {
                logLine("netssh", LOG_ERROR, hostname+" Sftp channel error: " + (string)err_msg);
            } else {
                logLine("netssh", LOG_ERROR, hostname+" Sftp channel error");
            }
        }
    }

    if (!scp->put(local_path.c_str(), remote_path.c_str())) {
        logLine("netssh", LOG_ERROR, hostname+" Unable to put file: " + local_path);

        err_msg = scp->getError();
        if ( err_msg != NULL) {
            logLine("netssh", LOG_ERROR, hostname+" Sftp channel error: " + (string)err_msg);
        } else {
            logLine("netssh", LOG_ERROR, hostname+" Sftp channel error");
        }

    }

    return 0;
}

/*!
 @method     download_file
 @abstract   download file from remote machine
 @discussion download file from remote machine with sftp and checking result
 */

int Worker::downloadFile(string command) {
    string remote_path = command.substr(0, command.find(" ")); trim(remote_path);
    string local_path = command.substr(command.find(" "), command.length() - command.find(" ")); trim(local_path);

    logLine("netssh", LOG_INFO,hostname+" Trying to get: \"" + local_path + "\" to \"" + remote_path + "\"");
    
    if (!scp) {
        scp = session->openSCP();
        if (!scp) {
            err_msg = scp->getError();
            if ( err_msg != NULL) {
                logLine("netssh", LOG_ERROR, hostname+" Sftp channel error: " + (string)err_msg);
            } else {
                logLine("netssh", LOG_ERROR, hostname+" Sftp channel error");
            }
        }
    }
        
    if (!scp->get(remote_path.c_str(), local_path.c_str())) {
        logLine("netssh", LOG_ERROR, hostname+" Unable to get file: " + local_path);

        err_msg = scp->getError();
        if (err_msg != NULL) {
            logLine("netssh", LOG_ERROR, hostname+" Sftp channel error: " + (string)err_msg);
        } else {
            logLine("netssh", LOG_ERROR, hostname+" Sftp channel error");
        }

    }

    return 0;
}

int Worker::readBuffer() {
    std::string data = shell->read();

    if (data.length()) {
        logLine("worker", LOG_DEBUG, hostname +" "+ (string)data);
        return 0;
    } else {
        return -1;
    }
}
