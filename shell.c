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

const char *STATUS_STRING[] = { "Executando", "Parado", "Concluido", "Terminado"};//implementar o terminado depois
const char *COMANDOBUILDIN[] = { "cd", "jobs", "fg", "bg", "exit", "clear"};

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
    int cont;
}jobs;

int jobsexistentes=0;
jobs* jobscriados;



void adicionaJobs(jobs job, pid_t pid_process, int modo, char* nome)
{
    
    
    job.id = jobsexistentes+1;

    strcpy(job.nome, nome);

    strcpy(job.status, STATUS_STRING[0]);

    job.pid_process = pid_process;
    
    job.modo = modo;

    job.cont =0;

    jobscriados[jobsexistentes] = job;

    jobsexistentes++;
    return;
}

void suspendeJobs(pid_t pid_process)
{
    for(int i =0;i<=jobsexistentes;i++)
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

void terminaJobs(pid_t pid_process)
{
    for(int i=0; i<=jobsexistentes; i++)
    {
        if(jobscriados[i].pid_process == pid_process)
        {
            strcpy(jobscriados[i].status, STATUS_STRING[2]);//seta como terminado
            //kill(pid_process, SIGSTOP); //Faz o processo parar
            break;
        }
    }
    return;
}

void eliminaJobs(pid_t pid_process) //seta o status do job como terminado
{
    for(int i =0; i<= jobsexistentes;i++)
    {
        if(jobscriados[i].pid_process == pid_process){
            strcpy(jobscriados[i].status, STATUS_STRING[3]);// seta como terminado
            if (jobscriados[i].modo==BACKGROUND_EXECUTION){
                printf("[%d] \t%s \t%s\n", jobscriados[i].id, jobscriados[i].status, jobscriados[i].nome);
            }
            kill(pid_process, SIGKILL); //
            break;
        }
    }
    return;
}

void listjobs()//printa os jobs que ainda não foram terminados
{ 
    for(int i=0;i<jobsexistentes;i++)
    {
        if(jobscriados[i].cont == 0 && jobscriados[i].id !=0)
        {
            printf("[%d] \t%s \t%s \tpid:%d\n", jobscriados[i].id, jobscriados[i].status, jobscriados[i].nome, jobscriados[i].pid_process);
            if(strcmp(jobscriados[i].status, STATUS_STRING[2])==0 || strcmp(jobscriados[i].status, STATUS_STRING[3])==0)//certifica de printar uma unica vez o terminado dps q ele acaba
                jobscriados[i].cont++;
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
    if(caminho == NULL)
    {
        printf("argumento esperado para cd\n");
        return;
    }
    else
    {
        if(chdir(caminho) != 0) 
        {
            perror("erro ao tentar mudar de diretorio\n");
            return;
        }
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
    fflush(stdout);
    if (execvp(comando[0], comando) == -1)
    {
        perror("Error");
        _exit;
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
    //char *comandobuildin[] = { "cd", "jobs", "fg", "bg", "exit", "clear"};
    
    for(int i=0 ; i< 6;i++)
    {
        if(strcmp(comando, COMANDOBUILDIN[i])==0)
        {
            return 1; // achou comando build in
        }
    }
    return 0; // nao achou um comando build in
}

void sig_handler(int sig) {
    
    
    if (sig == SIGINT)
    {
        
        eliminaJobs(pid); //termina o ultimo processo a ser ativado
        if (!jump_active) {
        
            return;
        }
        siglongjmp(env, 42);
        
        return;
    }
    if (sig ==SIGTSTP){
        
        suspendeJobs(pid); //uma funcao que suspenderia o filho
        
        if (!jump_active) { 
        
            return;
        }
        siglongjmp(env, 42);
        
        return;
    }
    if (sig == SIGCHLD){
        pid_t outro_pid;
        int status;
        
        
        while((outro_pid=waitpid(-1, &status, WNOHANG))>0){
            
            if(WIFEXITED(status))//se o filho terminou normalmente? deu exit veio pra cá
            {
                
                terminaJobs(outro_pid);
            }
            else if(WIFSIGNALED(status))//checar se eh control c 
            {
            
                
                eliminaJobs(outro_pid);
            }
            else if(WIFSTOPPED(status))
            {
            
                suspendeJobs(outro_pid);
            }
            
        }
        
    }

}

void crprocess_to_bg (){
    for(int i =0;i<=jobsexistentes;i++)
    {
        if(jobscriados[i].pid_process == pid)
        {
            jobscriados[i].modo = BACKGROUND_EXECUTION;//seta como background
            strcpy(jobscriados[i].status, STATUS_STRING[0]);//seta como executando
            break;
        }
    }
}

void process_to_bg (char* num_process){
    for(int i =0;i<=jobsexistentes;i++)
    {
        if(jobscriados[i].id == atoi(num_process) || strcmp(jobscriados[i].nome, num_process))
        {
            jobscriados[i].modo = BACKGROUND_EXECUTION;//seta como background
            strcpy(jobscriados[i].status, STATUS_STRING[0]);//seta como executando
            break;
        }
    }
}

void bg (char * num_process){
    if (num_process == NULL){
        crprocess_to_bg();

    }
    else{
        process_to_bg(num_process);

    }
}

int main()
{   
    

    jobscriados = (jobs *)malloc(20*sizeof(jobs));
    jobs novoJob;
    
    int status;

    __fpurge(stdin);
    limpaTerminal();

    char *comando= (char *) malloc(BUFFERLIMITE*sizeof(char));    
    char **caminho = (char **) malloc(BUFFERLIMITE*sizeof(char *));
    char **comandoSeparado =(char **) malloc(BUFFERLIMITE*sizeof(char *));
    for(int i=0; i<BUFFERLIMITE; i++) //alocaçao caminho
    {
        caminho[i]= (char*) malloc(BUFFERLIMITE*sizeof(char));
    }
    for(int i=0; i<BUFFERLIMITE; i++) //alocaçao comandoSeparado
    {
        comandoSeparado[i]= (char*) malloc(BUFFERLIMITE*sizeof(char));
    }
    
    if(caminho == NULL)return -1;
    if(comandoSeparado == NULL)return -1;


    caminho[0][0]= '~';
    caminho[1]=NULL;
    //setpgid();
    
    signal(SIGINT, sig_handler);//control c tratado na main
    signal(SIGTSTP, sig_handler);//control z tratado na main
    signal(SIGCHLD, sig_handler);// trata o termino de um filho
    
    
    while(1) //enquanto a concha existe ele está rodando
    {
        int modo = FOREGROUND_EXECUTION;
        if (sigsetjmp(env, 1) == 42)
        {
            printf("\n");
            continue;
        }
        jump_active = 1;
        printaCaminho();
        
        leInput(comando);
        
        if (comando[strlen(comando) - 1] == '&') 
        {
            modo = BACKGROUND_EXECUTION;
            comando[strlen(comando) - 1] = '\0';
        }
        
        parsedInput(comando, comandoSeparado, ' ');
        
        //identificar se o comando eh build in
        // se nao for cria filho
        if(!ehBuildin(comandoSeparado[0])){
            fflush(stdout);
            pid = fork();
            
            if(pid == 0) //se filho 
            {
                exec_nao_buildin(comandoSeparado);//posso fechar?
            }
            else //processo pai esperando filho retornar
            {
                adicionaJobs(novoJob, pid, modo, comando);
                if (modo == FOREGROUND_EXECUTION)
                {
                    waitpid(pid, &status, 0);
                    eliminaJobs(pid);
                }
                else{ 
                    setpgid(pid, BACKGROUND_EXECUTION); 
                    printf("[%d] \t%d\n", jobscriados[jobsexistentes-1].id, jobscriados[jobsexistentes-1].pid_process);
                    
                }
            }
        }
        else{
            exec_buildin(comandoSeparado);
        }
    }
    
    return 0;
}