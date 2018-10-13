#include <pthread.h>
#include <stdio.h>

#define NUM_CHILD 5

void* fib(void* v){
	long n = (long)v;
	if (n==0)
		return 0;
	if (n==1)
		return (void *)1;
	if (n==2)
		return (void *)1;

	void *ret1 = fib((void*)(n-1));
	void *ret2 = fib((void*)(n-2));
	long ret3 = (long)ret1 + (long)ret2;
	return (void*)ret3;
}

int main(){
	pthread_t children[NUM_CHILD];
	for(long i=1; i < NUM_CHILD; i++){
		pthread_t tid;
		int val = pthread_create(&tid, NULL, fib, (void*)i);
		if (val<0){
			return -1;
		}
		else{
			children[i] = tid;
		}
	}

	for(int i = 1; i < NUM_CHILD; i++){
		int *ret_val;
		pthread_join(children[i], (void**)&ret_val);
		printf("Child %d is %d\n",i,(int)ret_val);
	}

	return 0;
}