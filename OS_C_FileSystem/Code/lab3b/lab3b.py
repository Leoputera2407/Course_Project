
import sys,csv,math,numpy

super_block = None
group = None
bfree_list= []
ifree_list= []
inode_list= []
dirent_list= []
indirect_list= []
error=0

block_allocated = {} #Mapping block_num to {block_num,inode,offset} to check duplicates
block_duplicated = set() #Keep track of block that are duplicated.


class Superblock:
    def __init__(self, entry):
        self.total_block = int(entry[1])
        self.total_inode = int(entry[2])
        self.block_size = int(entry[3])
        self.inode_size = int(entry[4])
        self.blocks_per_group = int(entry[5])
        self.inodes_per_group = int(entry[6])
        self.first_non_reserved = int(entry[7])

class Group:
    def __init__(self,entry):
        self.group_number = int(entry[1])
        self.block_count = int(entry[2])
        self.inode_count = int(entry[3])
        self.free_block_count = int(entry[4])
        self.free_inode_count = int(entry[5])
        self.bitmap_block_num = int(entry[6])
        self.imap_block_num = int(entry[7])
        self.first_block = int(entry[8])

class Inode:
    def __init__(self,entry):
        self.inode_num = int(entry[1])
        self.file_type = entry[2]
        self.mode = entry[3]
        self.owner = int(entry[4])
        self.group = int(entry[5])
        self.link_count = int(entry[6])
        self.c_time = entry[7]
        self.m_time = entry[8]
        self.a_time = entry[9]
        self.file_size = int(entry[10])
        self.blocks_occupied = int(entry[11])
        self.direct_block = [int(block_num) for block_num in entry[12:24]]
        self.indirect = [int(block_num) for block_num in entry[24:27]]


class Dirent:
    def __init__(self,entry):
        self.parent_inode = int(entry[1])
        self.logical_offset = int(entry[2])
        self.inode_num = int(entry[3])
        self.rec_len = int(entry[4])
        self.name_len = int(entry[5])
        self.name = str(entry[6])

class Indirect:
    def __init__(self,entry):
        self.inode_num = int(entry[1])
        self.level = int(entry[2])
        self.offset = int(entry[3])
        self.curr_block_num = int(entry[4])
        self.ref_block_num = int(entry[5])

def check_csv(entry,line_num, num_expected_entry):
    if len(entry) != num_expected_entry:
        print('Entry {} at line number {} has {} entries when only {} is expected'.format(entry[0],line_num,len(entry), num_expected_entry))
        sys.exit(1)

def block_checker(FIRST_VALID_BLOCK, LAST_VALID_BLOCK, inode_num, block_num, logical_offset, level):
    global error
    indirect_level = {0: 'BLOCK', 1: 'INDIRECT BLOCK', 2: 'DOUBLE INDIRECT BLOCK', 3: 'TRIPLE INDIRECT BLOCK'}
    if block_num != 0:
        if block_num < 0 or block_num > LAST_VALID_BLOCK:
            print("INVALID %s %d IN INODE %d AT OFFSET %d" %(indirect_level[level], block_num, inode_num, logical_offset))
            error += 1
        elif block_num > 0  and block_num < FIRST_VALID_BLOCK:
            print("RESERVED %s %d IN INODE %d AT OFFSET %d" %(indirect_level[level],block_num, inode_num, logical_offset))
            error += 1
        elif block_num not in block_allocated:
            block_allocated[block_num] = [[block_num, inode_num, logical_offset,level]]
        else:
            block_duplicated.add(block_num)
            block_allocated[block_num].append([block_num, inode_num, logical_offset,level])

def check_duplicates():
    global error
    indirect_level = {0: 'BLOCK', 1: 'INDIRECT BLOCK', 2: 'DOUBLE INDIRECT BLOCK', 3: 'TRIPLE INDIRECT BLOCK'} 
    for block_num in block_duplicated:
        for entry in block_allocated[block_num]:
            block_desc = indirect_level[entry[-1]]
            print("DUPLICATE %s %d IN INODE %d AT OFFSET %d" %(block_desc, entry[0], entry[1], entry[2]))
            error +=1

def block_consistency_audit(super_block, group):
    global error
    # Ascertain where first data block is
    FIRST_VALID_BLOCK = int(group.first_block + math.ceil(super_block.inode_size*group.inode_count/super_block.block_size))
    LAST_VALID_BLOCK = super_block.total_block - 1
    indirect_offset = {1: 12, 2: 268, 3: 65804}
    #Check every inode allocated
    for inode in inode_list:
        logical_offset=0
        #Check directs
        for block_num in inode.direct_block:
            block_checker(FIRST_VALID_BLOCK, LAST_VALID_BLOCK, inode.inode_num,block_num, logical_offset, 0)
            logical_offset += 1
        #Check indirects
        for idx in range(len(inode.indirect)):
            logical_offset = indirect_offset[idx+1]
            block_checker(FIRST_VALID_BLOCK, LAST_VALID_BLOCK, inode.inode_num, inode.indirect[idx], logical_offset, idx+1)
    #Go through the indirects
    for indirect in indirect_list:
        logical_offset = indirect_offset[indirect.level]
        block_checker(FIRST_VALID_BLOCK,LAST_VALID_BLOCK, indirect.inode_num, indirect.ref_block_num,logical_offset, indirect.level)
    
    #Check unreferrence ie unfree, yet unallocated or allocation error ie free, yet allocated
    for block_num in range(FIRST_VALID_BLOCK,LAST_VALID_BLOCK):
        if block_num not in bfree_list and block_num not in block_allocated:
            print("UNREFERENCED BLOCK %d" %block_num)
            error += 1
        if block_num in bfree_list and block_num in block_allocated:
            print("ALLOCATED BLOCK %d ON FREELIST" %block_num)
            error += 1

    check_duplicates()

def inode_and_dir_audit(super_block,first_inode_pos, total_inode_count):
    global error
    allocated_inode = []
    #Create a list to keep track of each inodes' link count
    link_counter = numpy.zeros(super_block.first_non_reserved + super_block.total_inode) 
    #Create a map to keep of each inode's parent inode_num 
    parent = {2: 2}

    #Scan for all allocated_inode and check if they are allocated in the ifree_list
    for inode in inode_list:
        if inode.inode_num != 0:
            #If it's a valid type, inode is allocated and shouldn't appear in the ifree_list
            if inode.file_type != '0':
                allocated_inode.append(inode.inode_num)
                if inode.inode_num in ifree_list:
                    print("ALLOCATED INODE %d ON FREELIST" %(inode.inode_num))
                    error +=1 
            #Invalid type, it should be in the freelist        
            else:
                if inode.inode_num not in ifree_list:
                    print("UNALLOCATED INODE %d NOT ON FREELIST" %(inode.inode_num))
                    error +=1
            
    #Check for all inode that hasn't been allocated, but somehow isn't in the ifree too
    for inode_num in range(first_inode_pos, total_inode_count):
        if inode_num not in allocated_inode and inode_num not in ifree_list:
            print("UNALLOCATED INODE %d NOT ON FREELIST" %(inode_num))
            error +=1

    #Check if dirent is within the valid boundary, properly allocated and is consistent
    for dir in dirent_list:
        if dir.inode_num < 1 or dir.inode_num > super_block.total_inode:
            print("DIRECTORY INODE %d NAME %s INVALID INODE %d" %(dir.parent_inode, dir.name, dir.inode_num))
            error +=1
        elif dir.inode_num not in allocated_inode:
            print("DIRECTORY INODE %d NAME %s UNALLOCATED INODE %d" %(dir.parent_inode, dir.name, dir.inode_num))
            error +=1
        else:
            #If dirent is valid, increment link to that inode_num
            link_counter[dir.inode_num] += 1
            #Store the parent of that dirent
            if dir.name != "'.'" and dir.name != "'..'":
                parent[dir.inode_num] = dir.parent_inode

    #Check if each inode's link count matches what we just checked
    for inode in inode_list:
        if inode.link_count != link_counter[inode.inode_num]:
            print("INODE %d HAS %d LINKS BUT LINKCOUNT IS %d" %(inode.inode_num, link_counter[inode.inode_num], inode.link_count))
            error +=1 

    #Check '.' and  '..' consistency
    for dir in dirent_list:
        #If it's '.', it should refer to itself
        if dir.name == "'.'" and dir.inode_num != dir.parent_inode:
            print("DIRECTORY INODE %d NAME '.' LINK TO INODE %d SHOULD BE %d" %(dir.parent_inode, dir.inode_num, dir.parent_inode))
            error += 1
        #For '..', its parents is the parent_inode and it refers to the parent of the parent
        #Thus, parent[parent_inode] needs to match what it's referring to     
        if dir.name == "'..'" and parent[dir.parent_inode] != dir.inode_num:
             print("DIRECTORY INODE %d NAME '..' LINK TO INODE %d SHOULD BE %d" %(dir.parent_inode, dir.inode_num, parent[dir.parent_inode]))
             error +=1

def main():
    global error
    if len(sys.argv) != 2:
        sys.stderr.write('Usage Error: ./lab3b fileName \n')
        sys.exit(1)
    try:
        csv_input = open(sys.argv[1], 'r')
    except IOError:
        sys.stderr.write('Unable to open file \n')
        sys.exit(1)
    
    csvFile=csv.reader(csv_input)

    for line_num, entry in enumerate(csvFile, start=1):
        if entry[0] == "SUPERBLOCK":
            super_block = Superblock(entry)
            check_csv(entry,line_num, 8)
        elif entry[0] == "GROUP":
            group = Group(entry)
            check_csv(entry,line_num, 9)
        elif entry[0] == "BFREE":
            bfree_list.append(int(entry[1]))
            check_csv(entry,line_num, 2)
        elif entry[0] == "IFREE":
            ifree_list.append(int(entry[1]))
            check_csv(entry,line_num, 2)
        elif entry[0] == "INODE":
            inode_list.append(Inode(entry))
            check_csv(entry,line_num, 27)
        elif entry[0] == "DIRENT":
            dirent_list.append(Dirent(entry))
            check_csv(entry,line_num, 7)
        elif entry[0] == "INDIRECT":
            indirect_list.append(Indirect(entry))
            check_csv(entry,line_num, 6)
        else:
            sys.stderr.write('Error: Invalid data in csv.\n')
            sys.exit(1)

    block_consistency_audit(super_block, group)
    inode_and_dir_audit(super_block,super_block.first_non_reserved, super_block.total_inode)
    sys.exit(2) if error != 0 else sys.exit(0)

if __name__ == '__main__':
    main()