#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include <valgrind/valgrind.h>

#define NB_CTX 10

void func(int numero){
  printf("j'affiche le numéro %d\n", numero);
}

void factorielle(int n, int res){
  ucontext_t uc,previous;
  

  printf("Exécution pour la valeur %d\n",n);
  
  if (n==0)
    printf("Résultat: %d\n",res);
  else{
    
    getcontext(&uc);
    uc.uc_stack.ss_size = 64*1024;
    uc.uc_stack.ss_sp = malloc(uc.uc_stack.ss_size);
    int valgrind_stackid = VALGRIND_STACK_REGISTER(uc.uc_stack.ss_sp,
						   uc.uc_stack.ss_sp + uc.uc_stack.ss_size);

    uc.uc_link = &previous;
    
    makecontext(&uc,(void (*)(void))factorielle,2,n-1,res*n);
    swapcontext(&previous,&uc);

    printf("libération de la pile de %d\n",n);
    VALGRIND_STACK_DEREGISTER(valgrind_stackid);
    free(uc.uc_stack.ss_sp);
  }
  
}

int main(int argc, char* argv[]) {
  factorielle(atoi(argv[1]),1);
  /* ucontext_t uc, previous; */

  /* getcontext(&uc); /\* initialisation de uc avec valeurs coherentes */
  /* 		    * (pour éviter de tout remplir a la main ci-dessous) *\/ */

  /* ucontext_t context_pool[NB_CTX]; */

  /* getcontext(&context_pool[0]); */
  /* context_pool[0].uc_stack.ss_size = 64*1024; */
  /* context_pool[0].uc_stack.ss_sp = malloc(context_pool[0].uc_stack.ss_size); */
  /* context_pool[0].uc_link = NULL; */
  
  /* int i; */
  /* for (i = 1; i < NB_CTX; i++){ */
  /*   getcontext(&context_pool[i]); */
  /*   context_pool[i].uc_stack.ss_size = 64*1024; */
  /*   context_pool[i].uc_stack.ss_sp = malloc(context_pool[i].uc_stack.ss_size); */
  /*   if (i == 1) */
  /*     context_pool[i].uc_link = NULL; */
  /*   else */
  /*     context_pool[i].uc_link = &context_pool[i-1]; */
  /*   makecontext(&context_pool[i], (void (*)(void)) func, 1, i); */
  /* } */

  /* swapcontext(&context_pool[0], &context_pool[NB_CTX-1]); */
  
  /* setcontext(&uc); */
  /* printf("je ne reviens jamais ici\n"); */
  return 0;
}
