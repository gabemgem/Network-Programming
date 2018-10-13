#include <pthread.h>
#include <stdio.h>

#define NUM_CHILD 5

struct add_nums{
	int a;
	int b;
};

int add(int a, int b) {
	if(b<=0) {
		return a;
	}
	return 1+add(a,b-1);
}

void* addsetup(void* nums) {

	return (void*)add(((struct add_nums*)nums)->a, ((struct add_nums*)nums)->b);
}

int main(){
	int total_children = (NUM_CHILD-1)*(NUM_CHILD);
	pthread_t children[total_children+1];
	int child_id=1;
	struct add_nums nums_arr[total_children+1];
	for(long i=1; i<NUM_CHILD; ++i) {
		for(long j=1; j<=NUM_CHILD; ++j) {
			nums_arr[child_id].a=i;
			nums_arr[child_id].b=j;
			pthread_t tid;
			int val = pthread_create(&tid, NULL, addsetup, (void*)&(nums_arr[child_id]));
			if(val<0) {
				return -1;
			}
			else {
				children[child_id] = tid;
				child_id++;
			}
		}
	}

	for(int i=1; i<=total_children; ++i) {
		int* ret_val;
		pthread_join(children[i], (void**)&ret_val);
		printf("[%d + %d] Child %d is %d\n", nums_arr[i].a, nums_arr[i].b, i, (int)ret_val);

	}
	return 0;
}