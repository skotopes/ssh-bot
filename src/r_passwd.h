/*
 *  r_passwd.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 28.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_PASSWD_H
#define R_PASSWD_H

#include "r_core.h"
#include "r_crypt.h"
#include <stdio.h>
#include <stdlib.h>

#define PASSWORD_LEN 64
#define SALT_LEN 2
#define STRING_OF_TEST "This is the string of test!"

using namespace std;

class Passwd : public Core {
public:
    Passwd(string);
    ~Passwd();
    int read(char *pass_file);
    int write(char *pass_file);
private:
    string passphrase;
    Crypt context;
    string password2plain(string, string &);
    string plain2password(string, string &);
};

#endif
