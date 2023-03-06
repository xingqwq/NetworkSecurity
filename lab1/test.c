#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void getFileList(char *fileList)
{
    DIR *dirPtr;
    struct dirent *ptr;
    dirPtr = opendir("./server/");
    int totalLen = 0;
    while ((ptr = readdir(dirPtr)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
        {
            continue;
        }
        if (totalLen + strlen(ptr->d_name) > 1024)
        {
            break;
        }
        else
        {
            strcat(fileList, ptr->d_name);
            strcat(fileList, "\n");
            totalLen += strlen(ptr->d_name);
        }
    }
    closedir(dirPtr);
}
int main(void)
{
    char fileList[1024];
    memset(&fileList,0,1024);
    getFileList(fileList);
    printf("%s\n",fileList);
}
