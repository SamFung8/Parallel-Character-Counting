/*
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your group information here.
 *
 * Group No.: 10 (Join a project group in Canvas)
 * First member's full name: Tom Chan
 * First member's email address: cctom2-c@my.cityu.edu.hk
 * Second member's full name: (leave blank if none)
 * Second member's email address: (leave blank if none)
 * Third member's full name: (leave blank if none)
 * Third member's email address: (leave blank if none)
 */

#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>                           
#include <sys/stat.h>                     
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>

char *filenames[100];
int file_num = 0;
int cnt[100][26];
/*
PrintCount(int idx)
Output function, which is used to output file statistics
Enter file serial number for int.
It can output statistics about the file
*/
void PrintCount(int idx){
	int i;
	char c;
	printf("%s\n",filenames[idx]);//Output file path
	for(i=0;i<26;i++){//Output statistics of 26 letters
		if(cnt[idx][i] > 0){
			c = 'a' + i;//Because the letter 'a' was subtracted before, 
                        //it is now added back to become a normal ASCII character code
			printf("%c %d\n",c,cnt[idx][i]);
		}
	}
}
/*
void *CountFile(void *args)
Counting function, used to count the number of different letters in the file.
The input is a typeless pointer, 
but the general input is an int pointer, 
which points to the file serial number.

If there is no return, the data will be stored in the global variable **cnt
*/
void *CountFile(void *args){
    int *idx = (int*)args;
	int i;
	char *buf;
    struct stat st;
    
    FILE *fp = fopen(filenames[*idx],"r");//read file
    int fd = fileno(fp);//Gets the file descriptor used by the file stream
	fstat(fd, &st);//Get file status from file descriptor
	buf = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);//Memory mapping
    for(i=0;i<st.st_size;i++){
        if(buf[i]<'a' || buf[i]>'z') continue;//Because I found capital letters
		cnt[*idx][buf[i]-'a']++;//letter num ++
	}
	fclose(fp);
    return NULL;
}

/*
void get_all_files(char *path)
Directory judgment function
Enter the file address of char *
No return
Add the read file to the file address array. 
If a directory is encountered, 
add all the files in the directory to the file address array 
until the directory root is reached
*/
void get_all_files(char *path){
    DIR *dir;
    struct dirent *ptr;
    int path_len = strlen(path);

    // If the current path is not a directory, it defaults to file
    if((dir=opendir(path)) == NULL){
        filenames[file_num++] = path;
        return;
    }

    // At this time, you can conclude that the path is a folder path, 
    //and you need to remove the trailing slash
    if(path[path_len-1]=='/'){
        path[path_len-1]='\0';
        path_len--;
    }

    while((ptr=readdir(dir)) != NULL){
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0){
            //Not the current directory or parent directory
            continue;
        }
        int subfilepath_len = strlen(ptr->d_name);//Get address length
        char *subpath = (char*)malloc(path_len+subfilepath_len+2);
        //Create new address pointer
        strcpy(subpath, path);//copy
        strcpy(subpath+path_len, "/");
        strcpy(subpath+path_len+1, ptr->d_name);//to new dir
        get_all_files(subpath);//Enter secondary directory
    }
    closedir(dir);
}

// Swap two strings
void swap(char **a, char **b){
    char *t = *b;
    *b = *a;
    *a = t;
}

// Using bubble sorting method to sort files
//Allows files to be output in dictionary order
void bubble_sort(char *a[],int len){
    int i,j;
    for(i=0;i<len;i++){
        for(j=len-1;j>=i+1;j--){
            if(strcmp(a[j], a[j-1])<0){
                swap(&a[j], &a[j-1]);
            }
        }
    }
}

int main(int argc, char** argv){
    //sem_t semLock;
    int i;
	pthread_t *pThreads;

    //Null parameter
    if(argc==1){
        puts("pzip: file1 [file2 ...]");
        return 1;
    }
	
    // sem_init(&semLock,0,1);
    
    for(i=1;i<argc;i++){
        get_all_files((char*)argv[i]);
    }

    //Bubble dictionary sort
    bubble_sort(filenames, file_num);
    //Thread array
    pThreads = (pthread_t *)malloc(file_num*sizeof(pthread_t));
    //Based on single file, single thread traversal
    for(i=0;i<file_num;i++){
        int *indices = (int*)malloc(sizeof(int));
        *indices = i;
        pthread_create((pThreads+i), NULL, CountFile, indices);
    }
    //Wait for all threads to end
    for(i=0;i<file_num;i++){
        pthread_join(*(pThreads+i),NULL);  
    }
    //free
    free(pThreads);
    //print
    for(i=0;i<file_num;i++){
	    PrintCount(i);
    }

    return 0;  
}
