#include "provided.h"
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <fstream>
#include "MyHash.h"
using namespace std;

class WordListImpl
{
public:
    bool loadWordList(string filename);
    bool contains(string word) const;
    vector<string> findCandidates(string cipherWord, string currTranslation) const;
private:
    MyHash<string, vector<string> > m_hash;
    MyHash<string, bool> m_wordAdded;
    
    string getStructure(string word) const{
        string structure = "";
        char  curr_symbol = 'A';
        char letterToSymbol [26];
        for(int j =0 ; j < 26 ; j++)
            letterToSymbol[j] = '\0';
        for(int i=0; i< word.size(); i++){
            if(word[i] == '\''){
                structure += '\'';
                continue;
            }
            
            char wordUpper = toupper(word[i]);
            
            if(letterToSymbol[wordUpper - 'A']  == '\0'){
                letterToSymbol[wordUpper - 'A'] = curr_symbol;
                curr_symbol++;
            }
            structure += letterToSymbol[wordUpper - 'A'];
        }
        return structure;
    }
};



bool WordListImpl::loadWordList(string filename)
{
    ifstream inFile(filename);
    string x;
    if(inFile){
        m_hash.reset();
        while(getline(inFile, x)){
            string structure = "";
            bool containsInvalid = false;
            char curr_symbol = 'A';
            char letterToSymbol [26];
            for(int j =0 ; j < 26 ; j++)
                letterToSymbol[j] = '\0';
            for(int i=0; i< x.size(); i++){
                if(!isalpha(x[i]) && x[i] != '\''){
                    containsInvalid = true;
                    break;
                }
                
                if(x[i] == '\''){
                    structure += '\'';
                    continue;
                }
                
                x[i] = toupper(x[i]);
                
                if(letterToSymbol[x[i] - 'A']  == '\0'){
                    letterToSymbol[x[i] - 'A'] = curr_symbol;
                    curr_symbol++;
                }
                structure += letterToSymbol[x[i] - 'A'];
            }
            if(containsInvalid)
                continue;
            
            vector<string>* value = m_hash.find(structure);
            if(value == nullptr)
                m_hash.associate(structure, {x});
            else
                value->push_back(x);
            m_wordAdded.associate(x, true);
        }
        inFile.close();
        return true;
    }
    else
        return false;
}

bool WordListImpl::contains(string word) const
{
    //TODO: Make this O(1)
    for(int i=0; i<word.size(); i++)
        word[i] = toupper(word[i]);
    
    if(m_wordAdded.find(word) != nullptr)
        return true;
    else
        return false;
}

vector<string> WordListImpl::findCandidates(string cipherWord, string currTranslation) const
{
    for(int i=0; i< cipherWord.size(); i++){
        cipherWord[i] = toupper(cipherWord[i]);
        if(isalpha(currTranslation[i]))
            currTranslation[i] = toupper(currTranslation[i]);
    }
    string pattern = getStructure(cipherWord);
    
    
    vector<string> possibleCandidates;
    
    if(m_hash.find(pattern) == nullptr)
        return possibleCandidates;
    
    if(cipherWord.size() != currTranslation.size())
        return possibleCandidates;

    bool isMatched = true;
    
    vector<string> matchPattern = *m_hash.find(pattern);
    
    vector<string>:: const_iterator iter= matchPattern.begin();
    for(; iter != matchPattern.end(); iter++){
        for(int i=0; i<(*iter).size(); i++){
            if(currTranslation[i] != (*iter)[i] && currTranslation[i] != '?'){
                isMatched =  false;
                break;
            }
            isMatched = true;
        }
        if(isMatched)
            possibleCandidates.push_back(*iter);
    }
    return possibleCandidates; 
}

//***** hash functions for string, int, and char *****

unsigned int hash(const std::string& s)
{
    return std::hash<std::string>()(s);
}

unsigned int hash(const int& i)
{
    return std::hash<int>()(i);
}

unsigned int hash(const char& c)
{
    return std::hash<char>()(c);
}

//******************** WordList functions ************************************

// These functions simply delegate to WordListImpl's functions.
// You probably don't want to change any of this code.

WordList::WordList()
{
    m_impl = new WordListImpl;
}

WordList::~WordList()
{
    delete m_impl;
}

bool WordList::loadWordList(string filename)
{
    return m_impl->loadWordList(filename);
}

bool WordList::contains(string word) const
{
    return m_impl->contains(word);
}

vector<string> WordList::findCandidates(string cipherWord, string currTranslation) const
{
   return m_impl->findCandidates(cipherWord, currTranslation);
}
