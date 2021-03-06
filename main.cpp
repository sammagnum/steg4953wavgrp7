// main.cpp
//
// this program reads a wave file and displays the header info
//

//#include <windows.h>
#include "WaveMessageEmbedder.h"
#include <stdio.h>
#include "wave.h"
#include <vector>
#include <iostream>
//#include "bitWiseChar.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include<string.h>

using namespace std;


/* Default Parameters */
int BITS = 2;
int SAMPLE_TO_AVG = 4;
bool XTRACT = false;
bool EMBED = false;
bool M_ADDED = false;
string COVER_OR_STEGO;
string MESSAGE;

int twoBytes2Long(BYTE low , BYTE high){
    return (long long)(short)(high << 8) + low;
}


int readChunkHeader(FILE *fptr, W_CHUNK *pChunk)
{
	int x;

	// size = 1, count = 8 bytes
	x = (int) fread(pChunk, 1, 8, fptr);
	if(x != 8) return(FAILURE);

	return(SUCCESS);
} // readChunkHeader

// reads in the data portion of a chunk
BYTE *readChunkData(FILE *fptr, int size)
{
	BYTE *ptr;
	int tmp, x;

	tmp = size%2;	// size MUST be WORD aligned
	if(tmp != 0) size = size + 1;

	ptr = (BYTE *) malloc(size);
	if(ptr == NULL)
	{
		printf("\n\nError, could not allocate %d bytes of memory!\n\n", size);
		return(NULL);
	}

	x = (int) fread(ptr, 1, size, fptr);
	if(x != size)
	{
		printf("\n\nError reading chunkd data!\n\n");
		return(NULL);
	}

	return(ptr);
} // readChunkData

// prints out wave format info
void printFormat(W_FORMAT fmt)
{
	printf("\n\nWAVE FORMAT INFO\n\n");
	printf("Compression Code:		%d\n", fmt.compCode);
	printf("Number of Channels:		%d\n", fmt.numChannels);
	printf("Sample Rate: 			%d\n", (int)fmt.sampleRate);
	printf("Average Bytes/Second:		%d\n", (int)fmt.avgBytesPerSec);
	printf("Block Align: 			%d\n", fmt.blockAlign);
	printf("Bits per Sample: 		%d\n", fmt.bitsPerSample);
	return;
} // printFormat

/* example from opt arg std */
void getoperations(int argc,char ** argv){
	{

		
		int index;
		int c;
		

		opterr = 0;

		while ((c = getopt (argc, argv, "e:x:b:s:m:")) != -1)
		 switch (c)
		   {
		   case 'm':
			 M_ADDED = true;
			 MESSAGE = optarg;
			 break;
		   case 'e':
			 EMBED = true;
			 COVER_OR_STEGO = optarg;
			 
			 break;
		   case 'x':
			 XTRACT = true;
			 COVER_OR_STEGO = optarg;
			 break;
		   case 'b':
			 //bvalue = optarg;
			 BITS = atoi(optarg);
			 break;
		   case 's':
			 SAMPLE_TO_AVG = atoi(optarg);
			 break;
			
		   case '?':
			 if (optopt == 'b' || optopt == 's')
			   fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			 else if (isprint (optopt))
			   fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			 else
			   fprintf (stderr,
						"Unknown option character `\\x%x'.\n",
						optopt);
		
		   default:
			 abort ();
		   }
		   
		   if(EMBED == XTRACT)
		   {
				cout<< "Can't have your cake and eat it too: -e -x non-compatible and required" << endl;
				exit(-1);
		   }
		    if (BITS > 15 || BITS  < 1){
				cout << "Bits out of range: 1 - 15 " << endl;
				exit(-1);
			}	
	
		cout << endl << COVER_OR_STEGO << " is primary file.\n" << endl;
		cout << MESSAGE << " is message file.";

		//printf ("EMBED = %d, XTRACT = %d, BITS = %d, SAMPLE_TO_AVG = %d\n",
			   //EMBED, , cvalue);

		for (index = optind; index < argc; index++)
		 printf ("Non-option argument %s\n", argv[index]);
	
		}
}

int main(int argc, char ** argv)
{
	getoperations(argc,argv);
	FILE *fptr;
	DWORD fileSize;
	int x, cnt, dataFlag, formatFlag, noSampleFrames;
	W_CHUNK chunk[MAX_CHUNKS];		// assuming max of 8 chunks, should only be 3 for you
	BYTE *pChunkData[MAX_CHUNKS];
	W_FORMAT format;		// only 1 format chunk

	DWORD dataChunkSize;

	/*
    if(argc != 2)
	{
		printf("\n\nUSAGE:  wavereader filename.wav\n\n");
		system("pause");exit(-1);
	}
    */
	

	fptr = fopen((char *)COVER_OR_STEGO.c_str(), "rb");
	if(fptr == NULL)
	{
		printf("Could not file file named '%s.'\n\n", argv[1]);
		system("pause");exit(-1);
	}

	

	// pChunk[0] is the chunk representing the file header
	x = readChunkHeader(fptr, &chunk[0]);
	if(x == FAILURE)
    {
          printf("read failed\n\n");
          system("pause");exit(-1);
    }
	// check to make sure it is a RIFF file
	if(memcmp( &(chunk[0].chunkID), "RIFF", 4) != 0)
	{
		printf("\n\nError, file is NOT a RIFF file!\n\n");
		system("pause");exit(-1);
	}
	fileSize = chunk[0].chunkSize + 8;

	// check to make sure it is a wave file
	pChunkData[0] = readChunkData(fptr, 4);

	if(memcmp( pChunkData[0], "WAVE", 4) != 0)
	{
		printf("\n\nError, file is not a WAVE file!\n\n");
		system("pause");exit(-1);
	}

	// chunk[1] should be format chunk, but if not, skip
	cnt = 1;
	dataFlag = -1;
	formatFlag = -1;

	while(cnt < MAX_CHUNKS)
	{
		x = readChunkHeader(fptr, &chunk[cnt]);
		if(x == FAILURE)
        {
              system("pause");
              exit(-1);
        }
		// read in chunk data
		pChunkData[cnt] = readChunkData(fptr, chunk[cnt].chunkSize);
		if(pChunkData[cnt] == NULL)
        {
              system("pause");
              exit(-1);
        }

		if(memcmp( &(chunk[cnt].chunkID), "data", 4) == 0)
			dataFlag = cnt;	// if find data chunk, take note

		if(memcmp( &(chunk[cnt].chunkID), "fmt ", 4) == 0)
		{
			formatFlag = cnt;	//	marks which chunk has format data
			break;	// found format chunk
		}

		cnt++;
	}

	if(cnt == MAX_CHUNKS)
	{
		printf("\n\nError, format chunk not found after 8 tries!\n\n");
		system("pause");exit(-1);
	}

	// check format size to make sure this is not a fancy WAVE file
	if(chunk[cnt].chunkSize != 16)
	{
		printf("\n\nError, this WAVE file is not a standard format - we will not use this one!\n\n");
		system("pause");exit(-1);
	}

	// put format chunk in our special variable
	// format chunk data already contained in pChunkData[cnt]
    memcpy(&format, pChunkData[cnt], 16);

	// make sure we are working with uncompressed PCM data
	if(format.compCode != 1)
	{
		printf("\n\nError, this file does not contain uncompressed PCM data!\n\n");
		system("pause");exit(-1);
	}

	printFormat(format);

	if(dataFlag == -1)	// have not found data chunk yet
	{
		while(cnt < MAX_CHUNKS)
		{
			x = readChunkHeader(fptr, &chunk[cnt]);

			if(x == FAILURE)
            {
                 system("pause");
                 exit(-1);
            }
			// read in chunk data
			pChunkData[cnt] = readChunkData(fptr, chunk[cnt].chunkSize);
			if(pChunkData[cnt] == NULL)
            {
                system("pause");
                exit(-1);
            }

			if(memcmp( &(chunk[cnt].chunkID), "data", 4) == 0)
			{
				dataFlag = cnt;	// found data chunk
				break;
			}

			cnt++;
		}
	}



	// pChunkData[dataFlag] is a pointer to the begining of the WAVE data
	// if 8 bit, then it is unsigned	0 to 255
	// if 16 bit, then it is signed		-32768 to +32767
	// ask me any other questions
	// the web page should answer others

	dataChunkSize = (chunk[dataFlag].chunkSize);
	noSampleFrames = dataChunkSize/format.blockAlign;

	printf("Size of data chunk: %ld\nSample Frames: %d\n",dataChunkSize,noSampleFrames);


	cnt = 0;

    int sampleSize = 16;
	//int noOfBlks = 4, i;
	BYTE * start = pChunkData[dataFlag];
	//srand ( time(NULL) );
	//long long left,right; /* used long long as I anticipate large averages being required to minimize effect on cover */
	//int leftm, rightm,diff;
	
	ifstream::pos_type size;
	char * b_message;

    /* http://www.cplusplus.com/doc/tutorial/files/ */
    ifstream file ((char *)MESSAGE.c_str(), ios::in|ios::binary|ios::ate);



    if (file.is_open())
    {
        size = file.tellg();
        b_message = new char [size];
        file.seekg (0, ios::beg);
        file.read (b_message, size);
        file.close();


        


    }
    
    /* code quote ends here */


   //
   // stego method goes here: BYTE start * is cover data
   //                         char b_message * is message or hidden data
   //
	if(EMBED)
	{
	
	WaveMessageEmbedder w (b_message,size,start,dataChunkSize);
	
	start = w.getStegoData(BITS,SAMPLE_TO_AVG);
	

	
    ofstream outfile ("stegoFile.wav", ios::out | ios::trunc | ios::binary);
	
	cout<< " \nwriting to stegoFile.wav" << endl;
	



	if(outfile.is_open()){
		outfile.write((char *)&(chunk[0]),8);
	    outfile.write((char *)pChunkData[0],4);
	    outfile.write("fmt ",4);
		outfile.write((char *)&sampleSize,4);
        outfile.write((char *)&format,16);
        outfile.write((char *)&chunk[dataFlag],8);
	    outfile.write((char *)start,chunk[dataFlag].chunkSize); //new data
    }
	
    outfile.close();
	}
	if(XTRACT)
	{
	
		WaveMessageEmbedder w (b_message,0,start,dataChunkSize);
		BYTE * out = w.getExtractedData(BITS,SAMPLE_TO_AVG);
		if (!M_ADDED)
			MESSAGE = "message.out";
		ofstream outfile ((char *)MESSAGE.c_str(), ios::out | ios::trunc | ios::binary);
		
		if(outfile.is_open()){
			outfile.write((char *)out,w.getExtractedSize());
		}
		cout << "Message sucessfully written to: " << MESSAGE;
		outfile.close();
	
	}
   





	printf("\n");

	fclose(fptr);

	printf("Exiting...\n");
	
	exit(0);
} // main


