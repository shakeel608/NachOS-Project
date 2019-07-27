#include "copyright.h"
#include "system.h"
#include "frameprovider.h"
#include "bitmap.h"
#include <strings.h>		/* for bzero */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


FrameProvider::FrameProvider(unsigned int numOfFrames){
	srand (time(NULL));
	allocatedFrames = new BitMap(numOfFrames);
	numPages = numOfFrames;
}

FrameProvider::~FrameProvider()
{
	delete allocatedFrames;
}

unsigned int
FrameProvider::GetEmptyFrame(int allocationStrategie){
	int selectedFrame = 0;
	switch(allocationStrategie){
		case AS_ORDERED:
			selectedFrame = allocatedFrames->Find();
			break;

		case AS_RANDOM:
			int i, j;
			
			selectedFrame = rand() % numPages;

		    for(i = selectedFrame; i < numPages; i++)
		    	//Make sure selectedFrame is free, otherwise look for the next one
				if(!allocatedFrames->Test(i)){
					allocatedFrames->Mark(i);
			    	selectedFrame = i;
			    	break;
			  	}
			//If we reached the end of the list, start over from the beginnig
			if(i == numPages){
			    for(j = 0; j < numPages; j++)
					if(!allocatedFrames->Test(j)){
						allocatedFrames->Mark(j);
				    	selectedFrame = j;
				    	break;
				  	}
				//If we reached the end of the list, there is not empty place left
				if(i == selectedFrame)
					selectedFrame = -1;
			}
			break;

		case AS_INVERSE:
			for(i = numPages - 1; i >= 0; i--){
				if(!allocatedFrames->Test(i)){
					allocatedFrames->Mark(i);
			    	selectedFrame = i;
			    	break;
			  	}				
			}
			if(i < 0)
				selectedFrame = -1;
			break;

		default:
			break;
	}
	
	if(selectedFrame >= 0)
		bzero (&machine->mainMemory[PageSize*selectedFrame], PageSize);
	return selectedFrame;
}

void
FrameProvider::ReleaseFrame(unsigned int frameNumber){

	allocatedFrames->Clear(frameNumber);

}


unsigned int
FrameProvider::NumAvailFrame(){

	return allocatedFrames->NumClear();

}