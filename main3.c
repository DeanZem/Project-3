#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PAGE_SIZE 256
#define TLB_SIZE 16
#define PAGE_COUNT 256
#define FRAME_COUNT 128 /* Set to 128 for main3, but change for report data */
#define PHYSICAL_MEMORY (FRAME_COUNT * PAGE_SIZE)

typedef struct {
int page_num;
int frame_num;
} table;

table tlb[TLB_SIZE];
signed char phys_mem[PHYSICAL_MEMORY];
int page_tlb[PAGE_COUNT];
int last_used[FRAME_COUNT]; /* Tracks "time" for each frame */
int timer = 0; /* Global counter to simulate time */
int tlb_fifo = 0;
int total = 0;
int hits = 0;
int faults = 0;

int main(int argc, char** argv) {

if (argc != 2) {
printf("Usage: ./main3 addresses.txt\n");
return 1;
}

FILE* fp = fopen(argv[1], "rt");
FILE* bs = fopen("BACKING_STORE.bin", "rb");
FILE* out1 = fopen("out1.txt", "w");
FILE* out2 = fopen("out2.txt", "w");
FILE* out3 = fopen("out3.txt", "w");

if (fp == NULL || bs == NULL) return 1;

for (int i = 0; i < TLB_SIZE; ++i) {
tlb[i].page_num = -1;
tlb[i].frame_num = -1;
}
for (int i = 0; i < PAGE_COUNT; i++) {
page_tlb[i] = -1;
}
for (int i = 0; i < FRAME_COUNT; i++) {
last_used[i] = -1;
}

int address;
while (fscanf(fp, "%d", &address) != EOF) {
total++;
timer++;

address = address & 0xFFFF;
int page_num = (address >> 8) & 0xFF;
int offset = address & 0xFF;
int frame_num = -1;

/* 1. Check TLB */
for (int i = 0; i < TLB_SIZE; ++i) {
if (tlb[i].page_num == page_num) {
frame_num = tlb[i].frame_num;
hits++;
last_used[frame_num] = timer;
break;
}
}

/* 2. Check Page Table */
if (frame_num == -1) {
if (page_tlb[page_num] != -1) {
frame_num = page_tlb[page_num];
last_used[frame_num] = timer;
} else {

/* 3. Page Fault */
faults++;

int victim_frame = -1;
int min_time = 2147483647;

/* find empty frame first */
for (int i = 0; i < FRAME_COUNT; i++) {
if (last_used[i] == -1) {
victim_frame = i;
break;
}
}

/* if no empty frame, apply LRU */
if (victim_frame == -1) {
for (int i = 0; i < FRAME_COUNT; i++) {
if (last_used[i] < min_time) {
min_time = last_used[i];
victim_frame = i;
}
}
}

/* invalidate old page that was using this frame */
for (int i = 0; i < PAGE_COUNT; i++) {
if (page_tlb[i] == victim_frame) {
page_tlb[i] = -1;
break;
}
}

frame_num = victim_frame;

fseek(bs, page_num * PAGE_SIZE, SEEK_SET);
fread(&phys_mem[frame_num * PAGE_SIZE], sizeof(signed char), PAGE_SIZE, bs);

page_tlb[page_num] = frame_num;
last_used[frame_num] = timer;
}

/* update TLB using FIFO */
tlb[tlb_fifo].page_num = page_num;
tlb[tlb_fifo].frame_num = frame_num;
tlb_fifo = (tlb_fifo + 1) % TLB_SIZE;
}

int phys_add = (frame_num * PAGE_SIZE) + offset;
signed char value = phys_mem[phys_add];

fprintf(out1, "%d\n", address);
fprintf(out2, "%d\n", phys_add);
fprintf(out3, "%d\n", (int)value);
}

printf("Page-fault rate: %.4f\n", (float)faults / total);
printf("TLB hit rate: %.4f\n", (float)hits / total);

fclose(fp); fclose(bs);
fclose(out1); fclose(out2); fclose(out3);

return 0;
}
