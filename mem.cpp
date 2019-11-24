#include <bits/stdc++.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

using namespace std;

struct blockInfo
{
    void *startAdress;
    int size;
};

blockInfo *freeListHead = NULL;
blockInfo *freeListLast=NULL;

blockInfo *allocatedListHead = NULL;
blockInfo *allocatedListLast=NULL;

int freeBlocks = 1;
int allocatedBlocks = 0;
int currentFreeSpace;

void *ptr=NULL;

int Mem_init( int regionSize)
{
    if(regionSize<=0)
    {	
    	perror("Invalid size of region");
    	return -1;
    }

    int page_size = getpagesize();
    
    if (regionSize % page_size != 0)
        regionSize = (regionSize/page_size + 1) * page_size;  // Region size in units of page_size

    int fd = open("/dev/zero", O_RDWR); 
    if (fd < 0)
        return -1;
         
    ptr = mmap(NULL, sizeof(blockInfo)*regionSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0); 		
    
    if (ptr == MAP_FAILED) 
    { 
        perror("mmap"); 
        return -1;
    } 
    
    currentFreeSpace = regionSize;
    
    freeListHead = (blockInfo*)(ptr + sizeof(blockInfo)*regionSize - sizeof(blockInfo));
    allocatedListHead = (blockInfo*)(ptr + regionSize);
   
    allocatedListHead->size = 0;
    
    freeListHead->startAdress = ptr;
    freeListHead->size = regionSize;
    
    freeListLast = freeListHead;
    allocatedListLast = allocatedListHead;

    close(fd); 
    return 1; 
}

void *Mem_alloc( int spaceAlloc)
{
    if( spaceAlloc > currentFreeSpace)
    {
        perror("Not Enough Memory");
        return NULL;
    }

    blockInfo *bestFreeNode = NULL;
    blockInfo *currentNode = freeListHead;

    // finding free block for Best Fit Policy
    for( int i = 0; i < freeBlocks; i++)
    {
        if( (currentNode->size < bestFreeNode->size) && (currentNode->size >= spaceAlloc))
            bestFreeNode = currentNode;

        currentNode--;
    }

    if( bestFreeNode != NULL)
    {
        void *ptr = bestFreeNode->startAdress;
        int bestFreeNodeSize = bestFreeNode->size;

        if( bestFreeNode->size > spaceAlloc)
        {
            bestFreeNode->size = bestFreeNode->size - spaceAlloc;
            bestFreeNode->startAdress = bestFreeNode->startAdress + spaceAlloc;

            blockInfo *newAllocatedBlock;

            if( allocatedListHead->size != 0)
                newAllocatedBlock = allocatedListLast + 1;
            else
                newAllocatedBlock = allocatedListHead;

            newAllocatedBlock->size = spaceAlloc;
            newAllocatedBlock->startAdress = ptr;
            allocatedBlocks++;
            allocatedListLast = newAllocatedBlock;
            currentFreeSpace = currentFreeSpace - spaceAlloc;
        }
        else
        {
            blockInfo *newAllocatedBlock;

            if( allocatedListHead->size != 0)
                newAllocatedBlock = allocatedListLast + 1;
            else
                newAllocatedBlock = allocatedListHead;

            newAllocatedBlock->size = spaceAlloc;
            newAllocatedBlock->startAdress = ptr;
            allocatedBlocks++;
            allocatedListLast = newAllocatedBlock;
            currentFreeSpace = currentFreeSpace - spaceAlloc;

            if( bestFreeNode != freeListLast)
            {
                bestFreeNode->startAdress = freeListLast->startAdress;
                bestFreeNode->size = freeListLast->size;
            }

            freeListLast++;

            if( freeBlocks == 1)
            {
                freeListHead->startAdress = NULL;
                freeListHead->size = 0;
            }
            else
            {
                freeBlocks--;
            }

        }
        return ptr;
    }
    else
    {
        perror("Not Enough Memory!\n");
        return NULL;
    }
}
