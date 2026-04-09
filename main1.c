#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define PAGE_SIZE 256
#define TLB_SIZE 16
#define FRAME_SIZE 256
#define PHYSICAL_MEMORY (FRAME_SIZE * PAGE_SIZE)

typedef struct{
int page_num;
int frame_num;
} table;

table tlb[TLB_SIZE];
signed char phys_mem[PHYSICAL_MEMORY];
int page_tlb[FRAME_SIZE];
int fifo;
int next = 0;
int total = 0;
int hits = 0;
int faults = 0;


int main(int argc, char** argv){
	FILE* fp = fopen("addresses.txt", "rt");
	FILE* bs = fopen("BACKING_STORE.bin", "rb");
	if(fp == NULL){
	fprintf(stderr, "adress error\n");
	}if(bs == NULL){
	fprintf(stderr, "bs error\n");
	return 1;
	}
	for(int i = 0; i < TLB_SIZE; ++i){
tlb[i].page_num = -1;
tlb[i].frame_num = -1;
	}
	for(int i =0; i < FRAME_SIZE; i++){
	page_tlb[i] = -1;
	}
	int address;
	while (fscanf(fp, "%d", &address) != EOF){
	total++;
	int page_num = (address >> 8) & 0xFF;
	int offset = address & 0xFF;
	int frame_num = -1;

	for(int i = 0; i < TLB_SIZE; ++i){
	if(tlb[i].page_num == page_num){
	frame_num = tlb[i].frame_num;
	hits++;
	break;
	}
	}
	if(frame_num == -1){
if(page_tlb[page_num] != -1){
frame_num = page_tlb[page_num];
}else{
frame_num = next;
next++;
if(next> FRAME_SIZE)
{
next = 0;
}
faults++;

fseek(bs, page_num * PAGE_SIZE, SEEK_SET);
fread(&phys_mem[frame_num * PAGE_SIZE], sizeof(signed char), PAGE_SIZE, bs);
page_tlb[page_num] = frame_num;
}
tlb[fifo].page_num = page_num;
tlb[fifo].frame_num = frame_num;
fifo = (fifo +1 ) % TLB_SIZE;
}
int phys_add = (frame_num << 8) | offset;
signed char value = phys_mem[phys_add];
printf("logical: %5d Physical: %5d Value: %4d\n", address, phys_add, value);
}
printf("total: %d\n", total);
printf("hits: %d\n", hits);
printf("faults: %d\n", faults);
fclose(fp);
fclose(bs);
	return 0;
}

