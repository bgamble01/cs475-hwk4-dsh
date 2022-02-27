/*
 * dsh.c
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include "builtins.h"

/**
 *Putting it all together
 */
void dshMain(){
    motdFind();
    char* hist[1000];
    char* curr;
    int pos =0;
    char cmnd[MAXBUF];
   
     int check = 0;
    while (check != 1){
        printf("\n\ndsh>");
        fgets(cmnd, MAXBUF-1,stdin);
        if(cmnd[0]==' '){
            printf("Please try again without leading whitespace\n");
        }
        else{
        curr=strdup(cmnd);
        hist[pos]=curr;
        pos++;
        check = findPath2(cmnd);
            if(check ==2){
                int j=pos-1;
                int count =HISTORY_LEN;
                while(j>=0 && count!=0){
                    printf("%s",hist[j]);
                    j--;
                    count--;
                }
            }
        }

        

    } 
}

/**
 * Checks MOTD
 */
void motdFind(){
    char path[MAXBUF];
    strcat(path,getenv("HOME"));
    strcat(path,"/.dsh_motd");
    if (access(path, F_OK | X_OK) == 0) {
        // file exists and is executable
        FILE *motd;
        char c;
        motd = fopen(".dsh_motd","r");
        while((c=fgetc(motd))!=EOF) {
            printf("%c",c);
        }
         fclose(motd);
         printf("\n");

    }
        else {
        // file doesn't exist or is not executable
        }


}

/**
 * main functionality of dsh resides in this function
 * @param input String of user input to evaluate
 * @return numbers corresponding to current state of dsh
 */
int findPath2(char input[]){
    char cmnd[MAXBUF];
    strcpy(cmnd,input);
    
    char fpath[MAXBUF];
    char rest[MAXBUF];
    int i=0;
    while( !isspace(cmnd[i])){
        fpath[i]=cmnd[i];
        i++;
    }
    fpath[i]='\0';
    i++;
    int j=0;
    int len =strlen(cmnd);
    while(i<len){
        rest[j]=cmnd[i];
        i++;
        j++;
    }
    rest[j]='\0';
    rest[strcspn(rest,"\n")]=0;

    int compPath=0;
    if(cmnd[0]=='.' && cmnd[1]=='/'){
        char execName[MAXBUF];
        int m=0;
        while(fpath[m+1]!='\0'){
            execName[m]=fpath[m+1];
            m++;
        }
        execName[m]='\0';
        char dir[MAXBUF];
        getcwd(dir,MAXBUF);
        strcat(dir,execName);
        strcpy(fpath,dir);
        if (access(fpath, F_OK | X_OK) == 0){
            compPath=1;
        }
        else{
            printf("Error: Command not found");
            return 0;
        }
    }
    //This conditional checks if the user already input a complete path
    if(cmnd[0]=='/'){
        if (access(fpath, F_OK | X_OK) == 0){
            compPath=1;
        }
        else{
            printf("Error: Command not found");
            return 0;
        }
    }
    int check=0;

    if(compPath!=1){
        check = checkBuilts(fpath);
        if(check == 1){
            return 1;
        }
        if (check ==2){
            return 2;
        }
        if(check==4){
            return 0;
        }
        if(check!=3){
            char see[MAXBUF];
            char cpy[MAXBUF] = "";
            strcpy(see,pathExp(fpath,cpy));
            strcpy(fpath,see);
        }
    }

    if ((access(fpath, F_OK | X_OK) == 0)|| (check==3)) {
        // file exists and is executable
        char* tok;
        i=1;
        char* args[100];
        args[0]=fpath;
        tok = strtok(rest, " \n");
        while(tok !=NULL){
            args[i]=tok;
            tok=strtok(NULL, " ");
            i++;
        }
        args[i]= NULL;

        if (check==3){
             if(args[1]==NULL){
                chdir(getenv("HOME"));
            }
            else{
                char p[MAXBUF];
                getcwd(p,MAXBUF);
                strcat(p,"/");
                strcat(p,args[1]);
                char *point = p;
                chdir(point);
            }
        }
        else{

        int waiting =0;
        if(!strcmp(args[i-1],"&")){
            args[i-1]=NULL;
            waiting =1;
        }

        pid_t pid;
        pid =fork();

        if (pid==0){
            execv(fpath,args);
            return 0;
        }
        else{
            if(waiting==0){
                wait(NULL);
            }
        }
        }
    }
    else {
        // file doesn't exist or is not executable
        printf("Error: Command not found");
    } 

    return 0;
}

/**
 * Checks Builtins
 * @param input String of user input to evaluate
 * @return numbers corresponding to builtins
 */
int checkBuilts(char input[]){
    char cmnd[MAXBUF];
    strcpy(cmnd,input);
    cmd_t sim = chkBuiltin(cmnd);
    if(sim>0){
        if(sim==1){
            //cd
            return 3;
        }
        else if(sim==2){
            //pwd
            char dir[MAXBUF];
            getcwd(dir,MAXBUF);
            printf("%s",dir);
            return 4;
        }
        else if(sim==3){
            //history
            return 2;
        }
        else{
            return 1;
        }
        return 0;
    }
    else{
        return 0;
    }
}

/**
 * Cycles through possible paths
 * @param input String of user input to evaluate
 * @return newpath String to store new path
 */
char* pathExp (char input[],char newpath[]){
    char cmnd[MAXBUF];
    strcpy(cmnd,input);
    char path[MAXBUF];
    path[0]='\0';
    strcat(path,getenv("PATH"));
    char* con;
    char test[MAXBUF]="/";
    strcat(test,cmnd);
    char full[MAXBUF];
    full[0]='\0';
    
    con = strtok(path, ":");
    while(con !=NULL){
        strcat(full,con);
        strcat(full,test);
        if(access(full, F_OK | X_OK) == 0) {
            strcpy(newpath,full);
            full[0]='\0';
            return newpath;    
            }  
            con=strtok(NULL, ":");
            full[0]='\0';
    }
        path[0]='\0';
        newpath="X";
        return newpath;
}
