/**
 * shell
 * CS 241 - Spring 2022
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char** split(char* cmd, char* str);
int run_external_command(char* cmd);
void wirte_to_file(char* filename, vector* v);
void read_file(char* filename);
char* cmdat(int idx,char* filename,vector* v);
int run_buildin_command(char* cmd, char* filename, vector* v);
int is_buildin(char* cmd);
void hist_flag(char *argv[]);
char* cmd_beginwith(char* str, char* filename, vector* v);
int contain_logic_operator(char* cmd);
void with_logic_operator(char* cmd, char* filename, vector* v);
void noflag(char *argv[]);
int is_background_command(char* str);
void cleanup(int signal);
int is_redirection(char* cmd);


typedef struct process {
    char *command;
    pid_t pid;
} process;

static vector* process_v;
static long btime;

char** split(char* cmd, char* str) {
    char ** res  = NULL;
    char *  p    = strtok (cmd, str);
    int n_spaces = 0;

    while (p) {
        res = realloc (res, sizeof (char*) * ++n_spaces);

        if (res == NULL)
            exit (-1);

        res[n_spaces-1] = p;

        p = strtok (NULL, str);
    }

    res = realloc (res, sizeof (char*) * (n_spaces+1));
    res[n_spaces] = 0;
    return res;
}

int is_background_command(char* str) {
    return strlen(str)>2 && str[strlen(str)-1]=='&' && str[strlen(str)-2]==' ';
}

void cleanup(int signal) {
  int status;
  while (waitpid((pid_t) (-1), &status, WNOHANG) > 0) {}
}


int is_signal_command(char* str) {
    return !strncmp(str, "kill", 4) || !strncmp(str, "stop", 4) || !strncmp(str, "cont", 4);
}


int run_external_command(char* cmd) {
    pid_t pid = fork();

    int is_background=0;
    if(is_background_command(cmd)){
		signal(SIGCHLD, cleanup);
        is_background=1;
        process* p = malloc(sizeof(process));
        p->command=malloc(strlen(cmd)+1);
        strcpy(p->command,cmd);
        p->pid = pid;
        vector_push_back(process_v, p);
	}

    if (pid < 0) { // fork failure
        print_fork_failed();
        exit(1);
    } else if (pid > 0) {
        int status=0;
        if(is_background) {

        } else if (waitpid(pid, &status, 0)==1) {
            print_wait_failed();
        }
        if(is_signal_command(cmd)&&status==9) {
            print_killed_process(atoi(cmd+5), cmd);
        }
        return status;
    } else {
        if(is_background){
            
            cmd[strlen(cmd)-2] = '\0';
            
        }
        
        
        if (is_redirection(cmd)) {
            if(strstr(cmd," > ")!=NULL || strstr(cmd," >> ")!=NULL) {
                char* found = strstr(cmd,">");
                char* found2 = strstr(cmd,">>");
                int filedes = 0;
                char* realcmd = NULL;
                if(found2!=NULL){
                    filedes = open(found2 + 3, O_CREAT|O_RDWR|O_APPEND, S_IRUSR | S_IWUSR);
                    realcmd = malloc(strlen(cmd)+1);
                    strcpy(realcmd,cmd);
                    if(found2 - cmd - 1>=0) {
                        realcmd[found2 - cmd - 1]='\0';
                    }
                } else {
                    filedes = open(found + 2, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR | S_IWUSR);
                    realcmd = malloc(strlen(cmd)+1);
                    strcpy(realcmd,cmd);
                    if(found - cmd - 1>=0) {
                        realcmd[found- cmd - 1]='\0';
                    }
                }

                char** res = split(realcmd," ");
                print_command_executed(getpid());
                
                close(1);
                dup(filedes);
                close(filedes);
                execvp(res[0], res);
                print_exec_failed(res[0]);
                if(res!=NULL) {
                    free (res);
                }

                exit(1);
            } else {
                char* found = strstr(cmd,"<");
                int filedes = open(found + 2, O_CREAT|O_RDWR, S_IRUSR | S_IWUSR);

                char* realcmd = malloc(strlen(cmd)+1);
                strcpy(realcmd,cmd);
                if(found - cmd - 1>=0) {
                    realcmd[found- cmd - 1]='\0';
                }



                char** res = split(realcmd," ");
                print_command_executed(getpid());

                close(0);
                dup(filedes);
                close(filedes);

                execvp(res[0], res);
                print_exec_failed(res[0]);
                if(res!=NULL) {
                    free (res);
                }

                exit(1);
            }
        } else if(is_signal_command(cmd)) {
            if(strlen(cmd)==4 || (strcmp(cmd+5,"0") && atoi(cmd+5)==0)) {
                print_invalid_command(cmd);
                exit(1);
            }

            if(!strncmp(cmd, "kill ", 5)) {
                int killed_pid = atoi(cmd+5);
                int unsuc = kill(killed_pid, SIGKILL);
                if(unsuc) {
                    print_no_process_found(killed_pid);
                    exit(1);
                } else {
                    print_killed_process(killed_pid, cmd);
                    exit(0);
                }
            } else if(!strncmp(cmd, "stop ", 5)) {
                int stopped_pid = atoi(cmd+5);
                int unsuc = kill(stopped_pid, SIGSTOP);
                if(unsuc) {
                    print_no_process_found(stopped_pid);
                    exit(1);
                } else {
                    print_stopped_process(stopped_pid, cmd);
                    exit(0);
                }
            } else {
                int cont_pid = atoi(cmd+5);
                int unsuc = kill(cont_pid, SIGCONT);
                if(unsuc) {
                    print_no_process_found(cont_pid);
                    exit(1);
                } else {
                    print_continued_process(cont_pid, cmd);
                    exit(0);
                }
            }
            return 0;
        }
        else {
            char** res = split(cmd," ");
            print_command_executed(getpid());
            execvp(res[0], res);
            print_exec_failed(res[0]);
            if(res!=NULL) {
                free (res);
            }
            exit(1); 
        }
        
    }
}


void wirte_to_file(char* filename, vector* v) {
    
    if(v!=NULL && vector_empty(v)){
        return;
    }
    FILE* fWrite = fopen(filename,"a");
    for(size_t i = 0; i < vector_size(v); i++) {
        fprintf(fWrite,"%s\n",*(vector_at(v,i)));
    }
    fclose(fWrite);
    vector_clear(v);
}

void read_file(char* filename) {

    char* line=NULL;
    size_t len = 0;

    FILE* fr = fopen(filename,"r");
    int i=0;
    
    while(getline(&line, &len, fr)!=-1) {
        line[strlen(line)-1] = '\0';
        print_history_line(i, line);
        i++;
    }
    
    fclose(fr);
    if(line!=NULL) {
        free(line);
    }
    
}

void readvec(vector* vtemp) {
    if(vtemp==NULL) {
        return;
    }
    for(int i = 0; i < (int)(vector_size(vtemp)); i++) {
        print_history_line(i, *(vector_at(vtemp, (size_t)i)));
    }
}

char* cmdat(int idx,char* filename, vector* v) {

    if(filename!=NULL) {
        FILE* fr = fopen(filename,"r");
        char* line = NULL;
        size_t len = 0;
        int i=0;
        while(getline(&line, &len, fr)!=-1) {
            if(i==idx) {
                fclose(fr);
                line[strlen(line)-1] = '\0';
                return line;
            }
            i++;
        }
        if(line!=NULL) {
            free(line);
        }
        fclose(fr);
    } else {
        if(v==NULL || vector_empty(v)) {
            return NULL;
        }
        for(int i = 0; i < (int)(vector_size(v)); i++) {
            if(i==idx) {
                char* cpy = malloc(40);
                strcpy(cpy,*(vector_at(v, (size_t)i)));
                return cpy;
            }
        }
    }
    return NULL;
}


char* cmd_beginwith(char* str, char* filename, vector* v) {
    if(filename!=NULL) {
        FILE* fr = fopen(filename,"r");
        char* line = NULL;
        size_t len = 0;
        vector* vtemp = vector_create(string_copy_constructor, string_destructor, string_default_constructor); 
        while(getline(&line, &len, fr)!=-1) {
            line[strlen(line)-1] = '\0';
            vector_push_back(vtemp, line);
        }
        fclose(fr);
        if(line!=NULL) {
            free(line);
        }

        if(strlen(str)==0) {
            
            char* ret = malloc(40);
            strcpy(ret,*(vector_at(vtemp, vector_size(vtemp)-1)));
            vector_destroy(vtemp);
            return ret;
        }

        for(int i = (int)(vector_size(vtemp)-1); i >= 0; i--) {
            if(strncmp(*(vector_at(vtemp,(size_t)i)), str, strlen(str)) == 0) {
                char* ret = malloc(40);
                strcpy(ret,*(vector_at(vtemp,i)));
                vector_destroy(vtemp);
                return ret;
            }
        }
        vector_destroy(vtemp);
    } else {
        if(v==NULL || vector_empty(v)) {
            return NULL;
        }
        for(int i = (int)(vector_size(v)-1); i >= 0; i--) {
            if(strncmp(*(vector_at(v,(size_t)i)), str, strlen(str)) == 0) {
                char* ret = malloc(40);
                strcpy(ret,*(vector_at(v,i)));
                return ret;
            }
        }
    }
    return NULL;
}

int run_buildin_command(char* cmd, char* filename,vector* v) {
    if(!strncmp(cmd,"cd ",3)) {
        int exist = chdir(cmd + 3);
        if(exist == -1) {
            print_no_directory(cmd + 3);
            return 1;
        }
    } else if(!strcmp(cmd,"!history")) {
        if(filename!=NULL) {
            wirte_to_file(filename, v);
            read_file(filename);
        }
        else {
            readvec(v);
        }
        
    } else if(cmd[0] == '#') {
        if(filename!=NULL) {
            wirte_to_file(filename, v);
        }
        int idx = atoi(cmd+1);
        if (idx != 0 || strcmp(cmd,"#0") == 0) {
            char* fullcmd = cmdat(idx, filename,v);
            if(fullcmd==NULL) {
                print_invalid_index();
                return 1;
            }

            vector_push_back(v, fullcmd);
            print_command(fullcmd);

            if(contain_logic_operator(fullcmd)) {
                with_logic_operator(fullcmd, filename,v);
            } else if(is_buildin(fullcmd)) {
                run_buildin_command(fullcmd, filename,v);
            } else {
                run_external_command(fullcmd);
            }
            free(fullcmd);
        } else{
            print_invalid_command(cmd);
            return 1;
        }
    } else if(cmd[0] == '!') {
        if(filename!=NULL) {
            wirte_to_file(filename, v);
        }
        char* fullcmd = cmd_beginwith(cmd+1, filename, v);
        if(fullcmd==NULL) {
            print_no_history_match();
            return 1;
        }

        vector_push_back(v, fullcmd);
        print_command(fullcmd);

        if(contain_logic_operator(fullcmd)) {
            with_logic_operator(fullcmd, filename,v);
        } else if(is_buildin(fullcmd)) {
            run_buildin_command(fullcmd, filename,v);
        } else {
            run_external_command(fullcmd);
        }
        free(fullcmd);
    } else if (strcmp(cmd, "ps") == 0) {
        print_process_info_header();
        char file_path[40];
        char pidstr[20];
        process_info* pif = malloc(sizeof(process_info));
        process* p=NULL;
        
        char stat;
        long int vsize;
        unsigned long int nthread;
        long utime,stime, start;
        FILE* fp;
        pif->command=malloc(40);

        time_t starttime;
        size_t seconds;
        size_t minute;
        for(size_t i=0; i<vector_size(process_v);i++) {
            
            p=vector_get(process_v,i);

            sprintf(pidstr, "%d", (int)(p->pid));
            strcpy(file_path,"/proc/");
            strcat(file_path,pidstr);
            strcat(file_path,"/stat");
            fp= fopen(file_path, "r");
            
            if(fp!=NULL) {
                
                fscanf(fp, "%*s %*s %c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %ld %ld %*s %*s %*s %*s %ld %*s %ld %ld ....", &stat, &utime, &stime, &nthread, &start, &vsize);
                fclose(fp);
                pif->pid=p->pid;
                strcpy(pif->command,p->command);
                pif->state=stat;
                pif->nthreads = nthread;
                pif->vsize = vsize/1024;
                starttime=btime+start*0.01;
                pif->start_str=malloc(20);
                time_struct_to_string(pif->start_str, 20,localtime(&starttime));
                pif->time_str=malloc(20);
                seconds = (utime+stime)/sysconf(_SC_CLK_TCK);
                minute=seconds/60;
                seconds=seconds%60;
                execution_time_to_string(pif->time_str, 20, minute, seconds);
                print_process_info(pif); 
                free(pif->start_str);
                free(pif->command);
                free(pif->time_str);
            } else {
                vector_erase(process_v,i);
                i--;
            }
            
        }
        
        free(pif);
        

        
    }
    return 0;

}




int is_buildin(char* cmd) {
    return (!strncmp(cmd, "cd ", 3)) || (!strcmp(cmd, "!history")) || cmd[0] == '#' || cmd[0] == '!' || (!strcmp(cmd, "ps"));
}


int contain_logic_operator(char* cmd) {
    return strstr(cmd,"||")!=NULL || strstr(cmd,"&&")!=NULL || strstr(cmd,";")!=NULL;
}


int is_redirection(char* cmd) {
    return strstr(cmd," > ")!=NULL || strstr(cmd," >> ")!=NULL || strstr(cmd," < ")!=NULL;
}



void with_logic_operator(char* cmd, char* filename, vector* v) {
    if(cmd[0] == '!') {
        run_buildin_command(cmd, filename,v);
    } else if(strstr(cmd,"&&")!=NULL) {
        char* loc = strstr(cmd,"&&");
                
        if(strlen(cmd)<=4 || loc-cmd<2 || *(loc+2) != ' ' || *(loc-1) != ' ') {
            print_invalid_command(cmd);
            return;
        }

        char* second = loc+3;
                    
        char* first = malloc(strlen(cmd)+1);
        strcpy(first,cmd);
        if(loc-cmd-1>=0) {
            first[loc-cmd-1]='\0';
        }

        int fres;
        if(is_buildin(first)) {
            fres=run_buildin_command(first, filename,v);
        } else {
            fres=run_external_command(first);
        }

        if(fres == 0) {
            if(is_buildin(second)) {
                run_buildin_command(second, filename,v);
            } else {
                run_external_command(second);
            }
        }
        free(first);

    } else if(strstr(cmd,"||")!=NULL) {
        char* loc = strstr(cmd,"||");
                    
        if(strlen(cmd)<=5 || loc-cmd<2 || *(loc+2) != ' ' || *(loc-1) != ' ') {
            print_invalid_command(cmd);
            return;
        }

        char* second = loc+3;
                    
        char* first = malloc(strlen(cmd)+1);
        strcpy(first,cmd);
        if(loc-cmd-1>=0) {
            first[loc-cmd-1]='\0';
        }
                    
        int fres;
        if(is_buildin(first)) {
            fres=run_buildin_command(first, filename,v);
        } else {
            fres=run_external_command(first);
        }

        if(fres!=0) {
            if(is_buildin(second)) {
                run_buildin_command(second, filename,v);
            } else {
                run_external_command(second);
            }
        }
        free(first);
    } else if (strstr(cmd,";")!=NULL) {
        char* loc = strstr(cmd,";");
                    
        if(strlen(cmd)<=3 || loc-cmd < 1 || *(loc+1) != ' ') {
            print_invalid_command(cmd);
            return;
        }

        char* second = loc+2;
                    
        char* first = malloc(strlen(cmd)+1);
        strcpy(first,cmd);
        if(loc-cmd>=0) {
            first[loc-cmd]='\0';
        }

        if(is_buildin(first)) {
            run_buildin_command(first, filename,v);
        } else {
            run_external_command(first);
        }

        if(is_buildin(second)) {
            run_buildin_command(second, filename,v);
        } else {
            run_external_command(second);
        }
        free(first);
    }
}



void hist_flag(char *argv[]) {
    char* path;
    char* cmd = NULL;
    size_t len = 0;

    vector* v = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    
    signal(SIGINT,SIG_IGN);

    pid_t curpid = getpid();
    ssize_t res;

    char* filename = get_full_path(argv[2]);
    
    
    while(1) {
        if(curpid==getpid()) {
            path = get_current_dir_name();
            print_prompt(path, getpid());
            free(path);
            if((res = getline(&cmd, &len, stdin))==EOF) {
                wirte_to_file(filename, v);
                free(cmd);
                free(filename);
                vector_destroy(v);
                exit(0);
            }
            cmd[strlen(cmd)-1]='\0';
            

            if(strcmp(cmd,"exit")) {
                if(cmd[0] != '#' && cmd[0] != '!'){
                    vector_push_back(v, cmd);
                }
            } else {
                wirte_to_file(filename, v);
                free(cmd);
                free(filename);
                vector_destroy(v);
                exit(0);
            }
            
            if(contain_logic_operator(cmd)) {
                with_logic_operator(cmd, filename,v);
            } else if(is_buildin(cmd)) {
                run_buildin_command(cmd, filename,v);
            } else {
                run_external_command(cmd);
            }
        } else {
            free(cmd);
            free(filename);
            vector_destroy(v);
            exit(1);
        }
    }
}

int get_file_len(char* filename) {
    char* line = NULL;
    size_t len = 0;
    FILE* fr = fopen(filename,"r");
    int i=0;
    while(getline(&line, &len, fr)!=-1) {
        i++;
    }
    fclose(fr);
    free(line);
    return i;
}

void file_flag(char *argv[]) {
    char* path;
    char* cmd = NULL;
    size_t len = 0;

    signal(SIGINT,SIG_IGN);

    vector* v = vector_create(string_copy_constructor, string_destructor, string_default_constructor);

    pid_t curpid = getpid();

    char* filename = get_full_path(argv[2]);
    FILE* file = fopen(filename,"r");
    
    int i = 0;
    int size = get_file_len(filename);
    while(i<size) {
        if(curpid==getpid()) {
            path = get_current_dir_name();
            print_prompt(path, getpid());
            free(path);
            getline(&cmd, &len, file);
            cmd[strlen(cmd)-1]='\0';
            print_command(cmd);
            if(contain_logic_operator(cmd)) {
                with_logic_operator(cmd, filename,v);
            } else if(is_buildin(cmd)) {
                run_buildin_command(cmd, filename,v);
            } else {
                run_external_command(cmd);
            }
            i++;
        } else {
            free(cmd);
            free(filename);
            vector_destroy(v);
            exit(1);
        }
    }
    free(cmd);
    free(filename);
    vector_destroy(v);
}





void noflag(char *argv[]) {

    char* path;
    char* cmd = NULL;
    size_t len = 0;

    signal(SIGINT,SIG_IGN);

    vector* v = vector_create(string_copy_constructor, string_destructor, string_default_constructor);

    pid_t curpid = getpid();
    
    char* filename=NULL;


    ssize_t res;

    while(1) {
        if(curpid==getpid()) {
            path = get_current_dir_name();
            print_prompt(path, getpid());
            free(path);
            if((res = getline(&cmd, &len, stdin))==EOF) {
                free(cmd);
                vector_destroy(v);
                exit(0);
            }
            cmd[strlen(cmd)-1]='\0';
            
            if(strcmp(cmd,"exit")) {
                if(cmd[0] != '#' && cmd[0] != '!'){
                    vector_push_back(v, cmd);
                }
            } else {
                free(cmd);
                vector_destroy(v);
                exit(0);
            }
            
            if(contain_logic_operator(cmd)) {
                with_logic_operator(cmd, filename,v);
            } else if(is_buildin(cmd)) {
                run_buildin_command(cmd, filename,v);
            } else {
                run_external_command(cmd);
            }
        } else {
            free(cmd);
            vector_destroy(v);
            exit(1);
        }
    }
    free(cmd);
    vector_destroy(v);
}


int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    if (argc != 3 && argc != 1) {
        print_usage();
        exit(1);
    }
    

    process_v = vector_create(shallow_copy_constructor, shallow_destructor, shallow_default_constructor);
    process* always=malloc(sizeof(process));
    always->command="./shell";
    always->pid=getpid();
    vector_push_back(process_v,always);
            
    FILE* fp2 = fopen("/proc/stat", "r");
    char* line = NULL;
    size_t len;
    while(getline(&line, &len, fp2)!=-1) {
        line[strlen(line)-1] = '\0';
        if(strncmp(line,"btime",5)==0){
            btime=atol(line+6);
            break;
        }
    }        
    fclose(fp2);
    if(line!=NULL) {
        free(line);
    }

    if(argc==1) {
        noflag(argv);
    } else if(!strcmp(argv[1],"-h")) {
        FILE* fr = fopen(argv[2],"a");
        fclose(fr);
        hist_flag(argv);
    } else if(!strcmp(argv[1], "-f")) {
        file_flag(argv);
    } else {
        print_usage();
        exit(1);
    }
    vector_destroy(process_v);
    return 0;
}
