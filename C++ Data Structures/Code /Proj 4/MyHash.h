// MyHash.h

// Skeleton for the MyHash class template.  You must implement the first seven
// member functions; we have implemented the eighth.

#ifndef MyHash_h
#define MyHash_h

#include <iostream>
using namespace std;

template<typename KeyType, typename ValueType>
class MyHash
{
public:
    MyHash(double maxLoadFactor = 0.5);
    ~MyHash();
    void reset();
    void associate(const KeyType& key, const ValueType& value);
    int getNumItems() const;
    double getLoadFactor() const;

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const
    {
        int bucketIndex = getBucketNumber(key, m_bucket);
        HashNode* current = m_myhash[bucketIndex];
        while(current != nullptr){
            if(current->m_key == key)
                return &current->m_value;
            current = current->m_next;
        }
        return nullptr;
    }



    // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key){
        return const_cast<ValueType*>(const_cast<const MyHash*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    MyHash(const MyHash&) = delete;
    MyHash& operator=(const MyHash&) = delete;

private:
    
    struct HashNode{
        HashNode(const KeyType& k, const ValueType& v, HashNode* nextNode){
            m_key = k;
            m_value = v;
            m_next = nextNode;
        }
        
        ~HashNode(){
            if(m_next !=nullptr)
                delete m_next;
        }
        
        KeyType m_key;
        ValueType m_value;
        HashNode * m_next;
    };
    
    unsigned int getBucketNumber(const KeyType& k, int buckets) const{
        unsigned int hash(const KeyType& k);
        unsigned int h = hash(k);
        return h % buckets;
    }
    
    void removeHash(){
        
        for(int i=0; i<m_bucket ; i++){
            delete m_myhash[i];
        }
        delete [] m_myhash;
    }
    
    void addNode(const KeyType& key, const ValueType& value){
        m_stored++;
        int bucketIndex = getBucketNumber(key, m_bucket);
        //Chain the new node to old linked list from the head.
        //O(1) insertion.
        m_myhash[bucketIndex] = new HashNode(key, value, m_myhash[bucketIndex]);
    }
    
    HashNode** reHashed(){
        HashNode** newMyHash;
        
        //Initialize new hashtable
        int newM_bucket = m_bucket * 2;
        newMyHash = new HashNode* [newM_bucket];
        for(int i=0; i < newM_bucket; i++)
            newMyHash[i] = nullptr;
        
        //Effectively rehash every word in old hashtable. O(X) rehashing.
        //Visit each bucket in the old hashtable
        for(int i = 0; i < m_bucket; i++){
            HashNode* oldLinkedList = m_myhash[i];
            //All keys in old hashtables are unique, there's no need to check.
            //Traverse thru linked list in each bucket.
            while(oldLinkedList !=nullptr){
                //Find new hash and insert to appropriate bucket in new hashtable
                int newIndex = getBucketNumber(oldLinkedList->m_key, newM_bucket);
                newMyHash[newIndex] = new HashNode(oldLinkedList->m_key, oldLinkedList->m_value, newMyHash[newIndex]);
                oldLinkedList = oldLinkedList->m_next;
            }
        }
        //Remove old hashtable
        removeHash();
        return newMyHash;
    }
        
    HashNode **m_myhash;
    int m_bucket;
    int m_stored;
    double m_maxLoadFactor;

};


template<typename KeyType, typename ValueType>
MyHash<KeyType,ValueType>::MyHash(double maxLoadFactor){
    if(maxLoadFactor <= 0)
        maxLoadFactor = 0.5;
    if(maxLoadFactor >2.0)
        maxLoadFactor = 2.0;
    
    m_stored = 0;
    m_bucket = 100;
    m_maxLoadFactor = maxLoadFactor;
    
    //An array of pointers to HashNodes
    m_myhash = new HashNode*[m_bucket];
    for(int i=0; i <m_bucket; i++)
        m_myhash[i] = nullptr;
}

template<typename KeyType, typename ValueType>
MyHash<KeyType, ValueType>::~MyHash(){
    removeHash();
}

template<typename KeyType, typename ValueType>
void MyHash<KeyType, ValueType>::reset(){
    
    removeHash();
    m_stored = 0;
    m_bucket = 100;
    m_myhash = new HashNode*[m_bucket];
    for(int i=0; i <m_bucket; i++)
        m_myhash[i] = nullptr;

}

template<typename KeyType, typename ValueType>
void MyHash<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value){
    
    ValueType* found = find(key);
    if(found != nullptr){
        *found = value;
        return;
    }
    
    addNode(key, value);
    
    if(getLoadFactor() >= m_maxLoadFactor){
        m_myhash = reHashed();
        m_bucket = m_bucket * 2;
    }
}

template<typename KeyType, typename ValueType>
int MyHash<KeyType,ValueType>:: getNumItems() const{
    return m_stored;
}

template<typename KeyType, typename ValueType>
double MyHash<KeyType,ValueType>::getLoadFactor() const{
    return static_cast<double>(m_stored)/m_bucket;
}

#endif //MyHash_h






