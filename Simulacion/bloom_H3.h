/*
----------------------------------------------
				BLOOM FILTERS
----------------------------------------------
*/

#pragma once
#include <iostream>
#include <openssl/md5.h>
#include <vector>
#include <cstring>
#include <random>
using namespace std;
/*-----DEFINES-----*/
#define ADDRESS_LENGTH 8 //8 bytes

/*----- BLOOM FILTERS ----*/
struct Stats{
  uint64_t TP=0,TN=0,FP=0,FN=0;
};
class BloomFilter {
protected:
    struct Transformation{
        uint64_t number;
        char zero = '\0';
    };

    /*-----------Parameters----------*/
    unsigned int size;              //Size of bloomfilter (MAX 2^32 entries de 1 bit, 512 MB )
    vector<uint64_t> hashseeds_H3; //Seeds of 20b
	uint8_t nhash;					  //Number of Function Hashes
    unsigned int counter1 = 0;        //Number of 1's in BF
    float P;                          //Max percent. of 1's that a filter can have (A2,BASIC) or the number of random cells are decreased (SBF)
    Stats s;                          //STATS
    unsigned int dSet = 0, dReset = 0; //SR debug variables; number of address with set or reset functionality
    /*-----------Functions----------*/

    /*
        Assign bloom filter parameters and creates MD5 hashs
        Size: Number of BF cells
        Nhash: Number of BF hashes
        Debug: If set, hash seeds are static. Otherwise are created dynamically
        P: Max percent. of 1's that a filter can have (A2,BASIC) or the number of random cells are decreased (SBF)
    */
    BloomFilter(unsigned int size, uint8_t nhash, bool debug, float P);
    /*
        Return size(hashseeds) indexs of BF between 0-size 
        Address: @Block
    */
    vector<unsigned int> GetIndexs(uint64_t Address);
    /*
        The bloom filter remember the accessed address 
        Index: List of hash indexs
    */
    virtual void RememberMe(vector<unsigned int> Indexs) = 0;
    
    /*
        Clean BF 
    */
    virtual void clean() = 0;
    /*
        Return true if clean (nº 1's >= %P, A2 and Basic)
    */
    bool DoClean();
    

public:

    /*

        Returns true if the bloom filter remember that address (value=1 in all indexs)
        Otherwise return false and remember it for next time (at least one value is zero)
        Address: @Block
        onlyRead: If set, the BF wont modify his state.
    */
    virtual bool DoYouRememberMe(uint64_t Address, bool onlyRead) = 0;
    /*
        Update stats of BF (TP,TN,FP,FN)
        stats: "TP","TN","FP","FN" (Name of update state)
    */
    void AddStats(string stats);
    /*
        Print stats of BF (TP,TN,FP,FN)
    */
    void PrintStats();
    /*
        Return stats of BF (TP,TN,FP,FN)
    */
    Stats GetStats();

    /*  For Debug */
    
    /*
        Print BF storage
    */
    virtual void PrintStorage() = 0;
    /* 
        Return the number of directions that reset or set
    */
    unsigned int getDSet();
    unsigned int getDReset();


};

class Basic : virtual public BloomFilter{
    
    vector<bool> Storage;     //Memory of bloomfilter 
    void clean();
    void RememberMe(vector<unsigned int> Indexs);
public:
    /*
        Create an empty basic filter (all 0's) 
        Size: Number of cells
        nHash: Number of hashes
        debug: If set, hash seeds are static. Otherwise are created dynamically
        P: Max percent. of 1's that a filter can have 
    */
    Basic(unsigned int size, uint8_t nhash, bool debug, float P);  
    bool DoYouRememberMe(uint64_t Address, bool onlyRead);

    /*  For Debug*/
    
    /*
        Print BF storage
    */
    void PrintStorage();
  
};

class A2 : virtual public  BloomFilter{
    
    vector<vector<bool>> Storages; //Memory of bloomfilter 
    bool recentS; //0 is the one who has got recent address and 1 the olders
    void clean();
    void RememberMe(vector<unsigned int> Indexs);
public:
     /*
        Create an empty A2 filter (all 0's) 
        Size: Cells of each table
        nHash: Number of hashes
        debug: If set, hash seeds are static. Otherwise are created dynamically
        P: Max percent. of 1's that a filter can have 
    */
    A2(unsigned int size, uint8_t nhash, bool debug, float P);
    bool DoYouRememberMe(uint64_t Address, bool onlyRead);
 
    /*  For Debug */
    /*
        Print BF storage
    */
    void PrintStorage();

};

class SBF : virtual public  BloomFilter{
    
    vector<uint16_t> Storage; //Memory of bloomfilter with more than 1 bit X cell
    uint16_t MAX;             //Max value of the cell
    //Random generator 
    random_device rd;							    
    mt19937_64 gen;
    uniform_int_distribution<unsigned int> dist;

    void clean();
    void RememberMe(vector<unsigned int> Indexs);
public:
    /*
        Create an empty SBF filter (all 0's) 
        Size: Number of cells 
        nHash: Number of hashes
        debug: If set, hash seeds are static. Otherwise are created dynamically
        MAX: Cell max value
        P: Number of random cells that are decreased
    */
    SBF(unsigned int size, uint8_t nhash, bool debug, uint32_t MAX, float P);
    bool DoYouRememberMe(uint64_t Address, bool onlyRead);
    /*  For Debug */
    /*
        Print BF storage
    */
    void PrintStorage();

};
class SR : virtual public BloomFilter{
    
    vector<bool> Storage;           //Memory of bloomfilter 
    void clean();
    void RememberMe(vector<unsigned int> Indexs);
 
public:
    /*
        Create a SR filter with random 1's and 0's
        Nhash must be pair. Half nash is for pair address and the other half is for odds
        Size: Number of cells 
        nHash: Number of hashes
        debug: If set, hash seeds are static. Otherwise are created dynamically
    */
    SR(unsigned int size, uint8_t nhash, bool debug);  
    bool DoYouRememberMe(uint64_t Address, bool onlyRead);

    /*  For Debug
    */
    /*
        Print BF storage
    */
    void PrintStorage();
    
  
};









