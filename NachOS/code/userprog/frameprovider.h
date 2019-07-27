#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H


#include "copyright.h"
#include "bitmap.h"

#define AS_ORDERED 0
#define AS_RANDOM 1
#define AS_INVERSE 2


class FrameProvider
{
    public:
        FrameProvider(unsigned int numOfFrames);
        ~FrameProvider();

        unsigned int GetEmptyFrame(int allocationStrategie);
        void ReleaseFrame(unsigned int frameNumber);
        unsigned int NumAvailFrame();

    private:
        BitMap *allocatedFrames;
		int numPages;
};

#endif // FRAMEPROVIDER_H