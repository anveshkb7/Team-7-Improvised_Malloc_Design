//#include<bits/stdc++.h>
#include<stdio.h>
#include "mem1.h"
//using namespace std;
int main()
{
    if(!Mem_Init(8))
    {
        printf("INIT FAILED");
        return 1;
    }
    char *ptr=(char*)Mem_Alloc(4*sizeof(char));
    if(ptr==NULL)
        printf("NULL");
    //Mem_Free(ptr+20);
    //cout<<(uintptr_t)(ptr)<<endl;
    char *p=ptr;
    for(int i=0;i<4;i++,ptr++)
        *(ptr)=(char)(65+i);
    ptr=p;
   // cout<<Mem_GetSize(p);
   printf("%d/n",Mem_GetSize(p));
    Mem_Free(p+2);
   // cout<<Mem_GetSize(p);
    printf("%d",Mem_GetSize(p));
   
    return 1;
}
