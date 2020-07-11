/*************************************************************************
	> File Name: file_copy.c
	> Author: dwx
	> Mail: dengwanxiong@foxmail.com 
	> Created Time: 2020年07月11日 星期六 14时42分14秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/stat.h>

int main(int argc, char *argv[]){

	//指定进程的个数
	int n = 5;
	//打开要拷贝的文件
	int fd1 = open("test.txt", O_RDWR);
	if(fd1 < 0){
		perror("open error");
		exit(1);
	}
	int fd2 = open("destination.txt", O_RDWR | O_CREAT, 0644);
	if(fd2 < 0){
		perror("open error");
		exit(1);
	}
	//获取文件的大小
	struct stat file_info;
	int ret = fstat(fd1, &file_info);
	int size = 0;
	if(!ret){
		size = file_info.st_size;
	}
	printf("文件大小为%d\n", size);
	//根据文件大小拓展目标文件
	ftruncate(fd2, size);

	//为源文件创建映射
	char *mm1 = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
	if(mm1 == MAP_FAILED){
		perror("mmap error");
		exit(1);
	}
	//为目标文件创建映射
	char *mm2 = mmap(NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED, fd2, 0);
	if(mm2 == MAP_FAILED){
		perror("mmap2 error");
		exit(1);
	}
	char *p2 = mm2;
	char *p1 = mm1;
	//求出每一个字节应该拷贝的字节数
	//size为字符数
	printf("total = %d\n", size);
	int bsize = size / n;
	int left = size-(bsize*n);
	int i;
	//创建N个子进程
	for(i=0; i<n; i++){
		pid_t pid = fork();
		if(pid < 0){
			perror("fork error");
			exit(1);
		}
		else if(pid == 0)
		{
			if( i == n-1 ){
				printf("i=%d\n",i);
				printf("n=%d\n", n);
				printf("last copy %d\n", bsize+left);
				p1 = mm1 + (bsize * i);
				p2 = mm2 + (bsize * i);
				memcpy(p2, p1, bsize+left);
			}else{
				//进行拷贝
				printf("i=%d\n",i);
			    printf("n=%d\n", n);
				//指针移动的性质
				p1 = mm1 + (bsize * i);
				p2 = mm2 + (bsize * i);
				memcpy(p2, p1, bsize);
			}
			printf("已经完成一份拷贝\n");
			//memcpy(mm2+10, mm1+10, size-10);
			break;
		}else
		{
		}
	}
	//循环结束之后
	if( i < n){
		sleep(i);
		printf("child %d exit\n", i);
	}else{
		sleep(10);
		//回收子进程
		int k = n;
		while (k>0)
		{
			if(waitpid(-1, NULL, WNOHANG) > 0){
				printf("回收一个子进程\n");
				k--;
			}
			//printf("在循环中\n");
		}
		//释放映射区 应该交给父进程
		int ret1 = munmap(mm1, size);
		if(ret1 < 0){
			perror("munmap1 error");
			exit(1);
		}
		ret1 = munmap(mm2, size);
		if(ret1 < 0){
			perror("munmap1 error");
			exit(1);
		}
	}
	return 0;
}
