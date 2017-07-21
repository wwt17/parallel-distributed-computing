#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <sys/time.h>
#include <cassert>
#include <omp.h>
using namespace std;
typedef unsigned int uint32;
typedef unsigned long long int uint64;

inline void sieve(uint64 start,uint64 end,uint64& num,int threadNum)
{
	  assert(start>1);
	    bool* a =new bool[end+1];
		  memset(a+2,true,end+1);

		    #pragma omp parallel for num_threads(threadNum)
		    for (uint64 i = 2; i <=(uint64)sqrt(end); i++)
				  {
					      if (a[i])
							        for (uint64 j = i; i*j <= end; j++) 
										        a[i*j] = false;
						    }
			  uint64 prime_num=0;
			    if(start==2)
					    prime_num++;

				  #pragma omp parallel for num_threads(threadNum) reduction(+: prime_num)
				  for (uint64 i =(start%2==0?start+1:start); i <=end ;i += 2)
					    {
							    if (a[i])
									      prime_num++;
								  }
				    num=prime_num;
					  delete[] a;
} 

int main(int argc,char* argv[])
{
	    if(argc!=4){
			      fprintf(stderr, "usage: Eratosthenes start_number end_number threadNum\n");
				        exit(-1);
						    }
		    struct timeval ts,te;
			    uint64 start=atoi(argv[1]);
				    uint64 end=atoi(argv[2]);
					    int threadNum=atoi(argv[3]);
						    uint64 num=0;
							    gettimeofday(&ts,NULL);
								    sieve(start,end,num,threadNum);
									    gettimeofday(&te,NULL);
										    cout<<"count: "<<num<<endl;
											    cout<<"total time: "<<((te.tv_sec-ts.tv_sec)*1000+(te.tv_usec-ts.tv_usec)/1000)<<"ms"<<endl;
													    return 0;
}
