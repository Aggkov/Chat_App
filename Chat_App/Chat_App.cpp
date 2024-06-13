#include "includes/Server.h"

int main() {

    Server srv;
    srv.start("localhost", 8080); // exit(1)
    
    return 0;
}
