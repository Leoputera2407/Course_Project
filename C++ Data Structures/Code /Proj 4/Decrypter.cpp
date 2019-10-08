#include "provided.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

class DecrypterImpl
{
public:
    DecrypterImpl();
    bool load(string filename);
    vector<string> crack(const string& ciphertext);
private:
    WordList m_wordList;
    Tokenizer m_tokenizer;
    
    
    vector<string> returnListToProcess (vector<string> words, Translator& t){
        
        vector<string> hasMostUntranslated;
        int mostUntranslated = 0;
        
        for(auto it: words){
            int numberUntranslated = 0;
            string translated = t.getTranslation(it);
            for(int i=0; i<translated.size(); i++){
                if(translated[i] =='?')
                    numberUntranslated++;
            }
            if(numberUntranslated > mostUntranslated){
                hasMostUntranslated.clear();
                hasMostUntranslated.push_back(it);
            }
            
            if(numberUntranslated == mostUntranslated)
                hasMostUntranslated.push_back(it);
        }
        
        return hasMostUntranslated;
    }
    
        
    string findNextWord(vector<string> words, Translator& t){
            int indexOfMostUntranslated = 0;
            int mostUntranslated = 0;
            int curr_index = 0;
            for(auto it: words){
                int numberUntranslated = 0;
                string translated = t.getTranslation(it);
                for(int i=0; i<translated.size(); i++){
                    if(translated[i] =='?')
                        numberUntranslated++;
                }
                if(numberUntranslated > mostUntranslated)
                    indexOfMostUntranslated = curr_index;
                curr_index++;
            }
            return words[indexOfMostUntranslated];
    }
    

    bool fullyTranslated(string word){
        for(int i=0; i< word.size(); i++){
            if(word[i] =='?')
                return false;
        }
        return true;
    }
    
    
    bool allTranslatedFound(vector<string> translated){
        for(vector<string>::const_iterator it = translated.begin(); it != translated.end(); it++)
        {
            if(!m_wordList.contains(*it))
                return false;
        }
        return true;
    }
  
    string popMostUntranslated( vector<string>& tokens,Translator& t){
        string mostUntranslated;
        int maxNumQmarks=0;
        int maxIdx = 0;
        int idx = 0;
        for(auto token = tokens.begin(); token != tokens.end(); ++token){
            //gets most untranslated and pop it from tokens vector.
            string translatedToken = t.getTranslation(*token);
            int numQmarks=0;
            for(int i = 0; i < translatedToken.size();++i){
                if(translatedToken[i] =='?')
                    ++numQmarks;
            }
            if(numQmarks >= maxNumQmarks){
                maxNumQmarks = numQmarks;
                maxIdx = idx;
                mostUntranslated = *token;
            }
            ++idx;
        }
        tokens.erase(tokens.begin() + maxIdx);
        return mostUntranslated; //CONFIRM is this correct?
    }

    
   
    vector<string> orderByMostUntranslated(vector<string> tokens, Translator& t){
        vector<string> result;
        result.push_back(popMostUntranslated(tokens, t));
        return result;
    }

    void removeUsed(vector<string>& ciphers,vector<string> usedTokens){
        
        for(auto p = ciphers.begin() ; p != ciphers.end();){
            bool hasRemoved = false;
            for(auto q = usedTokens.begin(); q != usedTokens.end(); ++q){
                
                if( *p == *q){
                    ciphers.erase(p);
                    hasRemoved = true;
                    break;
                }
            }
            if(!hasRemoved)
                ++p;
        }
    }
    
    void process(vector<string>& output, vector<string> usedToken,const string& ciphertext, Translator& t){
            
        vector<string> tokenizedCipher = m_tokenizer.tokenize(ciphertext);
        vector<string> ordered = orderByMostUntranslated(tokenizedCipher, t);
        removeUsed(ordered, usedToken);
        string wordToProcess = ordered[0];
        
        usedToken.push_back(wordToProcess);
        
        string translation = t.getTranslation(wordToProcess);
        vector<string> candidates = m_wordList.findCandidates(wordToProcess, translation);
        
        if(candidates.empty()){
            t.popMapping();
            return;
        }
            
        for(auto candidate: candidates){
            if(!t.pushMapping(wordToProcess, candidate)){
                continue;
            }
            
            string newTranslation = t.getTranslation(ciphertext);
            vector<string> newTranslationTokens = m_tokenizer.tokenize(newTranslation);
            vector<string> translatedFully = {};
            vector<string> untranslated = {};
            
            for(auto translationToken: newTranslationTokens){
                if(fullyTranslated(translationToken))
                    translatedFully.push_back(translationToken);
                else
                    untranslated.push_back(translationToken);
            }
            
            if(!translatedFully.empty()){
                if(!allTranslatedFound(translatedFully)){
                    t.popMapping();
                    continue;
                }
                else{
                    if(untranslated.empty()){
                        output.push_back(newTranslation);
                        t.popMapping();
                        continue;
                    }
                    //Not fully translated
                    else{
                        process(output, usedToken, ciphertext, t);
                    }
                }
            }
        }
       t.popMapping();
        return;
    }
    
};

DecrypterImpl::DecrypterImpl()
:m_tokenizer(" ,;:.!()[]{}-\"#$%^&")
{
}

bool DecrypterImpl::load(string filename)
{
    return m_wordList.loadWordList(filename);
}

vector<string> DecrypterImpl::crack(const string& ciphertext)
{
    vector<string> output;
    if(!m_tokenizer.tokenize(ciphertext).empty()){
        Translator t;
        process(output, vector<string>() ,ciphertext,t);
        sort(output.begin(), output.end());
        return output;
    }
    output.push_back(ciphertext);
    return output;
}


//******************** Decrypter functions ************************************

// These functions simply delegate to DecrypterImpl's functions.
// You probably don't want to change any of this code.

Decrypter::Decrypter()
{
    m_impl = new DecrypterImpl;
}

Decrypter::~Decrypter()
{
    delete m_impl;
}

bool Decrypter::load(string filename)
{
    return m_impl->load(filename);
}

vector<string> Decrypter::crack(const string& ciphertext)
{
   return m_impl->crack(ciphertext);
}
