#include "myshheadr.h"

mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

int main() {
    char total_inp[BUF];        // where the entire input line is stored (constant length)
    char* inp;                  // points to each input found in total_inp separated by ';'
    char* com;                  // points to each command found in inp separated by '|'
    int pipe_count;
    myHistory hist;
    hist.size= 0;
    hist.top= -1;


    struct sigaction act;
    act.sa_handler= SIG_IGN;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);
    while(1) {
        curr_path();

        int tot_read= read(0, total_inp, BUF);
        total_inp[tot_read-1]= '\0';          // remove newline character

        inp= strsplt(total_inp, ";", total_inp);
        while(inp != NULL) {
            if(is_exit(inp))        //  shell exits if "exit" is inputted
                exit(0);

            int is_bg= 0;                   // command to run in background (1) or not (0). By default: 0
            int end_ch= strlen(inp)-1;
            if(inp[end_ch] == '&') {
                is_bg= 1;
                inp[end_ch]= '\0';
            }

            if(hist_call(inp, &hist)) {
                inp= strsplt(total_inp, ";", total_inp);
                continue;
            }

            pipe_count= 0;
            for(int i= 0; inp[i] != '\0'; i++) {
                if(inp[i] == '|')
                    pipe_count++;
            }

            int p[pipe_count*2];
            for(int i= 0; i< pipe_count; i++)  // for each pipe '|' in input, a pipe is made
                pipe(p +i*2);

            com= strtok(inp, "|");
            int compos= 0;                  // index of command from command line (command position)
            while(com != NULL) {
                int read= (compos-1)*2;     // the READ part of pipe n-1
                int wrt= compos*2+1;        // the WRITE part of pipe n
                int arrow= redir_detect(com);      //-1 if no arrow, 1 if '>', 2 if '>>'


                int fd= -1;
                int pid= fork();
                if(pid == -1) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                if(pid == 0) {
                    if(is_bg == 0) {
                        act.sa_handler= SIG_DFL;
                        sigaction(SIGINT, &act, NULL);
                        sigaction(SIGTSTP, &act, NULL);
                    }

                    if(arrow < 1)       // no redirection of output to file;
                        norm_exec(com, -1, p, pipe_count, compos);

                    else {
                        char* sep[2];       // sep[0]: command with arguments
                                            // sep[1]: fd of file for output to be redirected

                        if(arrow == 1) {
                            sep[0]= strtok(com, ">");
                            sep[1]= strtok(NULL, "> ");      // file to be redirected
                            fd= open(sep[1], O_CREAT | O_RDWR | O_TRUNC, mode);
                        }
                        else {
                            sep[0]= strtok(com, ">>");
                            sep[1]= strtok(NULL, ">> ");
                            fd= open(sep[1], O_CREAT | O_RDWR | O_APPEND, mode);
                        }

                        norm_exec(sep[0], fd, p, pipe_count, compos);
                    }
                }
                if(pipe_count) {
                    if(compos != 0)
                        close(p[read]);
                    if(compos != pipe_count)
                        close(p[wrt]);
                }

                int wstatus;
                if(is_bg == 0) {
                    if (waitpid(pid, &wstatus, WUNTRACED) == -1) {      // not in bg == wait for process to end
                        perror("Failed to waitpid()");
                    }
                }
                else {
                    if (waitpid(pid, &wstatus, WUNTRACED | WNOHANG) == -1) {        // in bg == don't wait for process to end
                        perror("Failed to waitpid()");
                    }
                }
                
                if (WIFSIGNALED(wstatus))
                    write(1, "Killed\n", 8);
                else if (WIFSTOPPED(wstatus))
                    write(1, "Stopped\n", 9);

                if(fd > 0)
                    close(fd);

                compos++;
                com= strtok(NULL, "|");
            }
            inp= strsplt(total_inp, ";", total_inp);
        }
    }
}