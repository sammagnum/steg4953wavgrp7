#ifndef WAVEMESSAGEEMBEDDER_H
#define WAVEMESSAGEEMBEDDER_H
#include <stdio.h>
#include <iostream>
#include <bitset>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <iomanip>
#include <pthread.h>
#include <semaphore.h>
#include <algorithm>
#include <queue>



//#include "windows.h"
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
	static const unsigned int TMAX = 30;

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

class WaveMessageEmbedder
{
private:
	
	std::queue<pthread_t> garbage;
    std::vector<char> message;
    unsigned short * cover;
	
    unsigned int lsb_bits;
	unsigned int n_bytes;
	void convertCoverToBYTE(BYTE  * bCover);
    //std::vector<BYTE> extractm;
    DWORD mByteCount;
    DWORD cByteCount;
	unsigned long long current;
	pthread_mutex_t mutexembed;
    unsigned long long currentbits;
    //std::vector<int> sampleVector;
    
	void prependSize(unsigned int size);
	void extractSize();
    void setMessageByte(BYTE val);
	
    void setCoverByte(BYTE val,BYTE hival,unsigned int cnt);
    unsigned int averageNLeftSamples(unsigned int n,unsigned long long c);
    unsigned int getlsb(unsigned int b,unsigned int value);
    unsigned int getNbitsFromMessage(unsigned int n);
   // int averageNLeftSamples(int n);
    unsigned int averageNRightSamples(unsigned int n, unsigned long long c);
    void extract(unsigned int b,unsigned int n);




public:


    //insert overloaded constructor with cover only for extraction
	void * embed();
	sem_t sem;
    WaveMessageEmbedder(char * message,unsigned int messageSize, unsigned char * cover, unsigned long coverSize);
    virtual ~WaveMessageEmbedder();
	unsigned int getExtractedSize();
    BYTE * getStegoData(unsigned int bitsPerSample,unsigned int noOfBytesToAverage);
    BYTE * getExtractedData(unsigned int bitsPerSample,unsigned int noOfBytesToAverage);
    void print();


};

#endif // WAVEMESSAGEEMBEDDER_H