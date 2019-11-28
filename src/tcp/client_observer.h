#ifndef INTERCOM_CLIENT_OBSERVER_H
#define INTERCOM_CLIENT_OBSERVER_H


#include <string>
#include "pipe_ret_t.h"


typedef void (clientIncomingPacketFunc)(const char * msg, size_t size);
typedef clientIncomingPacketFunc* clientIncomingPacketFunc_t;

typedef void (clientDisconnectedFunc)(const pipe_ret_t & ret);
typedef clientDisconnectedFunc* clientDisconnectedFunc_t;

struct client_observer_t {

    std::string wantedIp;
    clientIncomingPacketFunc_t clientIncomingPacketFunc;
    clientDisconnectedFunc_t clientDisconnectedFunc;

    client_observer_t() {
        wantedIp = "";
        clientIncomingPacketFunc = NULL;
        clientDisconnectedFunc = NULL;
    }
};


#endif //INTERCOM_CLIENT_OBSERVER_H
