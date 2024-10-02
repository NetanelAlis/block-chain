#include "Server.hpp"

Server::Server()
{
    this->createGenesisBlock(this->readDifficulty());
    this->createPipe();
}

void Server::createGenesisBlock(int i_Difficulty)
{
    int         	height = 0;
    int         	timestamp = 0;
    unsigned int    hash = 0;
    unsigned int    prev_hash = 0;
    int        	    difficulty = i_Difficulty;
    int         	nonce = 0;
    int         	relayed_by = 0;

    this->m_BlockChainHead = this->m_SuggestedBlock = { height, timestamp, hash, prev_hash, difficulty, nonce, relayed_by };
}

void Server::ManageBlockChain()
{
    TLV* tlv;
    int count = 0;

    while (true)
    {
        tlv = this->readTlvFromPipe();
        switch (tlv->type)
        {
            case SUBSCRIBE_OP_CODE:
                this->subscribeNewMiner(tlv);
                break;
            case BLOCK_OP_CODE:
                this->handleNewBlock(tlv);
                break;
            default:
                break;
        }
    }

    close(this->m_ReadFD);
    deletePipeNames();
    delete(tlv);
}

void Server::deletePipeNames()
{
    for(const auto& pipeName : this->m_MinerPipes)
    {
        delete(pipeName);
    }
}

void Server::handleNewBlock(const TLV* i_Tlv)
{
    this->m_SuggestedBlock = *i_Tlv->value.block;

    delete(i_Tlv->value.block);
    try
    {
        verifyProofOfWork();
        appendNewBlock();
        updateMiners();
    }
    catch(const Exception& error)
    {
        error.Print();
    }
}

void Server::subscribeNewMiner(const TLV* i_Tlv)
{
    std::ostringstream logger;
    const char* newMinerPipe = i_Tlv->value.pipeName;
    int newMinerID = getMinerID(newMinerPipe, MINER_ID_POS);

    logger << "Received connection request from miner id " << newMinerID << ", pipe name " << newMinerPipe << std::endl; 
    this->writeLog(logger.str());
    this->m_MinerPipes.push_back(newMinerPipe);
    this->sendHeadBlockToMiner(newMinerPipe);
}

void Server::sendHeadBlockToMiner(const char* i_PipeName) const
{
    int writeFD, len;

    writeFD = open(i_PipeName, O_WRONLY | O_NONBLOCK);
    if(errno == ENXIO && writeFD == -1)
    {
        return;
    }
    else if(writeFD == -1)
    {
        throw Exception("Error opening Miner pipe."); 
    }

    len = write(writeFD, &this->m_BlockChainHead, sizeof(BLOCK_T));
    if (len < 0) 
    {
        throw Exception("Error writing Head Block to Miner pipe."); 
    }
}

TLV* Server::readTlvFromPipe()
{
    int len = 0;
    TLV* tlv = new TLV();

    len = read(this->m_ReadFD, (void*)tlv, sizeof(tlv->type) + sizeof(tlv->length));
    if (len < 0 || len > sizeof(tlv->value))
    {
        delete(tlv); 
        throw Exception("Error reading Type and Length of TLV from Server pipe."); 
    }

    if(tlv->type == SUBSCRIBE_OP_CODE)
    {
        tlv->value.pipeName = new char[tlv->length + 1];
        len = read(this->m_ReadFD, tlv->value.pipeName, tlv->length);
        tlv->value.pipeName[tlv->length] = '\0';
    }
    else if(tlv->type == BLOCK_OP_CODE)
    {
        tlv->value.block = new BLOCK_T;
        len = read(this->m_ReadFD, tlv->value.block, tlv->length);
    }

    if (len < 0) 
    {
        delete(tlv); 
        throw Exception("Error reading Value of TLV from Server pipe.");
    }
    
    return tlv;
}

void Server::appendNewBlock() 
{
    this->m_BlockChainHead = this->m_SuggestedBlock;
   
    printHead();
}

void Server::updateMiners() const
{
    for(const auto& currentMinerPipe : this->m_MinerPipes)
    {
        this->sendHeadBlockToMiner(currentMinerPipe);
    }
}

void Server::printHead() const
{
    std::ostringstream logger;

    logger << "server: New block added by " << this->m_BlockChainHead.relayed_by << ", attributes: ";
    logger << "height(" << this->m_BlockChainHead.height << ") ";
    logger << "timestamp(" << this->m_BlockChainHead.timestamp << ") ";
    logger << "hash(0x" << std::hex << this->m_BlockChainHead.hash << ") ";
    logger << "prev_hash(0x" << this->m_BlockChainHead.prev_hash << std::dec << ") ";
    logger << "difficulty(" << this->m_BlockChainHead.difficulty << ") ";
    logger << "nonce(" << this->m_BlockChainHead.nonce << ") \n";
    this->writeLog(logger.str());
}

void Server::verifyProofOfWork() const
{
    unsigned int crcParams[] = {this->m_SuggestedBlock.height,
                                this->m_SuggestedBlock.timestamp,
                                this->m_SuggestedBlock.prev_hash,
                                this->m_SuggestedBlock.nonce,
                                this->m_SuggestedBlock.relayed_by};

    checkCRC(crc32(this->m_SuggestedBlock.prev_hash, (const Bytef*)crcParams, sizeof(crcParams)));
}

void Server::checkCRC(ulong checkSum) const
{
    if(this->m_SuggestedBlock.prev_hash != m_BlockChainHead.hash)
    {
        std::string meassage = ", received 0x" + Exception::toHexString(this->m_SuggestedBlock.prev_hash) + 
                               " but expected 0x" + Exception::toHexString(this->m_BlockChainHead.hash) ;

        throw Exception("prev_hash", meassage, this->m_SuggestedBlock);     
    }
    else if(this->m_SuggestedBlock.height - 1 != this->m_BlockChainHead.height)
    {
        std::string meassage = ", received " + std::to_string(this->m_SuggestedBlock.height) + 
                               " but expected " + std::to_string(this->m_BlockChainHead.height);

        throw Exception("height", meassage, this->m_SuggestedBlock);
    }   
    else if(checkSum != this->m_SuggestedBlock.hash)
    {
        std::string meassage = ", received 0x" + Exception::toHexString(this->m_SuggestedBlock.hash) + 
                               " but calculated 0x" + Exception::toHexString(checkSum);

        throw Exception("hash", meassage, this->m_SuggestedBlock);
    }
    else if(this->m_SuggestedBlock.hash > this->m_DifficultyLimit)
    {
        std::string meassage = ", not eneogh leading 0's";

        throw Exception("difficulty", meassage, this->m_SuggestedBlock); 
    }
}

void Server::createPipe()
{
    std::ostringstream logger;

    if(mkfifo(SERVER_PIPE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1 && errno != EEXIST)
    {
        throw Exception("Error creating Server pipe.");
    }

    logger << "Listening on " << SERVER_PIPE_PATH << std::endl;
    this->writeLog(logger.str());
    this->m_ReadFD = open(SERVER_PIPE_PATH, O_RDONLY);
    if (this->m_ReadFD < 0)
    {
        throw Exception("Error opening Server pipe.");
    }
}

int Server::readDifficulty()
{
    std::ifstream configFile(CONF_FILE_PATH);
    std::string line;
    std::ostringstream logger;
    int difficultyValue;

    logger << "Reading " << CONF_FILE_PATH << "..." << std::endl;
    this->writeLog(logger.str());
    logger.str("");
    logger.clear();
    if (configFile.is_open()) 
    {
        std::getline(configFile, line);
        difficultyValue = std::stoi(line.substr(DIFFICULTY_POS));
        configFile.close();
        Exception::ValidateDifficulty(difficultyValue);
        this->m_DifficultyLimit = pow(float(2), (float)(SIZE_OF_CRC_RESULT - difficultyValue));
        logger << "Difficulty set to "<< difficultyValue << std::endl;
        this->writeLog(logger.str());
    } 
    else 
    {
        throw Exception("Error opening file.");
    }

    return difficultyValue;
}

void Server::writeLog(const std::string& i_Log) const
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
