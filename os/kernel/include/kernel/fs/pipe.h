#pragma once

#include <kernel/fs/vfs.h>

struct process;

fs_node_t* pipe_create(char* path,uint32_t buffer_size);
void pipe_add_waiter(fs_node_t* node,struct process* waiter);
void init_pipe();
