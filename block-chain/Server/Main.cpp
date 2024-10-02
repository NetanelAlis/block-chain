#include "Server.hpp"
#include "Utils.hpp"
#include "Exception.hpp"

int main(int argc, char* argv[])
{
    try
    {
        Server server;
        server.ManageBlockChain();
    }
    catch(const Exception& error)
    {
        error.Print();
        exit(1);
    }
    
    return 0;
}
