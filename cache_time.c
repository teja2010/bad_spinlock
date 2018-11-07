#include<stdio.h>
#include<time.h>
#include<string.h>
#include <sys/mman.h>
#include <errno.h>
//#include <x86_64-linux-musl/sys/cachectl.h>
//#include <uapi/asm/cachectl.h>

int main()
{
	struct timespec tt, time[2];
	int *one;	//="are you in cache?";
	int len=100;	// = strlen(one);
	int i;
	int fd;

	one = (int*) mmap(0, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
	if(one == MAP_FAILED)
		printf("mmap failed %s\n", strerror(errno));

	/* lets get cache res */
	clock_getres(CLOCK_REALTIME, &tt);
	printf("CLOCK_REALTIME res %ldns\n", tt.tv_nsec);
	clock_getres(CLOCK_MONOTONIC, &tt);
	printf("CLOCK_MONOTONIC res %ldns\n", tt.tv_nsec);

	__clear_cache(one, one+len);
	for(i=0; i<len; i++) {
		__builtin_prefetch(&one[i], 0,3);
		one[i]=i*i;
	}
	/* ok, in cache*/

	clock_gettime(CLOCK_REALTIME, &time[0]);
	for(i=0; i<len; i++) {
	//for(i=0; i<strlen(one); i++){
		if(one[i]=='A')
			break;
	}
	clock_gettime(CLOCK_REALTIME, &time[1]);
	printf("time it took %ldns\n", time[1].tv_nsec - time[0].tv_nsec);

	__clear_cache(one, one+len);
	//__builtin___clear_cache(one, one+len);
	//__builtin___clear_cache(one, one+len);
//	if(cacheflush(&one, len, BCACHE)==-1)
//		printf("cacheflush failed\n");

	clock_gettime(CLOCK_REALTIME, &time[0]);
	for(i=0; i<len; i++) {
	//for(i=0; i<strlen(one); i++){
		if(one[i]=='A')
			break;
	}
	clock_gettime(CLOCK_REALTIME, &time[1]);
	printf("time it took %ldns\n", time[1].tv_nsec - time[0].tv_nsec);

	clock_gettime(CLOCK_REALTIME, &time[0]);
	for(i=0; i<len; i++) {
	//for(i=0; i<strlen(one); i++){
		if(one[i]=='A')
			break;
	}
	clock_gettime(CLOCK_REALTIME, &time[1]);
	printf("time it took %ldns\n", time[1].tv_nsec - time[0].tv_nsec);

	return 0;
}
