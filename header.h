#include <iostream>
#include <sched.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <fstream>
#include <cstdlib>


//path specified to the cgroup (pids) in which we want to write our process 
#define REQUIRED_CGROUP "/sys/fs/cgroup/container"
#define concat(a,b) (a"" b)
//path to the CPU cgroup to allot cpu quota to the container
#define CGROUP_CPU_FOLDER "/sys/fs/cgroup/container/cpu.max"
//path to the Memory cgroup to allocate memory to the container
#define CGROUP_MEMORY_FOLDER "/sys/fs/cgroup/container/memory.max"
#define CGROUP_SUBTREE "/sys/fs/cgroup/cgroup.subtree_control"

//restricting maximum allowed processes to prevent any memory overflow
#define MAX_ALLOWED_PROCESSES "5"

#define CONFIG_FILE "container_config.ini"