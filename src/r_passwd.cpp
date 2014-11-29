/*
 *  r_passwd.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 28.07.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_passwd.h"
#include <fstream>

Passwd::Passwd(string pphrase) {
    passphrase = pphrase;
    context.init(pphrase);
}

Passwd::~Passwd() {
    
}

static inline unsigned char h2c(char c) {
    if ((c>=0) && (c<=9)) return c+'0';
    if ((c>=10) && (c<=15)) return c+'A'-10;
    return 'X';
}

static inline unsigned char c2h(char c) {
    if ((c>='0') && (c<='9')) return c-'0';
    if ((c>='A') && (c<='F')) return c-'A'+10;
    if ((c>='a') && (c<='f')) return c-'a'+10;
    
    return 0xff;
}

static void get_salt(string &where, int len) {
    fstream rand;
    char *b=new char(len);
    int i;
    
    rand.open("/dev/urandom", fstream::in);
    rand.read(b, len);
    rand.close();
    
    where.resize(len);
    for (i=0; i<len; i++)
 	where[i]=b[i];
    
    return;
}


string Passwd::password2plain(string in, string &out) {
    string tmp;
    
    tmp.resize(in.size()/2);
    for (unsigned int i=0; i < in.size()/2; i++) {
        tmp[i]=( (c2h(in[2*i])<<4) | (c2h(in[2*i+1])) );
    }
    
    context.decrypt(tmp, out);
    
    out=out.substr(SALT_LEN, out.length()-SALT_LEN);
    return out;
}

string Passwd::plain2password(string in, string &out) {
    string salt;
    string tmp;    
    unsigned char c;
    
    get_salt(salt, SALT_LEN);
    salt+=in;
    context.encrypt(salt, tmp);

    out.resize(tmp.length()*2);
    
    for (unsigned int i=0; i<tmp.length(); i++) {
        c = tmp[i];
        out[2*i] = h2c(c>>4);
        out[2*i+1] = h2c(c&0xf);
    }
    
    return out;
}

int Passwd::read(char *pass_file) {
    fstream pwds;
    string host, user, pass, port, tmp;
    size_t pos, len;
    
    pwds.open(pass_file, fstream::in);
    
    if (!pwds.is_open()) {
        logLine("passwd",LOG_ERROR , "unable to open file");
        return -1;
    }

    pwds >> tmp;

    try {
        password2plain(tmp, pass);
    } catch (...) {
        logLine("passwd",LOG_ERROR , "unable to decode file with this passphrase");
        return -2;
    }
    
    if (pass.compare(STRING_OF_TEST) != 0) {
        logLine("passwd", LOG_ERROR, (string)STRING_OF_TEST + " != " + pass );
        return -2;
    }
	
    
    pass.clear();
    try {
        while(pwds >> tmp) {
            user = tmp.substr(0, tmp.find('@'));
            
            pos = tmp.find('@')+1;
            len = tmp.find(':') - pos;
            host = tmp.substr(pos, len);
            
            pos = tmp.find(':')+1;
            len = tmp.find('!') - pos;
            port = tmp.substr(pos, len);
            
            pass = tmp.substr(pos+len+1, tmp.size()-pos-len);
            
            password2plain(pass, tmp);
            pushHost(host, atoi(port.c_str()), user, tmp);
        }
    } catch (...) {
        logLine("passwd",LOG_ERROR , "file format error");
        return -3;
    }
    
    pwds.close();
    
    return 0;
}

int Passwd::write(char *pass_file) {
    map<string,hosts_t>::iterator it;
    hosts_t * h;
    fstream pwds;
    string out;
    
    pwds.open(pass_file, fstream::out);
    
    pwds << plain2password(STRING_OF_TEST, out) << endl;
    
    for ( it=hosts.begin(); it != hosts.end(); it++ ) {
        h = &(it->second);
        pwds << h->username << "@" << it->first << ":" << h->port << "!" << plain2password(h->password, out) << endl;
    }
    
    pwds.close();
    
    return 0;
}
