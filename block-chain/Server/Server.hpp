#ifndef SERVER_HPP
#define SERVER_HPP

#include "Utils.hpp"
#include "Exception.hpp"

class Server
{
private:
    std::list<const char*> m_MinerPipes;
    BLOCK_T m_SuggestedBlock;
    BLOCK_T m_BlockChainHead; 
    uLong m_DifficultyLimit = 0;
    int m_ReadFD = 0;
    
    int readDifficulty();
    void printHead() const;
    void appendNewBlock();
    void updateMiners() const;
    void verifyProofOfWork() const;
    void checkCRC(ulong checkSum) const;
    void sendHeadBlockToMiner(const char* i_PipeName) const;
    void createGenesisBlock(int i_Difficulty);
    void createPipe();
    void subscribeNewMiner(const TLV* i_Tlv);
    void handleNewBlock(const TLV* i_Tlv);
    void deletePipeNames();
    void writeLog(const std::string& i_Log) const;
    TLV* readTlvFromPipe();
    int getMinerID(const char* i_String, int i_Pos) { return std::atoi(i_String + i_Pos); };
   
public:
    Server();
    void ManageBlockChain();
};

#endif // SERVER_HPP
