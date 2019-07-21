#pragma warning(disable: 4996)
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "msgs.h"

//#define ID_OFFSET 0x086441CC
//#define SIZE  0x00F81000

#define ID_OFFSET 0x084AEF34
#define START 0x08000000
#define SIZE  0x00F81000

static int offset;

/*
typedef struct
{
	int start;
	int size;
}RAM_INFO;
*/

typedef struct
{
	int YokaiID;
	int FaceID;
	char* YokaiName;
}YOKAI;

YOKAI YOKAI_INFO;

void printMSG(char index);
void SetRamOffset(FILE* RamBin);
int CheckRamBinary(int* CRB, FILE* RamBin);
int CheckIsYokai(int AddrYokaiID, FILE* RamBin);
int ReturnYokaiID(FILE* RamBin);
int MoveToAddressValue(int YokaiPointer, FILE* RamBin);
int ReturnYokaiNameLen(int NameAddress, FILE* RamBin);
char* YokaiNameStr(int NameAddress, FILE* RamBin);

int main(int argc, char** argv)
{
	int res;
	printMSG(7);
	FILE* fp;
	if ((fp = fopen(argv[1], "rb")) == 0)
	{
		printMSG(0);
		return 1;
	}
	//RAM_INFO* RMI;
	int RMI[2] = { 0 };

	int index = 0;
	int offset = ID_OFFSET;

	if (!(CheckRamBinary(RMI, fp)))
	{
		int i;
		for (i = 1; i <= 4; i++)
		{
			printMSG(i);
		}
		fclose(fp);
		return 0;
	}
	printf("IDを抽出しますか?[yes = 1, no = 0]\n");
	scanf_s("%d", &res);
	if (res)
	{
		printMSG(5);
		FILE* wp = fopen("yw.txt", "w");
		while (CheckIsYokai(offset, fp))
		{
			int RIP = ReturnInfPointer(offset, fp);
			int MTAV = MoveToAddressValue(RIP, fp);
			int StrAddress = MoveToAddressValue(MTAV, fp);
			YOKAI_INFO.YokaiName = YokaiNameStr(StrAddress, fp);
			printf("妖怪ID : %08X\n", YOKAI_INFO.YokaiID);
			printf("顔ID : %08X\n", YOKAI_INFO.FaceID);
			printf("名前 : %s\n", YOKAI_INFO.YokaiName);
			printMSG(6);
			fprintf(wp,"妖怪ID : %08X\n",YOKAI_INFO.YokaiID);
			fprintf(wp,"顔ID : %08X\n",YOKAI_INFO.FaceID);
			fprintf(wp,"名前 : %s\n",YOKAI_INFO.YokaiName);
			fprintf(wp,"------------------\n");
			//offset += 0x48;
			offset += 0x58;
		}
		fclose(wp);
		fclose(fp);
		return 0;
	}
	return 0;
}

void printMSG(char index)
{
	printf("%s", MSGS[index]);
}

void SetRamOffset(FILE* RamBin)
{
	fseek(RamBin, sizeof(int) * 5, SEEK_SET);
}

int CheckRamBinary(int* CRB, FILE* RamBin)
{
	/*
	** CRB->start = START;
	** CRB->size  = SIZE;
	*/

	fseek(RamBin, sizeof(int) * 2, SEEK_SET);
	fread(&(CRB[0]), sizeof(int), 1, RamBin);
	fseek(RamBin, sizeof(int) * 4, SEEK_SET);
	fread(&(CRB[1]), sizeof(int), 1, RamBin);
	rewind(RamBin);
	if ((CRB[0] == START) && (CRB[1] == SIZE))
	{
		offset = CRB[0] ^ 0xFFFFFEC;
		return 1;
	}
	return 0;
}

int CheckIsYokai(int AddrYokaiID, FILE* RamBin)
{
	char Zero;
	char FF;
	SetRamOffset(RamBin);
	//fseek(RamBin, (AddrYokaiID ^ START) - 0x21, SEEK_CUR);
	fseek(RamBin, (AddrYokaiID ^ START) -0x08, SEEK_CUR);
	fread(&Zero, sizeof(char), 1, RamBin);
	//fseek(RamBin, 0x0B, SEEK_CUR);
	fread(&FF, sizeof(char), 1, RamBin);
	//if ((Zero == 0) || (FF == -1) || (FF == 0))
	if ((Zero == 0) || (FF == 0x08EC7000))
		return 1;
	else
		return 0;
}

int ReturnYokaiID(FILE * RamBin)
{
	int YokaiID;
	fread(&YokaiID, sizeof(int), 1, RamBin);
	return YokaiID;
}

int ReturnInfPointer(int AddrYokaiID, FILE * RamBin)
{
	int res, ywid2, IDcount = 1, address = 0, Searchaddress[100] = { 0 };
	SetRamOffset(RamBin);
	fseek(RamBin, (AddrYokaiID ^ START), SEEK_CUR);
	YOKAI_INFO.YokaiID = ReturnYokaiID(RamBin);
	fread(&ywid2, sizeof(int), 1, RamBin);
	YOKAI_INFO.FaceID = ywid2;
	printf("\n|--{ %08X }--|\n|\t\t |\n", ywid2);
	SetRamOffset(RamBin);
	while (!(feof(RamBin)))
	{
		fread(&res, sizeof(int), 1, RamBin);
		if (res == ywid2)
		{
			printf("|  [%02d]%08X  |\n", IDcount, address | START);
			Searchaddress[IDcount - 1] = address;
			IDcount++;
		}
		address += sizeof(int);
	}
	printf("|\t\t |\n|--{ %08X }--|\n\n", ywid2);
	return (Searchaddress[0] + 4) ^ START;
}

int MoveToAddressValue(int YokaiPointer, FILE * RamBin)
{
	int InfoPointer;
	SetRamOffset(RamBin);
	fseek(RamBin, (YokaiPointer ^ START), SEEK_CUR);
	fread(&InfoPointer, sizeof(int), 1, RamBin);
	return InfoPointer;
}

int ReturnYokaiNameLen(int NameAddress, FILE * RamBin)
{
	char strCounter = 0, scl = 1;
	SetRamOffset(RamBin);
	fseek(RamBin, (NameAddress ^ START), SEEK_CUR);
	while (scl != '\0')
	{
		fread(&scl, sizeof(char), 1, RamBin);
		strCounter++;
	}
	return strCounter;
}

char* YokaiNameStr(int NameAddress, FILE * RamBin)
{

	char i, NameLen = ReturnYokaiNameLen(NameAddress, RamBin);
	char* YName = (char*)malloc(sizeof(char) * NameLen);
	SetRamOffset(RamBin);
	fseek(RamBin, (NameAddress ^ START), SEEK_CUR);
	for (i = 0; i < NameLen; i++)
	{
		fread(&YName[i], sizeof(char), 1, RamBin);
	}
	return YName;
}