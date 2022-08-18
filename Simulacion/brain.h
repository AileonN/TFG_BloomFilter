/*
----------------------------------------------
					BRAIN
----------------------------------------------
*/

#pragma once
#include <iostream>
#include <openssl/md5.h>
#include <fstream>
#include <vector>
using namespace std;

#define ADDRESS_LENGTH 8
/*-------------BRAIN CLASS----------*/
#define N 1000000             //Table length (rows) 
#define M 100                    //Number of @'s in each row (columns) 


//Brain type of data
struct Element{                 
	uint64_t address;
	uint64_t timestamp;     //Stamp when this addres was seen
};
class Brain{
private:
	struct Transformation{
        uint64_t number;
        char zero = '\0';
    };
    MD5_CTX hcontext;               //Hash used for index brain
    vector<uint64_t> elementsInRow; //Vector of the number of elements each row has
    uint32_t MAX_TIMESTAMP = 0;     //Number of address to remember
    
public:
    uint64_t global_timestamp = 0;  //Number of access in to brain (2^64 MAX)
    /*
        Create BRAIN
    */
    Brain(uint32_t BrainAddress);
    /*
        Try to find Address in the Storage, if its not there -> remember and return false else return true
        Address: @Block
    */
    bool Find(uint64_t Address); 
    /*For debug*/
    /*
        Print memory of brain
    */
    void PrintStorage();
    /*
        Write memory of brain into a file
        filename: Path to the file
    */
    void WriteStorage(string filename); 
};
