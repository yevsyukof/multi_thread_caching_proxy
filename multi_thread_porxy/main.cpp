#include <iostream>
#include <csignal>
#include "Proxy.h"

#define LISTENING_PORT 55555

Proxy cachingProxy;

void set_sig_handler() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, nullptr);
}

int main() {
    set_sig_handler();
    cachingProxy = Proxy(LISTENING_PORT);
    cachingProxy.run();
}
