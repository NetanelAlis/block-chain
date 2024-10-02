#include "Exception.hpp"

Exception::Exception(std::string i_ValueName, std::string i_Detail,const BLOCK_T& i_SuggestedBlock)
{
    this->BuildErrorMessage(i_ValueName, i_Detail, i_SuggestedBlock); 
}

Exception::Exception(std::string i_ErrorMessage)
{
    this->m_ErrorMessage = i_ErrorMessage;
}

void Exception::BuildErrorMessage(std::string i_ValueName, std::string i_Detail, const BLOCK_T& i_SuggestedBlock)
{
    this->m_ErrorMessage = "wrong " + i_ValueName + " for block #" + std::to_string(i_SuggestedBlock.height) +
                                       " by miner " + std::to_string(i_SuggestedBlock.relayed_by) + i_Detail;
}

void Exception::ValidateDifficulty(const int i_Difficulty)
{
    if(i_Difficulty < 0 || i_Difficulty >= SIZE_OF_CRC_RESULT)
    {
        throw Exception("Error: Invalid Difficulty value!");
    }
}

std::string Exception::toHexString(unsigned int i_Numer)
{
    std::stringstream hexNumber;
    hexNumber << std::hex << i_Numer;
    std::string hexString = hexNumber.str();
    return hexString;
}
