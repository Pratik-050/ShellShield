#include"header.h"
#include"INIParser.h"

/*check for any errors using a custom try function
params: takes an int status and a pointer to a character array (string) messege
return: returns the same int status if code block works fine or prints the error msg by calling 
the perror function and returns the error msg using exit function.
The C standard specifies two constants, EXIT_SUCCESS and EXIT_FAILURE, that may 
be passed to exit() to indicate successful or unsuccessful termination, respectively.*/
int isOK(int status,const char* msg){
    if(status == -1)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return status;
}

/*Custom write function to write in specified files of cgroups.
open the desired file using open(), write the required value in the file and call close()
the return value of open() is a file descriptor, a small, non-negetive value
params: takes two string pointers path and value.
return: void*/

void WRITE(const char* path,const char* value){
    int fd = open(path, O_WRONLY|O_APPEND);
    if(fd == -1) {
        std::cout<<"error while opening\n";
        exit(1);
    }
    ssize_t bytes = write(fd,value,strlen(value));
    if(bytes == -1){
        std::cout<<"error while writing\n";
        exit(1);
    }
    close(fd);
}

//setting hostname of the container
//param: takes hostname as string
void setHostname(std::string hostname){
    sethostname(hostname.c_str(),hostname.size());
}

//setup environment variables for the container
void setupVariables(){
    clearenv();
    // we use 0 as overwrite as we want to check if the variable already exist in the environment, and if it does exist then don't change it, if doesn't exist then add that variable
    setenv("TERM", "xterm-256color", 0);
    setenv("PATH", "/bin/:/sbin/:usr/bin:/usr/sbin", 0);
}

//allocating 65kb of stack memory to use in the clone function
char* stack_memory() {
    const int stackSize = 65*1024;
    //nothrow object is used to throw nullptr if memory can not be allocated
    auto *stack = new (std::nothrow) char[stackSize];

    if (stack == nullptr) {
        printf("Cannot allocate memory \n");
        exit(EXIT_FAILURE);
    }

    return stack+stackSize;  //move the pointer to the end of the array because the stack grows backward.
}

//custom run function to execute a process
int run(const char *name) {
    char *_args[] = {(char *)name, (char *)0 };
    return execvp(name, _args);
}

//setup the root
//param: takes path of the folder as char array
void setupRoot(const char* folder){
    isOK(chroot(folder),"can't set root: ");
    isOK(chdir("/"),"chdir");
}

//custom clone function to make the child processes
//param: function that is to be cloned and flags to send clone function that makes the new namespace
void cloneProcess(int (*function)(void*), int flags){
    auto pid = clone(function, stack_memory(), flags, 0);
    isOK(pid,"clone");
    wait(nullptr);
}

/*restrict process creation
return: void; 
We can change number of processes using the container_config 
file. Also this function sets other
resource limits for the container i.e-> the jailed process*/
void setupJail(){
    INIParser parser;
    if(!parser.load("container_config.ini")){
        std::cout<<"can't load .ini\n";
        exit(1);
    }
    const char* root = parser.getValue("container","custom_root","./root").c_str();
    const char* maxProcess = parser.getValue("container", "maxProcesses", "5").c_str();
    const char* cpuPeriod = parser.getValue("container","cpu_period","100000").c_str();
    const char* cpuQuota = parser.getValue("container","cpu_quota","20000").c_str();
    const char* memoryQuota = parser.getValue("container","memory_quota","50M").c_str();
    const char* hostName = parser.getValue("container","host_name","my-container").c_str();
    
    long long memory = atoi(memoryQuota);
    memory = memory << 20;
    memoryQuota = std::to_string(memory).c_str();

    //The c_str() method converts a string to an array of characters with a null character at the end.
    const char* pid = std::to_string(getpid()).c_str();
    std::cout<<"Child PID: "<<pid<<std::endl;

    WRITE(concat(REQUIRED_CGROUP,"cgroup.procs"),pid);
    WRITE(concat(CGROUP_CPU_FOLDER,"cpu.cfs_quota_us"),cpuQuota);
    WRITE(concat(CGROUP_CPU_FOLDER,"cpu.cfs_period_us"),cpuPeriod);
    WRITE(concat(CGROUP_MEMORY_FOLDER,"memory.limit_in_bytes"),memoryQuota);
    WRITE(concat(REQUIRED_CGROUP, "pids.max"), maxProcess);
    
    //setup the container
    setHostname(hostName);
    setupVariables();
    setupRoot(root);

}

void makeCgroup(){

    //make a directory in the pids cgroup to run a container    
    mkdir(REQUIRED_CGROUP, S_IRUSR|S_IWUSR);
    //make a directory in the cpu cgroup in which our container will run
    mkdir(CGROUP_CPU_FOLDER, S_IRUSR | S_IWUSR);
    //make a directory in the memory cgroup in which our container will run
    mkdir(CGROUP_MEMORY_FOLDER, S_IRUSR | S_IWUSR);

}

//remove all the directories after termination of the container
void removeCgroup(){
    rmdir(REQUIRED_CGROUP);
    rmdir(CGROUP_CPU_FOLDER);
    rmdir(CGROUP_MEMORY_FOLDER);
}

int jail(void* args){
    setupJail();

    //attach the proc file system (procfs) to the file hierarchy
    mount("proc","/proc","proc",0,0);

    pid_t shellPid = fork();
    isOK(shellPid,"can't create shell: ");
    if(shellPid == 0){
        run("/bin/sh");
        exit(0);
    }

    while(wait(nullptr) > 0);

    ///unmount the file system when the process ends
    umount("proc");

    return EXIT_SUCCESS;
}

//parent process
int main(int argc, char** argv){
    std::cout<<"Parent pid: "<<getpid()<<std::endl;
    makeCgroup();

    cloneProcess(jail,CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD);

    removeCgroup();

    return EXIT_SUCCESS;
}