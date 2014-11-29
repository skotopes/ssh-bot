#include "ssh.h"
#include <assert.h>
#include <ctime>

namespace ssh {

/*
 * SSH Session
 */

Session::Session():
    _session(ssh_new())
{
    assert(_session);
}

Session::~Session()
{
    if (_session) {
        if (isConnected()) {
            ssh_disconnect(_session);
        }

        ssh_free(_session); _session = 0;
    }
}

bool Session::isConnected()
{
    return ssh_is_connected(_session) == 1;
}

bool Session::connectWithKey(const char *hostname, unsigned int port, const char *username, const char *key)
{
    int rc;
    // set session host and port
    ssh_options_set(_session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(_session, SSH_OPTIONS_PORT, (const void *)port);
    ssh_options_set(_session, SSH_OPTIONS_USER, username);
    // establish connection
    rc = ssh_connect(_session);
    if (rc != SSH_OK) {
        _last_error = ssh_get_error(_session);
        return false;
    }
    // athenticate
    rc = ssh_userauth_publickey_auto(_session, username, NULL);
    if (rc != SSH_AUTH_SUCCESS)
    {
        _last_error = ssh_get_error(_session);
        return false;
    }

    return true;
}

bool Session::connectWithPassword(const char *hostname, int port, const char *username, const char *password, bool b_v, int i_v)
{
    int rc;
    // set session host and port
    ssh_options_set(_session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(_session, SSH_OPTIONS_PORT, (const void *)port);
    ssh_options_set(_session, SSH_OPTIONS_USER, username);
    // establish connection
    rc = ssh_connect(_session);
    if (rc != SSH_OK) {
        _last_error = ssh_get_error(_session);
        return false;
    }
    // authenticate
    rc = ssh_userauth_password(_session, username, password);
    if (rc != SSH_AUTH_SUCCESS)
    {
        _last_error = ssh_get_error(_session);
        return false;
    }

    return true;
}

Shell * Session::openShell()
{
    return new Shell(_session);
}

SFTP * Session::openSCP()
{
    return new SFTP(_session);
}

/*
 * SSH Shell
 */

Shell::Shell(ssh_session session):
    _channel(ssh_channel_new(session))
{
    assert(_channel);
}

Shell::~Shell()
{
    if (_channel) {
        ssh_channel_close(_channel);
        ssh_channel_send_eof(_channel);
        ssh_channel_free(_channel); _channel = 0;
    }
}

bool Shell::open()
{
    int rc = ssh_channel_open_session(_channel);
    if (rc != SSH_OK) {
        return false;
    }
    rc = ssh_channel_request_shell(_channel);
    if (rc != SSH_OK) {
        return false;
    }

    return true;
}

void Shell::send(const char * data_ptr)
{
    ssh_channel_write(_channel, data_ptr, strlen(data_ptr));
}

std::string Shell::read()
{
    std::string ret = _stdout;
    _stdout.clear();
    return ret;
}

bool Shell::waitFor(const char * data_ptr, int timeout)
{
    char buffer[4096];
    std::time_t till = std::time(nullptr) + timeout;


    while (std::time(nullptr) < till) {
        int nbytes = ssh_channel_read_nonblocking(_channel, buffer, sizeof(buffer), 1);
        if (nbytes > 0) {
            _stdout.append(buffer, nbytes);
        }
        nbytes = ssh_channel_read_nonblocking(_channel, buffer, sizeof(buffer), 0);
        if (nbytes > 0) {
            _stdout.append(buffer, nbytes);
        }
        if (_stdout.find(data_ptr) != std::string::npos) {
            return true;
        }
    }
    return false;
}


/*
 * SSH SFT
 */

SFTP::SFTP(ssh_session session):
    _sftp(sftp_new(session))
{
    assert(_sftp);
}

SFTP::~SFTP()
{
    if (_sftp) {
        sftp_free(_sftp);
    }
}

bool SFTP::open()
{
    int rc = sftp_init(_sftp);
    if (rc != SSH_OK)
    {
        return false;
    }

    return true;
}

bool SFTP::get(const char * remote_path, const char * local_path)
{
    return true;
}

bool SFTP::put(const char * local_path, const char * remote_path)
{
    return true;
}

} // namespace ssh
