
#include <iostream>

#include "proxy_server.h"

using namespace std;


int main() {
    proxy_server server(1239);
    server.run();
    return 0;

}