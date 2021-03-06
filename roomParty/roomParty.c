#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Problema de Room Party, seção 7.3

int studentsInRoom = 0;
int deanInRoom = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t deanLeft = PTHREAD_COND_INITIALIZER;
pthread_cond_t studentsLeft = PTHREAD_COND_INITIALIZER;

void *dean(void *arg) {
  while(1) {
    // dean tentando entrar na Room
    while(deanInRoom == 0) {
      pthread_mutex_lock(&mutex);
      if (studentsInRoom == 0 || studentsInRoom > 50) {
        deanInRoom = 1;
        printf("Dean entrou na sala pois há %d alunos na sala\n", studentsInRoom);
      }
      pthread_mutex_unlock(&mutex);
    }

    // dean expulsando galera (se necessário) e saindo da room
    pthread_mutex_lock(&mutex);
    while(studentsInRoom > 0) {
      pthread_cond_wait(&studentsLeft, &mutex);
    }
    printf("Dean saindo da sala\n");
    deanInRoom = 0;
    // Após sair, reitor avisa aos alunos esperando que ele saiu
    pthread_cond_broadcast(&deanLeft);
    pthread_mutex_unlock(&mutex);
  }
}

void *student(void *arg) {
  int insideRoom = 0;
  // Alunos ficam tentando entrar e sair da sala sempre (party = festa)
  while(1) {
    pthread_mutex_lock(&mutex);
    // enquanto aluno estiver fora da sala e o Reitor estiver dentro, ele espera ele sair para entrar
    while (insideRoom == 0 && deanInRoom) {
      printf("Aluno esperando dean sair para entrar na sala\n");
      pthread_cond_wait(&deanLeft, &mutex);
    }
    printf("Aluno entrando na sala\n");
    insideRoom = 1;
    studentsInRoom++;
    pthread_mutex_unlock(&mutex);
    // Aluno Perde o Lock após entrar na sala

    // Aluno pega o Lock novamente para sair da sala
    pthread_mutex_lock(&mutex);
    printf("Aluno saindo da sala\n");
    insideRoom = 0;
    studentsInRoom--;
    // Se o aluno for o último a sair e o reitor estiver na sala, ele avisa o reitor que não há mais estudantes na sala
    if (studentsInRoom == 0 && deanInRoom) {
      pthread_cond_signal(&studentsLeft);
    }
    pthread_mutex_unlock(&mutex);
  }
}


int main(void) {
  pthread_t students[100];
  pthread_t Dean;
  // Criando Thread para o Reitor
  pthread_create(&Dean, NULL, dean, NULL);

  for (int i = 0; i < 100; i++) {
    pthread_create(&students[i], NULL, student, NULL);
  }

  // Programa só finaliza quando terminar a execução da thread do Reitor (nunca)
  pthread_join(Dean, NULL);
  return 0;
}
