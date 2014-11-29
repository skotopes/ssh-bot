/*
 *  r_passfile.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 04.08.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_passfile.h"
#include <unistd.h>

Passfile::Passfile() {
    
}

Passfile::~Passfile() {
    
}

int Passfile::go(int argc, char **argv) {
    int c;
    bool wdne = false, new_file = false;
    f_passwd = NULL;
    
    while ((c = getopt (argc, argv, "hp:n")) != -1)
        switch (c) {
            case 'p':
                f_passwd = optarg;
                break;                
            case 'n':
                new_file = true;
                break;
            case 'h':
            default:
                usage();
                return 255;
        }
    
    if (f_passwd == NULL) {
        usage();
        return 255;
    }
    
    string pp;
    cout << "Passphrase for password file please: ";
    cin >> pp;
    
    Passwd pwd(pp);

    if (!new_file) {
        int ec = pwd.read(f_passwd);
        if ( ec < 0) {
            cout << "Unable to decode password file " << ec << endl;
            return 254;
        }
    }
        
    while (!wdne) {
        string command;
        cout << "L - list, D - delete, A - add, P - change passphrase, S - save, E - exit: ";
        cin >> command;
        
        trim(command);

        if (command.length() !=1) 
        {
            cout << "Wrong command" << endl;
        }
        
        if (command.find_first_of("Ll")!=string::npos) 
        {
            map<string,hosts_t>::iterator it;
            int number = 0;
            
            cout << "Host list:" << endl;
            
            for ( it=hosts.begin(); it != hosts.end(); it++ ) {
                hosts_t * h = &(it->second);
                cout << number << " " << h->username << "@" << it->first 
                    << ":" << h->port << "!" << h->password << endl;
                number ++;
            }

            cout << "End List" << endl; 
        }
        else if (command.find_first_of("Dd")!=string::npos) 
        {
            map<string,hosts_t>::iterator it;
            unsigned int number;
            
            cout << "host number: ";
            cin >> number;
            
            if (number == 0 || number >= hosts.size()) {
                cout << "Wrong number" << endl;
                continue;
            }
            
            it = hosts.begin();
            
            for (unsigned int i=0; i<number ;i++) it++;
            
            hosts.erase(it);
        }
        else if (command.find_first_of("Aa")!=string::npos) 
        {
            string hostname, username, password;
            int port=22;
            cout << "Hostname: ";
            cin >> hostname;
	    cout << "Port: ";
            cin >> port;
            cout << "Username: ";
            cin >> username;
            cout << "Password: ";
            cin >> password;
            pushHost(hostname, port, username, password);
        }
        else if (command.find_first_of("Ss")!=string::npos) 
        {
            cout << "Saving pass file: ";
            Passwd npwd(pp);
            if (npwd.write(f_passwd) < 0) {
                cout << "FAIL" << endl;
                return 254;
            } else {
                cout << "OK" << endl;
            }
        }
        else if (command.find_first_of("Ee")!=string::npos) 
        {
            cout << "Bye! Bye!" << endl;
            wdne = true;
        }
        else if (command.find_first_of("Pp")!=string::npos) 
        {
            cout << "Enter new passphrase!";
            cin >> pp;
        }
        else 
        {
            cout << "wtf?" << endl;
        }
    }
    
    return 0;
}

void Passfile::usage() {
    cout << "At least password file expected." << endl
    << "Usage ./passfile [args]" << endl
    << "Possible args:"<< endl
    << "-n \t\t\t\t Create new file" << endl
    << "-p [passwd_file] \t\t Encrypted password file" << endl;
}

/*!
 @abstract   enter point
 */

int main(int argc, char *argv[]){   
    Passfile *psf = new Passfile;
    return psf->go(argc, argv);
}
