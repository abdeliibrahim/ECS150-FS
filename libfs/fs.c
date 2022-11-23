#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

int MOUNTED = -1;
int FILE_COUNT = 0;

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

struct __attribute__((packed)) RootDir {
	uint8_t filename[FS_FILENAME_LEN];
	uint32_t fileSize;
	uint16_t firstBlockIn;
	// 1 byte * 10
	uint8_t padding[10];
};

struct __attribute__((packed)) openFileContent {
    size_t offset;
    uint8_t filename[FS_FILENAME_LEN];
};

//create fd table
struct openFileContent fdir[FS_OPEN_MAX_COUNT];
// global Superblock, Root Directory, and FAT
struct Superblock superblock;
struct FAT fat;
struct RootDir rd[FS_FILE_MAX_COUNT];

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
		//fprintf(stderr, "Signature not accepted");
		return -1;
	}

	if (block_disk_count() != superblock.totalBlocks) {
		return -1;
	} 

	fat.flatArray = malloc(BLOCK_SIZE * superblock.fatBlocks * 2);
	
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
    MOUNTED = 0;
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

    MOUNTED = -1;
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
		if(rd[i].filename[0] == NULL)
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
    uint16_t FAT_EOC = 0xFFFF;

   	if(strlen(filename) >= FS_FILENAME_LEN || filename == NULL || MOUNTED == -1 || FILE_COUNT >= FS_FILE_MAX_COUNT) {
        return -1;
    }

    // check if the file exits and count number of existed files
    for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        // check if the file already exists. if exist return -1
        if(strcmp(filename, (char*)rd[i].filename) == 0) {
            return -1;
        }
    }

    for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        // check for empty entry in root directory.
		// https://www.tutorialandexample.com/null-character-in-c null characters
        if(rd[i].filename[0] == '\0') {
            rd[i].firstBlockIn = FAT_EOC;
            memcpy(rd[i].filename, filename, FS_FILENAME_LEN);
            rd[i].fileSize = 0;
            FILE_COUNT++;
            return 0;
        }
   	}
    return -1;
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */

    uint16_t FAT_EOC = 0xFFFF;
    uint16_t starting_data_index = 0xFFFF;

    if(filename == NULL || MOUNTED == -1) {
        return -1;
    }

    for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        if(strcmp((char*)rd[i].filename, filename) == 0) {
            // file’s entry must be emptied
            starting_data_index = rd[i].firstBlockIn;
            rd[i].filename[0] = '\0';
            rd[i].fileSize = 0;
            rd[i].firstBlockIn = FAT_EOC;
            FILE_COUNT--;
            // all the data blocks containing the file’s contents must be freed in the FAT??????
            for(int i = starting_data_index; i < sizeof(*fat.flatArray)/sizeof(fat.flatArray[0]) - 1; i = starting_data_index) {
                uint16_t next = fat.flatArray[i];
                if(fat.flatArray[i] != 0xFFFF) {
                    fat.flatArray[i] = 0;
                }

                if (fat.flatArray[i] == 0xFFFF) { break;}
                starting_data_index = next;
            }
            return 0;
        }
    }


    return -1;
}

int fs_ls(void)
{
    /* TODO: Phase 2 */
    if(MOUNTED == -1) {
        return -1;
    }

	printf("FS Ls:\n");
	for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        	if(rd[i].filename[0] != '\0') {
            		printf("file: %s, size: %d, data_blk: %d\n", rd[i].filename, rd[i].fileSize, rd[i].firstBlockIn);
        	}
    }
	return 0;
}

	
	
int fs_open(const char *filename)
{
	// check if file exists in root directory; we can use a similar loop from our create file
	int fExists = -1;
	for(int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		
        	if(!strcmp((char*)rd[i].filename, filename)) {
            		fExists = 0;
					//printf("EXISTS %d\n", fExists);
					break;
        	}
    	}
	// VALIDATION
	if (MOUNTED == -1 || strlen(filename) > FS_FILENAME_LEN || filename == NULL || fExists)
		return -1;

	for(int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (fdir[i].filename[0] == '\0') {
			fdir[i].offset = 0;
			// "man memcpy" command to understand how it works
			memcpy(fdir[i].filename, filename, FS_FILENAME_LEN);
		
			return i;
		}
			
	}

	return -1;
}

int fs_close(int fd)
{
	if (fdir[fd].filename[0] == '\0' || MOUNTED == -1 || fd > FS_OPEN_MAX_COUNT || fd < 0)
		return -1;
	fdir[fd].filename[0] = '\0';
	fdir[fd].offset = 0;

	
	/* TODO: Phase 3 */
	return 0;
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
    // Return -1 if no FS is currently mounted, or fd is out of bound, or it is not currently open
    if(MOUNTED == -1 || fd > FS_OPEN_MAX_COUNT || fd < 0 || fdir[fd].filename[0] == '\0') {
        return -1;
    }

    //first find the open file in the root directory, then, return the size of the open file
    for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        if(strcmp((char*)fdir[fd].filename, (char*)rd[i].filename) == 0) {
            return rd[i].fileSize;
        }
    }

    //if file not found, return -1
	return -1;
}

int fs_lseek(int fd, size_t offset)
{
	// to do: check if fd is valid
    if(MOUNTED == -1 || fd > FS_OPEN_MAX_COUNT || fd < 0 || fdir[fd].filename[0] == '\0') {
        return -1;
    }



    // set the offset
	fdir[fd].offset = offset;
	return 0;
}

// helper function to find first index of data block corresponding to file's offset
int dbFind(int fd, size_t offset) {
	int dbIndex;
	 for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        if(strcmp((char*)fdir[fd].filename, (char*)rd[i].filename) == 0){
            return rd[i].firstBlockIn + (offset/BLOCK_SIZE);
		}
	 }
	
}
int dbFind2(int fd, size_t offset) {
	int dbIndex;
	 for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        if(strcmp((char*)fdir[fd].filename, (char*)rd[i].filename) == 0){
            return i;
		}
	 }
	
}
int rootIn(fd) {
 for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        if(strcmp((char*)fdir[fd].filename, (char*)rd[i].filename) == 0){
            return i;
		}

}
}
int emptyFat() {
	uint16_t FAT_EOC = 0xFFFF;
	int i = 1;
	for(i; i<superblock.dataBlockCt; i++) {
		if(fat.flatArray[i] == 0) {
            fat.flatArray[i] = FAT_EOC;
            return i;
        }
	}
	return -1;
}




int fs_write(int fd, void *buf, size_t count)
{
    /* TODO: Phase 4 */
	/* what needs to be done :
		IDEAL CASE: to be written fits within a single datablock
		- start by reading datablock to bounce buffer
		- copy buffer to bounce+offset
		- then write bounce to datablock
	*/
    //Return: -1 if no FS is currently mounted, or if file descriptor @fd is
    //invalid (out of bounds or not currently open), or if @buf is NULL.
    if(MOUNTED == -1 || fd >= FS_OPEN_MAX_COUNT || fd < 0 || buf == NULL || fdir[fd].filename[0] == '\0') {
        return -1;
    }
	//int curFat = emptyFat(); // delete

    int rIn = rootIn(fd);
    if(fs_stat(fd) == 0) {
        int nFat = emptyFat();
        rd[rIn].firstBlockIn = nFat;
    }
	

    void *bounce = (void*)malloc(BLOCK_SIZE);
	int db = dbFind2(fd, fdir[fd].offset);
	// int db = emptyFat();
	int curFat = emptyFat();
	
	
	//fat.flatArray[db] = curFat;
	// if (block_read(db + superblock.dataBlockStart, bounce))
	// 	return -1;
	int tempDB = db + superblock.dataBlockStart;
	fat.flatArray[tempDB] = curFat;
	int bounceOffset = fdir[fd].offset % BLOCK_SIZE;

		

	// if(rd[rIn].fileSize == 0) {
	// 	int nFat = emptyFat();
	// 	fat.flatArray[tempDB] = nFat;
		
	// }
	int i = 0;
	size_t reset = 0;
	int nFat = emptyFat();
	while (i < count) {
		nFat = emptyFat();
		fat.flatArray[curFat] = nFat;
		
		int tempCur = curFat;
		curFat = nFat;
				while (reset <  BLOCK_SIZE) {
				
					memcpy(&bounce[bounceOffset], &buf[i], 1);
					
					fdir[fd].offset++;
					bounceOffset++;
					i++;
					reset++;
					
					rd[rIn].fileSize++;
					if (i >= count)
					
						break;
					
				}
		
		printf("%d\n", tempDB);
		if(rd[rIn].fileSize > 0) 
			rd[rIn].firstBlockIn = rIn;
		block_write((size_t)tempDB, bounce);
		reset = 0;
		bounceOffset = 0;
		//tempDB++;
		
//		 tempDB = fat.flatArray[tempCur] + superblock.dataBlockStart;
//		 fat.flatArray[nFat] = 0xFFFF;
//		 int nFat = emptyFat();
        int nextFatBlock = emptyFat();
        if(nextFatBlock != -1) {
            tempDB = nextFatBlock;
            block_read(tempDB+superblock.dataBlockStart, bounce);
        }
	}
	
	/*
	set fat[cur] to next available fat
	set tempcur to cur
	set cur to next fat

	fill block with data
	write block to disk
	update tempDB with fat[tempcur]
	
	*/

	


	// for(i; i<superblock.dataBlockCt; i++) {
	// 	if(fat.flatArray[i] == 0)
	// 	fat.flatArray[i] = 0xFFFF;
	// } // delete
    return i;
}


int fs_read(int fd, void *buf, size_t count)
{	
		/* 
	first, we need to read our data block into a bounced buffer. 

	assuming a file's offset is at value X, we need to also adjust the bounced
	buffer's offset so that we copy our bounce to buf with the correct offset.
	the offset would be = fileOffset % BLOCK_SIZE

	assuming that we read over the data block size, we need to continue reading from
	the next data block until we complete our count.
 
 	memcpy(void *restrict dst, const void *restrict src, size_t n);
     The memcpy() function copies n bytes from memory area src to memory area dst.  If dst and src overlap, behavior is undefined.
     Applications in which dst and src might overlap should use memmove(3) instead.
	 

	*/

	/*
	block size 16 bytes
	read 20 bytes
	file offset is at 50

	[16] [16] [16] [16]
	50 % 16


	
	*/
	if (MOUNTED == -1 || fd >= FS_OPEN_MAX_COUNT || fd < 0 || fdir[fd].filename[0] == '\0' || buf == NULL) {
		return -1;
	}
	
int rootIn;
	//int bytes = 0;
	for(int i=0; i < FS_FILE_MAX_COUNT; i++) {
        if(strcmp((char*)fdir[fd].filename, (char*)rd[i].filename) == 0){
            rootIn = rd[i].firstBlockIn;
		}
	 }
	//start by reading first datablock
	void *bounce = (void*)malloc(BLOCK_SIZE);
	size_t db = fdir[fd].offset / BLOCK_SIZE;
	block_read(dbFind(fd, fdir[fd].offset) + superblock.dataBlockStart, bounce);
	// if (block_read(db + 1 + superblock.dataBlockStart, bounce))
	// 	return -1;
	
	int tempDB = dbFind(fd, fdir[fd].offset) + superblock.dataBlockStart;
	//int tempDB = fdir[fd].offset/BLOCK_SIZE + 1 + superblock.dataBlockStart;
	int bounceOffset = fdir[fd].offset % BLOCK_SIZE;
	int i = 0;
	int bytes = 0;
	while (i < count) {
	if (i + BLOCK_SIZE-bounceOffset > fs_stat(fd)) {
		for (int j = i; i<count; j++) {
			memcpy(&buf[j], &bounce[bounceOffset], 1);
			bounceOffset++;
			bytes++;
			i++;
		}
		
		return bytes;
	}

	memcpy(&buf[i], &bounce[bounceOffset], BLOCK_SIZE-bounceOffset); // |          |           |           |
	//fdir[fd].offset += BLOCK_SIZE-bounceOffset;
	
	tempDB++;
	// db  = fat.flatArray[db];
	
	// tempDB = db + superblock.dataBlockStart;
	block_read((size_t)(tempDB), bounce);
	bounceOffset = 0;
	
	i+= BLOCK_SIZE-bounceOffset;
	bytes+= BLOCK_SIZE-bounceOffset;
	}

	
	return bytes;
}

