//ONLY STD LIBRARIES

#include <assert.h>
#include <corecrt_math_defines.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define NUM_BUCKETS 100
#define SHORT_TIME_PERIOD 0.2f // sec
#define NUMBER_OF_TOP_FREQUENCIES 10
#define BASE_FREQUENCY 100.f //hz
#define MAX_FREQ 10000
#define HASH_INTERVAL 5  //always smaller than the clip 
#define SCATTER 10
#define OFFSET 30

typedef struct
{
    FILE* wavFile;
    char command; // either add to database (a) or get recommendations (r)
} input_s;


typedef struct
{
    //directly from file header

    unsigned int channels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned int dataSize; //in bytes
    
    //additional info
    unsigned int sampleCount;
    float duration;
    int16_t* samples;
} wavInfo_s;

typedef struct
{
    int* data;
    size_t rows;
    size_t cols;
} ampBand_s;

//for now we take it so that the sampleRate and byterate are same across all songs and clips
typedef struct
{
    size_t audioId;
    char name[101];
    ampBand_s ampBandclubbed; //flag for stop comparing is 0
    ampBand_s ampBandFilter2;
    ampBand_s ampBandFull; // ignore the last column
} audioInfo_s;

typedef struct 
{
    size_t numIds;
    audioInfo_s* audioInfos;
    size_t reserved;
} audioCat_s;

typedef struct 
{
    size_t numIds;
    size_t* audioIds;
} audioSet_s;

typedef struct entry_s
{
    size_t audioId;
    size_t row;
    size_t col;
    struct entry_s* next;
} entry_s;

typedef struct
{
    size_t numBuckets;
    entry_s** buckets;
} hashIndex_s;

typedef struct
{
    size_t num;
    size_t* vals; // row 0 col 0-whatever then row 0 col whatever + whatever
} clipHashVals_s;

// function declarations
int initHashTable(hashIndex_s*);
size_t bootDatabase(hashIndex_s*);
input_s getInput(); //ND
wavInfo_s wavDecoder(FILE*);
void getSamples(FILE*, int16_t*);
ampBand_s fullAmpBand(const wavInfo_s*);
ampBand_s clubAmpBand(ampBand_s);
int addToDatabase(audioInfo_s);
int addToHashTable(audioInfo_s, hashIndex_s);
int appendHashEntry(entry_s**, entry_s*);
int hashClip(clipHashVals_s*, ampBand_s);
int filter1(audioSet_s*, hashIndex_s, audioInfo_s);
int filter2(audioSet_s*, hashIndex_s, audioInfo_s);

int main()
{
    hashIndex_s hashIndex;
    hashIndex.numBuckets = NUM_BUCKETS;
    initHashTable(&hashIndex);
    size_t currId = bootDatabase(&hashIndex);
    
    input_s input = getInput();
    const wavInfo_s wavInfo = wavDecoder(input.wavFile); //the samples will be in heap

    audioInfo_s audioInfo;
    audioInfo.audioId = currId + 1;
    
    audioInfo.ampBandFull = fullAmpBand(&wavInfo); //dont remember why I passed the address
    audioInfo.ampBandclubbed = clubAmpBand(audioInfo.ampBandFull);
    
    //add to database
    if (input.command == 'a')
    {
        printf("adding to data base...\n");
        addToDatabase(audioInfo);
    }
    //give recommendations
    else if (input.command == 'r')
    {
        clipHashVals_s clipHashVals;
        hashClip(&clipHashVals, audioInfo.ampBandclubbed);
        audioSet_s filteredAudios = {0, NULL};
        filter1(&filteredAudios, hashIndex, audioInfo); //passing address so that i can change count..This will need to access the database
        filter2(&filteredAudios, );
    }
    
}

//function definitions

int appentToCat(audioCat_s *audioCat, audioInfo_s audioinfo)
{
    if (audioCat->numIds <= audioCat->reserved)
    {
        audioCat->reserved *= 2;
        audioCat->audioInfos = realloc(audioCat->audioInfos, audioCat->reserved*sizeof(audioInfo_s));
    }
    audioCat->audioInfos[audioCat->numIds+1] = audioinfo;
    audioCat->numIds++;
}

size_t bootDatabase(hashIndex_s* hashIndex)
{
    audioCat_s catalogue;
    catalogue.numIds = 100;
    catalogue.audioInfos = malloc(catalogue.numIds*sizeof(*catalogue.audioInfos));
    
    size_t i = 0;
    for (; ; i++) 
    {
        audioInfo_s audioInfo; //get song from file on disk (give this code)
        appentToCat(&catalogue, audioInfo);
        addToHashTable(audioInfo, *hashIndex);
    }
    
}


int initHashTable(hashIndex_s* hashIndex)
{
    hashIndex->buckets = calloc(hashIndex->numBuckets, sizeof(entry_s*));
}

input_s getInput()
{
    input_s result;
    printf("file name please (don't forget the .wav) MAX NAME LENGTH IS 20 (inclusive of the .wav (4 char))\n");
    char fileName[21];
    scanf("%s", fileName); 
    printf("command -\n");
    scanf("%c", &result.command);
    result.wavFile = fopen(fileName, "r");
    if (result.wavFile == NULL)
    {
        printf("failed to open file\n");
        exit(1);
    }
}


wavInfo_s wavDecoder(FILE* wavFile)
{
    wavInfo_s wavInfo;
    //maybe add a check to make sure that the file is a wav file internally
    

    wavInfo.sampleCount = wavInfo.sampleRate*wavInfo.dataSize/wavInfo.byteRate;
    wavInfo.samples = malloc(wavInfo.sampleCount*sizeof(*wavInfo.samples));

    //cpy samples to samples hahaha
    
    return wavInfo;
}


int shortFourier(const wavInfo_s*, float*, size_t, size_t);
int convertToNotes(float, float*, size_t, int*);
int checkForSameFreq(int*, int*, size_t,  bool*); // matches the frequency indices in c with that of c-1
int ascendingOrder(int*, size_t, bool*);


ampBand_s fullAmpBand(const wavInfo_s* wavInfo)
{
    ampBand_s ampBand;
    ampBand.cols = (size_t)((wavInfo->duration + 1 - SHORT_TIME_PERIOD) / SHORT_TIME_PERIOD);
    ampBand.rows = NUMBER_OF_TOP_FREQUENCIES; //not be remain a compile const maybe
    ampBand.data = malloc((ampBand.cols*ampBand.rows)*sizeof(*ampBand.data));
    float freq[ampBand.rows];
    bool commonIndicesFlags[ampBand.rows];
    memset(commonIndicesFlags, 0, ampBand.rows*sizeof(*commonIndicesFlags));
    for (size_t c = 0; c < ampBand.cols; c++)
    {
        shortFourier(wavInfo, freq, ampBand.rows, c);
        convertToNotes(BASE_FREQUENCY, freq, ampBand.rows, (ampBand.data + c*ampBand.rows));
        if (c > 0)
        {
            checkForSameFreq((ampBand.data + (c)*ampBand.rows), (ampBand.data + (c-1)*ampBand.rows), ampBand.rows, commonIndicesFlags);
        }
        ascendingOrder(ampBand.data + c*ampBand.rows, ampBand.rows, commonIndicesFlags);
        if (c > 0)
        {
            for (size_t i = 0; i < ampBand.rows; i++)
            {
                ampBand.data[(c-1)*ampBand.rows + i] = ampBand.data[c*ampBand.rows + i] - ampBand.data[(c-1)*ampBand.rows + i];
            }
        }
    }

    return ampBand;
}
//writes the top 'numOfTopFreq' frequencies (not their amplitudes just the frequency in hz)
int shortFourier(const wavInfo_s* wavInfo, float* pWrite, size_t numOfTopFreq, size_t col)
{
    float temp[2][numOfTopFreq]; //first col is freq
    memset(temp, 0, sizeof(temp));
    size_t totalSamples = (size_t)(wavInfo->sampleRate*SHORT_TIME_PERIOD);
    size_t windowStart = (size_t)(col*SHORT_TIME_PERIOD*wavInfo->sampleRate);
    
    for (float freq = BASE_FREQUENCY; freq < MAX_FREQ; freq += 1 / SHORT_TIME_PERIOD) 
    {
        float amp = 0;
        float x = 0;
        float y = 0;
        
        float delta = (2*M_PI*freq)/wavInfo->sampleRate; //increase per sample
        float deltaSin = sinf(delta);
        float deltaCos = cosf(delta);
        float currSin = 0;
        float currCos = 1;
        for (size_t i = 0; i < totalSamples; i++)
        {
            x += wavInfo->samples[windowStart + i]*currSin;
            y += wavInfo->samples[windowStart + i]*currCos;
            float sinTemp = currSin;
            currSin = currSin*deltaCos + currCos*deltaSin;
            currCos = currCos*deltaCos - sinTemp*deltaSin;
        }
        amp = sqrtf(x*x + y*y) / freq;
        for (size_t i = 0; i < numOfTopFreq; i++) 
        {
            if(amp > temp[1][i])
            {
                for (size_t j = numOfTopFreq - 1; j > i; j += -1)
                {
                    temp[1][j] = temp[1][j-1];
                    temp[0][j] = temp[0][j-1];
                }
                temp[1][i] = amp;
                temp[0][i] = freq;
                break;
            }
        }
    }
    for (size_t i = 0; i < numOfTopFreq; i++) 
    {
        pWrite[i] = temp[0][i];
    }
    
    return 0;
}

int convertToNotes(float baseFreq, float* freqData, size_t eleCount, int* wNotes)
{
    for (size_t i = 0; i < eleCount; i++)
    {
        wNotes[i] = roundf(12 * log2f(freqData[i] / baseFreq)); //gives us the half steps from base note
    }
}

int checkForSameFreq(int* curr, int* prev, size_t eleCount,  bool* commonIndicesFlags)
{
    for (size_t i = 0; i < eleCount; i++)
    {
        for (size_t j = 0; j < eleCount; j++)
        {
            if (prev[i] == curr[j])
            {
                int temp = curr[i];
                curr[i] = curr[j];
                curr[j] = temp;
                commonIndicesFlags[i] = true;
                break;
            }
            commonIndicesFlags[j] = false;
        }
    }
}

int ascendingOrder(int* data, size_t eleCount, bool* commonIndicesFlags)
{
    for (size_t i = 0; i < eleCount; i++) 
    {
        if (commonIndicesFlags[i] == true)
        {
            continue;
        }
        for (size_t j = i+1; j < eleCount; j++)
        {
            if (commonIndicesFlags[j] == true)
            {
                continue;
            }
            if (data[j] < data[i]) 
            {
                float temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
    }
}

ampBand_s clubAmpBand(ampBand_s ampBandFull)
{
    ampBand_s clubbed;
    clubbed.rows = ampBandFull.rows;
    clubbed.cols = 0;
    int* temp = calloc(ampBandFull.cols*ampBandFull.rows, sizeof(*clubbed.data));
    for (size_t r = 0; r < clubbed.rows; r++)
    {
        size_t cols = 0;
        for (size_t i = 0; i < ampBandFull.cols - 1; i++)
        {
            if(abs(ampBandFull.data[r + i*ampBandFull.rows] + temp[cols*ampBandFull.rows + r]) >= abs(temp[r + cols*ampBandFull.rows]))
            {
                temp[r + cols*ampBandFull.rows] = ampBandFull.data[r + i*ampBandFull.rows] + temp[r + cols*ampBandFull.rows];
            }
            else 
            {
                cols += 1;
                temp[r + cols*ampBandFull.rows] = ampBandFull.data[r + i*ampBandFull.rows];
            }
        }
        if (cols + 1 > clubbed.cols)
        {
            clubbed.cols = cols + 1;
        }
    }
    clubbed.data = malloc(clubbed.cols*clubbed.rows*sizeof(*clubbed.data));
    for (size_t c = 0; c < clubbed.cols; c++)
    {
        memcpy(clubbed.data + c*clubbed.rows, (temp + c*ampBandFull.rows), clubbed.rows*sizeof(*clubbed.data));
    }
    free(temp);
    return clubbed;
}


int appendToHash(size_t, size_t, size_t, size_t, hashIndex_s);

int addToHashTable(audioInfo_s audioInfo,  hashIndex_s hashIndex)
{
    for (size_t r = 0; r < audioInfo.ampBandclubbed.rows; r++)
    {
        for (size_t c = 0; c < audioInfo.ampBandclubbed.cols - HASH_INTERVAL + 1; c++) 
        {
            size_t hashValue = 0;
            if (audioInfo.ampBandclubbed.data[c*audioInfo.ampBandclubbed.rows + r] == 0)
            {
                break;
            }
            for (size_t i = 0; i < HASH_INTERVAL; i++)
            {
                hashValue = hashValue*SCATTER + audioInfo.ampBandclubbed.data[(c + i)*audioInfo.ampBandclubbed.rows + r] + OFFSET;
            }
            appendToHash(audioInfo.audioId, r, c, hashValue, hashIndex);
        }
    }
}

int appendToHash(size_t audioId, size_t row, size_t col, size_t hashvalue, hashIndex_s hashIndex)
{
    size_t position = hashvalue % hashIndex.numBuckets;
    entry_s* pEntry= malloc(sizeof(entry_s));
    pEntry->audioId = audioId;
    pEntry->col = col;
    pEntry->row = row;
    appendHashEntry(&hashIndex.buckets[position], pEntry);
}

int appendHashEntry(entry_s** pexisting, entry_s* toAppend)
{
    if (*pexisting == NULL)
    {
        *pexisting = toAppend;
    }
    else 
    {
        entry_s* temp = *pexisting;
        for (;temp->next != NULL; temp = temp->next)
        {}
        temp->next = toAppend;
    }
    toAppend->next = NULL;
}

int hashClip(clipHashVals_s* clipHashVals, ampBand_s clubbedAmpBand)
{
    size_t divisions = (size_t)(clubbedAmpBand.cols / HASH_INTERVAL); //divs in one row
    clipHashVals->num = divisions*clubbedAmpBand.rows;
    clipHashVals->vals = malloc(clipHashVals->num*sizeof(*clipHashVals->vals));
    for (size_t i = 0; i < clubbedAmpBand.rows; i++) 
    {
        size_t hashVal = 0;
        for (size_t j = 0; j < divisions; j += HASH_INTERVAL)
        {
            for (size_t k = 0 ; k < HASH_INTERVAL; k++)
            {
                hashVal = hashVal*SCATTER + clubbedAmpBand.data[i*clubbedAmpBand.cols + j*divisions + k] + OFFSET;
            }
            clipHashVals->vals[i*divisions + j] = hashVal % NUM_BUCKETS;
        }
    }
}

int addToDatabase(audioInfo_s audioInfo)
{
    
}



int filter1(audioSet_s* pWriteAudioId, hashIndex_s hashIndex, audioInfo_s audioInfo)
{
    
}
