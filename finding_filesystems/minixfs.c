/**
 * finding_filesystems
 * CS 241 - Spring 2022
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!

    inode* ind = get_inode(fs, path);
    if(ind == NULL) {
        errno = ENOENT;
        return -1;
    }
    ind->mode = (ind->mode & 0xfe00) | new_permissions;
    clock_gettime(CLOCK_REALTIME, &(ind->ctim));
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* ind = get_inode(fs, path);

    if(ind == NULL) {
        errno = ENOENT;
        return -1;
    }

    if (owner != (uid_t)-1) {
        ind->uid = owner;
    }
    if (group != (gid_t)-1) {
        ind->gid = group;
    }

    clock_gettime(CLOCK_REALTIME, &(ind->ctim));

    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    if(get_inode(fs, path) != NULL) {
        return NULL;
    }

    inode_number indn = first_unused_inode(fs);
    if(indn == -1) {
        return NULL;
    }
    
    const char* filename;
    inode* parent_ind =  parent_directory(fs, path, &filename);
    if(!valid_filename(filename)) {
        return NULL;
    }

    init_inode(parent_ind, fs->inode_root + indn);
    
    minixfs_dirent dnt;
    dnt.inode_num = indn;
    dnt.name = strdup(filename);
    
    data_block_number num_block = parent_ind->size / (sizeof(data_block));
    data_block_number offset = parent_ind->size - num_block * sizeof(data_block);
    
    if (num_block < NUM_DIRECT_BLOCKS) {
        if(offset!=0) {
            if((unsigned long) (offset + FILE_NAME_ENTRY) > sizeof(data_block)) {
                num_block++;
                if(num_block >= NUM_DIRECT_BLOCKS) {
                    return NULL;
                }
                parent_ind->size += sizeof(data_block) - offset;
                offset = 0;
                add_data_block_to_inode(fs, parent_ind);
            }
        } else {
            add_data_block_to_inode(fs, parent_ind);
        }
        make_string_from_dirent(fs->data_root[parent_ind->direct[num_block]].data + offset, dnt);
        parent_ind->size += FILE_NAME_ENTRY;
    } else {
        return NULL;
    }

    free(dnt.name);
    return fs->inode_root + indn;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        char* map = GET_DATA_MAP(fs->meta);
        ssize_t block_used = 0;
        for(uint64_t i = 0; i < fs->meta->dblock_count; i++) {
            if (map[i] == 1) {
                block_used++;
            }
        }

        char* info_str = block_info_string(block_used);
        if((unsigned long) *off > strlen(info_str)) {
            return 0;
        }
        
        size_t ret = count;
        if(ret > strlen(info_str) - *off) {
            ret = strlen(info_str);
        }
 
        memmove(buf, info_str + *off, ret);
        *off += ret;

        return ret;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    if(*off + count > (NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS) * sizeof(data_block)) {
        errno = ENOSPC;
        return -1;
    }

    inode* ind = get_inode(fs, path);
    if(ind == NULL) {
        ind = minixfs_create_inode_for_path(fs, path);
        if(ind == NULL) {
            errno = ENOSPC;
            return -1;
        }
    }
    
    
    data_block_number num_block = (*off + count) / (sizeof(data_block));
    data_block_number remainder = (*off + count) % (sizeof(data_block));
    if(remainder != 0) {
        num_block++;
    }

    if(minixfs_min_blockcount(fs, path, num_block) == -1) {
        errno = ENOSPC;
        return -1;
    }

    data_block_number block_idx = *off / (sizeof(data_block));
    data_block_number offset = *off % (sizeof(data_block));
    data_block* blk = NULL;
    size_t byte_writen = 0;
    size_t byte_to_write = 0;
    while (byte_writen < count) {
        if(byte_writen + sizeof(data_block) - offset <= count) {
            byte_to_write = sizeof(data_block) - offset;
        } else {
            byte_to_write = remainder - offset;
        }

        
        if(block_idx < NUM_DIRECT_BLOCKS) {
            blk = fs->data_root + ind->direct[block_idx];
        } else {
            blk = fs->data_root + ind->indirect;
            data_block_number* inderect_idx = (data_block_number*) blk->data + block_idx - NUM_DIRECT_BLOCKS;
            blk = fs->data_root + *inderect_idx;
        }
        
        memcpy(blk->data + offset, buf, byte_to_write);
        byte_writen += byte_to_write;

        if(offset != 0) {
            offset = 0;
        }

        block_idx++;
        buf += byte_to_write;
    }    

    if(*off + count > ind->size) {
        ind->size = *off + count;
    }

    clock_gettime(CLOCK_REALTIME, &(ind->atim));
    clock_gettime(CLOCK_REALTIME, &(ind->mtim));

    *off += count;

    return count;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    inode* ind = get_inode(fs, path);
    if (ind == NULL) {
      errno = ENOENT;
      return -1;
    }

    if(ind->size < (unsigned long)*off) {
        return 0;
    }

    ssize_t ret = count;
    if((unsigned long) ret >  ind->size-*off) {
        ret = ind->size-*off;
    }

    //data_block_number num_block = (*off + ret) / (sizeof(data_block));
    data_block_number remainder = (*off + ret) % (sizeof(data_block));
    data_block_number block_idx = *off / (sizeof(data_block));
    data_block_number offset = *off % (sizeof(data_block));
    ssize_t byte_read = 0;
    ssize_t byte_to_read = 0;
    data_block* blk = NULL;
    while (byte_read < ret) {
        if(byte_read + sizeof(data_block) - offset <= (unsigned long)ret) {
            byte_to_read = sizeof(data_block) - offset;
        } else {
            byte_to_read = remainder - offset;
        }
        
        if(block_idx < NUM_DIRECT_BLOCKS) {
            blk = fs->data_root + ind->direct[block_idx];
        } else {
            blk = fs->data_root + ind->indirect;
            data_block_number* inderect_idx = (data_block_number*) blk->data + block_idx - NUM_DIRECT_BLOCKS;
            blk = fs->data_root + *inderect_idx;
        }
        
        memcpy(buf, blk->data + offset, byte_to_read);
        byte_read += byte_to_read;

        if(offset != 0) {
            offset = 0;
        }

        block_idx++;
        buf += byte_to_read;
    }
    
    clock_gettime(CLOCK_REALTIME, &(ind->atim));

    *off += ret;

    return ret;

}
