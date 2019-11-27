#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include<stdbool.h>


typedef struct blockInfo                            // Structure for storing block information
{
    void *startAdress;
    int size;
}blockInfo;

blockInfo *freeListHead = NULL;                     // Pointer to the Head of Free List
blockInfo *freeListLast=NULL;                       // Pointer to the Tail of Free List

blockInfo *allocatedListHead = NULL;                // Pointer to the Head of Allocated List
blockInfo *allocatedListLast=NULL;                  // Pointer to the Tail of Allocated List

int freeBlocks = 1;                                 // Number of free Blocks available
int allocatedBlocks = 0;                            // Number of Blocks allocated
int currentFreeSpace;                               // Total free space avialable

void *ptr=NULL;

int Mem_GetSize(void *ptr1)
{
    blockInfo *currentnode=allocatedListHead;
    uintptr_t low,high;
    
    for(int i=0;i<allocatedBlocks;i++)               // Traverse the allocated blocks to find the particular alloc header
    {
        
        low=(uintptr_t)currentnode->startAdress;
        high=low + currentnode->size;

        if(low<=(uintptr_t)ptr1 && (uintptr_t)ptr1 < high)
        {
            return currentnode->size;                // Return size of desired header
        }
        currentnode++;
    }
    return -1;

}

int Mem_IsValid(void *user_ptr)
{
    blockInfo *currentnode=allocatedListHead;
    uintptr_t low,high;       
    for(int i=0;i<allocatedBlocks;i++)              // Traversing Allocated Blocks to check if pointer
    {                                               // lies within valid range
        low=(uintptr_t)currentnode->startAdress;
        high=low + currentnode->size;
        if(low<=(uintptr_t)user_ptr && (uintptr_t)user_ptr<high)
            return 1;
        currentnode++;
    }
    return 0;
}

int Mem_Init( int regionSize)
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
         
    ptr = mmap(NULL, (sizeof(blockInfo)+1)*regionSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0); 		
    
    if (ptr == MAP_FAILED) 
    { 
        perror("mmap"); 
        return -1;
    } 
    
    currentFreeSpace = regionSize;
    
    freeListHead = (blockInfo*)(ptr + (sizeof(blockInfo)+1)*regionSize - sizeof(blockInfo));
    allocatedListHead = (blockInfo*)(ptr + regionSize);
   
    allocatedListHead->size = 0;
    
    freeListHead->startAdress = ptr;
    freeListHead->size = regionSize;
    
    freeListLast = freeListHead;
    allocatedListLast = allocatedListHead;

    close(fd); 
    return 1; 
}

void *Mem_Alloc( int spaceAlloc)
{
    if( spaceAlloc > currentFreeSpace)
    {
        perror("Not Enough Memory");
        return NULL;
    }

    blockInfo *bestFreeNode = NULL;
    blockInfo *currentNode = freeListHead;

    for( int i = 0; i < freeBlocks; i++)                // Finding Free block using Best Fit Policy
    {
        if( bestFreeNode == NULL && (currentNode->size >= spaceAlloc))
        {
            bestFreeNode = currentNode;
        }
        else if( (currentNode->size < bestFreeNode->size) && (currentNode->size >= spaceAlloc))
        {
            bestFreeNode = currentNode;
        }
        currentNode--;
    }

    if( bestFreeNode != NULL)
    {
        void *ptr = bestFreeNode->startAdress;
        int bestFreeNodeSize = bestFreeNode->size;

        if( bestFreeNode->size > spaceAlloc)            // Best Available Node having space greater than
        {                                               // needed space
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
        else                                            // Best Available Node having space equal to
        {                                               // needed space
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

void Merge(void *ptr1)                            // To merge contiguous memory blocks
{
    bool flag=true;
    while(flag)
    {
        flag=false;
        blockInfo *ptr=(blockInfo*)ptr1;
        uintptr_t low=(uintptr_t)ptr->startAdress;
        uintptr_t high=low + ptr->size;
        blockInfo *currentBlock=freeListHead;
        uintptr_t low1,high1;
        for (int i = 0; i < freeBlocks; ++i)      // Traverse the free blocks to find free contiguous memory blocks
        {
            low1=(uintptr_t)currentBlock->startAdress;
            high1=(uintptr_t)currentBlock->startAdress + currentBlock->size;
            if(high == low1)                        
            {
                ptr->size+=currentBlock->size;       
                currentBlock->size=freeListLast->size;   
                currentBlock->startAdress=freeListLast->startAdress;
                freeListLast++;
                freeBlocks--;
                flag=true;                      
                break;
            }
            else if(high1 == low)                       
            {
                currentBlock->size+=ptr->size;       
                freeListLast++;
                freeBlocks--;
                ptr1=currentBlock;
                flag=true;                      
                break;
            }       
            currentBlock--;
        }
    }
    
}

int Mem_Free(void *ptr1)
{
    if(ptr1==NULL)
        return -1;


    if(!Mem_IsValid(ptr1))                      // Check if ptr provided by user is within the valid range
    {   
      printf("NOT A VALID POINTER");
        return -1;
    }   
    blockInfo *currentnode=allocatedListHead;
    uintptr_t low,high;
    
    for(int i=0;i<allocatedBlocks;i++)       
    {
        
        low=(uintptr_t)currentnode->startAdress;
        high=low + currentnode->size;

        if(low<=(uintptr_t)ptr1 && (uintptr_t)ptr1<high)
            break;
        currentnode++;  
    }
    int actual_size=currentnode->size;
    currentnode->size=(uintptr_t)ptr1 - low;
    
    blockInfo *newfree;
    newfree=freeListLast - 1; 
    if(currentnode->size!=0)            //partially freed
    {  
        newfree->size = actual_size - currentnode->size;
    }

    else                                //fully freed
    {
        newfree->size = actual_size;
        currentnode->size=allocatedListLast->size;
        currentnode->startAdress=allocatedListLast->startAdress;
        allocatedListLast--;
        allocatedBlocks--;
    }   
    newfree->startAdress=ptr1;
    freeBlocks++;
    freeListLast=newfree;
    currentFreeSpace+=newfree->size;

    Merge((void*)newfree);                 
    
    return 0;
}

/*int main()
{
    if(!Mem_Init(8))
    {
        cout<<"init failed";
        return 1;
    }
    char *ptr=(char*)Mem_Alloc(4*sizeof(char));
    if(ptr==NULL)
        cout<<"NULL";
    //Mem_Free(ptr+20);
    //cout<<(uintptr_t)(ptr)<<endl;
    char *p=ptr;
    for(int i=0;i<4;i++,ptr++)
        *(ptr)=(char)(65+i);
    ptr=p;
    cout<<Mem_GetSize(p);
    Mem_Free(p+2);
    cout<<Mem_GetSize(p);
   
    return 0;
}*/