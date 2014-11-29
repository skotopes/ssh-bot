/*
 *  r_crypt.cpp
 *  robot
 *
 *  Created by Alexandr Kutuzov on 21.04.09.
 *  Copyright 2009 White Label ltd. All rights reserved.
 *
 */

#include "r_crypt.h"

Crypt::Crypt() {
    // we d like to get initied botan library with thread safe env
    LibraryInitializer initializer("thread_safe=true");
}

Crypt::~Crypt() {
    
}

int Crypt::init(std::string pphrase) {
    passphrase = pphrase;
    return 0;
}

int Crypt::encrypt(std::string &in_string, std::string &out_string)
{
    HashFunction* hash = get_hash("SHA-1"); 
    SymmetricKey key = hash->process(passphrase); 
    SecureVector<byte> raw_iv = hash->process("0" + passphrase); 
    InitializationVector iv(raw_iv, 8);
    Pipe pipe(get_cipher("Blowfish/CBC/PKCS7", key, iv, ENCRYPTION)); 
    
    
    std::istringstream in(in_string,std::istringstream::in);
    std::ostringstream out(std::ostringstream::out);
    
    pipe.start_msg();
    in >> pipe;
    pipe.end_msg(); 
    out << pipe;
    
    out_string = out.str();
    
    return 0;   
}

int Crypt::decrypt(std::string &in_string, std::string &out_string)
{
    HashFunction* hash = get_hash("SHA-1"); 
    SymmetricKey key = hash->process(passphrase); 
    SecureVector<byte> raw_iv = hash->process("0" + passphrase); 
    InitializationVector iv(raw_iv, 8);
    Pipe pipe(get_cipher("Blowfish/CBC/PKCS7", key, iv, DECRYPTION)); 
    
    std::istringstream in(in_string,std::istringstream::in);
    std::ostringstream out(std::ostringstream::out);
    
    pipe.start_msg(); 
    in >> pipe; 
    pipe.end_msg(); 
    out << pipe;
    out_string = out.str();
    
    return 0;
}
