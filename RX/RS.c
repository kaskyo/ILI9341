#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



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

uint8_t interleaveTable[60];
uint8_t deinterleaveTable[60];

void GenerateInttables()
{
    for (int i=0; i<6; i++)
    {
        for (int j=0; j<10; j++)
        {
            interleaveTable[10*i+j] = ((j*6) % 60) + i;
            deinterleaveTable[((j*6) % 60) + i] = 10*i+j;
        }
    }
}

uint64_t interleave(uint64_t word)
{
    uint64_t out = 0;
    for (int i=0; i<60; i++)
    {
        out |= ((word>>i)&1)<<interleaveTable[i];
    }
    return out;
}

uint64_t deinterleave(uint64_t word)
{
    uint64_t out = 0;
    for (int i=0; i<60; i++)
    {
        out |= ((word>>i)&1)<<deinterleaveTable[i];
    }
    return out;
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

uint64_t RS152Code(uint8_t byte)
{
    printf("%d\n", byte);
    printf("Original byte: \t");
    printBin(byte);
    uint8_t b;
    uint64_t coded = 0;

    for (int i=0; i<4; i++)
    {
        b = (byte & (0x03<<(i*2))) >> (i*2);
        coded |= (G[b]*shift[i]);
    }
    printf("Coded word: \t");
    printBin(coded);

    uint64_t interleaved = interleave(coded);

    printf("Interleaved: \t");
    printBin(interleaved);
    return interleaved;
}

uint8_t RS152Decode(uint64_t w)
{
    uint64_t deinterleaved = deinterleave(w);
    printf("Deinterleaved: \t");
    printBin(deinterleaved);
    uint8_t out = 0;
    uint64_t word;
    uint8_t min, minW;
    for (int i=0; i<4; i++)
    {
        word = (deinterleaved>>(15*i))&0x7FFF;
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
    printf("Resulting byte:\t");
    printBin(out);
    return out;
}

int distance64 (uint64_t a, uint64_t b)
{
    uint64_t c = a ^ b;
    int cnt = 0;
    for (int i=0; i<64; i++)
    {
        if ((c&1) == 1)
            cnt++;
        c = c>>1;
    }
    return cnt;
}

int main()
{
    GenerateInttables();
    uint64_t coded = RS152Code(27);
    //             [4444444444444][3333333333333][2222222222222][1111111111111]
    uint64_t e = 0b000010011000000000111111111111000001100000111100000000000000;
    printf("Added %d errors\n",distance64(e,0));
    coded = coded ^ e;
    uint8_t decoded = RS152Decode(coded);
    printf("%d",decoded);
    return 0;
}
