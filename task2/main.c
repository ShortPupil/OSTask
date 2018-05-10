#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMGSIZE 1440*1024
#define TRUE 1
#define FALSE 0

unsigned char image[IMGSIZE];

struct BPB
{
    int BytsPerSec; //每扇区字节数  
    int SecPerClus; //每簇扇区数  
    int RsvdSecCnt; //Boot记录占用的扇区数
    int NumFATs; //FAT表个数  
    int RootEntCnt; //根目录最大文件数  
    int TotSec16;  //扇区总数
    int Media;  //介质描述符
    int FATSz16;   //FAT扇区数  
    int SecPerTrk;  //每磁道扇区数
    int NumHeads;  //磁头数
    int HiddSec;  //隐藏扇区数
    int TotSec32;  //如果BPB_FATSz16为0，该值为FAT扇区数  
} bpb;

//条目
struct Entry
{
    char Name[9];
    char Ext[4];
    char Path[16];
    int FstClus, Type; // 0 for file, 1 for folder
    struct Entry *Next;
    struct Entry *Children;
} root;

struct Counter
{
    char* Line;
    int NumFile;
    int NumDir;
};


typedef struct Node {
    struct Entry* Element;        //    数据域
    struct Node * Next;
}NODE, *PNODE;


typedef struct QNode {
    PNODE Front, Rear;        //    队列头，尾指针
} Queue, *PQueue;

//    声明函数
void InitQueue(PQueue);
_Bool IsEmptyQueue(PQueue);
void InsertQueue(PQueue, struct Entry* val);
void DeleteQueue(PQueue,struct Entry* * val);
void DestroyQueue(PQueue); 
void ClearQueue(PQueue); 
int LengthQueue(PQueue);  

void theprint(char* c);
void printchar(char c);

void InitQueue(PQueue queue) {
    queue->Front = queue->Rear = (PNODE)malloc(sizeof(NODE));   
    if (queue->Front == NULL) {       
        exit(-1);
    }
    queue->Front->Next = NULL;
    //printf("创建队列成功...\n");
}

_Bool IsEmptyQueue(PQueue queue) {
    if (queue->Front == queue->Rear) {
        //printf("队列为空...8\n");
        return TRUE;
    }
    else {
        //printf("队列不为空...\n");
        return FALSE;
    }      
}

void InsertQueue(PQueue queue,struct Entry* val) {
    PNODE P = (PNODE)malloc(sizeof(NODE));  
    if (P == NULL) {
        //printf("内存分配失败，无法插入数据%d...", val);
        exit(-1);
    }
    P->Element = val; 
    P->Next = NULL;     
    queue->Rear->Next = P;
    queue->Rear = P; 
    //printf("插入数据 %s 成功...\n", val->Name);
}


void DeleteQueue(PQueue queue,struct Entry** val) {
    if (IsEmptyQueue(queue)) {
        printf("队列已经空，无法出队...\n");
        exit(-1);
    }
    PNODE  P= queue->Front->Next;
    *val = P->Element; 
    queue->Front->Next = P->Next; 
    if (queue->Rear==P)
        queue->Rear = queue->Front;
    free(P); 
    P = NULL;
    //printf("出栈成功，出栈值为 %s\n", *val);
}


void DestroyQueue(PQueue queue) {

    while (queue->Front != NULL) {
        queue->Rear = queue->Front->Next;
        free(queue->Front);
        queue->Front = queue->Rear;
    }
    //printf("摧毁队列成功...\n");
}

void ClearQueue(PQueue queue) {
    PNODE P = queue->Front->Next;   
    PNODE Q = NULL;      
    queue->Rear = queue->Front; 
    queue->Front->Next = NULL;
    while (P != NULL) {
        Q = P;
        P = P->Next;
        free(Q);
    }
   // printf("清空队列成功...\n");

}

int LengthQueue(PQueue queue){
    int length = 0;
    if (IsEmptyQueue(queue)) {
        exit(-1);
    }        
    PNODE P = queue->Front->Next; 
    //printf("遍历队列结果为：");
    while (P != NULL) {
        length++;
        P = P->Next;
    }
    return length;
}

//大写转小写
void _strlwr(char* str) {
    int i;
    for (i = 0; str[i] != 0; ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 'a' - 'A';
        }
    }
}

//数字转字符串
void _itoa(int i, char* a){
    char const digit[] = "0123456789";
    char* p = a;
    int s = i;
    do {
        p++;
        s /= 10;
    } while (s);
    *p = 0;
    do {
        *(--p) = digit[i % 10];
        i /= 10;
    } while (i);
}

int get_int(int offset, int len)
{
    int result = 0;
    for (int i = offset + len - 1; i >= offset; --i) {
        result = result * 256 + image[i];
    }
    return result;
}

//计算下一个表簇
int get_next_clus(int curr_clus)
{
    int res = 0;
    int clus_id = curr_clus / 2;
    int  fat_value = get_int(bpb.RsvdSecCnt * bpb.BytsPerSec + clus_id * 3, 3);
    if (curr_clus % 2 == 0) {
        res = fat_value & 0x000fff;
    } else {
        res = (fat_value & 0xfff000) / 0x1000;
    }
    return res;
}

void get_dir(struct Entry* entry)
{
    int clus = entry->FstClus, is_root = 1, i,
        offset = (bpb.RsvdSecCnt + bpb.FATSz16 * bpb.NumFATs) * bpb.BytsPerSec;
    if (clus >= 2) {
        is_root = 0;
        offset += bpb.RootEntCnt * 32 + (clus - 2) * bpb.BytsPerSec * bpb.SecPerClus;
    }
    for (i = offset; ; i += 32) {
        if (is_root == 1 && i >= offset + bpb.RootEntCnt * 32) {
            break;
        } else if (is_root == 0 && i >= offset + bpb.BytsPerSec * bpb.SecPerClus) {
            if ((clus = get_next_clus(clus)) >= 0xff7) {
                break;
            } else {
                i = offset = (bpb.RsvdSecCnt + bpb.FATSz16 * bpb.NumFATs) * bpb.BytsPerSec
                        + bpb.RootEntCnt * 32 + (clus - 2) * bpb.BytsPerSec * bpb.SecPerClus;
            }
        }
        if (image[i] != '.' && image[i] != 0 && image[i] != 5 && image[i] != 0xE5
                && image[i + 0xB] != 0xF && (image[i + 0xB] & 0x2) == 0) {
            struct Entry* new_entry = malloc(sizeof(struct Entry));
            int j;
            for (j = i; j < i + 8 && image[j] != 0x20; j++) {
                new_entry->Name[j-i] = image[j];
            }
            new_entry->Name[j-i] = 0;
            _strlwr(new_entry->Name);
            if ((image[i + 0xB] & 0x10) == 0) {
                new_entry->Type = 0;
                for (j = i + 8; j < i + 0xB && image[j] != 0x20; j++) {
                    new_entry->Ext[j - i - 8] = image[j];
                }
                new_entry->Ext[j - i - 8] = 0;
                _strlwr(new_entry->Ext);
            } else {
                new_entry->Type = 1;
            }
            new_entry->FstClus = get_int(i + 26, 2);
            new_entry->Next = NULL;
            new_entry->Children = NULL;
            if (entry->Children == NULL) {
                entry->Children = new_entry;
            } else {
                struct Entry* ptr = entry->Children;
                while (ptr->Next != NULL) {
                    ptr = ptr->Next;
                }
                ptr->Next = new_entry;
            }
            if (new_entry->Type == 1) {
                get_dir(new_entry);
            }
        }
    }
}

void print_dir(struct Entry* entry, char* fullpath)
{
    Queue queue;    //    创建队列变量
    InitQueue(&queue);    //    调用初始化队列函数

    struct Entry* ptr = entry->Children;
    InsertQueue(&queue, entry); 

    while(!IsEmptyQueue(&queue)){    
        
        DeleteQueue(&queue, &ptr);       
        char* new_path = malloc(strlen(fullpath) + strlen(ptr->Name) + strlen(ptr->Path) + 16);
        strcpy(new_path, fullpath);        
        if (ptr->Type == 0) {
            theprint("\x1b[0m");
            theprint(ptr->Name);
            theprint(".");
            theprint(ptr->Ext);   
            // printf("%d\n",LengthQueue(&queue));       
        } else if (ptr->Type == 1){   
            strcpy(new_path, ptr->Path);
            strcat(new_path, ptr->Name);
            strcat(new_path, "/"); 
            //printf("%s", new_path);
            theprint("\x1b[36m");
            theprint(ptr->Name);
            theprint("\x1b[0m");
               
            struct Entry* path = (struct Entry*) malloc(sizeof(struct Entry));
            char* name = (char* )malloc(16);
            strcpy(name, new_path);
            strcpy(path->Name, name);
            path->Type = 2;
            InsertQueue(&queue, path); //记录路径的数据
            
            ptr = ptr->Children; 
            //printf("%d\n",LengthQueue(&queue));
            while(ptr!=NULL){
                 //printf("%s\n",name);
                  strcat(ptr->Path, name);
                  strcat(ptr->Path, "/");
                  InsertQueue(&queue, ptr);
                  //printf("%d\n",LengthQueue(&queue));
                  ptr = ptr->Next;   
           }
           name=NULL;
          // free(path);
        } else if( ptr->Type == 2){    
             //TraverseQueue(&queue);
            // printf("%d\n",LengthQueue(&queue));
             theprint("\n\n");      
             theprint(ptr->Name);
             theprint(":");
             theprint("\n");
        }
        theprint(" ");
        free(new_path);
    }
    ptr=NULL;
    theprint("\n");
    DestroyQueue(&queue);
}

//文件输出 512字节的文件
void print_file(struct Entry* entry) {
    int c = entry->FstClus;
    while (TRUE) {
        int temp = (c - 2) * bpb.BytsPerSec * bpb.SecPerClus;
        temp = bpb.RootEntCnt * 32 + temp;
        int offset = (bpb.RsvdSecCnt + bpb.FATSz16 * bpb.NumFATs) * bpb.BytsPerSec + temp;
        for (int i = 0; i < bpb.BytsPerSec * bpb.SecPerClus; ++i) {
            printchar(image[offset + i]);
        }
        if ((c = get_next_clus(c)) >= 0xff7) { //值大于或等于0xFF8，表示当前簇已经是文件的最后一个簇。0xFF7表示坏簇。
            break;
        }
    }
}

struct Entry* find_file(struct Entry* entry, char* path, char* t)
{
    if (strcmp(path, t) == 0) {
        return entry;
    }
    struct Entry* ptr = entry->Children;
    while (ptr != NULL) {
        char* new_path = malloc(strlen(path) + strlen(ptr->Name) + strlen(ptr->Ext) + 2);
        strcpy(new_path, path);
        strcat(new_path, ptr->Name);
        if (ptr->Type == 0) {
            strcat(new_path, ".");
            strcat(new_path, ptr->Ext);
        }
        if (strcmp(new_path, t) == 0) {
            free(new_path);
            return ptr;
        }
        if (ptr->Type == 1) {
            strcat(new_path, "/");
            struct Entry* result = find_file(ptr, new_path, t);
            if (result != NULL) {
                free(new_path);
                return result;
            }
        }
        free(new_path);
        ptr = ptr->Next;
    }
    return NULL;
}

struct Counter* count_dir(struct Entry* entry, int depth)
{
    struct Counter* counter = malloc(sizeof(struct Counter));
    counter->Line = malloc(1);
    counter->Line[0] = 0;
    counter->NumFile = 0;
    counter->NumDir = 0;
    struct Entry* ptr = entry->Children;
    while (ptr != NULL) {
        if (ptr->Type == 1) {
            struct Counter* new_counter = count_dir(ptr, depth + 1);
            counter->NumDir++;
            char* new_line = malloc(strlen(counter->Line) + strlen(new_counter->Line) + 1);
            strcpy(new_line, counter->Line);
            strcat(new_line, new_counter->Line);
            free(new_counter->Line);
            free(new_counter);
            free(counter->Line);
            counter->Line = new_line;
        } else {
            counter->NumFile++;
        }
        ptr = ptr->Next;
    }
    char* new_line = malloc(strlen(counter->Line) + 128);
    new_line[0] = 0;
    int i;
    for (i = 0; i < depth; ++i) {
        strcat(new_line, "  ");
    }
    if (*entry->Name == 0) {
        *entry->Name = '/';
    }
    strcat(new_line, entry->Name);
    strcat(new_line, " : ");
    _itoa(counter->NumFile, new_line + strlen(new_line));
    strcat(new_line, " file(s), ");
    _itoa(counter->NumDir, new_line + strlen(new_line));
    strcat(new_line, " dir(s)\n");
    strcat(new_line, counter->Line);
    free(counter->Line);
    counter->Line = new_line;
    return counter;
}

int main()
{ 
    FILE *f = fopen("a_64.img", "r");
    fread(image, IMGSIZE, 1, f);

    fclose(f);
    bpb.BytsPerSec = get_int(11, 2);
    bpb.SecPerClus = get_int(13, 1);
    bpb.RsvdSecCnt = get_int(14, 2);
    bpb.NumFATs = get_int(16, 1);
    bpb.RootEntCnt = get_int(17, 2);
    bpb.FATSz16 = get_int(22, 2);

    root.Type = 1;
    get_dir(&root);
    //print_dir(&root, "");
    char command[1024];
    while (TRUE) {
        theprint("> ");
        fgets(command, 1024, stdin);
        _strlwr(command);
        int len = strlen(command);
        if (command[len - 1] == '\n'|| command[len - 1] == '\r' || command[len - 1] == '/') {
            command[--len] = 0;
        }
        if (strcmp(command, "exit") == 0) {
            return 0;
        }
		//"count" command
        if (strstr(command, "count ") == command) {
            char *path = command + 6;
            if (*path == '/') {
                path++;
            }
            struct Entry *result = find_file(&root, "", path);
            if (result == NULL || result->Type != 1) {
                theprint(path);
                theprint(" is not a directory!\n");
            } else {
                struct Counter *count = count_dir(result, 0);
                theprint(count->Line);
            }
        }
        //"cat" command
	else if (strstr(command, "cat ") == command) {
            char *path = command + 4;
            if (*path == '/') {
                path++;
            }
            struct Entry *result = find_file(&root, "", path);
            if (result == NULL) {
                theprint("Unknown file\n");
            } else if (result->Type == 0) {
                print_file(result);
            } else {
                theprint("Error: ");
                theprint(--path);
                theprint(" is not a file!\n");
            }
        }

	else if((strstr(command, "ls ") == command)&&(strlen(command)>2)){
	    char *path = command + 3;
            if (*path == '/') {
                path++;
            }
            struct Entry *result = find_file(&root, "", path);
            if (result == NULL || result->Type != 1) {
                theprint(path);
                theprint(" is not a directory!\n");
            } else {
                 print_dir(result, command);
            }
        }
	//"ls" command
	else if(strcmp(command, "ls") == 0){
		print_dir(&root, "");
	}
    }
    return 0;
}
