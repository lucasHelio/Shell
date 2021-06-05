#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include <sys/types.h>
#include<setjmp.h>


#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1

#define BUFFERLIMITE 200
static jmp_buf env;
int jump_active = 0;
pid_t pid;

typedef struct jobs
{
    char nome[20];
    int id;
    int modo;
    char  status[20];
    pid_t pid_process;

}jobs;

int jobsexistentes=1;
jobs* jobscriados;

char *getcwd(char *buf, size_t size);



const char *STATUS_STRING[] = { "Executando", "Suspenso", "Terminado"};

void adicionaJobs(jobs job, pid_t pid_process, int modo, char* nome)
{
    
    
    job.id = jobsexistentes;

    strcpy(job.nome, nome);

    strcpy(job.status, STATUS_STRING[0]);

    job.pid_process = pid_process;
    
    job.modo = modo;

    jobscriados[jobsexistentes-1] = job;

    jobsexistentes++;
    return;
}

void suspendeJobs(pid_t pid_process)
{
    for(int i =0;i<jobsexistentes;i++)
    {
        if(jobscriados[i].pid_process == pid_process)
        {
            strcpy(jobscriados[i].status, STATUS_STRING[1]);//seta como suspenso
            kill(pid_process, SIGSTOP); //Faz o processo parar
            break;
        }
    }
    return;
}

void eliminaJobs(pid_t pid_process) //seta o status do job como terminado
{
    for(int i =0; i< jobsexistentes;i++)
    {
        if(jobscriados[i].pid_process == pid_process){
            strcpy(jobscriados[i].status, STATUS_STRING[2]);// seta como terminado
            kill(pid_process, SIGSTOP); //seria SIGSTOP msm?
            break;
        }
    }
    return;
}

void listjobs()//printa os jobs que ainda não foram terminados
{ 
    for(int i=0;i<jobsexistentes;i++)
    {
        if(jobscriados->status!=STATUS_STRING[2])
        {
            printf("id: [%d] nome: %s \tstatus: %s pid process: %d\n", jobscriados[i].id, jobscriados[i].nome, jobscriados[i].status, jobscriados[i].pid_process);
        }
    }
    return;
}

void leInput(char *input)
{
    //fgets (input, BUFFERLIMITE, stdin); //ele coloca \0 no final
    //scanf("%[^\n]", input); 
    //printf("o comando digitado foi: %s\n", input);
    __fpurge(stdin);
    int bufsize = BUFFERLIMITE;
    int position = 0;
    //char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!input) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(1);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            input[position] = '\0';
            return;
        } else {
            input[position] = c;
        }
        position++;

        if (position >= bufsize) {
            bufsize += BUFFERLIMITE;
            input = realloc(input, bufsize);
            if (!input) {
                fprintf(stderr, "concha: allocation error\n");
                exit(1);
            }
        }
    }
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
    for (int i=0; i< BUFFERLIMITE;i++)
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
    char cwd[BUFFERLIMITE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char **parsed = (char **) malloc (sizeof(char *));
        for(int i =0; i<BUFFERLIMITE; i++)
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
    //printf("comando[0]: %s\n", comando[0]);
    //printf("comando[1]: %s\n", comando[1]);
    //printf("comando[0]: %s", comando[0]);
    if (execvp(comando[0], comando) == -1) {
    perror("Error");
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
        listjobs();
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



void sigint_handler(int sig) {
    puts("entrou sighand");
    if (sig == SIGINT){
        if (!jump_active) {
            return;
        }
        siglongjmp(env, 42);
        return;
    }
    if (sig ==SIGTSTP){
        puts("entro sigstop");
        suspendeJobs(pid); //uma funcao que suspenderia o filho
        //kill(pid filho, SIGSTOP);
        //kill(pid pai, SIGCONT);
        if (!jump_active) {
            return;
        }
        siglongjmp(env, 42);
        return;
    }

}

int main()
{   
    jobscriados = (jobs *)malloc(20*sizeof(jobs));//como aloca isso?
    jobs novoJob;
    //pid_t pid;
    int modo = FOREGROUND_EXECUTION;
    int status;
    


    __fpurge(stdin);
    limpaTerminal();
    char **caminho = (char **) malloc(BUFFERLIMITE*sizeof(char *));
    for(int i=0; i<BUFFERLIMITE; i++)
    {
        caminho[i]= (char*) malloc(BUFFERLIMITE*sizeof(char));
    }
    if(caminho == NULL)return -1;
    
    char *comando= (char *) malloc(BUFFERLIMITE*sizeof(char));    
    char **comandoSeparado =(char **) malloc(BUFFERLIMITE*sizeof(char *));
    for(int i=0; i<BUFFERLIMITE; i++)
    {
        comandoSeparado[i]= (char*) malloc(BUFFERLIMITE*sizeof(char));
    }

    caminho[0][0]= '~';
    caminho[1]=NULL;
    
    signal(SIGINT, sigint_handler);//control c tratado na main
    signal(SIGTSTP, sigint_handler);//control z tratado na main

    while(1) //enquanto a concha existe ele está rodando
    {
        if (sigsetjmp(env, 1) == 42)
        {
            printf("\n");
            continue;
        }
        jump_active = 1;
        printaCaminho();
        
        leInput(comando);
        
        if (comando[strlen(comando) - 1] == '&') {
            modo = BACKGROUND_EXECUTION;
            comando[strlen(comando) - 1] = '\0';
        }
        
        parsedInput(comando, comandoSeparado, ' ');
        
        //identificar se o comando eh build in
        // se nao for cria filho
        if(!ehBuildin(comandoSeparado[0])){
            pid = fork();
            //if(fork()== 0){   //criando filho
            if(pid == 0){   //criando filho
                //printf("valor do if: %d\n", sigsetjmp(env, 1));
                
                //jump_active = 1;
                exec_nao_buildin(comandoSeparado);//posso fechar?
            }
            else //processo pai esperando filho retornar
            {
                adicionaJobs(novoJob, pid, modo, comando);
                if (modo == FOREGROUND_EXECUTION)
                {
                    waitpid(pid, &status, 0);
                }
                //else{
                //
                //}    
            }
        }

        else{
            exec_buildin(comandoSeparado);
        }
    }
    
    return 0;
}