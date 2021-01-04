/**************************************************************
* Class:  CSC-415-03 Fall 2020 
* Name: Naweeda (920462358), Rhetta (917065622), Raza (917648503), Amy (917683954) 
* Student ID:
* Project: File System Project
*
* File: bitMap.c
*
* Description: This file is our implementation of a bitmap, the purpose of this is to track used and free blocks and takes care of our free space bit vector
*
**************************************************************/
#include "bitMap.h"

void setBitMap(int A[], int b) 
{
	A[b/32] |= 1 << (b % 32);
}

void clearBitMap(int A[], int b) 
{
	A[b / 32] &= ~(1 << (b % 32));
}

int findBitMap(int A[], int b) 
{
	return (A[ b / 32] & (1 << (b % 32)) != 0);
}
