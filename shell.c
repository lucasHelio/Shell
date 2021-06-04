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
    }
    else 
    {
        chdir(caminho); 
        //kill(0,SIGTERM);
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
        if(tipo == NULL)
        {
            //faz ls do diretorio x
            //tipo = "";
            execl("/usr/bin/ls", "ls", diretorio, NULL);
            
            return;//teoricamente nunca chega aqui
        }
        else
        {
            //faz o ls de tipo t no diretorio x
            execl("/usr/bin/ls", "ls", tipo, diretorio, NULL);
            
            
            return;//teoricamente nunca chega aqui
        }
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
    
    
    if(strcmp(comando[0], "ls") == 0)//executa o comando ls
    {
        char cwd[bufferLimite];
        
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

        //char cwd[bufferLimite];
        
        printaCaminho();
        
        leInput(comando);
        parsedInput(comando, comandoSeparado, ' ');
        if(fork()== 0){
            identificaComando(comandoSeparado);
        }
        else{
            wait(NULL);
        }
        
        
        
    }
    
    return 0;
}