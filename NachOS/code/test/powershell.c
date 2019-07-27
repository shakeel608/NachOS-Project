#include "syscall.h"

#define BUFFERLENGHT 50
#define STRINGLENGHT 10

int compString(char *str1, char *str2){
	for(int i = 0; i < STRINGLENGHT; i++){
		if(str1[i] != str2[i])
			return 0;
		if(str1[i] == '\0')
			return 1;
	}
	return 0;
}

int getPartialString(int index, char *input, char *output){

	int i = index;
	int j = 0;

	if(i < 0)
		return i;

	//get rid of initial whitespace
	while(input[i] == ' ' && i < BUFFERLENGHT)
		i++;
	for(j = 0; i < BUFFERLENGHT && j < STRINGLENGHT; i++, j++){
		if(input[i] == '\0'){
			output[j] = '\0';
			return -2;
		}
		if(input[i] == ' '){
			output[j] = '\0';
			return i;
		}
		output[j] = input[i];
	}
	return -1;
}


void split(char *input, char *cmd, char *arg1, char *arg2, char *arg3, char *arg4){
	
	int i = 0;
	
	i = getPartialString(i, input, cmd);
	i = getPartialString(i, input, arg1);
	i = getPartialString(i, input, arg2);
	i = getPartialString(i, input, arg3);
	i = getPartialString(i, input, arg4);

}

int main () {

	int newProc;
    char buffer[BUFFERLENGHT];
	char cmd[STRINGLENGHT];
	char arg1[STRINGLENGHT];
	char arg2[STRINGLENGHT];
	char arg3[STRINGLENGHT];
	char arg4[STRINGLENGHT];

	while (1){
		PutString(">> ");
		GetString(buffer, BUFFERLENGHT); 

		if (buffer[0] != '\0'){
			cmd[0] = '\0';
			arg1[0] = '\0';
			arg2[0] = '\0';
			arg3[0] = '\0';
			arg4[0] = '\0';
			split(buffer, cmd, arg1, arg2, arg3, arg4);
			if(compString(cmd, "rm")) rm(arg1);
			else if(compString(cmd, "ls")) ls();
			else if(compString(cmd, "mkdir")) mkdir(arg1);
			else if(compString(cmd, "cd")) cd(arg1);
			else if(compString(cmd, "cpunix")) cpunix(arg1, arg2);
			else if(compString(cmd, "filepr")) filepr(arg1);
			else if(compString(cmd, "DIR")) DIR();
			else{
				newProc = ForkExec(buffer);
				if(newProc<0){
					continue;
				}
				UserWaitPid(newProc);					
			}
		}
	}
}
