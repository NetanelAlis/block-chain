#include "Miner.hpp"

Miner::Miner()
{
    this->readConfigurationFile();
    this->updateNextMinerID();
    this->createPipe();
}

void Miner::Mine()
{   
    while (true)
    {
        this->readNewBlockFromPipe();
        this->getHash();
        if(this->isValidHash())
        {
            this->suggestBlock();
        }  
    }
}

void Miner::readNewBlockFromPipe()
{
    ssize_t len = read(this->m_ReadFD, &this->m_CurrentBlock, sizeof(BLOCK_T));
    if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        throw Exception("Error reading from Miner pipe.");
    }

    if(len > 0)
    {
        this->printNewBlockReceived();
        this->getNewBlockData();
    }
}

void Miner::getNewBlockData()
{
    this->m_CurrentBlock.height++;
    this->m_CurrentBlock.prev_hash = this->m_CurrentBlock.hash;
    this->m_CurrentBlock.nonce = 0;
    this->m_CurrentBlock.relayed_by = this->m_MinerID;
}

void Miner::printNewBlockReceived()
{
    std::ostringstream logger;

    logger << "Miner #" << this->m_MinerID << " received ";
    if(this->m_IsFirstBlock)
    {
        logger << "first ";
        this->m_IsFirstBlock = false;
    }

    logger << "block: relayed_by(" << this->m_CurrentBlock.relayed_by << "), ";
    logger << "height(" << this->m_CurrentBlock.height << "), ";
    logger << "timestamp(" << this->m_CurrentBlock.timestamp << "), ";
    logger << "hash(0x" << std::hex << this->m_CurrentBlock.hash << "), ";
    logger << "prev_hash(0x" << this->m_CurrentBlock.prev_hash << std::dec << "), ";
    logger << "difficulty(" << this->m_CurrentBlock.difficulty << "), ";
    logger << "nonce(" << this->m_CurrentBlock.nonce << ")" << std::endl;
    this->writeLog(logger.str());
}

void Miner::suggestBlock()
{
    TLV tlv;

    tlv.type = BLOCK_OP_CODE;
    tlv.length = sizeof(BLOCK_T);
    tlv.value.block = &this->m_CurrentBlock; 
    this->writeTlvToPipe(&tlv);
    this->printSuggestion();
}

void Miner::printSuggestion() const
{
    std::ostringstream logger;

    logger << "Miner #" << this->m_MinerID;
    logger << ": mined a new block #" << this->m_CurrentBlock.height;
    logger << " with the hash 0x" <<  std::hex << this->m_CurrentBlock.hash << std::dec << std::endl;
    this->writeLog(logger.str());
}

bool Miner::isValidHash() const
{
   return this->m_CurrentBlock.hash < this->m_DifficultyLimit;
}

void Miner::getHash()
{
    this->m_CurrentBlock.nonce++;
    this->m_CurrentBlock.timestamp = std::time(nullptr);
    unsigned int crcParams[] = {this->m_CurrentBlock.height,
                                this->m_CurrentBlock.timestamp,
                                this->m_CurrentBlock.prev_hash,
                                this->m_CurrentBlock.nonce,
                                this->m_CurrentBlock.relayed_by};

    this->m_CurrentBlock.hash = crc32(this->m_CurrentBlock.prev_hash, (const Bytef*)crcParams, sizeof(crcParams));
}

void Miner::readConfigurationFile()
{
    std::ifstream configFile(CONF_FILE_PATH);
    std::string line;

    if (configFile.is_open()) 
    {
        std::getline(configFile, line);
        this->m_CurrentBlock.difficulty = std::stoi(line.substr(DIFFICULTY_POS));
        std::getline(configFile, line);
        this->m_MinerID = this->m_CurrentBlock.relayed_by = std::stoi(line.substr(MINER_ID_POS));
        configFile.close();
        Exception::ValidateDifficulty(this->m_CurrentBlock.difficulty);
        this->m_DifficultyLimit = pow(float(2), (float)(SIZE_OF_CRC_RESULT - this->m_CurrentBlock.difficulty));
    } 
    else 
    {
        throw Exception("Error opening file.");
    }
}

void Miner::updateNextMinerID() const
{
    std::ofstream configFile(CONF_FILE_PATH);
    int nextMinerID = this->m_MinerID + 1;

    if (configFile.is_open()) 
    {
        configFile << "DIFFICULTY=" << this->m_CurrentBlock.difficulty << std::endl << "NEXT_MINER_ID=" << nextMinerID;
        configFile.close();
    }
    else 
    {
        throw Exception("Error opening Conf file.");
    }
}

void Miner::createPipeName()
{
    char numStr[20]; 
    size_t len;

    strcpy(this->m_PipeName, MINER_PIPE_PATH);
    len  = strlen(this->m_PipeName);
    snprintf(numStr, sizeof(numStr), "%d", this->m_MinerID);
    strcat(this->m_PipeName, numStr);
}

void Miner::createPipe()
{
    this->createPipeName();
    if(mkfifo(this->m_PipeName, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1 && errno != EEXIST)
    {
        throw Exception("Error creating Miner pipe.");
    }

    this->m_ReadFD = open(this->m_PipeName, O_RDONLY | O_NONBLOCK);
    if (this->m_ReadFD < 0)
    {
        throw Exception("Error opening Miner pipe.");
    }
}

void Miner::SubscribeToServer() const
{
    TLV tlv;
    tlv.type = SUBSCRIBE_OP_CODE;
    tlv.length = strlen(this->m_PipeName);
    tlv.value.pipeName = this->m_PipeName;
    this->writeTlvToPipe(&tlv);
    this->printSubscribeMessage();
}

void Miner::printSubscribeMessage() const
{
    std::ostringstream logger;

    logger << "Miner " << this->m_MinerID << " sent connect request on " << this->m_PipeName << std::endl;
    this->writeLog(logger.str());
}

void Miner::writeTlvToPipe(const TLV* i_Tlv) const
{
    int writeFD, len = 0;

    writeFD = open(SERVER_PIPE_PATH, O_WRONLY);
    if(writeFD == -1)
    {
        throw Exception("Error opening Server pipe.");
    }

    len = write(writeFD, (void*)i_Tlv, sizeof(i_Tlv->type) + sizeof(i_Tlv->length));
    if (len < 0)
    {
        throw Exception("Error writing Type and Length of TLV to Server pipe.");
    }
    
    if(i_Tlv->type == SUBSCRIBE_OP_CODE)
    {
        len = write(writeFD, i_Tlv->value.pipeName, i_Tlv->length);
    }
    else if(i_Tlv->type == BLOCK_OP_CODE)
    {
        len = write(writeFD, i_Tlv->value.block, i_Tlv->length);
    }

    if (len < 0)
    {
        throw Exception("Error writing Value of TLV to Server pipe."); 
    }
}

void Miner::writeLog(const std::string& i_Log) const
{
    std::ofstream logsFile;

    logsFile.open(LOGS_FILE_PATH, std::ios_base::app);
    if (logsFile.is_open()) 
    {
        logsFile << i_Log;
        logsFile.close();
    }
    else 
    {
        throw Exception("Error writing to Logs file.");
    }
}
