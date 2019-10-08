#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#ifndef EXT2_FS_H
#define EXT2_FS_H
#include "ext2_fs.h"
#endif

#define SUPERBLOCK_OFFSET 1024
int imageFd = -1;
int num_groups = 1;
__u32 block_size; //set by superblock
__u32 inode_size; //set by superblock
__u32 inodes_per_group;	//set by superblock
__u32 blocks_per_group; //set by superblock
__u32 firstInodeBlock; //set by group
__u32 freeBlockBitmap; //set by group
__u32 freeInodeBitmap; //set by group
struct ext2_super_block superBlock;
struct ext2_inode inode;
struct ext2_group_desc group;
struct ext2_dir_noname_entry{
	__u32 inode;
	__u16 rec_len;
	__u8 name_len;
	__u8 file_type;
};
struct ext2_dir_noname_entry dir;


void report_error_and_exit(const char* msg){
	fprintf(stderr,"%s: %s\n", msg, strerror(errno));
	exit(2);
}

void print_superblock(){
	if(pread(imageFd, &superBlock, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET) < 0){
		report_error_and_exit("Pread failed!");
	}
	block_size = EXT2_MIN_BLOCK_SIZE << superBlock.s_log_block_size;
	inodes_per_group = superBlock.s_inodes_per_group;
	blocks_per_group = superBlock.s_blocks_per_group;
	inode_size = superBlock.s_inode_size;
	printf("%s,%u,%u,%u,%u,%u,%u,%u\n","SUPERBLOCK",
		superBlock.s_blocks_count,
		superBlock.s_inodes_count,
		block_size,
		inode_size,
		blocks_per_group,
		inodes_per_group,
		superBlock.s_first_ino);
}


void check_free_block(__u32 blockBitmapPost){
	__u32 blockBitmap_offset = blockBitmapPost * (int) block_size;
	//Number of meaningful bytes in bitmap 
	//One byte stores 8 blocks' state
	int num_Meaningful_Bytes = blocks_per_group/8;
	//Remainder of meaningful bits in bitmap's last byte
	int remainderBits = blocks_per_group%8;
	int upperlimit;

	if(remainderBits == 0)
		upperlimit = num_Meaningful_Bytes;
	else
		upperlimit = num_Meaningful_Bytes-1;

	//Read all meaningful bytes. Starting from 0 byte
	int byte;
	for(byte=0;byte<upperlimit;byte++){
		__u8 buff;		
		if(pread(imageFd,&buff,sizeof(__u8),blockBitmap_offset+byte) < 0)
			report_error_and_exit("Pread failed!");
		//Each bits in a byte represent 1 block
		//So, check each bit to see if block is allocated
		int bit;
		for(bit=0;bit<8;bit++){
			int checkFree = ((buff >> bit) & 1);
			int blockNum = byte*8 + bit;
			if(checkFree == 0)
				printf("BFREE,%d\n", blockNum+1);
		}
	}
	if(remainderBits){	
		__u8 buff;		
		//Read last byte	
		if(pread(imageFd,&buff,sizeof(__u8),blockBitmap_offset + num_Meaningful_Bytes) < 0)
			report_error_and_exit("Pread failed!");
		//Check each bit up to the last meaningful bit				
		int bit;
		for(bit=0;bit< remainderBits-1;bit++){
			int checkFree = ((buff >> bit) & 1);
			int blockNum = num_Meaningful_Bytes*8 + bit;
			if(checkFree == 0)
				printf("BFREE,%d\n", blockNum+1);
		}
	}			
}

void convert_to_GMT(char* buff, __u32 time){
	time_t raw_time = time;
	struct tm* time_gmt = gmtime(&raw_time);
	if (time_gmt == NULL)
		report_error_and_exit("Error converting time to GMT");
	 strftime(buff, 32, "%m/%d/%y %H:%M:%S", time_gmt);
}


__u32 logical_offset;
void print_directory_entries(__u32 BASE_BLOCK_OFFSET, int parent_inode){
	__u32 block_offset = 0;
	//Loop to find all directories entry in the block
	while(block_offset < block_size){
		__u32 DIR_OFFSET = BASE_BLOCK_OFFSET + block_offset;
		if(pread(imageFd, &dir, sizeof(struct ext2_dir_noname_entry), DIR_OFFSET) < 0){
			report_error_and_exit("Pread Failed!");
		}
		__u32 NAME_OFFSET = DIR_OFFSET + sizeof(struct ext2_dir_noname_entry);
		char name[dir.name_len +1];	
		if(pread(imageFd, name, dir.name_len, NAME_OFFSET) < 0){
			report_error_and_exit("Pread Failed!");
		}
		name[dir.name_len] = 0;
		if(dir.inode != 0){
			printf("%s,%u,%d,%u,%u,%u,'%s'\n","DIRENT",
			parent_inode,
			logical_offset,
			dir.inode,
			dir.rec_len,
			dir.name_len,
			name);
			logical_offset += dir.rec_len;
		}	
		//Displace to the next directory entry
		block_offset += dir.rec_len;	
	}
}


void print_indirect(__u32 block_num, int parent_inode, int offset, int level, char fileType){
	__u32 total_pointers = block_size/4;
	__u32 pointers[total_pointers];
	//Read the datablock
	if(pread(imageFd, pointers, block_size, block_size*block_num) < 0)
	       report_error_and_exit("Pread Fail!");
	__u32 i;
	for(i=0; i<total_pointers;i++){
		if(pointers[i] != 0){
			printf("%s,%d,%d,%d,%u,%u\n", "INDIRECT",
					parent_inode,
					level,
					offset,
					block_num,
					pointers[i]);	
			//This is a higher level, recurse
			if(level == 2 || level ==3)
				print_indirect(pointers[i], parent_inode, offset, level-1, fileType);

			if(level==1 && fileType == 'd')
				print_directory_entries(pointers[i]*block_size, parent_inode);

		}
		
		//Each entry only 1 data block
		if(level==1)
			offset++;
		//Each entry in level 2, go thru level 1 indirect so +=256
		else if(level==2)
			offset+=256;
		//Each entry in level 3 go thru level 2 infirect so += 256*256
		else if(level==3)
			offset+=256*256;
	}
}

void print_directory(struct ext2_inode* inode, int parent_inode, char fileType){
	__u32 i;
	logical_offset=0;
	//Loop thru all 12 blocks
	for(i=0; i <EXT2_NDIR_BLOCKS; i++){
		if(inode->i_block[i]!=0){
			print_directory_entries(inode->i_block[i]*block_size, parent_inode);
		}
	}
	
	//For single-indirect,Logical offsets starts at 12 (0-11 used alr above)
	if(inode->i_block[EXT2_IND_BLOCK] != 0)
		print_indirect(inode->i_block[EXT2_IND_BLOCK], parent_inode, 12, 1, fileType );
	//For double, offsets starts at 12 + 256 (since single filled alr) = 268 		
	if(inode->i_block[EXT2_DIND_BLOCK] != 0)
		print_indirect(inode->i_block[EXT2_DIND_BLOCK], parent_inode, 268, 2, fileType);
	//For tirple, starts at 12 + 256 + 256^2 (doubel filled) = 65804
	if(inode->i_block[EXT2_TIND_BLOCK] != 0)
		print_indirect(inode->i_block[EXT2_TIND_BLOCK], parent_inode, 65804, 3, fileType);
}	

void print_inode(__u32 firstBlock, __u32 inode_num){
	char fileType;
	__u32 INODE_OFFSET = firstBlock * block_size + inode_num * sizeof(struct ext2_inode);
	if (pread(imageFd, &inode, inode_size, INODE_OFFSET) <0)
		report_error_and_exit("Pread fails!");
	if(inode.i_mode == 0 && inode.i_links_count == 0)
		return;
	__u16 mask = 0xF000;
	__u16 format = inode.i_mode & mask;
	if(format == 0x8000)
		fileType = 'f';
	else if(format == 0x4000)
		fileType = 'd';
	else if(format == 0xA000)
		fileType = 's';
	else
		fileType = '?';
	__u16 mode = inode.i_mode & 0xFFF; //To get lowest 12 bits
	char ctime_str[32];
 	char mtime_str[32];
	char atime_str[32];
	convert_to_GMT(ctime_str, inode.i_ctime);
	convert_to_GMT(mtime_str, inode.i_mtime);
	convert_to_GMT(atime_str, inode.i_atime);
	printf("%s,%d,%c,%o,%u,%u,%u,%s,%s,%s,%d,%d", "INODE",
			inode_num + 1,
			fileType,
			mode,
			inode.i_uid,
			inode.i_gid,
			inode.i_links_count,
			ctime_str,
			mtime_str,
			atime_str,
			inode.i_size,
			inode.i_blocks);
	if(!(fileType == 's' && inode.i_size < 60)){
		int j;
		for(j=0 ;j<15; j++){
			printf(",%u", inode.i_block[j]);
		}
	}else
		printf(",%u", inode.i_block[0]);
	printf("\n");

	if(fileType == 'd')
		print_directory(&inode, inode_num + 1, fileType);

	if(!(fileType == 's' && inode.i_size < 60) && !(fileType == 'd')){
		//For single-indirect,Logical offsets starts at 12 (0-11 used alr above)
		if(inode.i_block[EXT2_IND_BLOCK] != 0)
			print_indirect(inode.i_block[EXT2_IND_BLOCK], inode_num + 1, 12, 1, fileType);
		//For double, offsets starts at 12 + 256 (since single filled alr) = 268 		
		if(inode.i_block[EXT2_DIND_BLOCK] != 0)
			print_indirect(inode.i_block[EXT2_DIND_BLOCK], inode_num + 1, 268, 2, fileType);
		//For tirple, starts at 12 + 256 + 256^2 (doubel filled) = 65804
		if(inode.i_block[EXT2_TIND_BLOCK] != 0)
			print_indirect(inode.i_block[EXT2_TIND_BLOCK], inode_num + 1, 65804, 3, fileType);
	}
}


void check_inode_and_print(__u32 inodeBitmapPost){	
	__u32 inodeBitmap_offset = inodeBitmapPost * (int) block_size;
	//Number of meaningful bytes in bitmap
	//One byte stores 8 inode's state
	int num_Meaningful_Bytes = inodes_per_group/8;

	int inode_num = 0;
	//Read all meaningful bytes. Starting from 0 byte
	int byte;
	for(byte=0;byte<num_Meaningful_Bytes;byte++){
		__u8 buff;		
		if(pread(imageFd,&buff,sizeof(__u8),inodeBitmap_offset+byte) < 0)
			report_error_and_exit("Pread failed!");
		//Each bits in a byte represent 1 block
		//So, check each bit to see if block is allocated
		int bit;
		for(bit=0;bit<8;bit++){
			int checkFree = ((buff >> bit) & 1);
			int blockNum = byte*8 + bit;
			if(checkFree == 0){
				printf("IFREE,%d\n", blockNum+1);
				inode_num++;
				continue;
			}
			//If allocated, print the inode block of that offset 
			print_inode(firstInodeBlock, inode_num);
			inode_num++;
		}
	}
}


void print_group(){
	int GROUP_SUPERBLOCK_OFFSET = sizeof(struct ext2_super_block);
	int group_offset = SUPERBLOCK_OFFSET + GROUP_SUPERBLOCK_OFFSET;
	if(pread(imageFd, &group, sizeof(struct ext2_group_desc), group_offset) < 0){					report_error_and_exit("Pread failed!");
	}
	freeBlockBitmap = group.bg_block_bitmap;
	freeInodeBitmap = group.bg_inode_bitmap;
	firstInodeBlock = group.bg_inode_table;
	__u32 block_count;
	if(superBlock.s_blocks_count > blocks_per_group)
		block_count = blocks_per_group;
	else
		block_count = superBlock.s_blocks_count%blocks_per_group;

	printf("%s,0,%u,%u,%u,%u,%u,%u,%u\n","GROUP",
		block_count,
		inodes_per_group,
		group.bg_free_blocks_count,
		group.bg_free_inodes_count,
		freeBlockBitmap,
		freeInodeBitmap,
		firstInodeBlock);	
	check_free_block(freeBlockBitmap);
       	check_inode_and_print(freeInodeBitmap);	
}
	
		



int main(int argc, char* argv[]){
	if(argc!=2){
		fprintf(stderr, "Invalid argument. Usage: ./lab3a file \n");
		exit(1);
	}
	if((imageFd = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "Error opening image file.\n");
		exit(1);
	}
	print_superblock();
	print_group();

}
