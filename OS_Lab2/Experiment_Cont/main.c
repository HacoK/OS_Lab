#include<stdio.h>
#include<string.h>
#define ENTRY_SIZE 0x20
#define ROOTDIR_OFFSET 0x2600
#define FAT_OFFSET 0x200
#define DATA_OFFSET 0x4200
#define CLUS_SIZE 0x200
#define TRUE 1
#define FALSE 0

const char* path = "a.img";


struct Entry
{
	char fileName[11]; 
	char attr; 
	char reversed[14];
	char fstClus[2];
	char fileSize[4];
};

struct FileInfo
{
	char path[120];
	char fileName[13];
	char fstClus[2];
} fileInfo[50];

struct DirInfo
{
	char path[120];
	char fileName[13];
	char fstClus[2];
} dirInfo[50];

FILE* file;
struct Entry entry;
struct Entry* entryPtr = &entry;
int filePointer = -1;
int dirPointer = -1;
int procPointer = -1;

extern void myPrint(char* str, int length);

void readFiles(int offset, char* path);
int isEntryEmpty(struct Entry* entryPtr);
void formatFileName(char fileName[], char* rawFileName);
int getClus(char clus[2]);
long getOffsetByClus(int clus);
void getCommand();
void print_ls(char* path);
int isSubDir(char* sub, char* super);
int getFATValue(int index);
void itoa(char* buffer, int num);
void printCount(char* dir, char* origin, char* name);
int isFileNameValid(struct Entry* entryPtr);
void toUpper(char* input);
int countDepth(char* path,char* origin);

int main(){
	file = fopen(path, "rb");
	if (file == NULL){
		char* prompt = "Open a.img failed!\n";
		myPrint(prompt, strlen(prompt));

	}

	readFiles(ROOTDIR_OFFSET, "");

	getCommand();

	fclose(file);
}

void readFiles(int offset, char* path){
	fseek(file, offset, SEEK_SET);
	while (procPointer <= dirPointer){
		fread(entryPtr, ENTRY_SIZE, 1, file);
		if (isEntryEmpty(entryPtr)){
			procPointer++;
			if (procPointer <= dirPointer)
			{
				char tempPath[120] = "";
				strcpy(tempPath, dirInfo[procPointer].path);
				strcat(tempPath, "/");
				strcat(tempPath, dirInfo[procPointer].fileName);
				readFiles(getOffsetByClus(getClus(dirInfo[procPointer].fstClus)), tempPath);
			}
			else
				return;
		}
		else{
			if (!isFileNameValid(entryPtr))
				continue;
			char fFileName[13];
			formatFileName(fFileName, entryPtr->fileName);
			if (entryPtr->attr == 0x10){
				dirPointer++;
				strcpy(dirInfo[dirPointer].fileName, fFileName);
				strcpy(dirInfo[dirPointer].path, path);
				strcpy(dirInfo[dirPointer].fstClus, entryPtr->fstClus);
			}
			else{
				filePointer++;
				strcpy(fileInfo[filePointer].fileName, fFileName);
				strcpy(fileInfo[filePointer].path, path);
				strcpy(fileInfo[filePointer].fstClus, entryPtr->fstClus);
			}
		}
	}
	return;
}

int isEntryEmpty(struct Entry* entryPtr){
	int i = 0;
	for (i = 0; i < ENTRY_SIZE; i++){
		if (((char*)entryPtr)[i] != 0){
			return FALSE;
		}
	}
	return TRUE;
}

void formatFileName(char fileName[], char* rawFileName){
	int index = 0;
	int lastIndex;
	for (int i = 7; i >= 0; i--){
		if (rawFileName[i] != ' '){
			lastIndex = i;
			break;
		}
	}
	for (int i = 0; i <= lastIndex; ++i){
		fileName[index] = rawFileName[i];
		index++;
	}
	if (rawFileName[8] != ' '){
		fileName[index] = '.';
		index++;
		for (int i = 8; i < 11; ++i){
			if (rawFileName[i] != ' '){
				fileName[index] = rawFileName[i];
				index++;
			}
			else {
				break;
			}
		}
	}
	fileName[index] = '\0';
}

int getClus(char clus[2]){
	return (unsigned char)clus[1] * 256 + (unsigned char)clus[0];
}

long getOffsetByClus(int clus){
	return DATA_OFFSET + (clus - 2) * CLUS_SIZE;
}

void getCommand(){

	char* prompt = "Enter your command: \n";
	char* subPrompt = ">";
	char* blue = "\033[34;1m";
	char* white = "\033[37;0m";
	myPrint(prompt, strlen(prompt));

	while (TRUE){
		myPrint(blue, strlen(blue));
		myPrint(subPrompt, strlen(subPrompt));
		myPrint(white, strlen(white));
		char input[50];
		scanf("%[^\n]%*c",input);
		toUpper(input);
		if (strcmp(input, "EXIT") == 0){
			break;
		}
		if (input[0] == 'L'&&input[1] == 'S') {
			if (strlen(input) == 2){
				print_ls("");
				char link[120];
				for (int i = 0; i <= dirPointer; i++){
					strcpy(link, dirInfo[i].path);
					strcat(link, "/");
					strcat(link, dirInfo[i].fileName);
					print_ls(link);
				}
			}
			else{
				char tempStr[120];
				strcpy(tempStr, input + 3);
				int isPathValid = 0;
				char link[120];
				for (int i = 0; i <= dirPointer; i++){
					strcpy(link, dirInfo[i].path);
					strcat(link, "/");
					strcat(link, dirInfo[i].fileName);
					if (!strcmp(link, tempStr))
						isPathValid = 1;
				}
				if (isPathValid){					
					 for (int i = 0; i <= dirPointer; i++){
						 strcpy(link, dirInfo[i].path);
					     strcat(link, "/");
					     strcat(link, dirInfo[i].fileName);
						 if (isSubDir(link, tempStr))
							 print_ls(link);
					 }
				}
				else{
					char* red = "\033[31;1m";
				    myPrint(red, strlen(red));
					myPrint("Error:", 6);
					char error_msg[120];
					strcpy(error_msg, tempStr);
					strcat(error_msg, " is not a valid dir!\n");
					myPrint(error_msg, strlen(error_msg));
					char* white = "\033[37;0m";
			        myPrint(white, strlen(white));
				}
			}
		}
		else if (input[0] == 'C'&&input[1] == 'A'&&input[2] == 'T'){
			char tempFile[120];
			strcpy(tempFile, input + 4);
			int isFileValid = 0;
			int index = 0;
			char link[120];
			for (; index <= filePointer; index++){
				strcpy(link, fileInfo[index].path);
				strcat(link, "/");
				strcat(link, fileInfo[index].fileName);
				if (!strcmp(link, tempFile)){
					isFileValid = 1;
					break;
				}
			}
			if (isFileValid){
				int refer = getClus(fileInfo[index].fstClus);
				while (refer != 0x00 && refer < 0xFF7){
					fseek(file, getOffsetByClus(refer), SEEK_SET);
					char content[1];
					int count = 0;
					fread(content, sizeof(char), 1, file);
					while (content[0] != 0 && count < CLUS_SIZE){
						myPrint(content, 1);
						fread(content, sizeof(char), 1, file);
						count++;
					}
					refer = getFATValue(refer);
				}
			}
			else{
				char* red = "\033[31;1m";
				myPrint(red, strlen(red));
				myPrint("Error:", 6);
				char error_msg[120];
				strcpy(error_msg, tempFile);
				strcat(error_msg, " is not a valid file!\n");
				myPrint(error_msg, strlen(error_msg));
				char* white = "\033[37;0m";
			    myPrint(white, strlen(white));
			}
		}
		else if (input[0] == 'C'&&input[1] == 'O'&&input[2] == 'U'&&input[3] == 'N'&&input[4] == 'T'){
			char countDir[120];
			if(strlen(input)==5)
				strcpy(countDir,"");
			else
				strcpy(countDir, input + 6);
			int isValid = 0;
			char link[120];
			int index = 0;
			if(!strcmp(countDir,"")){
				isValid=1;
			}
			for (; index <= dirPointer; index++){
				strcpy(link, dirInfo[index].path);
				strcat(link, "/");
				strcat(link, dirInfo[index].fileName);
				if (!strcmp(link, countDir)){
					isValid = 1;
					break;
				}
			}
			if (isValid){
				if(!strcmp(countDir,"")){
					printCount("",countDir,"ROOT");
				}
				for(int i=0;i<=dirPointer;i++){
					strcpy(link, dirInfo[i].path);
				    strcat(link, "/");
				    strcat(link, dirInfo[i].fileName);
					if (isSubDir(link, countDir)){
						printCount(link,countDir,dirInfo[i].fileName);
				    }
				}
			}
			else{
				char* red = "\033[31;1m";
				myPrint(red, strlen(red));
				myPrint("Error:", 6);
				char error_msg[120];
				strcpy(error_msg, countDir);
				strcat(error_msg, " is not a valid dir!\n");
				myPrint(error_msg, strlen(error_msg));
				char* white = "\033[37;0m";
			    myPrint(white, strlen(white));
			}
		}
		else{
			char* red = "\033[31;1m";
			myPrint(red, strlen(red));
			myPrint("Error:", 6);
			char error_msg[120];
			strcpy(error_msg, input);
			strcat(error_msg, " is not a valid command!\n");
			myPrint(error_msg, strlen(error_msg));
			char* white = "\033[37;0m";
			myPrint(white, strlen(white));
		}
	}

}

void print_ls(char* path){
	char* blue = "\033[34;1m";
	char* white = "\033[37;0m";
	char* green = "\033[36;1m";
	myPrint(green, strlen(green));
	myPrint(path, strlen(path));
	myPrint("/:", 2);
	myPrint("\n", 1);
	myPrint(blue, strlen(blue));
	for (int i = 0; i <= dirPointer; i++){
		if (!strcmp(dirInfo[i].path, path)){
			myPrint(dirInfo[i].fileName, strlen(dirInfo[i].fileName));
			myPrint(" ", 1);
		}
	}
	myPrint(white, strlen(white));
	for (int i = 0; i <= filePointer; i++){
		if (!strcmp(fileInfo[i].path, path)){
			myPrint(fileInfo[i].fileName, strlen(fileInfo[i].fileName));
			myPrint(" ", 1);
		}
	}
	myPrint("\n", 1);
}

int isSubDir(char* sub, char* super){
	for (int i = 0; i<strlen(super); i++){
		if (sub[i] != super[i])
			return FALSE;
	}
	return TRUE;
}

int getFATValue(int index){
	int fatIndex = FAT_OFFSET + index * 3 / 2;
	fseek(file, fatIndex, SEEK_SET);
	short fatValue = 0;
	fread(&fatValue, 2, 1, file);
	if (fatValue % 2){
		fatValue = (unsigned short)(fatValue & 0x0FFF);
	}
	else{
		fatValue = fatValue >> 4;
	}
	return fatValue;
}

void itoa(char* buffer, int num){
	if (num == 0){
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}
	int count = 0;
	int temp = num;
	while (temp > 0){
		temp /= 10;
		count++;
	}
	temp = num;
	buffer[count] = '\0';
	for (int i = 0; i < count; ++i) {
		int bit = temp % 10;
		temp /= 10;
		buffer[count - 1 - i] = (char)(bit + 48);
	}
}

void printCount(char* dir, char* origin, char* name){
	int dirCount = 0;
	int fileCount = 0;
	for (int i = 0; i <= dirPointer; i++){
		if (isSubDir(dirInfo[i].path, dir))
			dirCount++;
	}
	for (int i = 0; i <= filePointer; i++){
		if (isSubDir(fileInfo[i].path, dir))
			fileCount++;
	}
	char fileNumStr[10];
	char dirNumStr[10];
	itoa(fileNumStr, fileCount);
	itoa(dirNumStr, dirCount);
	int depth=countDepth(dir,origin);
	char output[120] = "";
	for(int i=0;i<depth;i++){
		strcat(output, "  ");
	}
	strcat(output, name);
	strcat(output, ": ");
	strcat(output, fileNumStr);
	strcat(output, " file(s), ");
	strcat(output, dirNumStr);
	strcat(output, " dir(s)\n");
	myPrint(output, strlen(output));
}

int isFileNameValid(struct Entry* entryPtr){
	int i;
	for (i = 0; i < 11; ++i){
		char b = entryPtr->fileName[i];
		if (!(b == 32 || (b >= 48 && b <= 57) || (b >= 65 && b <= 90) || (b >= 97 && b <= 122))){
			return FALSE;
		}
	}
	return TRUE;
}

void toUpper(char* input){
    for (int i = 0; i < strlen(input); ++i) {
        if(input[i] >= 'a' && input[i] <= 'z'){
            input[i] += 'A' - 'a';
        }
    }
}

int countDepth(char* path,char* origin){
	int count=0;
	for(int i=0;i<strlen(path);i++){
		if(path[i]=='/'){
			count++;
		}
	}
	for(int i=0;i<strlen(origin);i++){
		if(path[i]=='/'){
			count--;
		}
	}
	return count;
}