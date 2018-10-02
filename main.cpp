#include <QCoreApplication>



#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)


// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

//////
/// \brief ImgProcess::YuyvToRgb32 YUV420 to RGBA 32
/// \param pYuv input image
/// \param pRgb     output RGB32 buffer, must be allocated by caller before call
/// \param uFirst   true if pYuv is YUYV, false if YVYU
///
void YuyvToRgb32(unsigned char* pYuv, int width, int height, unsigned char* pRgb, bool uFirst, bool bMirror)
{
    //YVYU - format
    int nBps = width*4;//stride in RGB data
    unsigned char* pY1 = pYuv;

    unsigned char* pV;
    unsigned char* pU;

    int nStride = width*2;
    if (bMirror) {
        pY1 = pYuv +(height-1)* nStride;
        nStride = -nStride;
    }

    if (uFirst) {
        pU = pY1+1; pV = pU+2;
    } else {
        pV = pY1+1; pU = pV+2;
    }


    unsigned char* pLine1 = pRgb;

    unsigned char y1,u,v;
    for (unsigned int i=0; i<height; i++)
    {
        for (unsigned int j=0; j<width; j+=2)
        {
            y1 = pY1[2*j];
            u = pU[2*j];
            v = pV[2*j];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y1 = pY1[2*j+2];
            pLine1[j*4+4] = YUV2B(y1, u, v);//b
            pLine1[j*4+5] = YUV2G(y1, u, v);//g
            pLine1[j*4+6] = YUV2R(y1, u, v);//r
            pLine1[j*4+7] = 0xff;
        }
        pY1 += nStride;
        pV += nStride;
        pU += nStride;
        pLine1 += nBps;

    }
}

void printUsage(char* argv)
{
    printf("Convert YUYV to RGB32\n");
    printf("Usage: %s <yuv file> <width> <height>\n", argv);
    exit(-1);
}
int main(int argc, char *argv[])
{
    if (argc <4)
        printUsage(argv[0]);
    FILE* fpin = fopen(argv[1], "r");
    if (!fpin) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return -1;
    }
    char outname[] = "output.rgb";
    FILE* fpout = fopen(outname, "w");
    if (!fpout) {
        fclose(fpin);
        fprintf(stderr, "failed to opwn output file %s!", outname);
        return -1;
    }
    int width = atoi(argv[2]);
    int height = atoi(argv[3]);
    int lenin = width*height*2;
    int lenout = width*height*4;
    unsigned char* bufferin = (unsigned char*)malloc(lenin);
    unsigned char* bufferout = (unsigned char*)malloc(lenout);

    int nFrame = 0;
    while (bufferin && bufferout) {
        if(lenin != fread(bufferin, 1, lenin, fpin)) //EOF
            break;
        ///
        YuyvToRgb32(bufferin, width, height, bufferout, true, false);
        ///
        if(lenout != fwrite(bufferout, 1, lenout, fpout)) //EOF
            break;
        nFrame ++;
    }

    printf("convert %d frames.\n", nFrame);
    if(bufferin)
        free(bufferin);
    if(bufferout)
        free(bufferout);
    if(fpin)
        fclose(fpin);
    if(fpout)
        fclose(fpout);


    return 0;
}
