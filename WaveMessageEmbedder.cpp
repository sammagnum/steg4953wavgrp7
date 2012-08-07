#include "WaveMessageEmbedder.h"

using std::setw;
using std::right;
using std::cout;
using std::endl;
using std::string;


WaveMessageEmbedder::WaveMessageEmbedder(char * m, unsigned int mSize, unsigned char * c, unsigned long cSize)
{
    unsigned int cnt;
    mByteCount = mSize;
    cByteCount = cSize;
	if(mSize != 0)
		std::string sm (m,mSize);
    current = 0;
    lsb_bits = 0;


    cover = new unsigned short [cSize];
   
	message.clear();
	prependSize(mSize);
	
    for( cnt = 0 ; cnt < mSize; cnt++)
        setMessageByte(m[cnt]);
    
    for( cnt = 0 ; cnt < cSize; cnt+=2)
        setCoverByte(c[cnt],c[cnt+1],cnt);

}

WaveMessageEmbedder::~WaveMessageEmbedder()
{

    delete [] cover;
    
}

void WaveMessageEmbedder::prependSize(unsigned int size)
{
	unsigned int i;
    std::bitset<32> bin (size);
	
	for( i = 0; i < 32 ; i++)
	{
		
		message.push_back(bin[32 - i - 1]);
	}
	
	
}

void printXProgBar( int p) {

string bar;
	for(int i = 0; i < 50; i++){
		if( i < (p>>1)){
		  bar.replace(i,1,"=");
		}else if( i == (p>>1)){
		  bar.replace(i,1,">");
		 
		}else{
		  bar.replace(i,1," ");
		 
		}
	}	
	cout<< "\r" "[" << bar << "] ";
	cout.width( 3 );
	cout<< p << "%     "  ;
} 
void printProgBar( int percent, int percent2 ){
  string bar;
  string bar2;

  for(int i = 0; i < 25; i++){
    if( i < (percent>>2)){
      bar.replace(i,1,"=");
	}else if( i == (percent>>2)){
      bar.replace(i,1,">");
	 
    }else{
      bar.replace(i,1," ");
	 
    }
  }
  
  for(int i = 0; i < 25; i++){
    if( i < (percent2>>2)){
      bar2.replace(i,1,"=");
    }else if( i == (percent2>>2)){
	  bar2.replace(i,1,">");
    }else{
      bar2.replace(i,1," ");
    }
  }
 

  cout<< "\r" "[" << bar << "] ";
  cout.width( 3 );
  cout<< percent << "%     "  ;
  cout<< "[" << bar2 << "] ";
  cout.width( 3 );
  cout<< percent2 << "%     " << std::flush;
}
void WaveMessageEmbedder::setMessageByte(BYTE val)
{
    unsigned int i;
    std::bitset<8> bin (val);

    for(i = 0; i < 8; i++)
    {
  
        message.push_back(bin[8 - i - 1]);

    }
}

void WaveMessageEmbedder::setCoverByte(BYTE val, BYTE hival,unsigned int cnt)
{

    cover[cnt>>1] = (val+256*hival);
}

unsigned int WaveMessageEmbedder::getNbitsFromMessage(unsigned int n)
{
    unsigned int i,token = 0;
	
	
    for(i = 0; i < n ;i++)
    {
        
        token += pow(2,n-i-1)*(unsigned int)message.front();
        message.erase(message.begin());

    }
    return token;

}

unsigned int WaveMessageEmbedder::averageNLeftSamples(unsigned int n)
{
    unsigned int i;
    unsigned int sum = 0;
    for(  i = current; i < n *2 + current ; i+=2 )
    {
        sum += cover[i];
    }
    return sum/n;
}

unsigned int WaveMessageEmbedder::averageNRightSamples(unsigned int n)
{
    unsigned int i;
    unsigned int sum = 0;
    for(  i = current + 1; i < n * 2 + current ; i+=2 )
    {
		sum += cover[i];      
    }
    return sum/n;
}

unsigned int WaveMessageEmbedder::getlsb(unsigned int b,unsigned int value)
{
    unsigned int i,mask = 0;
    for(i = 0; i < b; i++)
    {
        mask += pow(2,i);

    }

    return value & mask;
}



void * WaveMessageEmbedder::embed()
{
    //cout << current ;
	unsigned int b = lsb_bits;
	unsigned int n = n_bytes;
    unsigned int changeSample = 0;
	unsigned int token;
	unsigned int average_lsb = getlsb(b,averageNLeftSamples(n));
	unsigned int average_rsb = getlsb(b,averageNRightSamples(n));
    unsigned int num,max;
	bool first_time = true;
	// get b bits from message
	pthread_mutex_lock (&mutexembed);
	if(message.size() > b)
	{
		token = getNbitsFromMessage(b);
	}
	else
	{
		int i = message.size();
		token = getNbitsFromMessage(i);
		token = token << (b-i);
	}
	int c = current;
	
	
	pthread_mutex_unlock (&mutexembed);
    max = (unsigned int) pow(2,b) - 1;

    while(average_lsb != token)
    {
        while(changeSample < n)
		{		
			num = getlsb(b,cover[c + changeSample * 2]);
						
			if(first_time == true)
			{
			   // sets all samples involved to zero
			   cover[c + changeSample * 2] -= num;
			   if (changeSample == n -1)
					first_time = false;
			}
			else
			{
				if (num == max)
					cover[c + changeSample * 2] -= max;
				else
					cover[c + changeSample * 2]++;
			}
			average_lsb = getlsb(b,averageNLeftSamples(n));
			if (average_lsb == token)
				break;
			changeSample++;
		}
		changeSample = 0;       
    }
	
	first_time = true;
	
	// get b bits from message
	if(message.size() > b)
	{
		token = getNbitsFromMessage(b);
	}
	else
	{
		int i = message.size();
		token = getNbitsFromMessage(i);
		token = token << (b-i);
	}

    max = (unsigned int) pow(2,b) - 1;

    while(average_rsb != token)
    {
        while(changeSample < n)
		{		
			num = getlsb(b,cover[c + 1 + changeSample * 2]);
						
			if(first_time == true)
			{
			   // sets all samples involved to zero
			   cover[c + 1 + changeSample * 2] -= num;
			   if (changeSample == n -1)
					first_time = false;
			}
			else
			{
				if (num == max)
					cover[c + 1 + changeSample * 2] -= max;
				else
					cover[c + 1 + changeSample * 2]++;
			}
			average_rsb = getlsb(b,averageNRightSamples(n));
			if (average_rsb == token)
				break;
			changeSample++;
		}
		changeSample = 0;       
    }
     
	
	
    

pthread_exit((void*) 0);	
	
	
	
    
}

// void do_join(std::thread& t)
// {
//     t.join();
// }
// 
// void join_all(std::vector<std::thread>& v)
// {
//     for_each(v.begin(),v.end(),do_join);
// }

static void * embed_helper(void *context)
{
        return ((WaveMessageEmbedder *)context)->embed();
}

BYTE * WaveMessageEmbedder::getStegoData(unsigned int bitsPerSample,unsigned int noOfBytesToAverage)
{
	unsigned int net = cByteCount / noOfBytesToAverage;
	net = net * noOfBytesToAverage;
	unsigned int msize = (message.size()/bitsPerSample)/2 + 1;
	unsigned int t = 0, i=0;
	
	unsigned int size = min(net,msize);
	pthread_t threads[size];
	pthread_attr_t attr;

	void *status;
   
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   
    lsb_bits = bitsPerSample;
	n_bytes = noOfBytesToAverage;
	currentbits = (mByteCount << 3) + 32 ;
	
	cout << "Embedding Data...\n\nPercentage of Message Embedded       Percentage of Cover used" << endl;
	pthread_mutex_init(&mutexembed, NULL);
	//pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    while( current * 2 < net && !message.empty() )
	{gi
		if(!(pthread_create(&threads[t], &attr, &embed_helper,this)))
        //t.push_back(std::thread (embed));
		t++;
		
		current += noOfBytesToAverage << 1;
		float f = ((bitsPerSample*current/noOfBytesToAverage)/(float)currentbits);
		float g = (current << 1)/(float)(cByteCount);
		printProgBar(f*100,g*100);
	}
	pthread_attr_destroy(&attr);
 	
	/* Wait on the other threads */
	for(i=0;  i < t; i++)
	{
		  pthread_join(threads[i], NULL);
	}
	BYTE * bCover = new BYTE [cByteCount]();
    convertCoverToBYTE(bCover);
	pthread_mutex_destroy(&mutexembed);
	   //pthread_exit(NULL);
	
	
	return bCover;
}



void WaveMessageEmbedder::convertCoverToBYTE(BYTE  * bCover)
{
	unsigned int i;
	for( i = 0; i < cByteCount>>1; i++ )
	{
		bCover[2 * i] = cover[ i] & 0xFF;
		bCover[2*i + 1] = cover[i] >> 8;
	}
}	
void WaveMessageEmbedder::extract(unsigned int b,unsigned int n)
{

    unsigned int average_lsb = getlsb(b,averageNLeftSamples(n));
	unsigned int average_rsb = getlsb(b,averageNRightSamples(n));
    std::vector<unsigned int> temp;
   
    unsigned int i;
	
    for(i = currentbits ; i < currentbits + b; i ++)
    {

        temp.insert(temp.begin(),average_lsb%2);
        average_lsb = average_lsb >> 1;
       

    }
	 for( i = 0; i < temp.size(); i++)
    {
        message.push_back((bool)temp[i]);
    }
	temp.clear();
    	
	for(i = currentbits ; i < currentbits + b; i ++)
    {

        temp.insert(temp.begin(),average_rsb%2);
        average_rsb = average_rsb >> 1;
   

    }
    for( i = 0; i < temp.size(); i++)
    {
        message.push_back((bool)temp[i]);
    }

    temp.clear();
    current += 2 *n;
    currentbits += 2 *  b;

}

unsigned int WaveMessageEmbedder::getExtractedSize(){
return message.size()/8 - 4;
}


BYTE * WaveMessageEmbedder::getExtractedData(unsigned int bitsPerSample,unsigned int noOfBytesToAverage)
{
	bool retrievedSize = false;
    lsb_bits = bitsPerSample;
    message.clear();
	mByteCount = 0;
    current = 0;
    currentbits = 0;

	
    while((current < cByteCount/2 && currentbits < (mByteCount + 4) * 8) || retrievedSize == false) /* ||  (current/noOfBytesToAverage) *bitsPerSample < mByteCount * 8 - 32) &&  retrievedSize == false*/ 
	{
	
		if(retrievedSize == false && message.size() >= 32)
		{
			extractSize();
			
			retrievedSize = true;
		}	
        extract(bitsPerSample,noOfBytesToAverage);
	}
	
	
    //convert message to BYTE *
	BYTE * out; //should move to fields and free on destruct
	unsigned int i = 0,j=0;
	out = new BYTE [message.size()/8 - 4];

	
	while(i < message.size()/8 - 4)
	
	{
		out[i] = 0;
		while(j < 8)
		{
			
			out[i] += message[32 + i*8 + j]<<(7-j);
			j++;
		}
		j = 0;
		i++;
		
	}
	

	
    return out;
}

void WaveMessageEmbedder::extractSize()
{
	unsigned int size = 0,i;
	for(i = 0 ; i < 31; i ++){
		
		size += message[i]<<(31 - i);
	}	
   
 	mByteCount = size;
}

void WaveMessageEmbedder::print()
{
    unsigned int cnt;
    cout << "Message Bits"  <<message.size() <<endl;

    for( cnt = 32 ; cnt < message.size()  ; cnt++){
        cout << (bool)message[cnt];
        if ( cnt % 8== 7)
            cout << ", " ;
        else if (cnt % 80 == 79)
            cout << endl;
    }
    

    cout << endl;

    cout << "Cover Bytes" << endl;
    cout << "        Left                 Right       " <<endl;
    for( cnt = 0 ; cnt < cByteCount/2; cnt++){
        if( cnt % 2 == 1 )
			cout << "      ";
	
       
			cout<<setw(10)<< right << cover[cnt];
        if ( cnt % 2 == 1)
            cout << endl;
    }

        cout << endl;
            cout << endl;
                cout << endl;

}

