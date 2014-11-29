#ifndef SSH_H
#define SSH_H

#include <list>
#include <string>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace ssh {

class Session;
class Shell;
class SFTP;

class Session
{
public:
    Session();
    ~Session();

    bool isConnected();

    bool connectWithKey(const char *hostname, unsigned int port, const char *username, const char *key);
    bool connectWithPassword(const char *hostname, int port, const char *username, const char *password, bool b_v=false, int i_v=0);

    Shell * openShell();
    SFTP * openSCP();

    const char * getError() { return _last_error; }

private:
    ssh_session _session;
    const char * _last_error;
};

class Shell
{
public:
    Shell(ssh_session _session);
    ~Shell();
    bool open();

    void send(const char * data_ptr);
    std::string read();
    bool waitFor(const char * data_ptr, int timeout=0);

    const char * getError() { return _last_error; }

private:
    ssh_channel _channel;
    std::string _stdout;
    const char * _last_error;

};

class SFTP
{
public:
    SFTP(ssh_session _session);
    ~SFTP();

    bool open();

    bool get(const char * remote_path, const char * local_path);
    bool put(const char * local_path, const char * remote_path);

    const char * getError() { return _last_error; }

private:
    sftp_session _sftp;
    const char * _last_error;

};

}

#endif // SSH_H
