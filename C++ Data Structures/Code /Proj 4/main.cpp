#include "provided.h"
#include "MyHash.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cctype>
#include <random>
#include <algorithm>
#include <numeric>
using namespace std;
//
//void f()
//{
//    WordList wl;
//    if ( ! wl.loadWordList("/Users/Laputa9072/Desktop/Proj 4/Proj 4/wordlist.txt"))
//    {
//        cout << "Unable to load word list" << endl;
//        return;
//    }
//    if (wl.contains("onomatopoeia"))
//        cout << "I found onomatopoeia!" << endl;
//    else
//        cout << "Onomatopoeia is not in the word list!" << endl;
//    string cipher = "xyqbbq";
//    string decodedSoFar = "?r????";
//    vector<string> v = wl.findCandidates(cipher, decodedSoFar);
//    if (v.empty())
//        cout << "No matches found" << endl;
//    else
//    {
//        cout << "Found these matches:" << endl;
//        for (size_t k = 0; k < v.size(); k++)
//            cout << v[k] << endl; // writes grotto and troppo
//    }
//}


const string WORDLIST_FILE = "/Users/Laputa9072/Desktop/Proj 4/Proj 4/wordlist.txt";


int main(){
    Decrypter dl;
    WordList wl;
    wl.loadWordList(WORDLIST_FILE);
    dl.load(WORDLIST_FILE);
    vector<string> answer = dl.crack("y qook ra bdttook yqkook");
//    vector<string> answer = dl.crack("Jxwpjq qwrla glcu pcx qcn xkvv dw uclw ekarbbckpjwe dq jzw jzkpta jzrj qcn ekep'j ec jzrp dq jzw cpwa qcn eke ec. -Urls Jxrkp");
//    vector<string> answer = dl.crack("Trcy oyc koon oz rweelycbb vmobcb, wyogrcn oecyb; hjg ozgcy tc moox bo moya wg grc vmobck koon grwg tc ko yog bcc grc oyc trlvr rwb hccy oecyck zon jb. -Rcmcy Xcmmcn");
//    vector<string> answer = dl.crack("Xjzwq gjz cuvq xz huri arwqvudiy fuk ufjrqoq svquxiy. -Lzjk Nqkkqcy");
//    vector<string> answer = dl.crack("Axevfvu lvnelvp bxqp mvpprjv rgl bvoop Grnxvgkvuj dqupb jvbp buvrbvl be lqggvu.");
//    vector<string> answer = dl.crack("kg");
//    vector<string> answer = dl.crack("!!!!!!");

    
    for(vector<string>::const_iterator it = answer.begin(); it != answer.end(); it++){
        cout<<*it<<endl;
    }
    
  }
