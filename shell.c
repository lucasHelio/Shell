#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h>
#include<signal.h>
#include<sys/wait.h>

#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1

typedef struct jobs
{
    int id;
    int modo;
    char  status[20];
    pid_t pid_process;

}jobs;

int jobsexistentes=1;
jobs* jobscriados;

char *getcwd(char *buf, size_t size);

#define bufferLimite 200

const char *status_string[] = { "rodando", "travado", "em chok"};

void adicionaJobs(jobs job, pid_t pid_process, char* modo)
{
    //jobs novojob;
    
    job.id = jobsexistentes;

    strcpy(job.status, status_string[0]);

    job.pid_process = pid_process;

    if(modo == "&") //modo em background
    {
        job.modo = 0;

    }
    else //modo em foreground
    { 
        job.modo = 1;
    }

    jobscriados[jobsexistentes-1] = job;

    jobsexistentes++;
    return;
}

void eliminaJobs(pid_t pid_process) //seta o status do job como terminado
{
    for(int i =0; i< jobsexistentes;i++)
    {
        if(jobscriados[i].pid_process == pid_process){
            strcpy(jobscriados[i].status, status_string[2]);
            break;
        }
    }
    return;
}

void listjobs()//printa os jobs que ainda não foram terminados
{ 
    for(int i=0;i<jobsexistentes;i++)
    {
        if(jobscriados->status!=status_string[2])
        {
            printf("id: [%d] status: %s pid process: %d\n", jobscriados[i].id, jobscriados[i].status, jobscriados[i].pid_process);
        }
    }
    return;
}

void leInput(char *input)
{
    scanf("%[^\n]", input); 
    //printf("o comando digitado foi: %s\n", input);
    __fpurge(stdin);
    return;
}




void limpaTerminal()
{
    printf("\033[H\033[J");//limpa o terminal
}








void parsedInput(char *comando, char ** destino, char separador) //divide os comandos dados na entrada e armazena no vetor destino
{
    char *parsed;
    int index =0;
    parsed = strtok(comando, &separador);
    
    
    while(parsed!= NULL)
    {
        
        destino[index] = parsed;
        
        index++;
        parsed = strtok(NULL,&separador);
    }
    destino[index] = NULL;
    
    return;
}


char * ultimoElemCaminho(char ** caminho)
{
    for (int i=0; i< bufferLimite;i++)
    {
        if(caminho[i+1] == NULL)
            return caminho[i];
    }
}


void cd(char *caminho)
{
    if(chdir(caminho) != 0) 
    {
        perror("erro ao tentar mudar de diretorio\n");
        return;
    }
    else 
    {
        chdir(caminho); 
    }
    return;
}

void printaCaminho()
{
    char cwd[bufferLimite];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char **parsed = (char **) malloc (sizeof(char *));
        for(int i =0; i<bufferLimite; i++)
        {
            parsed[i]= (char *) malloc(sizeof(char));
        }
        
        parsedInput(cwd, parsed, '/');
        

        printf("\033[92mconcha:\033[34m%s\033[0m$ ",ultimoElemCaminho(parsed));
        
    }
    else 
    {
        perror("Erro ao procurar diretorio");
        return;
    }
}

void exec_nao_buildin(char **comando)//, char *tipo, char *diretorio)
{
    
    char caminho[20] = "/usr/bin/";
    char cwd[bufferLimite];
        

    strcat(caminho, comando[0]);
    if(comando[1] == NULL) //comando do diretorio atual
    {
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            execl(caminho, comando[0], cwd, NULL);
        }
        else{perror("");return;}
    }

    else if(comando[1][0]=='-')
    {
        if(comando[2]==NULL)
        {
            if(getcwd(cwd, sizeof(cwd))!=NULL)
            {
                execl(caminho, comando[0], comando[1], cwd, NULL); //comando de tipo t do diretorio atual
            }
            else{perror(""); return;}
        }
        else{ //comando de tipo t do diretorio x
            execl(caminho, comando[0], comando[1], comando[2], NULL);
        }
    }
    
    else//comando do diretorio x
    {
        execl(caminho, comando[0], comando[1], NULL);
    }
}


void exec_buildin(char **comando)//identifica o comando e redireciona para o proprio;
{
    if (strcmp(comando[0], "cd") == 0)//excuta o comando cd
    {
        cd(comando[1]);
        return;
    }
        
    if (strcmp(comando[0], "jobs")==0 ) //executa o comando jobs  //ToDo
    {
        return;
    }

    if(strcmp(comando[0], "fg")==0 )// executa o comando fg    //ToDo
    {
        return;
    }

    if(strcmp(comando[0], "bg")==0)//executa o comando bg    //ToDo
    {
        return;
    }
    
    if(strcmp(comando[0], "exit")==0)//executa o exit./terminal
    {
        printf("flwww\n");
        exit(0);    //hmmmmm
    }

    if(strcmp(comando[0], "clear")==0)// limpa o terminalc
    {
        limpaTerminal();
        return;
    }

    /* //apartir dq realocar o codigo
    if(strcmp(comando[0], "rm") == 0 ||strcmp(comando[0], "ls")==0)//executa o comando ls //ls n eh build-in 
    {                                                       //deveria ficar junto?
        char cwd[bufferLimite];                 //pra dar ls tem q ser o filho
                                                //mas n deve ficar junto dos built-in
        if (comando[1]==NULL) //ls do proprio diretorio
        {
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                ls(comando[0], NULL, cwd);
            }
            else 
            {
                perror("talvez aqui");//erro no diretorio que a gente tá??
                return;
            }
            return;//teoricamente nunca chegamos nesse return
        }

        if (comando[1][0]=='-'){ //ls de tipo t do diretorio x
            if (comando[2]==NULL)
            {
                if (getcwd(cwd, sizeof(cwd)) != NULL) 
                {
                    ls(comando[0], comando[1], cwd); //comando[1] - tipo do ls do propio diretorio
                }
                else 
                {
                    perror("tamo aqui");//erro no diretorio que a gente tá??
                    return;
                }
                return;//teoricamente nunca chegamos nesse return
                
            }
            else
            {
                ls(comando[0], comando[1], comando[2]); //comando[1] - tipo do ls   comando[2] - diretorio
                return;//teoricamente nunca chegamos nesse return
            }
        }
        else  //ls do diretorio x
        { 
            ls(comando[0], NULL, comando[1]);
            return;//teoricamente nunca chegamos nesse return
        }
    }*/

}




int ehBuildin(char *comando) //valida se o comando eh buildin ou nao
{
    char *comandobuildin[] = { "cd", "jobs", "fg", "bg", "exit", "clear"};
    //comandobuildin[0]="cd";
    //comandobuildin[1]="jobs";
    //comandobuildin[2]="fg";
    //comandobuildin[3]="bg";
    //comandobuildin[4]="exit";
    //comandobuildin[5]="clear";
    for(int i=0 ; i< 6;i++)
    {
        if(strcmp(comando, comandobuildin[i])==0)
        {
            return 1; // achou comando build in
        }
    }
    return 0; // nao achou um comando build in
}


int main()
{   
    jobscriados = (jobs *)malloc(20*sizeof(jobs));//como aloca isso?
    jobs novoJob;
    pid_t pid;
    __fpurge(stdin);
    limpaTerminal();
    char **caminho = (char **) malloc(bufferLimite*sizeof(char *));
    for(int i=0; i<bufferLimite; i++)
    {
        caminho[i]= (char*) malloc(bufferLimite*sizeof(char));
    }
    if(caminho == NULL)return -1;
    
    char *comando= (char *) malloc(bufferLimite*sizeof(char));    
    char **comandoSeparado =(char **) malloc(bufferLimite*sizeof(char *));
    for(int i=0; i<bufferLimite; i++)
    {
        comandoSeparado[i]= (char*) malloc(bufferLimite*sizeof(char));
    }

    caminho[0][0]= '~';
    caminho[1]=NULL;
    
    while(1) //enquanto a concha existe ele está rodando
    {
        
        printaCaminho();
        
        leInput(comando);
        parsedInput(comando, comandoSeparado, ' ');
        
        //identificar se o comando eh build in
        // se nao for cria filho
        if(!ehBuildin(comandoSeparado[0])){
            pid = fork();
            //if(fork()== 0){//criando filho
            if(pid == 0){
                exec_nao_buildin(comandoSeparado);
            }
            else //processo pai esperando filho retornar
            {
                //printf("pid do pai: %d\n",getpid());
                adicionaJobs(novoJob, pid, comandoSeparado[3]);
                wait(NULL);
            }
        }

        else{
            exec_buildin(comandoSeparado);
        }
        printf("job id: %d\n",jobscriados[0].id );
    }
    
    return 0;
}