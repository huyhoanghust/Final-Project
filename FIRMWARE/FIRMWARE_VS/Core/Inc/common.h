/*
 * common.h
 *
 *  Created on: Feb 8, 2023
 *      Author: dung
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_


#define DEBUG   1

void Log_Init(void);

void LOG(const char *TAG, char *data);
void logPC(const char *fmt, ...);

void ftoa(float n, char* res, int afterpoint);
int intToStr(int x, char str[], int d);

void ftoa0(float n, char* res, int afterpoint);
int intToStr0(int x, char str[], int d);

int random_number(int min_num, int max_num);

#endif /* INC_COMMON_H_ */
