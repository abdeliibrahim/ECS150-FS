#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"


/* On packing a struct: 
https://stackoverflow.com/questions/4306186/structure-padding-and-packing */



struct __attribute__((packed)) Superblock {
	uint8_t sig[8];
	// 2 bytes would require a 16 bit int, 1 byte requires 8 bit int
	uint16_t totalBlocks;
	uint16_t rootBlockIndex;
	uint16_t dataBlockStart;
	uint16_t dataBlockCt;
	uint8_t fatBlocks;

	// 1 byte * 4079
	uint8_t padding[4079];


};

struct __attribute__((packed)) FAT {
	uint16_t *flatArray;

};

struct __attribute__((packed)) Entry {
	uint8_t filename[FS_FILENAME_LEN];
	uint32_t fileSize;
	uint16_t firstBlockIn;

	// 1 byte * 10
	uint8_t padding[10];

};
struct __attribute__((packed)) RootDir {
	struct Entry rootDir[FS_FILE_MAX_COUNT];
};


// global Superblock, Root Directory, and FAT
 struct Superblock superblock;
 struct RootDir rd;
 struct FAT fat;

/* TODO: Phase 1 */

int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
	
	// open disk, return -1 if open errors
	if (block_disk_open(diskname))
		return -1;
	
	/* read 0th block from the @disk to the superblock,
	return -1 if errors */
	if (block_read(0, &superblock))
		return -1;

	/* now that the first block is in the superblock,
	check if the signature is correct */
	if (strcmp("ECS150FS", superblock.sig) == -1) {
		fprintf(stderr, "Signature not accepted");
		return -1;
	}

	if (block_disk_count() != superblock.totalBlocks) {
		return -1;
	} 

	fat.flatArray = malloc(sizeof(uint16_t) * superblock.dataBlockCt);
	
	/* start at 1 since signature is 0th index */
	for(int i = 1; i <= superblock.fatBlocks; i++) {
		if(block_read(i, &fat.flatArray[i-1]))
			return -1;
		
	}
		uint16_t FAT_EOC = 0xFFFF;
		if (fat.flatArray[0] != FAT_EOC) {
		return -1;
	}

	if (block_read(superblock.rootBlockIndex, &rd)) {
		return -1;
	}


	return 0;
}

int fs_umount(void)
{

	/* write from superblock to disk. 
	here, we simulate saving the changes to our disk
	 */
	
	if (block_write(0, &superblock))
		

	for(int i = 1; 1<= superblock.fatBlocks; i++) {
		if(block_write(i, &fat.flatArray[i-1]))
			return -1;
	}
	
	if (block_write(superblock.rootBlockIndex, &rd))
		return -1;

		
	

	if (block_disk_close())
		return -1;
	return 0;
}


int fs_info(void)
{
	/* TODO: Phase 1 */

	int i = 0, fatFree = 0, rdFree =0;
	
	
	/* Calculating fat free blocks */
	for(i; i<superblock.dataBlockCt; i++) {
		if(fat.flatArray[i] == 0)
			fatFree++;
	}
	/* Calculating rdir free files. */
	for(i=0; i<FS_FILE_MAX_COUNT; i++) {
		/* "An empty entry is defined by the first character of
		the entry’s filename being equal to the NULL character." */
		if(rd.rootDir[i].filename[0] == NULL)
			rdFree++;
	}

	/* On format specifiers for (un)signed integers:
	https://utat-ss.readthedocs.io/en/master/c-programming/print-formatting.html */
	printf("FS Info:\n");
	printf("total_blk_count=%u\n",superblock.totalBlocks);
	printf("fat_blk_count=%u\n",superblock.fatBlocks);
	printf("rdir_blk=%u\n",superblock.rootBlockIndex);
	printf("data_blk=%u\n",superblock.dataBlockStart);
	printf("data_blk_count=%u\n",superblock.dataBlockCt);
	printf("fat_free_ratio=%d/%u\n", fatFree, superblock.dataBlockCt);
	printf("rdir_free_ratio=%d/%d\n", rdFree, FS_FILE_MAX_COUNT);
	


	return 0;

}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	/* Intuition:

	Init superblock blocks, datablockct, fatblocks(nodes)

	malloc all fat blocks

	malloc all data blocks
	
	*/
	// superblock = (struct Superblock*) malloc(FS_FILE_MAX_COUNT * superblock);
    // 	superblock.fatBlocks = malloc();

    // 	// amount of data block to zero?
   	// superblock.dataBlockCt = 0;


    	//error management
   	// how to check if the dist is mounted???
   	if(strlen(filename) > FS_FILENAME_LEN || filename == NULL) { return -1; }


    	// check for empty entry in the root directory
    	for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        	if(rd.rootDir[i].fileSize == 0) {
           	 //fill it out with proper information
        	}
    	}



    	// still needs some work
	return 0;
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
	return 0;
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
	return 0;
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
	return 0;
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
	return 0;
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
	return 0;
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
	return 0;
}

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	return 0;
}

int fs_read(int fd, void *buf, size_t count)
{
	//struct Superblock obj;
	//block_read(fd, sizeof(obj));
	//slide 15 project disc


	/* TODO: Phase 4 */
	return 0;
}

