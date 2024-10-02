#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include "Utils.hpp"

class Exception
{
private:

    std::string m_ErrorMessage;

public:
    static std::string toHexString(unsigned int i_Numer);
    static void ValidateDifficulty(const int i_Difficulty);
    Exception(std::string i_ValueName, std::string i_Detail,const BLOCK_T& i_SuggestedBlock); 
    Exception(std::string i_ErrorMessage);
    void BuildErrorMessage(std::string i_ValueName, std::string i_Detail,const BLOCK_T& i_SuggestedBlock);
    void Print() const {std::cout << this->m_ErrorMessage << std::endl;}
};

#endif // EXCEPTION_HPP
