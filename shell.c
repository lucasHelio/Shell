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
int jump_active2 = 0;
pid_t pid_crp;

typedef struct jobs
{
    char nome[20];
    int id;
    int modo;
    char  status[20];
    pid_t pid;
    int cont;
}jobs;

int numTotalJobs=0;
jobs* jobsCriados;



//void AdicionaJobs(jobs job, pid_t pid_process, int modo, char* nome)
void AdicionaJobs(int modo, char* nome)
{
    jobs job;
    
    job.id = numTotalJobs+1;

    strcpy(job.nome, nome);

    strcpy(job.status, STATUS_STRING[0]);

    job.pid = pid_crp;
    
    job.modo = modo;

    job.cont =0;

    jobsCriados[numTotalJobs] = job;

    numTotalJobs++;
    return;
}

void SuspendeJobs(pid_t pid)
{
    for(int i =0;i<=numTotalJobs;i++)
    {
        if(jobsCriados[i].pid == pid)
        {
            strcpy(jobsCriados[i].status, STATUS_STRING[1]);//seta como suspenso
            kill(pid, SIGSTOP); //Faz o processo parar
            break;
        }
    }
    return;
}

void ConcluiJobs(pid_t pid)
{
    for(int i=0; i<=numTotalJobs; i++)
    {
        if(jobsCriados[i].pid == pid)
        {
            strcpy(jobsCriados[i].status, STATUS_STRING[2]);//seta como concluido
            //kill(pid_process, SIGSTOP); //Faz o processo parar
            break;
        }
    }
    return;
}

void EliminaJobs(pid_t pid) //seta o status do job como terminado
{
    for(int i =0; i<= numTotalJobs;i++)
    {
        if(jobsCriados[i].pid == pid){
            strcpy(jobsCriados[i].status, STATUS_STRING[3]);// seta como terminado
            if (jobsCriados[i].modo==BACKGROUND_EXECUTION){
                printf("[%d] \t%s \t%s\n", jobsCriados[i].id, jobsCriados[i].status, jobsCriados[i].nome); //OLHAR ISSO COM MAIS ATENCAO
            }
            kill(pid, SIGKILL); //
            break;
        }
    }
    return;
}

void Listjobs()//printa os jobs que ainda não foram terminados
{ 
    for(int i=0;i<numTotalJobs;i++)
    {
        if(jobsCriados[i].cont == 0 /*&& jobsCriados[i].id !=0*/)
        {
            printf("[%d] \t%s \t%s \tpid:%d\n", jobsCriados[i].id, jobsCriados[i].status, jobsCriados[i].nome, jobsCriados[i].pid);
            if(strcmp(jobsCriados[i].status, STATUS_STRING[2])==0 || strcmp(jobsCriados[i].status, STATUS_STRING[3])==0)//certifica de printar uma unica vez o terminado dps q ele acaba
                jobsCriados[i].cont++;
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
        fprintf(stderr, "Erro de alocação\n");
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

void LimpaTerminal()
{
    printf("\033[H\033[J");//limpa o terminal
}

void ParsedInput(char *comando, char ** destino, char separador) //divide os comandos dados na entrada e armazena no vetor destino
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

char * UltimoElemCaminho(char ** caminho)
{
    for (int i=0; i< BUFFERLIMITE;i++)
    {
        if(caminho[i+1] == NULL)
            return caminho[i];
    }
}

void Cd(char *caminho)
{
    if(caminho == NULL)
    {
        printf("argumento esperado para Cd\n");
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

void CrProcessToBg (){
    for(int i =0;i<=numTotalJobs;i++)
    {
        if(jobsCriados[i].pid == pid_crp)
        {
            jobsCriados[i].modo = BACKGROUND_EXECUTION;//seta como background
            setpgid(pid_crp, BACKGROUND_EXECUTION);
            strcpy(jobsCriados[i].status, STATUS_STRING[0]);//seta como executando
            kill(pid_crp, SIGCONT);
            break;
        }
    }
}




void ProcessToBg (char* index){
    for(int i =0;i<=numTotalJobs;i++)
    {
        
        if(jobsCriados[i].id == strtol(index, NULL, 10)) /*|| strcmp(jobsCriados[i].nome, index)*/
        {
            
            if(strcmp(jobsCriados[i].status, STATUS_STRING[1])==0)
            {
                jobsCriados[i].modo = BACKGROUND_EXECUTION;//seta como background
                setpgid(jobsCriados[i].pid, BACKGROUND_EXECUTION);
                strcpy(jobsCriados[i].status, STATUS_STRING[0]);//seta como executando
                kill(jobsCriados[i].pid, SIGCONT);
                return;
            }
            else
            {
                printf("Não eh possivel colocar o processo em background\n");
                return;
            }
        }
    } 
    printf("Não existe processo com esse id\n");
    return;
        
}

void Bg (char * index){
    if (index == NULL){
        CrProcessToBg();
    }
    else{
        //printf("index: %s\n", index);
        //printf("index++: %s\n", index++);
        //index++; vo ver o fg
        ProcessToBg(index+1);
    }
}

void ProcessToFg(char* index){ //aqui ele pode entrar em dois momento, se esta parado ou se esta rodando em background
    for(int i =0;i<=numTotalJobs;i++)
    {
        printf("estou verificando os casos, index: %ld\n", strtol(index, NULL, 10));
        
        if(jobsCriados[i].id == strtol(index, NULL, 10)) /*|| strcmp(jobsCriados[i].nome, index)*/
        {
            printf("entrei no if do fg, pid: %d\n", jobsCriados[i].id);
            
            if(strcmp(jobsCriados[i].status, STATUS_STRING[1])==0 || strcmp(jobsCriados[i].status, STATUS_STRING[0])==0  /*&& (jobsCriados[i].modo)== BACKGROUND_EXECUTION)*/)
            {//
                printf("estou dentro do caso aceito\n");//eu acho q quando a gente manda esse sigcont ele manda um sinal q o filho voltou
                jobsCriados[i].modo = FOREGROUND_EXECUTION;//seta como background
                setpgid(jobsCriados[i].pid, FOREGROUND_EXECUTION);
                strcpy(jobsCriados[i].status, STATUS_STRING[0]);//seta como executando
                kill(jobsCriados[i].pid, SIGCONT); //
                return;//to pensando acho q vai tter q rolar o jump
            }
            else
            {
                printf("Não eh possivel colocar o processo em foreground\n");
                return;
            }
        }
        
    }
    printf("Não existe processo com esse id\n");
    return;
}

void Fg (char* index){
    if (index == NULL){
        puts ("fg necessita de um argumento."); //precisa? precisa
    }
    else{
        
        ProcessToFg(index+1);
    }
}

void PrintaCaminho()
{
    char cwd[BUFFERLIMITE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char **parsed = (char **) malloc (sizeof(char *));
        for(int i =0; i<BUFFERLIMITE; i++)
        {
            parsed[i]= (char *) malloc(sizeof(char));
        }
        
        ParsedInput(cwd, parsed, '/');
        

        printf("\033[92mconcha:\033[34m%s\033[0m$ ",UltimoElemCaminho(parsed));
        
    }
    else  
    {
        perror("Erro ao procurar diretorio");
        return;
    }
}

void ExecNaoBuildin(char **comando)//, char *tipo, char *diretorio)
{
    fflush(stdout);
    if (execvp(comando[0], comando) == -1)
    {
        perror("Error");
        _exit;
    }
}

void ExecBuiltin(char **comando)//identifica o comando e redireciona para o proprio;
{
    if (strcmp(comando[0], "cd") == 0)//excuta o comando Cd
    {
        Cd(comando[1]);
        return;
    }
        
    if (strcmp(comando[0], "jobs")==0 ) //executa o comando jobs  //ToDo
    {
        Listjobs();
        return;
    }

    if(strcmp(comando[0], "fg")==0 )// executa o comando fg    //ToDo
    {
        Fg(comando[1]);
        return;
    }

    if(strcmp(comando[0], "bg")==0)//executa o comando Bg    //ToDo
    {
        Bg(comando[1]);
        return;
    }

    if (strcmp(comando[0], ""))
    
    if(strcmp(comando[0], "exit")==0)//executa o exit./terminal
    {
        printf("Where there is a shell, there is a way...\n\n\n\n\n\n");
        exit(0);    //hmmmmm
    }

    if(strcmp(comando[0], "clear")==0)// limpa o terminalc
    {
        LimpaTerminal();
        return;
    }

}

int EhBuiltin(char *comando) //valida se o comando eh buildin ou nao
{
    //char *comandobuildin[] = { "Cd", "jobs", "fg", "Bg", "exit", "clear"};
    
    for(int i=0 ; i< 6;i++)
    {
        if(strcmp(comando, COMANDOBUILDIN[i])==0)
        {
            return 1; // achou comando build in
        }
    }
    return 0; // nao achou um comando build in
}

void SigHandler(int sig) //tem que tratar o sigcont no handler
{
    //printf("chegou um sinal\n");
    if (sig == SIGINT)
    {
        
        EliminaJobs(pid_crp); //termina o ultimo processo a ser ativado
        if (!jump_active) {
        
            return;
        }
        siglongjmp(env, 42);
        
        return;
    }
    if (sig ==SIGTSTP){
        
        SuspendeJobs(pid_crp); //uma funcao que suspenderia o filho
        
        if (!jump_active) { 
        
            return;
        }
        siglongjmp(env, 42);
        
        return;
    }

    if (sig == SIGCHLD)
    {
        //printf("chegou um sinal sigchild\n");
        pid_t outro_pid;
        //siginfo_t child_info;
        int status;

        
        
        //temos q verificar se eh background?
        while((outro_pid=waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED))>0){//so background entra aqui?
        //while(waitid(P_ALL,0,&child_info,WEXITED|WSTOPPED)==0){
        
            //printf("entrei no while\n");
            if(WIFEXITED(status))//se o filho terminou normalmente? deu exit veio pra cá
            {
                //printf("entrou em Concluijobs\n");
                //printf("pid do conclui jobs: %d\n", outro_pid);
                ConcluiJobs(outro_pid);
                return;
            }
            else if(WIFSIGNALED(status))//checar se eh control c //checar pq ele deu terminado num processo foreground que terminou normalmente
            {
                //printf("entrou em Eliminajobs\n");
                //if()//foi control c usa elimina jobs
                //printf("pid do elimina jobs: %d\n", outro_pid);
                EliminaJobs(outro_pid);
                //else //usa termina?
                return;
            }
            else if(WIFSTOPPED(status))
            {
            
                SuspendeJobs(outro_pid);
                return;
            }
            else if(WIFCONTINUED(status))//um filho foi continuado
            {
                //signal(SIGTTOU, SIG_IGN);
                //tcsetpgrp(0, getpid());
                //signal(SIGTTOU, SIG_DFL);
                if (!jump_active) { 
                    return;
                }
                siglongjmp(env, 60);
                
                return;
            }
            
        }
        
        ConcluiJobs(pid_crp);
        return;
    }

}

int main()
{   
    

    jobsCriados = (jobs *)malloc(20*sizeof(jobs));
    jobs novoJob;
    
    int status;

    __fpurge(stdin);
    LimpaTerminal();

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
    
    signal(SIGINT, SigHandler);//control c tratado na main
    signal(SIGTSTP, SigHandler);//control z tratado na main
    signal(SIGCHLD, SigHandler);// trata o termino de um filho
    
    
    while(1) //enquanto a concha existe ele está rodando
    {
        int modo = FOREGROUND_EXECUTION;
        if (sigsetjmp(env, 1) == 42)
        {
            printf("\n");
            continue;
        }
        jump_active = 1;
        PrintaCaminho();
        
        leInput(comando);
        
        if (comando[strlen(comando) - 1] == '&') 
        {
            modo = BACKGROUND_EXECUTION;
            comando[strlen(comando) - 1] = '\0';
        }
        
        ParsedInput(comando, comandoSeparado, ' ');
        
        //identificar se o comando eh build in
        // se nao for cria filho
        if(!EhBuiltin(comandoSeparado[0])){
            fflush(stdout);
            pid_crp = fork();
            
            if(pid_crp == 0) //se filho 
            {
                
                ExecNaoBuildin(comandoSeparado);//posso fechar?
            }
            else //processo pai esperando filho retornard: 0
            //index: 0//de onde veio isso???
            {
                
                //AdicionaJobs(novoJob, pid, modo, comando);
                AdicionaJobs(modo, comando);
                if (modo == FOREGROUND_EXECUTION)
                {
                    setpgid(pid_crp, FOREGROUND_EXECUTION);
                    //jumpa pra ca
                    
                    waitpid(pid_crp, &status, 0);
                    //ConcluiJobs(pid_crp);
                }
                else{ 
                    setpgid(pid_crp, BACKGROUND_EXECUTION); 
                    printf("[%d] \t%d\n", jobsCriados[numTotalJobs-1].id, jobsCriados[numTotalJobs-1].pid);
                    
                }
            }
        }
        else{
            ExecBuiltin(comandoSeparado);
        }
    }
    
    return 0;
}