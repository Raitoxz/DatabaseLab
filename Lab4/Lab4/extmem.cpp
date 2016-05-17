/*
* extmem.c
* Zhaonian Zou
* Harbin Institute of Technology
* Jun 22, 2011
*/

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "extmem.h"

//��ʼ�������������������bufSizeΪ��������С����λ���ֽڣ���
//blkSizeΪ��Ĵ�С����λ���ֽڣ���bufΪָ�����ʼ���Ļ�������ָ�롣
//����������ʼ���ɹ�����ú�������ָ��û������ĵ�ַ�����򣬷���NULL��
Buffer *initBuffer(size_t bufSize, size_t blkSize, Buffer *buf)
{
	int i;

	buf->numIO = 0;
	buf->bufSize = bufSize;
	buf->blkSize = blkSize;
	buf->numAllBlk = bufSize / (blkSize + 1);
	buf->numFreeBlk = buf->numAllBlk;
	buf->data = (unsigned char*)malloc(bufSize * sizeof(unsigned char));

	if (!buf->data)
	{
		perror("Buffer Initialization Failed!\n");
		return NULL;
	}

	memset(buf->data, 0, bufSize * sizeof(unsigned char));
	return buf;
}

//�ͷŻ�����bufռ�õ��ڴ�ռ䡣
void freeBuffer(Buffer *buf)
{
	free(buf->data);
}

//�ڻ�����buf������һ���µĿ顣������ɹ����򷵻ظÿ����ʼ��ַ�����򣬷���NULL��
unsigned char *getNewBlockInBuffer(Buffer *buf)
{
	unsigned char *blkPtr;

	if (buf->numFreeBlk == 0)
	{
		perror("Buffer is full!\n");
		return NULL;
	}

	blkPtr = buf->data;

	//buf->blkSize + 1����Ϊÿ���鶼��һ����־λ��BLOCK_AVAILABLE����ã�BLOCK_UNAVAILABLE������
	while (blkPtr < buf->data + (buf->blkSize + 1) * buf->numAllBlk)
	{
		if (*blkPtr == BLOCK_AVAILABLE)//BLOCK_AVAILABLE = 0
			break;
		else
			blkPtr += buf->blkSize + 1;
	}

	*blkPtr = BLOCK_UNAVAILABLE;//BLOCK_UNAVAILABLE = 1����ʾ���block�Ѿ���ʹ����
	buf->numFreeBlk--;
	return blkPtr + 1;//���ص��ǻ���������ʼλ��������־λ
}

//�����blk�Ի������ڴ��ռ�ã�����blkռ�ݵ��ڴ�������Ϊ���á�
void freeBlockInBuffer(unsigned char *blk, Buffer *buf)
{
	*(blk - 1) = BLOCK_AVAILABLE;
	buf->numFreeBlk++;
}

//�Ӵ�����ɾ����ַΪaddr�Ĵ��̿��ڵ����ݡ���ɾ���ɹ����򷵻�0�����򣬷���-1��
int dropBlockOnDisk(unsigned int addr)
{
	char filename[40];

	sprintf(filename, "%d.blk", addr);
	//remove()��������ɾ��ָ�����ļ�����ԭ�����£�
    //int remove(char * filename);
	//�ɹ��򷵻�0��ʧ���򷵻�-1������ԭ�����errno��
	if (remove(filename) == -1)
	{
		perror("Dropping Block Fails!\n");
		return -1;
	}

	return 0;
}

//�������ϵ�ַΪaddr�Ĵ��̿���뻺����buf��
//����ȡ�ɹ����򷵻ػ������ڸÿ�ĵ�ַ��������buf��I/O������1����
//���򣬷���NULL��
unsigned char *readBlockFromDisk(unsigned int addr, Buffer *buf)
{
	char filename[40];
	unsigned char *blkPtr, *bytePtr;
	char ch;

	if (buf->numFreeBlk == 0)
	{
		perror("Buffer Overflows!\n");
		return NULL;
	}

	blkPtr = buf->data;

	while (blkPtr < buf->data + (buf->blkSize + 1) * buf->numAllBlk)
	{
		if (*blkPtr == BLOCK_AVAILABLE)
			break;
		else
			blkPtr += buf->blkSize + 1;
	}

	sprintf(filename, "%d.blk", addr);
	FILE *fp = fopen(filename, "r");

	if (!fp)
	{
		perror("Reading Block Failed!\n");
		return NULL;
	}

	//���ɹ�����ȷ�����������
	*blkPtr = BLOCK_UNAVAILABLE;
	blkPtr++;//�ƶ��������������
	bytePtr = blkPtr;

	while (bytePtr < blkPtr + buf->blkSize)
	{
		ch = fgetc(fp);
		*bytePtr = ch;
		bytePtr++;
	}

	fclose(fp);
	buf->numFreeBlk--;
	buf->numIO++;
	return blkPtr;//���ػ����������
}

//��������buf�ڵĿ�blkд������ϵ�ַΪaddr�Ĵ��̿顣
//��д��ɹ����򷵻�0��
//���򣬷���-1��ͬʱ��������buf��I/O������1��
int writeBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf)
{
	char filename[40];
	unsigned char *bytePtr;

	sprintf(filename, "%d.blk", addr);
	FILE *fp = fopen(filename, "w");

	if (!fp)
	{
		perror("Writing Block Failed!\n");
		return -1;
	}

	for (bytePtr = blkPtr; bytePtr < blkPtr + buf->blkSize; bytePtr++)
		fputc((int)(*bytePtr), fp);

	fclose(fp);
	*(blkPtr - 1) = BLOCK_AVAILABLE;
	buf->numFreeBlk++;
	buf->numIO++;
	return 0;
}
