// bibliotecas
#include <stdio.h> // entrada e saída
#include <stdlib.h> // manipulação de memória e conversão de tipos
#include <unistd.h> // maioria das funções relacionadas ao SO (incluindo fork() e exec())
#include <sys/types.h> // inclui o tipo pid de variável
#include <sys/wait.h> // funções e constantes relacionadas ao wait()
#include <string.h> // tratamento de strings
#include <signal.h> // tratamento de sinais

// função para execução de programas em foreground
void foreground(char *prog, char *parameters[]){
  
  // execução do fork (criando processo filho)
  pid_t pid = fork();

  // checando execução do fork
  if (pid == -1){  
    perror("Erro na execucao do fork, processo filho nao foi criado.\n"); 
    exit(EXIT_FAILURE); 
  }
  
  if (pid == 0) {  // processo filho 
     execvp (prog, parameters); 
     perror("Erro na execucao do execvp no processo filho.\n");
    exit(EXIT_FAILURE);
  
  } else // processo pai
     waitpid(pid, NULL, 0);
}

// função para tratamento de zumbis em background

void zombie (int signal){
  
    int status;
    pid_t pid;

    // WNOHANG: impede o waitpid de bloquear o processo pai enquanto o filho estiver em execução
    // WIFEXITED: verifica se o processo finalizou e sem erros
    // WIFSIGNALED: verifica se o processo finalizou a partir de um sinal 

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("O processo de pid %d terminou normalmente\n ", pid);
          
        }
        else if (WIFSIGNALED(status)) 
            printf("O processo de pid %d terminou com o recebimento de um sinal.\n", pid);  
    }
}

// função para execução de programas em background
void background(char *prog, char *parameters[]){

  // tratando zumbis a partir do sinal SIGCHLD
  signal(SIGCHLD, zombie);
  
  // execução do fork (criando processo filho)
  pid_t pid = fork();

  // checando execução do fork
  
  if (pid == -1){  
    perror("Erro na execucao do fork, processo filho nao foi criado.\n"); 
    exit(EXIT_FAILURE); 
  }
  
  if (pid == 0) {  // processo filho 
    
     execvp (prog, parameters); 
     perror("Erro na execucao do execl no processo filho.\n");
    exit(EXIT_FAILURE);
  }
  else // processo pai
      printf("Processo de PID %d executando em background\n", pid);
}

// função principal
int main(void) {

  while(1){
    char command[100];
    char directory[30];
    
    // impressão do diretório atual e obtenção do comando
    getcwd(directory, sizeof(directory)); 
    printf("~%s$ ", directory);
    fgets(command, sizeof(command), stdin);
        
    // verificar tecla enter
    if (strcmp(command, "\n") == 0) 
          continue; 
    
    // leitura do comando
    const char delimiters[] = " ";
    char *tokens[20];
    char *token = strtok(command, delimiters);
    int wordCounter = 0; 
    int bg = 0; 
  
    // separação do comando em palavras (tokens)
    while (token != NULL) {
        token[strcspn(token, "\n")] = 0;  // tira a quebra de linha
  
        if (strcmp(token, "&") != 0) {  // ignora & na leitura dos tokens
          tokens[wordCounter] = token;
          wordCounter++;
        }
        else 
          bg = 1;
      
        token = strtok(NULL, delimiters);
    }
    tokens[wordCounter] = NULL;
  
    // checando se o comando de finalização do shell foi chamado
    if (strcmp(tokens[0], "exit") == 0)
      exit(EXIT_SUCCESS);

    // checando se o comando cd foi chamado
    if (strcmp(tokens[0], "cd") == 0) {

      // nenhum parâmetro passado: manda para home
      if (tokens[1] == NULL){
        
        // chamando o trocador de diretórios e checando se houveram erros na execução
        if (chdir(getenv("HOME")) != 0)
          perror("Erro ao transferir para a home.\n");
      }
      else {
        if ((chdir(tokens[1]) != 0))
          perror("Erro ao transferir para o diretório.\n");
      }
      continue;
    }
    
    // definindo primeiro token como o comando a ser executado
    char *prog = tokens[0];
    
    // analisando se o último token é "&" e definindo se a execução é em foreground ou background
    
    if (bg == 1) {
      background(prog, tokens);
    }
      
    else{
      foreground(prog, tokens);
    }
  }
  return 0;
}
