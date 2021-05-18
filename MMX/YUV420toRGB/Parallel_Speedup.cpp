// Parallel_Speedup.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <assert.h>

using namespace std;

static  short CoefficientsRGBY[256][4];
static  short CoefficientsRGBU[256][4];
static  short CoefficientsRGBV[256][4];
void init_coefficients()
{
    int i;
    for (i = 0; i < 256; i++)
    {
        CoefficientsRGBY[i][0] = (short)(1.164 * 64 * (i - 16) + 0.5);
        CoefficientsRGBY[i][1] = (short)(1.164 * 64 * (i - 16) + 0.5);
        CoefficientsRGBY[i][2] = (short)(1.164 * 64 * (i - 16) + 0.5);
        CoefficientsRGBY[i][3] = 0x00;

        CoefficientsRGBU[i][0] = (short)(2.018 * 64 * (i - 128) + 0.5);
        CoefficientsRGBU[i][1] = (short)(-0.391 * 64 * (i - 128) + 0.5);
        CoefficientsRGBU[i][2] = 0x00;
        CoefficientsRGBU[i][3] = 0x00;

        CoefficientsRGBV[i][0] = 0x00;
        CoefficientsRGBV[i][1] = (short)(-0.813 * 64 * (i - 128) + 0.5);
        CoefficientsRGBV[i][2] = (short)(1.596 * 64 * (i - 128) + 0.5);
        CoefficientsRGBV[i][3] = 0x00;
    }
}

int YUV420_RGB32_mmx(uint32_t* rgb, int width, int height, uint8_t* y, uint8_t* u, uint8_t* v)
{
    __asm {
        pushad
        finit
        xor eax, eax
        mov ebx, height
        mov ecx, width
        mov edx, y
        mov edi, v
        mov esi, u
        mov ebp, rgb
        hloop :
        push ebx
            mov ebx, ecx
            wloop :
        push ebx
            xor ebx, ebx
            mov al, [edi]
            mov bl, [esi]
            movq mm0, [CoefficientsRGBU + 8 * eax]
            paddw mm0, [CoefficientsRGBV + 8 * ebx]
            mov al, [edx]
            mov bl, [edx + 1]
            movq mm1, [CoefficientsRGBY + 8 * eax]
            movq mm2, [CoefficientsRGBY + 8 * ebx]
            mov al, [edx + ecx]
            mov bl, [edx + ecx + 1]
            movq mm3, [CoefficientsRGBY + 8 * eax]
            movq mm4, [CoefficientsRGBY + 8 * ebx]
            paddw mm1, mm0
            paddw mm2, mm0
            paddw mm3, mm0
            paddw mm4, mm0
            psraw mm1, 6
            psraw mm2, 6
            psraw mm3, 6
            psraw mm4, 6
            packuswb mm1, mm2
            packuswb mm3, mm4
            movq[ebp], mm1
            movq[ebp + 4 * ecx], mm3
            add ebp, 8
            add edx, 2
            add edi, 1
            add esi, 1
            pop ebx
            sub ebx, 2
            jnz wloop
            lea ebp, [ebp + 4 * ecx]
            add edx, ecx
            pop ebx
            sub ebx, 2
            jnz hloop
            emms
            popad
    }
}

const char* inPath = "akiyo_cif.yuv";
const char* outPath = "miss_out.cif";
const char* outRGBPath = "rgba.txt";
//https://gist.github.com/muiz6/51139825c7d38aa58b03034ed997a1aa
int main()
{
    init_coefficients();
    fstream f;
    f.open(inPath, ios::in | ios::binary);
    if (!f.is_open())
    {
        cout << "CANNOT OPEN" << endl;
    }
    f.seekg(0, ios::end);
    long long size = f.tellg();
    f.seekg(0, ios::beg);

    char* inBuffer = new char[size];
    f.read(inBuffer, size);
    f.close();

    char* outBuffer = new char[size];
    int width = 352, height = 288, inFrameSize = width * height * 3 / 2, UFrameSize = width* height * 1.25;
    char* outRGBBuffer = new char[width*height*4];
    int frameNum = size / inFrameSize;

    //keep Y, make U,V as 128
    for (int i = 0; i < frameNum; ++i) {
        for (int j = 0; j < inFrameSize; ++j) {
            outBuffer[i * inFrameSize + j] = j < width* height ? inBuffer[i * inFrameSize + j] : 128;
            YUV420_RGB32_mmx((uint32_t*)outRGBBuffer, width, height, (uint8_t*)&outRGBBuffer[i * inFrameSize], (uint8_t*)&outRGBBuffer[i * inFrameSize + width * height], (uint8_t*)&outRGBBuffer[i * inFrameSize + UFrameSize]);
            ofstream out(outRGBPath, ios::binary);
            out.write(outRGBBuffer, width*height*4);
            out.close();
        }
    }
    ofstream out(outPath, ios::binary);
    out.write(outBuffer, size);
    out.close();

    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
