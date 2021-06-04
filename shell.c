#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h>
#include<signal.h>
#include<sys/wait.h>

char *getcwd(char *buf, size_t size);

#define bufferLimite 200



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

void ls(char *tipo, char *diretorio)
{
    //if(fork ==0)
    //{
        if(tipo == NULL)
        {
            //faz ls do diretorio x
            //tipo = "";
            //execl("/usr/bin/ls", "ls", diretorio, NULL);
            execl("/usr/bin/ls", "/usr/bin/ls", diretorio, NULL);
            
            
            return;//teoricamente nunca chega aqui
        }
        else
        {
            //faz o ls de tipo t no diretorio x
            execl("/usr/bin/ls", "ls", tipo, diretorio, NULL);
            
            exit(0);
            return;//teoricamente nunca chega aqui
        }
    //}
    //else 
    //    wait(NULL);
        return;
}


void identificaComando(char **comando)//identifica o comando e redireciona para o proprio;
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

    if(strcmp(comando[0], "ls") == 0)//executa o comando ls //ls n eh build-in 
    {                                                       //deveria ficar junto?
        char cwd[bufferLimite];                 //pra dar ls tem q ser o filho
                                                //mas n deve ficar junto dos built-in
        if (comando[1]==NULL) //ls do proprio diretorio
        {
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                ls(NULL, cwd);
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
                    ls(comando[1], cwd); //comando[1] - tipo do ls do propio diretorio
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
                ls(comando[1], comando[2]); //comando[1] - tipo do ls   comando[2] - diretorio
                return;//teoricamente nunca chegamos nesse return
            }
        }
        else  //ls do diretorio x
        { 
            ls(NULL, comando[1]);
            return;//teoricamente nunca chegamos nesse return
        }
    }

}


int ehBuildin(char *comando) //valida se o comando eh buildin ou nao
{
    char **comandobuildin = (char**) malloc(6);
    for(int i=0;i<bufferLimite;i++)
    {
        comandobuildin[i]=(char*) malloc(6);
    }
    comandobuildin[0]="cd";
    comandobuildin[1]="jobs";
    comandobuildin[2]="fg";
    comandobuildin[3]="bg";
    comandobuildin[4]="exit";
    comandobuildin[5]="clear";
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
            
            if(fork()== 0)//criando filho
                identificaComando(comandoSeparado);
            
            else //processo pai esperando filho retornar
                wait(NULL);
        }

        else{
            identificaComando(comandoSeparado);
        }
    }
    
    return 0;
}