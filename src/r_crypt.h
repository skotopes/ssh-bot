/*
 *  r_crypt.h
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.04.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#ifndef R_CRYPT_H
#define R_CRYPT_H

#include <botan/botan.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace Botan;

class Crypt {
public:
   Crypt();
   ~Crypt();
   int init(std::string pphrase);
   int encrypt(std::string &in_string, std::string &out_string);
   int decrypt(std::string &in_string, std::string &out_string);
private:
   std::string passphrase;
};

#endif
