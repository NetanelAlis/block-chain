#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include "Utils.hpp"

class Exception
{
private:

    std::string m_ErrorMessage;

public:
    static void ValidateDifficulty(const int i_Difficulty);
    Exception(std::string i_ErrorMessage);
    void Print() const {std::cout << this->m_ErrorMessage << '\n';}
};

#endif // EXCEPTION_HPP
