#include "myshheadr.h"


void norm_exec(char* com, int fd, int* p, int pipe_count, int compos) {
    char* arg[32];
    int read= (compos-1)*2;
    int wrt= compos*2+1;

    arg[0]= strtok(com, " ");   //given command is separated by space (" "). Each token (arg[i]) is an argument for exec
    int i= 0;
    while(arg[i] != NULL) {
        i++;
        arg[i]= strtok(NULL, " ");
    }

    if(fd != -1)                // either the file descriptor or the pipe may dup2 the stdoutput
        dup2(fd, 1);
    
    if(pipe_count) {

        if((compos != pipe_count) && fd == -1) {          // Anything that exec writes, goes to the pipe
            if(dup2(p[wrt], 1) < 0) { perror("dup2 WRITE"); exit(1);}
            close(p[wrt]);
        }
        
        if(compos != 0) {       // Anything that exec reads, is obtained from the pipe
            if(dup2(p[read], 0) < 0) { perror("dup2 READ"); exit(1);}
            close(p[read]);
        }
        
    }


    if(strchr(arg[i-1], '*') || strchr(arg[i-1], '?'))
        wildch_matc(arg[i-1]);

    execvp(arg[0], arg);

    write(2, strcat(arg[0], ": command not found\n"), strlen(arg[0]) +20);
    exit(1);
}



int hist_call(char* com, myHistory* hist) {
    char* inp;
    if((inp= strstr(com, "myHistory")) != NULL) {

        inp= strtok(com, "myHistory");
        if(inp == NULL) {                                             //
            if(hist->size == 20) {
                int counter= 0;
                for(int i= hist->top+1; counter < 20; i++) {
                    if(i == 20)
                        i= 0;
                    printf("%d: %s\n", counter+1, hist->h[i]);              // print history
                    counter++;
                }
            }
            else {
                for(int i= 0; i <= hist->top; i++)
                    printf("%d: %s\n", i+1, hist->h[i]);
            }
            fflush(NULL);
            return 1;
        }                                                           //
        else {
            int pos= atoi(inp);
            if((pos < 1 || pos > 20) || (pos > hist->top+1 && hist->size != 20)) {
                write(2, "myHistory: Out of range\n", 25); return 1;
            }
            inp= hist_get(hist, pos);
            strcpy(com, inp);

            return 0;
        }
    }
    else {
        hist_update(hist);
        strcpy(hist->h[hist->top], com);

        return 0;
    }
}



char* hist_get(myHistory* hist, int pos) {
    if(hist->size == 20) {
        if(hist->top + pos > hist->size-1)
            return hist->h[hist->top + pos - hist->size];
        else
            return hist->h[hist->top + pos];
    }
    else
        if(pos > hist->size) { perror("myHistory"); exit(1); }

    return hist->h[pos-1];

}



void hist_update(myHistory* hist) {
    if(hist->size < 20)
        hist->size++;
    if(hist->top < 19)
        hist->top++;
    else
        hist->top= 0;
}



int redir_detect(char* inp) {
    int arrow= -1;
    for(int i= 0; inp[i] != '\0'; i++) {
        if(inp[i] == '<')                   // 'sth < input' equivalent to 'sth input'
            inp[i]= ' ';
        else if(inp[i] == '>') {
            arrow= 1;
            if(inp[i+1] == '>') {
                arrow= 2;
                i++;
            }
        }
    }
    return arrow;
}



void wildch_matc(char* arg) {
    glob_t globbuf;
    int r;
    r = glob(arg, GLOB_ERR , NULL, &globbuf);

    if(r!=0) {
        write(2, "No matches found\n", 18);
        exit(1);
    }

    int i= 0;
    while(globbuf.gl_pathv[i] != NULL)
        printf("%s\n", globbuf.gl_pathv[i++]);

    exit(0);
}



char* strsplt(char* src, char* splt, char* dest) {
    if(src[0] == '\0')
        return NULL;

    char* temp= strdup(src);
    char* ret= strtok(temp, splt);

    int size= strlen(src) - strlen(ret);

    if(size <= 1)           // it means this was the last split
        dest[0]= '\0';

    else {
        int offset= 1;
        if(src[strlen(ret)+1] == ' ')
            offset= 2;

        strcpy(dest, &src[strlen(ret)+offset]);
    }
    //free(temp);
    return ret;
}



int is_exit(char* com) {
    if(strstr(com, "exit") && strlen(com) == 4)
        return 1;
    return 0;
}



void curr_path() {
    char mes[15]= "in-mysh-now:> ";
    write(1, mes, 15); 
}