#include"header.h"
#include "parser.h" //for parsing config files

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
    // return 0 for successful case other wise -1
    isOK(sethostname(hostname.c_str(),hostname.size()),"HostName Error");
}

//setup environment variables for the container
void setupVariables(){
    clearenv();
    // we use 0 as overwrite as we want to check if the variable already exist in the environment, and if it does exist then don't change it, if doesn't exist then add that variable
    isOK(setenv("TERM", "xterm-256color", 0),"Terminal Env Error"); //show colors in terminal
    // shell will search for executable files in path (/bin,/sbin,usr/bin,usr/sbin)
    isOK(setenv("PATH", "/bin/:/sbin/:usr/bin:/usr/sbin", 0),"Path Env Error"); 
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

    // extract data from config file
    //The c_str() method converts a string to an array of characters with a null character at the end.
    map<string, string> config = parseIni(CONFIG_FILE);
    const char* root = config["custom_root"].c_str();
    const char* maxProcess = config["maxProcesses"].c_str();
    const char* cpuManage = config["cpu_manage"].c_str();
    const char* memoryQuota = config["memory_quota"].c_str();
    const char* hostName = config["host_name"].c_str();
    cout<<config["custom_root"];

    //write values to respective controller files 
    WRITE(CGROUP_MEMORY_FOLDER,memoryQuota);
    WRITE(CGROUP_CPU_FOLDER,cpuManage);
    WRITE(concat(REQUIRED_CGROUP, "/pids.max"), maxProcess);
    
    //setup the container
    setHostname(hostName);
    setupVariables();
    setupRoot(root);

}



void makeCgroup(){
    //enables cpu,memory,pids controller in root cgroup,allowing for the use of these controllers in other cgroups
    WRITE(CGROUP_SUBTREE,"+cpu +memory +pids");
    //make a directory in cgroup to run a container    
    mkdir(REQUIRED_CGROUP, S_IRUSR|S_IWUSR);

    //writes the pid to the cgroup.procs file within REQUIRED_CGROUP,
    const char* pid = std::to_string(getpid()).c_str();
    std::cout<<"Child PID: "<<pid<<std::endl;
    WRITE(concat(REQUIRED_CGROUP,"/cgroup.procs"),pid);
    
    
}

int jail(void* args){
    setupJail();

    //attach the proc file system (procfs) to the file hierarchy
    mount("proc","/proc","proc",0,0);

    pid_t shellPid = fork(); //creating shell in container cgroup 
    isOK(shellPid,"can't create shell: ");
    if(shellPid == 0){
        run("/bin/bash");
        exit(0);
    }

    while(wait(nullptr) > 0); //wait until all child processes are completed

    ///unmount the file system when the process ends
    umount("proc");

    return EXIT_SUCCESS;
}

//parent process
int main(int argc, char** argv){
    std::cout<<"Parent pid: "<<getpid()<<std::endl;
    
    makeCgroup();

    cloneProcess(jail,CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD);


    return EXIT_SUCCESS;
}