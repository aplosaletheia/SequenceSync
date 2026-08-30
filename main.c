#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#define SHORT_TIME_PERIOD = 0.2; // sec
#define NUMBER_OF_TOP_FREQUENCIES 10
#define BASE_FREQUENCY 100 //hz


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
    int16_t* pSamples;
} wavInfo_s;

typedef struct
{
    unsigned int* data;
    size_t rows;
    size_t cols;
} ampBand_s;

// function declarations
input_s getInput(); //ND
wavInfo_s wavDecoder(FILE*);
void getSamples(FILE*, int16_t*);
ampBand_s dataProcessing(const wavInfo_s*);
int addToDatabase(ampBand_s);



int main()
{
    bootDatabase(); //probably definately have the database pre compiled
    
    input_s input = getInput();
    const wavInfo_s wavInfo = wavDecoder(input.wavFile);
    
    ampBand_s ampBand = dataProcessing(&wavInfo);

    //add to database
    if (input.command == 'a')
    {
        printf("adding to data base...\n");
        addToDatabase(ampBand);
    }
    //give recommendations
    else if (input.command == 'r')
    {
        
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
    return wavInfo;
}


int shortFourier(const wavInfo_s*, float*, size_t);
int convertToNotes(float, float*, size_t);
int checkForSameFreq(unsigned int*, unsigned int*, size_t,  bool*); // matches the frequency indices in c with that of c-1
int ascendingOrder(unsigned int*, size_t, bool*);


ampBand_s dataProcessing(const wavInfo_s* wavInfo)
{
    ampBand_s ampBand;
    ampBand.cols = wavInfo->sampleCount;
    ampBand.rows = NUMBER_OF_TOP_FREQUENCIES; //not be remain a compile const
    unsigned int notes[ampBand.cols*ampBand.rows];
    ampBand.data = notes;
    float freq[ampBand.rows];
    bool commonIndicesFlags[ampBand.rows];
    for (size_t c = 0; c < wavInfo->sampleCount; c++)
    {
        shortFourier(wavInfo, freq, ampBand.rows);
        convertToNotes(BASE_FREQUENCY, freq, ampBand.rows);
        if (c > 0)
        {
            checkForSameFreq((ampBand.data + c*ampBand.rows), (ampBand.data + (c-1)*ampBand.rows), ampBand.rows, commonIndicesFlags);
        }
        ascendingOrder((ampBand.data + c*ampBand.rows), ampBand.rows, commonIndicesFlags);
        if (c > 0)
        {
            for (size_t i = 0; i < ampBand.rows; i++)
            {
                ampBand.data[(c-1)*ampBand.rows + i] = ampBand.data[c*ampBand.rows] - ampBand.data[(c-1)*ampBand.rows + i];
            }
        }
    }
    
}

//writes the top 'numOfTopFreq' frequencies (not their amplitudes just the frequency in hz)
int shortFourier(const wavInfo_s* wavInfo, float* pWrite, size_t numOfTopFreq)
{
    
}

int convertToNotes(float baseFreq, float* freqData, size_t eleCount)
{
    for (size_t i = 0; i < eleCount; i++)
    {
        freqData[i] = roundf(12 * log2f(freqData[i] / baseFreq)); //gives us the half steps from base note
    }
}

int checkForSameFreq(unsigned int* curr, unsigned int* prev, size_t eleCount,  bool* commonIndicesFlags)
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

int ascendingOrder(unsigned int* data, size_t eleCount, bool* commonIndicesFlags)
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


int addToDatabase(ampBand_s ampBand)
{
    
}
