#include "WaveMessageEmbedder.h"

using std::setw;
using std::right;
using std::cout;
using std::endl;


WaveMessageEmbedder::WaveMessageEmbedder(char * m, unsigned int mSize, BYTE * c, DWORD cSize)
{
    unsigned int cnt;
    mByteCount = mSize;
    cByteCount = cSize;
    std::string sm (m);
    current = 0;
    lsb_bits = 0;

//    std::bitset<(size_t) mSize>  temp (sm);
    cover = new unsigned short [cSize];
    //message = new std::vector<bool>;
	message.clear();
	prependSize(mSize);
    for( cnt = 0 ; cnt < mSize; cnt++)
        setMessageByte(m[cnt]);
    
    for( cnt = 0 ; cnt < cSize; cnt+=2)
        setCoverByte(c[cnt],c[cnt+1],cnt);

}

WaveMessageEmbedder::~WaveMessageEmbedder()
{
//    delete [] message;
    delete [] cover;
}

void WaveMessageEmbedder::prependSize(unsigned int size)
{
	unsigned int i;
    std::bitset<32> bin (size);
	
	for( i = 0; i < 32 ; i++)
	{
		cout << " i = " << i << "bin "<< bin[32 - i - 1] << endl ;
		message.push_back(bin[32 - i - 1]);
	}
	
	
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
        //cout << "message.front " << message.front() << "current: " << current << endl;
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



void WaveMessageEmbedder::embed(unsigned int b,unsigned int n)
{
    

    unsigned int changeSample = 0;
	unsigned int token;
	unsigned int average_lsb = getlsb(b,averageNLeftSamples(n));
	unsigned int average_rsb = getlsb(b,averageNRightSamples(n));
    unsigned int num,max;
	bool first_time = true;
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

    while(average_lsb != token)
    {
        while(changeSample < n)
		{		
			num = getlsb(b,cover[current + changeSample * 2]);
						
			if(first_time == true)
			{
			   // sets all samples involved to zero
			   cover[current + changeSample * 2] -= num;
			   if (changeSample == n -1)
					first_time = false;
			}
			else
			{
				if (num == max)
					cover[current + changeSample * 2] -= max;
				else
					cover[current + changeSample * 2]++;
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
			num = getlsb(b,cover[current + 1 + changeSample * 2]);
						
			if(first_time == true)
			{
			   // sets all samples involved to zero
			   cover[current + 1 + changeSample * 2] -= num;
			   if (changeSample == n -1)
					first_time = false;
			}
			else
			{
				if (num == max)
					cover[current + 1 + changeSample * 2] -= max;
				else
					cover[current + 1 + changeSample * 2]++;
			}
			average_rsb = getlsb(b,averageNRightSamples(n));
			if (average_rsb == token)
				break;
			changeSample++;
		}
		changeSample = 0;       
    }
     
	cout << "averages " << average_lsb << " " << average_rsb << endl ;

    current += 2 * n;
    //unsigned int averageandgetlsbs(int d,unsigned int e,unsigned int f,unsigned int g);
    // increment or decrement average until lsb of a = b bit255,255,255,
    //increment current by n * 2
    // e = randomly generate
}

BYTE * WaveMessageEmbedder::getStegoData(unsigned int bitsPerSample,unsigned int noOfBytesToAverage)
{
    lsb_bits = bitsPerSample;
	unsigned int i;
	int net = cByteCount / noOfBytesToAverage;
	net = net * noOfBytesToAverage;
	cout << " condition in embed loop : " << (current * 2 < net && message.size() > 0);
    while( current * 2 < net && !message.empty() )
        embed(bitsPerSample,noOfBytesToAverage);
	BYTE * bCover = new BYTE [cByteCount]();
    convertCoverToBYTE(bCover);
	/*for( i = 0; i < cByteCount/2; i++ )
	{
		cout << "orig: " << cover[ i] << "copy: "<<(short)( bCover[2 * i] + 256 * bCover[2 * i+1]) << endl;
	}
	*/
	
	return bCover;
}


void WaveMessageEmbedder::convertCoverToBYTE(BYTE  * bCover)
{
	unsigned int i;
	for( i = 0; i < cByteCount/2; i++ )
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
    cout<< "\non extraction: average_lsb = " << average_lsb << "current = " << current <<endl ;
    unsigned int i;
	
    for(i = currentbits ; i < currentbits + b; i ++)
    {

        temp.insert(temp.begin(),average_lsb%2);
        average_lsb = average_lsb >> 1;
       // cout<<(bool)temp[i];

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
       // cout<<(bool)temp[i];

    }
    for( i = 0; i < temp.size(); i++)
    {
        message.push_back((bool)temp[i]);
    }

    temp.clear();
    current += 2 *n;
    currentbits += 2 *  b;

}



BYTE * WaveMessageEmbedder::getExtractedData(unsigned int bitsPerSample,unsigned int noOfBytesToAverage)
{
	bool retrievedSize = false;
    lsb_bits = bitsPerSample;
    message.clear();
	mByteCount = 0;
    current = 0;
    currentbits = 0;
	//extractSize();

    while(current < cByteCount/2/* ||  (current/noOfBytesToAverage) *bitsPerSample < mByteCount * 8 - 32) &&  retrievedSize == false*/ )
	{
		cout << mByteCount << " is my mByteCount" << endl;
		if(retrievedSize == false && message.size() >= 32)
		{
			extractSize();
			
			retrievedSize = true;
		}	
        extract(bitsPerSample,noOfBytesToAverage);
	}
	cout << "mByteCount is " << mByteCount << "current is " << current << endl;
	
    //convert message to BYTE *
	BYTE * out;
	unsigned int i = 0,j=0;
	out = new BYTE [mByteCount];

	
	while(i < mByteCount)
	//while(i < (message.size()/8));
	{
		out[i] = 0;
		while(j < 8)
		{
			//cout << i << " is out " << j << "is in. " << pow(message[32 + i*8 + j],j)<<endl;
			
			out[i] += message[32 + i*8 + j]<<(7-j);
			j++;
		}
		j = 0;
		i++;
		//cout << i << " is out " << j << "is in." << endl;
	}
	

	/*cout << "out: " << message.size() << endl;
	for(i =0; i < message.size() / 8 -4;i++)
	{
		printf("%c ",out[i]);
	
	}
	cout << endl;*/
    return out;
}

void WaveMessageEmbedder::extractSize()
{
	unsigned int size = 0,i;
	for(i = 0 ; i < 31; i ++){
		
		size += message[i]<<(31 - i);
	}	
    cout << "Extract Size mByteCount: " << mByteCount << endl;
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
    //cout << message;

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

static const unsigned int COVERSZ = 64;
static const unsigned int MSZ = 26;
static const unsigned int SAMPLES = 1;
static const unsigned int BITS = 8;
int main()
{
    char message [] = "abcdEFGHIJKLMNOPQRSTUVWXYZ";
    BYTE cover [COVERSZ];
    srand(time(NULL));
    unsigned int i;
    for(i = 0; i < COVERSZ; i ++)
        cover[i] = rand() % COVERSZ;
    WaveMessageEmbedder w (message,MSZ,cover,COVERSZ);
    //cout << w.averageNLeftSamples(4)<< endl;
    //cout << w.getlsb(2,w.averageNLeftSamples(4)) << endl;
    w.print();
    w.getStegoData(BITS,SAMPLES);
    w.print();
    w.getExtractedData(BITS,SAMPLES);
    w.print();

    //delete &w;





    return 0;
}