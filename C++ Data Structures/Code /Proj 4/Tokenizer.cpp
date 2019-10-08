#include "provided.h"
#include <string>
#include <sstream>
#include <vector>
using namespace std;

class TokenizerImpl
{
public:
    TokenizerImpl(string separators);
    vector<string> tokenize(const std::string& s) const;
private:
    vector<char> m_tokens;
    string m_separators;
};

TokenizerImpl::TokenizerImpl(string separators)
{
    m_separators = separators;
}

vector<string> TokenizerImpl::tokenize(const std::string& s) const
{
    vector<string> result;
    
    string next = "";

    
    for(int i=0; i< s.size();){
        bool foundSeparator = false;
        for(int j= 0; j<m_separators.size(); j++){
            if(s[i] == m_separators[j]){
                foundSeparator = true;
                break;
            }
        }
        if(foundSeparator){
            if(next != ""){
                result.push_back(next);
            }
            next = "";
        }else{
            next += s[i];
        }
        i++;
    }
    if(next != ""){
        result.push_back(next);
    }
    
    return result;
}
    
    


//******************** Tokenizer functions ************************************

// These functions simply delegate to TokenizerImpl's functions.
// You probably don't want to change any of this code.

Tokenizer::Tokenizer(string separators)
{
    m_impl = new TokenizerImpl(separators);
}

Tokenizer::~Tokenizer()
{
    delete m_impl;
}

vector<string> Tokenizer::tokenize(const std::string& s) const
{
    return m_impl->tokenize(s);
}
