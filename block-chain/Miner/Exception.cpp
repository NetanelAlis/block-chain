#include "Exception.hpp"

Exception::Exception(std::string i_ErrorMessage)
{
    this->m_ErrorMessage = i_ErrorMessage;
}

void Exception::ValidateDifficulty(const int i_Difficulty)
{
    if(i_Difficulty < 0 || i_Difficulty >= SIZE_OF_CRC_RESULT)
    {
        throw Exception("Error: Invalid Difficulty value!");
    }
}
