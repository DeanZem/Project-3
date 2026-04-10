#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PAGE_SIZE 256
#define TLB_SIZE 16
#define PAGE_COUNT 256
#define FRAME_COUNT 256
#define PHYSICAL_MEMORY (FRAME_COUNT * PAGE_SIZE)

typedef struct {
	int page_num;
	int frame_num;
} table;

table tlb[TLB_SIZE];
signed char phys_mem[PHYSICAL_MEMORY];
int page_tlb[PAGE_COUNT];
int fifo = 0;
int next = 0;
int total = 0;
int hits = 0;
int faults = 0;

int main(int argc, char** argv) {

if (argc != 2) {
printf("Usage: ./main1 addresses.txt\n");
return 1;
}

	FILE* fp = fopen(argv[1], "rt");
	FILE* bs = fopen("BACKING_STORE.bin", "rb");
	FILE* out1 = fopen("out1.txt", "w");
	FILE* out2 = fopen("out2.txt", "w");
	FILE* out3 = fopen("out3.txt", "w");

if (fp == NULL || bs == NULL) {
return 1;
}

for (int i = 0; i < TLB_SIZE; ++i) {
tlb[i].page_num = -1;
tlb[i].frame_num = -1;
}
for (int i = 0; i < PAGE_COUNT; i++) {
page_tlb[i] = -1;
}

int address;
while (fscanf(fp, "%d", &address) != EOF) {
total++;

address = address & 0xFFFF; /* mask to 16 bits */

int page_num = (address >> 8) & 0xFF;
int offset = address & 0xFF;
int frame_num = -1;

/* check TLB */
for (int i = 0; i < TLB_SIZE; ++i) {
	if (tlb[i].page_num == page_num) {
frame_num = tlb[i].frame_num;
hits++;
break;
}
}

if (frame_num == -1) {
	if (page_tlb[page_num] != -1) {
frame_num = page_tlb[page_num];
} else {

/* page fault: load into next free frame */
frame_num = next;

faults++;
fseek(bs, page_num * PAGE_SIZE, SEEK_SET);
fread(&phys_mem[frame_num * PAGE_SIZE], sizeof(signed char), PAGE_SIZE, bs);
page_tlb[page_num] = frame_num;

next++;  /* no wraparound in main1 */
}

/* update TLB */
tlb[fifo].page_num = page_num;
tlb[fifo].frame_num = frame_num;
fifo = (fifo + 1) % TLB_SIZE;
}

int phys_add = (frame_num * PAGE_SIZE) + offset;
signed char value = phys_mem[phys_add];

	fprintf(out1, "%d\n", address);
	fprintf(out2, "%d\n", phys_add);
	fprintf(out3, "%d\n", (int)value);
}

printf("Page-fault rate: %.4f\n", (float)faults / total);
printf("TLB hit rate: %.4f\n", (float)hits / total);

fclose(fp);
fclose(bs);
fclose(out1);
fclose(out2);
fclose(out3);

return 0;
}
