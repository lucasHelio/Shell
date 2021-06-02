#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h> 

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









void identificaComando(char **comando, char ** caminho)//identifica o comando e redireciona para o proprio;
{
    if (strcmp(comando[0], "cd") == 0)//excuta o comando cd
    {
        if(chdir(comando[1]) != 0) {
            perror("erro ao tentar mudar de diretorio\n");
        }
        else 
        {
            
            parsedInput(comando[1], caminho, '/');
            char *palavratemp;
            char *novocaminho;
            int i=0;
            while(caminho[i] != NULL)
            {
                
                if(strcmp(caminho[i],".." )==0)
                {
                    novocaminho = caminho[i-2];
                }
                i++;
            }
        }
        
        //debug print
        //for(int i=0; i<bufferLimite;i++)
        //{
        //printf("caminho %d: %s\n", i, caminho[i]);
        //}     
        
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

char * ultimoElemCaminho(char ** caminho)
{
    for (int i=0; i< bufferLimite;i++)
    {
        if(caminho[i+1] == NULL)
            return caminho[i];
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
    //strcpy(caminho[0], "~");
    //caminho[0][0] = '~';
    //caminho[1]= NULL;
    caminho[0][0] = '/';
    caminho[1]="/home";
    caminho[2][0]='~';
    caminho[3]= NULL;
    char *comando= (char *) malloc(bufferLimite*sizeof(char));
    
    //alocaMem1D(comando);
    //alocaMem2D(caminho);
    
    while(1) //enquanto a concha existe ele está rodando
    {
        
        //ultimoElemCaminho(caminho);
        //printf("\033[34mThis is yellow\033[0m");//azul
        //printf("\033[92mThis is yellow\033[0m");//verde
        
        printf("\033[92mconcha:\033[34m%s\033[0m$ ", ultimoElemCaminho(caminho));
        //printf("concha:~%s$ ", caminho[1]);
        leInput(comando);
        char **comandoSeparado = malloc(8*sizeof(char *));
        //alocaMem2D(comandoSeparado);

        parsedInput(comando, comandoSeparado, ' ');
        
        identificaComando(comandoSeparado, caminho);
    
        //printf("%s\n", comandoSeparado[1]);
                
        
        //break;
        
    }
    
    return 0;
}