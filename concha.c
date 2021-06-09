#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include <sys/types.h>
#include<setjmp.h>

/* Cores */
#define COR_NEUTRA "\033[0m"
#define COR_VERDE "\033[92m"
#define COR_AZUL "\033[34m"

/* Grupos de processos */
#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1

/* Constantes definidas */
#define BUFFERLIMITE 200
#define NUMCOMBUILTIN 8

/* Arrays com Status possiveis e Comandos built-in reconhecidos */
const char *STATUS_STRING[] = { "Executando", "Parado", "Concluido", "Terminado"};
const char *COMANDOBUILTIN[] = { "cd", "jobs", "fg", "bg", "exit", "clear", "kill", "help"};

/* Struct de um Job */
typedef struct Jobs
{
    char nome[20]; //nome do processo
    int id;         //id do processo
    int modo;       //modo foreground
    char  status[20];
    pid_t pid;
    int flag;       //para saber se um processo terminado ou concluido ja foi printado
}Jobs;

/* Variáveis Globais */
Jobs* jobsCriados;
pid_t g_pid_crp; //pid do processo atual
static jmp_buf g_env;
int g_jump_active = 0;
int g_numTotalJobs=0;


/**
 * Imprime todos os jobs não terminados ou recentemente 
 * terminados no terminal.
*/
void Listjobs()
{ 
    for(int i=0;i<g_numTotalJobs;i++)
    {
        if(jobsCriados[i].flag == 0)
        {
            printf("[%d] \t%s \t%s \tpid:%d\n", jobsCriados[i].id, jobsCriados[i].status, jobsCriados[i].nome, jobsCriados[i].pid);
            if(strcmp(jobsCriados[i].status, STATUS_STRING[2])==0 || strcmp(jobsCriados[i].status, STATUS_STRING[3])==0) //certifica de printar uma unica vez o terminado depois que ele acaba
                jobsCriados[i].flag++;
        }
    }
    return;
}

/**
 * Função para criar e adicionar um novo Job na lista 
 * global de Jobs.
 *
 * @param modo definição se é BACKGROUND | FOREGROUND
 * @param nome nome do processo que será adicionado
*/
void AdicionaJobs(int modo, char* nome)
{
    Jobs job;
    
    strcpy(job.nome, nome);
    strcpy(job.status, STATUS_STRING[0]);
    job.id = g_numTotalJobs+1;
    job.pid = g_pid_crp;
    job.modo = modo;
    job.flag = 0;
    jobsCriados[g_numTotalJobs] = job;
    g_numTotalJobs++;
    
    return;
}
 
/**
 * Função que elimina um job da lista de jobs, mudando seu
 * status (para Terminado) e matando o processo.
 * 
 * É chamada quando ocorre um ctrl+C  ou um comando kill.
 *
 * @param pid id do processo que será eliminado 
*/
void EliminaJobs(pid_t pid)
{
    for(int i =0; i<= g_numTotalJobs;i++)
    {
        if(jobsCriados[i].pid == pid){
            strcpy(jobsCriados[i].status, STATUS_STRING[3]);// seta como Terminado
            if (jobsCriados[i].modo==BACKGROUND_EXECUTION){
                printf("[%d] \t%s \t%s\n", jobsCriados[i].id, jobsCriados[i].status, jobsCriados[i].nome);
                jobsCriados[i].flag=1;
            }
            kill(pid, SIGKILL); // manda o sinal para matar o processo
            break;
        }
    }
    return;
}
 
 /**
  * Função que supende um job, mudando o status do processo
  * para Parado.
  * 
  * É chamada quando ocorre um ctrl+z.
  * 
  * @param pid id do processo que será parado.
  */
void SuspendeJobs(pid_t pid)
{
    for(int i =0;i<=g_numTotalJobs;i++)
    {
        if(jobsCriados[i].pid == pid)
        {
            strcpy(jobsCriados[i].status, STATUS_STRING[1]); //seta como Parado
            kill(pid, SIGSTOP); //Manda um sinal para o processo entrar em suspensão
            break;
        }
    }
    return;
}

/**
 * Função que conclui um job, mudando o status do processo 
 * para Concluído.
 * 
 * É chamada quando um processo termina naturalmente.
 * 
 * @param pid id do processo que será concluido
 */
void ConcluiJobs(pid_t pid)
{
    for(int i=0; i<=g_numTotalJobs; i++)
    {
        if(jobsCriados[i].pid == pid)
        {
            strcpy(jobsCriados[i].status, STATUS_STRING[2]); //seta como Concluido
            break;
        }
    }
    return;
}

/**
 * Função que implementa o comando cd do terminal.
 * É chamada quando o usuário digita cd.
 * 
 * @param caminho caminho para o diretório escolhido
 */
void Cd(char *caminho)
{
    if(caminho == NULL) //caso o usuário não tenha digitado um caminho
    {
        printf("o comando necessita argumentos\n");
        return;
    }
    else
    {
        if(chdir(caminho) != 0) //checa se a mudança de diretório foi bem sucedida.
        {
            perror("erro ao tentar mudar de diretorio\n");
            return;
        }
    }
    return;
}

/**
 * Função que coloca um processo em FOREGROUND, este 
 * podendo estar parado ou rodando em BACKGROUND.
 * 
 * É chamada quando o usuário chama fg corretamente.
 * 
 * @param index indicie do processo escolhido, pode ser 
 * verificado com o comando jobs.
 */
void ProcessToFg(char* index){
    for(int i =0;i<=g_numTotalJobs;i++)
    {
        if(jobsCriados[i].id == strtol(index, NULL, 10))
        {
            if(strcmp(jobsCriados[i].status, STATUS_STRING[1])==0 || strcmp(jobsCriados[i].status, STATUS_STRING[0])==0)
            {   //entra aqui quando status for igual a "Executando",pois pode estar em BACKGROUND, ou "Parado"
            
                jobsCriados[i].modo = FOREGROUND_EXECUTION;
                setpgid(jobsCriados[i].pid, FOREGROUND_EXECUTION); //muda o grupo do processo
                strcpy(jobsCriados[i].status, STATUS_STRING[0]);
                kill(jobsCriados[i].pid, SIGCONT);
                
                return;
            }
            else //caso o processo esteja concluido ou terminado
            {
                printf("Não eh possivel colocar o processo em foreground\n");
                return;
            }
        } 
    }
    printf("Não existe processo com esse id\n");
    return;
}

/**
 * Função que chama ProcessToFg caso o usuário tenha 
 * passado argumentos na chamada do comando.
 * 
 * É chamada quando o usuário digita fg.
 * 
 * @param index indice do processo a ser colocado em 
 * FOREGROUND, pode ser verificado com o comando jobs.
 */ 
void Fg (char* index){
    if (index == NULL){
        puts ("fg necessita de um argumento. \n");
    }
    else{    
        ProcessToFg(index+1);
    }
}

/**
 * Função que coloca o processo atual em BACKGROUND.
 * É chamada quando o usuário chama bg sem argumentos.
 */
void CrProcessToBg (){
    for(int i =0;i<=g_numTotalJobs;i++)
    {
        if(jobsCriados[i].pid == g_pid_crp)
        {
            jobsCriados[i].modo = BACKGROUND_EXECUTION; //seta como background
            setpgid(g_pid_crp, BACKGROUND_EXECUTION);
            strcpy(jobsCriados[i].status, STATUS_STRING[0]); //seta como executando
            kill(g_pid_crp, SIGCONT); // Manda sinal ao processo para continuar a execução
            break;
        }
    }
}

/**
 * Função que coloca o processo em BACKGROUND.
 * É chamada quando o usuário chama bg com argumentos.
 * 
 * @param index indice do processo a ser colocado no
 * BACKGROUND
 */ 
void ProcessToBg (char* index){
    for(int i =0;i<=g_numTotalJobs;i++)
    {
        
        if(jobsCriados[i].id == strtol(index, NULL, 10))
        {
            
            if(strcmp(jobsCriados[i].status, STATUS_STRING[1])==0) //checa se o processo esta parado
            {
                jobsCriados[i].modo = BACKGROUND_EXECUTION; //seta como background
                setpgid(jobsCriados[i].pid, BACKGROUND_EXECUTION);
                strcpy(jobsCriados[i].status, STATUS_STRING[0]); //seta como executando
                kill(jobsCriados[i].pid, SIGCONT); //manda sinal para o processo continuar a execução
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

/**
 * Função que chama a função respectiva a quantidade de
 * argumentos que o usuário passou.
 *
 * É chamada quando o usuário digita bg.
 * 
 * @param index indice do processo a ser colocado 
 * em background, pode ser verificado com o comando jobs.
 */ 
void Bg (char * index){
    if (index == NULL){ //caso o usuário não tenha passado argumentos ele esta se referenciando ao processo atual.
        CrProcessToBg();
    }
    else{
        ProcessToBg(index+1);
    }
}

/**
 * Funcão que limpa o terminal
 * É chamada quando o usuário digita clear.
 */ 
void Clear()
{
    printf("\033[H\033[J");
}

/**
 * Função que mata um processo escolhido pelo usuário caso
 * este não tenha sido terminado ou concluído.
 *
 * É chamada quando o usuário digita kill.
 * 
 * @param index indice do processo a ser matado, pode 
 * ser verificado com o comando jobs.
 */ 
void Kill(char *index)
{
    if (index == NULL){
        printf("kill necessita de um argumento. \n");
    }
    else 
    {
        for(int i=0;i<g_numTotalJobs;i++)
        {
            if(jobsCriados[i].id == strtol(index+1, NULL, 10))
            {
                if(strcmp(jobsCriados[i].status,STATUS_STRING[2]) !=0 && strcmp(jobsCriados[i].status, STATUS_STRING[3])!= 0)
                {
                    EliminaJobs(jobsCriados[i].pid);
                    return;
                }
                else{
                    printf("O processo já foi Concluido/Terminado\n");
                    return;
                }
                
            }
        }
        printf("O index informado não é válido\n");
        return; 
    }
}

/**
 * Função que lista os comandos built-in disponiveis e 
 * apresenta algumas informações.
 */ 
void Help(){
    int i;
    printf(COR_VERDE "\n\n\tConcha" COR_NEUTRA"\n\n\t\t\tDesenvolvido por : Victoria Almeida e Lucas Helio\n\n\n");
    printf("\tPara executar programas digite ./nome_do_programa argumento, e pressione enter.\n\n\n");
    printf("\tA seguir a lista de comandos built in:\n\n");

    for (i = 0; i < NUMCOMBUILTIN; i++) {
        printf("\t\t\t-%s\n\n", COMANDOBUILTIN[i]);
    }

    printf("\tUse o comando man para informações de outros comandos.\n\n");
    return ;
}

/**
 * Função que valida se o comando digitado pelo usuário é 
 * builtin.
 * 
 * Retornando 1 caso ele seja, e 0 caso não seja.
 * 
 * @param comando comando que o usuário digitou.
 * 
 * @return 1 caso seja um comando built in e 0 caso 
 * não seja.
 */ 
int EhBuiltin(char *comando) 
{
    for(int i=0 ; i< NUMCOMBUILTIN;i++)
    {
        if(strcmp(comando, COMANDOBUILTIN[i])==0)
        {
            return 1; // achou comando built in
        }
    }
    return 0; // nao achou um comando built in
}

/**
 * Função que determina qual comando built in o usuário 
 * digitou chamando sua respectiva função.
 * 
 * @param comando comando built-in digitado pelo usuário.
 */ 
void ExecBuiltin(char **comando)
{
    if (strcmp(comando[0], "cd") == 0) //excuta o comando Cd
    {
        Cd(comando[1]);
        return;
    }
        
    if (strcmp(comando[0], "jobs")==0 ) //executa o comando jobs
    {
        Listjobs();
        return;
    }

    if(strcmp(comando[0], "fg")==0 ) // executa o comando fg
    {
        Fg(comando[1]);
        return;
    }

    if(strcmp(comando[0], "bg")==0) //executa o comando bg  
    {
        Bg(comando[1]);
        return;
    }
    
    if(strcmp(comando[0], "exit")==0) //executa o comando exit
    {
        printf("Where there is a shell, there is a way...\n\n\n\n\n\n");
        exit(0);    //hmmmmm
    }

    if(strcmp(comando[0], "clear")==0)// limpa o terminalc
    {
        Clear();
        return;
    }
    if(strcmp(comando[0], "kill")==0)// limpa o terminalc
    {
        Kill(comando[1]);
        return;
    }
    if(strcmp(comando[0], "help")==0)// limpa o terminalc
    {
        Help();
        return;
    }

}

/**
 * Função que executa o comando não built in digitado pelo 
 * usuário.
 * 
 * @param comando comando não built-in que o usuário 
 * digitou.
 */ 
void ExecNaoBuildin(char **comando)
{
    fflush(stdout);
    if (execvp(comando[0], comando) == -1) //checa se o execvp falhou
    {
        perror("Erro ao executar o comando");
        exit(EXIT_FAILURE);
    }
}

/**
 * Função que recebe um sinal do filho ou do usuáe 
 * trata de acordo com o esperado.
 * 
 * @param sig sinal recebido.
 */ 
void SigHandler(int sig) 
{
    if (sig == SIGINT) //caso o usuário tenha digitado ctrl+c
    {
        EliminaJobs(g_pid_crp); //termina o processo filho que estava em foreground
        if (!g_jump_active) //volta ao começo do loop principal do terminal
            return;
        siglongjmp(g_env, 42);   
        return;
    }
    if (sig ==SIGTSTP) //caso o usuário tenha digitado ctrl+z
    {    
        SuspendeJobs(g_pid_crp); //suspende o processo filho que estava em foreground
        if (!g_jump_active) //volta ao começo do loop principal do terminal
            return;
        siglongjmp(g_env, 42);
        return;
    }

    if (sig == SIGCHLD) //caso algum filho tenha enviado um sinal ao pai
    {
        pid_t child_pid;
        int status;

        while((child_pid=waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED))>0){
            
            if(WIFEXITED(status)) //caso um filho tenha terminado normalmente
            {
                ConcluiJobs(child_pid);
                return;
            }
            else if(WIFSIGNALED(status)) //caso um filho tenha terminado com um sinal 
            {
                //EliminaJobs(child_pid);
                return;
            }
            else if(WIFSTOPPED(status)) //caso um filho tenha parado
            {
                //SuspendeJobs(child_pid);
                return;
            }
            else if(WIFCONTINUED(status)) //caso um filho tenha continuado
            {
                //signal(SIGTTOU, SIG_IGN);
                //tcsetpgrp(0, getpid());
                //signal(SIGTTOU, SIG_DFL);
                
                return;
            }    
        }
        
        ConcluiJobs(g_pid_crp);
        return;
    }

}

/**
 * Função que le e armazena a entrada do usuário.
 * 
 * @param input endereço de onde a entrada do usuário 
 * será armazenado.
 */ 
void leInput(char *input)
{
    __fpurge(stdin);
    int bufsize = BUFFERLIMITE;
    int position = 0;
    int c;

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            input[position] = '\0';
            return;
        } else {
            input[position] = c;
        }
        position++;

        if (position >= bufsize) { //caso a entrada exceda o limite do buffer
            bufsize += BUFFERLIMITE;
            input = realloc(input, bufsize);
            if (!input) {
                fprintf(stderr, "concha: erro de alocção\n");
                exit(1);
            }
        }
    }
    return;
}

/**
 * Função que quebra a entrada do usuário em partes 
 * determinadas pelo espaço.
 * 
 * @param comando string que será parseada
 * @param destino destino para colocar a string parseada 
 * @param separador separador escolhido para parsear a string
 */ 
void ParsedInput(char *comando, char ** destino, char separador) 
{
    char *parsed;
    int index =0;
    
    parsed = strtok(comando, &separador);
    
    while(parsed != NULL)  
    {   
        destino[index] = parsed;
        
        index++;
        parsed = strtok(NULL,&separador);
    }
    destino[index] = NULL;
    
    return;
}

/**
 * Função que retorna o ultimo diretorio do caminho.
 * 
 * @param caminho ponteiro de array contendo todos diretorios
 * do caminho.
 */ 
char * UltimoElemCaminho(char ** caminho)
{
    for (int i=0; i< BUFFERLIMITE;i++)
    {
        if(caminho[i+1] == NULL)
            return caminho[i];
    }
}

/**
 * Função que imprime o caminho de onde o usuário está.
 */
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
        

        printf(COR_VERDE "Concha:" COR_AZUL "%s" COR_NEUTRA"$ ",UltimoElemCaminho(parsed));
        
    }
    else //caso tenha ocorrido uma falha na chamada do getcwd 
    {
        perror("Erro ao procurar diretorio");
        return;
    }
}


int main()
{   
    

    jobsCriados = (Jobs *)malloc(20*sizeof(Jobs));
    Jobs novoJob;
    
    int status;

    __fpurge(stdin);
    Clear();

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
    if (comando == NULL)return-1;
    if(comandoSeparado == NULL)return -1;
    

    caminho[0][0]= '~';
    caminho[1]=NULL;

    
    signal(SIGINT, SigHandler);//control c tratado na main
    signal(SIGTSTP, SigHandler);//control z tratado na main
    signal(SIGCHLD, SigHandler);// trata o termino de um filho
    
    
    while(1) //enquanto a concha existe ele está rodando
    {
        int modo = FOREGROUND_EXECUTION;
        if (sigsetjmp(g_env, 1) == 42)
        {
            printf("\n");
            continue;
        }
        g_jump_active = 1;
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
            g_pid_crp = fork();
            
            if(g_pid_crp == 0) //se filho 
            {
                
                ExecNaoBuildin(comandoSeparado);//posso fechar?
            }
            else //processo pai esperando filho retornard: 0
            
            {
                AdicionaJobs(modo, comando);
                if (modo == FOREGROUND_EXECUTION)
                {
                    setpgid(g_pid_crp, FOREGROUND_EXECUTION);
                    waitpid(g_pid_crp, &status, 0);
                }
                else{ 
                    setpgid(g_pid_crp, BACKGROUND_EXECUTION); 
                    printf("[%d] \t%d\n", jobsCriados[g_numTotalJobs-1].id, jobsCriados[g_numTotalJobs-1].pid);
                }
            }
        }
        else{
            ExecBuiltin(comandoSeparado);
        }
    }
    return 0;
}