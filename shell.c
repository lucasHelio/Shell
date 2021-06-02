#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h>
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
        //char cwd[bufferLimite];
        //getcwd(cwd, sizeof(cwd));
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


void identificaComando(char **comando, char ** caminho)//identifica o comando e redireciona para o proprio;
{
    if (strcmp(comando[0], "cd") == 0)//excuta o comando cd
    {
        cd(comando[1]);
        return;
    }
        
    if (strcmp(comando[0], "jobs")==0 ) //executa o comando jobs
    {
        return;
    }

    if(strcmp(comando[0], "fg")==0 )// executa o comando fg
    {
        return;
    }

    if(strcmp(comando[0], "bg")==0)//executa o comando bg
    {
        return;
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
    
    //printaCaminho();
    while(1) //enquanto a concha existe ele está rodando
    {
        
        
        char cwd[bufferLimite];
        //*caminho = cwd;
        
        //if (getcwd(cwd, sizeof(cwd)) != NULL) {
        
        //printf("\033[92mconcha:\033[34m%s\033[0m$ ", cwd);
        printaCaminho();
        //printf("\033[92mconcha:\033[34m%s\033[0m$ ", ultimoElemCaminho(caminho));
        //}
        //else {
        //    perror("Erro ao procurar diretorio");
        //    return 1;
        //}
        leInput(comando);
        parsedInput(comando, comandoSeparado, ' ');



        identificaComando(comandoSeparado, caminho);
        //printf("caminho 1: %s\n", caminho[0]);
        //printf("caminho 2: %s\n", caminho[1]);
        //printf("caminho 3: %s\n", caminho[2]);
        
    }
    
    return 0;
}