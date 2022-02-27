/*
 * dsh.h
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 */

#define MAXBUF 256
#define HISTORY_LEN 100

// TODO: Your function prototypes below

void dshMain();
void motdFind();
int findPath2(char input[]);
int checkBuilts(char input[]);
char* pathExp (char input[],char newpath[]);
