#include"required.h"

//path specified to the cgroup (pids) in which we want to write our process 
#define REQUIRED_CGROUP "/sys/fs/cgroup/pids/container/"
#define concat(a,b) (a"" b)

//restricting maximum allowed processes to prevent any memory overflow
#define MAX_ALLOWED_PROCESSES "5"

/*check for any errors using a custom try function
params: takes an int status and a pointer to a character array (string) messege
return: returns the same int status if code block works fine or prints the error msg by calling the perror function and returns the error msg using exit function.
The C standard specifies two constants, EXIT_SUCCESS and EXIT_FAILURE, that may be passed to exit() to indicate successful or unsuccessful termination, respectively.*/

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
    int targetFile = open(path, O_WRONLY|O_APPEND);
    write(targetFile,value,strlen(value));
    close(targetFile);
}

//restrict process creation
//return: void; 
//it just restricts the number of maximum allowed processes to be 5. More precisely saying 3, as the container itself and the shell that we are going to launch taes up 2 processes.

void setMaxProcessNum(){
    mkdir(REQUIRED_CGROUP, S_IRUSR|S_IWUSR);

//The c_str() method converts a string to an array of characters with a null character at the end.
    const char* pid = std::to_string(getpid()).c_str();

    WRITE(concat(REQUIRED_CGROUP, "pids.max"), MAX_ALLOWED_PROCESSES);
    WRITE(concat(REQUIRED_CGROUP,"notify_on_release"),"1");
    WRITE(concat(REQUIRED_CGROUP,"cgroup.procs"),pid);
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
    const int stackSize = 65536;
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
    chroot(folder);
    chdir("/");
    isOK(chdir("/"),"chdir");
}

//custom clone function to make the child process
//using template for generic type.
//param: function that is to be cloned and flags to send clone
// template <typename Function>
void cloneProcess(int (*function)(void*), int flags){
    auto pid = clone(function, stack_memory(), flags, 0);
    isOK(pid,"clone");
    wait(nullptr);
}


//the child process
int jail(void* args){
    setMaxProcessNum();
    // createCgroup(CGROUP_NAME);
    std::cout<<"Child pid: "<<getpid()<<std::endl;
    setHostname("my-container");
    setupVariables();
    setupRoot("./root");
    //attach the proc file system (procfs) to the file hierarchy
    mount("proc","/proc","proc",0,0);

    //SIGCHILD flag tells the process to emit a signal when the process is finished
    cloneProcess([](void *args) ->int { run("/bin/sh"); return 0;}, SIGCHLD);

    ///unmount the file system when the process ends
    umount("proc");
    return EXIT_SUCCESS;
}

//parent process
int main(int argc, char** argv){
    std::cout<<"Parent pid: "<<getpid()<<std::endl;
    cloneProcess(jail,CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD);

    return EXIT_SUCCESS;
}