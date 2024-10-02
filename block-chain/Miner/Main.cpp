#include "Miner.hpp"
#include "Utils.hpp"
#include "Exception.hpp"

int main(int argc, char* argv[])
{
    try
    {
        Miner miner;
        miner.SubscribeToServer();
        miner.Mine();
    }
    catch(const Exception& error)
    {
        error.Print();
        exit(1);
    }

    return 0;
}
