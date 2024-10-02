#ifndef MINER_HPP
#define MINER_HPP

#include "Utils.hpp"
#include "Exception.hpp"

class Miner
{
private:
    BLOCK_T m_CurrentBlock = {0};       
    uLong m_DifficultyLimit = 0;
    char m_PipeName[30];
    bool m_IsFirstBlock = true;
    int m_MinerID = 0;
    int m_ReadFD = 0;

    void getHash();
    void suggestBlock();
    void printSuggestion() const;
    bool isValidHash() const;
    void readConfigurationFile();
    void updateNextMinerID() const;
    void createPipe();
    void createPipeName();
    void readNewBlockFromPipe();
    void writeTlvToPipe(const TLV* i_Tlv) const;
    void getNewBlockData();
    void printNewBlockReceived();
    void printSubscribeMessage() const;
    void writeLog(const std::string& i_Log) const;
    
public:
    Miner();
    void Mine();
    void SubscribeToServer() const;
};

#endif // MINER_HPP
