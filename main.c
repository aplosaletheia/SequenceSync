#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#define SHORT_TIME_PERIOD 0.2f // sec
#define NUMBER_OF_TOP_FREQUENCIES 10
#define BASE_FREQUENCY 100.f //hz


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
    ampBand_s ampBandclubbed; //flag for stop comparing is 0
    ampBand_s ampBandFilter2;
    ampBand_s ampBandFull; // ignore the last column
} audioInfo_s;

typedef struct
{
    char name[21];
    audioInfo_s* pAudioInfo;
} audioId_s;

typedef struct 
{
    size_t numIds;
    audioId_s* pAuidioIds;
} audioSet_s;

// function declarations
input_s getInput(); //ND
wavInfo_s wavDecoder(FILE*);
void getSamples(FILE*, int16_t*);
ampBand_s fullAmpBand(const wavInfo_s*);
ampBand_s clubAmpBand(ampBand_s);
int addToDatabase(audioInfo_s);

int main()
{
    bootDatabase(); //probably definately have the database pre compiled
    
    input_s input = getInput();
    const wavInfo_s wavInfo = wavDecoder(input.wavFile); //the samples will be in heap

    audioId_s audioId;
    audioInfo_s audioInfo;
    audioId.pAudioInfo = &audioInfo;
    
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
        audioSet_s filteredAudios;
        filter1(&filteredAudios); //passing address so that i can change count..This will need to access the database
        filteredAudios = filter2();
    }
    
}

//function definitions

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


int shortFourier(const wavInfo_s*, float*, size_t);
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
    for (size_t c = 0; c < ampBand.cols; c++)
    {
        shortFourier(wavInfo, freq, ampBand.rows);
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
                ampBand.data[(c-1)*ampBand.rows + i] = ampBand.data[c*ampBand.rows] - ampBand.data[(c-1)*ampBand.rows + i];
            }
        }
    }

    return ampBand;
}
//writes the top 'numOfTopFreq' frequencies (not their amplitudes just the frequency in hz)
int shortFourier(const wavInfo_s* wavInfo, float* pWrite, size_t numOfTopFreq)
{
    
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
                float temp = curr[i];
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
            if(abs(ampBandFull.data[r*ampBandFull.cols + i] + temp[r*ampBandFull.cols + cols]) >= abs(temp[r*ampBandFull.cols + cols]))
            {
                temp[r*ampBandFull.cols + cols] = ampBandFull.data[r*ampBandFull.cols + i] + temp[r*ampBandFull.cols + cols];
            }
            else 
            {
                cols += 1;
                temp[r*ampBandFull.cols + cols] = ampBandFull.data[r*ampBandFull.cols + i];
            }
        }
        if (cols + 1 > clubbed.cols)
        {
            clubbed.cols = cols + 1;
        }
    }
    clubbed.data = malloc(clubbed.cols*clubbed.rows*sizeof(*clubbed.data));
    for (size_t r = 0; r < clubbed.rows; r++)
    {
        memcpy(clubbed.data + r*clubbed.cols, (temp + r*ampBandFull.cols), clubbed.cols*sizeof(*clubbed.data));
    }
    free(temp);
    return clubbed;
}



int addToDatabase(audioInfo_s audioInfo)
{
    
}
