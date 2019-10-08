#include "provided.h"
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class TranslatorImpl
{
public:
    TranslatorImpl();
    bool pushMapping(string ciphertext, string plaintext);
    bool popMapping();
    string getTranslation(const string& ciphertext) const;
private:
    
    struct mymap{
        mymap(){
            for(int i = 0; i < 26; i++){
                m_cipherToPlain[i] = '?';
                m_plainToCipher[i] = '?';
            }
        }
        char m_cipherToPlain[26];
        char m_plainToCipher[26];
    };
    
    mymap m_map;
    vector<mymap> m_stack;
    
};

TranslatorImpl::TranslatorImpl(){
    m_stack.push_back(m_map);
}


bool TranslatorImpl::pushMapping(string ciphertext, string plaintext)
{

    if(ciphertext.size() != plaintext.size())
        return false;
    
     mymap newMap(m_map);
    
    for(int i=0; i< ciphertext.size(); i++){
        if(!isalpha(ciphertext[i]) || !isalpha(plaintext[i]) )
            return false;
        
        int cipherToPlainIndex = toupper(ciphertext[i]) - 'A';
        int plainToCipherIndex = toupper(plaintext[i]) - 'A';
        
        char cipher = toupper(ciphertext[i]);
        char plain = toupper(plaintext[i]);
        
        //No current mapping, add to both arrays
        if(newMap.m_plainToCipher[plainToCipherIndex] == '?' && newMap.m_cipherToPlain[cipherToPlainIndex] == '?'){
            newMap.m_plainToCipher[plainToCipherIndex] = cipher;
            newMap.m_cipherToPlain[cipherToPlainIndex] = plain;
        }
        //If there's already a current mapping, make sure it's consistent
        else{
            if(cipher == newMap.m_plainToCipher[plainToCipherIndex] && plain == newMap.m_cipherToPlain[cipherToPlainIndex])
                continue;
            //Inconsistent, then false.
            return false;
        }
    }
    
    m_map = newMap;
    m_stack.push_back(m_map);
    return true;
}


bool TranslatorImpl::popMapping()
{
    if(m_stack.size() <= 1)
        return false;
    else{
        m_stack.pop_back();
        m_map = m_stack.back();
        return true;
    }
}

string TranslatorImpl::getTranslation(const string& ciphertext) const
{
    string translated = "";
    
    for(int i=0; i<ciphertext.size(); i++){
        if(!isalpha(ciphertext[i])){
            translated += ciphertext[i];
            continue;
        }
        
        if(islower(ciphertext[i])){
            int cipherToPlainIndex = toupper(ciphertext[i]) - 'A';
            translated += tolower(m_map.m_cipherToPlain[cipherToPlainIndex]);
        }
        else{
            int cipherToPlainIndex = toupper(ciphertext[i]) - 'A';
            translated += m_map.m_cipherToPlain[cipherToPlainIndex];
        }
    }
    return translated; 
}

//******************** Translator functions ************************************

// These functions simply delegate to TranslatorImpl's functions.
// You probably don't want to change any of this code.

Translator::Translator()
{
    m_impl = new TranslatorImpl;
}

Translator::~Translator()
{
    delete m_impl;
}

bool Translator::pushMapping(string ciphertext, string plaintext)
{
    return m_impl->pushMapping(ciphertext, plaintext);
}

bool Translator::popMapping()
{
    return m_impl->popMapping();
}

string Translator::getTranslation(const string& ciphertext) const
{
    return m_impl->getTranslation(ciphertext);
}
