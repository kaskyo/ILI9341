#include "RS152.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
void printBin(uint64_t value)
{
    char output[65];
    int bitsCount = 64;
    int i;
    output[bitsCount] = '\0';
    for (i = bitsCount - 1; i >= 0; --i, value >>= 1)
    {
        output[i] = (value & 1) + '0';
    }
    printf("%s\n",output);
}


uint16_t G[4] = {0x0000, 0x3333, 0x5555, 0x6666};
uint64_t shift[4] = { 0x1, 0x8000, 0x40000000, 0x2000000000 };

uint8_t interleaveTable[64];
uint8_t deinterleaveTable[64];

void GenerateInttables()
{
    for (int i=0; i<8; i++)
    {
        for (int j=0; j<8; j++)
        {
            interleaveTable[8*i+j] = ((j*8) % 64) + i;
            deinterleaveTable[((j*8) % 64) + i] = 8*i+j;
        }
    }
}

void interleave(uint8_t word[8], uint8_t* result)
{
    uint8_t out[8] = {0};
    for (int i=0; i<64; i++)
    {
        int num_byte_orig = (int)floor(i/8);
        int num_bit_orig = 7 - (i % 8);
        int num_byte_dest = (int)floor(interleaveTable[i]/8);
        int num_bit_dest = 7 - (interleaveTable[i] % 8);
        out[num_byte_dest] |= ((word[num_byte_orig]>>num_bit_orig)&1)<<num_bit_dest;
    }
    memcpy(result,out,8);
}

void deinterleave(uint8_t* word, uint8_t* result)
{
    uint8_t out[8] = {0};
    for (int i=0; i<64; i++)
    {
        int num_byte_orig = (int)floor(i/8);
        int num_bit_orig = 7 - (i % 8);
        int num_byte_dest = (int)floor(deinterleaveTable[i]/8);
        int num_bit_dest = 7 - (deinterleaveTable[i] % 8);
        out[num_byte_dest] |= ((word[num_byte_orig]>>num_bit_orig)&1)<<num_bit_dest;
    }
    memcpy(result,out,8);
}

int distance (uint16_t a, uint16_t b)
{
    uint16_t c = a ^ b;
    int cnt = 0;
    for (int i=0; i<15; i++)
    {
        if ((c&1) == 1)
            cnt++;
        c = c>>1;
    }
    return cnt;
}

void RS152Code(uint8_t byte, uint8_t* ptr)
{
    //printf("%d\n", byte);
    //printf("Original byte: \t");
    //printf("%02x\n",byte);
    uint8_t b;
    uint8_t coded[8] = {0};

    for (int i=0; i<4; i++)
    {
        b = (byte & (0x03<<(i*2))) >> (i*2);
        coded[2*i] = (G[b]>>8)&0xff;
        coded[2*i+1] = G[b]&0xff;
    }
    //printf("Coded word: \t");
    //for (int i=0; i<8; i++) printf("%02x ",coded[i]);
    //printf("\n");
    uint8_t interleaved[8] = {0};
    interleave(coded, interleaved);

    //printf("Interleaved: \t");
    //for (int i=0; i<8; i++) printf("%02x ",interleaved[i]);
    //printf("\n");

    memcpy(ptr,interleaved,8);
}

uint8_t RS152Decode(uint8_t* w)
{
    uint8_t deinterleaved[8] = {0};
    deinterleave(w, deinterleaved);
    //printf("Deinterleaved: \t");
    //for (int i=0; i<8; i++) printf("%02x ",deinterleaved[i]);
    //printf("\n");

    uint8_t out = 0;
    uint16_t word;
    uint8_t min, minW;
    for (int i=0; i<4; i++)
    {
        word = (deinterleaved[i*2]<<8) | deinterleaved[i*2+1]&0xff;
        //printf("%04x\n",word);
        min = 16; minW = -1;
        for (int j=0; j<4; j++)
        {
            int d = distance(word,G[j]);
            if (d<min)
            {
                min = d;
                minW = j;
            }
        }
        out |= minW << (i*2);
    }
    //printf("Resulting byte:\t");
    //printf("%02x\n",out);
    return out;
}