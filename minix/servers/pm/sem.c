/* Miscellaneous system calls.				Author: Kees J. Bot
 *								31 Mar 2000
 * The entry points into this file are:
 *   do_reboot: kill all processes, then reboot system
 *   do_getsysinfo: request copy of PM data structure  (Jorrit N. Herder)
 *   do_getprocnr: lookup endpoint by process ID
 *   do_getepinfo: get the pid/uid/gid of a process given its endpoint
 *   do_getsetpriority: get/set process priority
 *   do_svrctl: process manager control
 */

#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/ds.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include "mproc.h"
#include <sem.h>
#include <stdlib.h>
#include <stdio.h>

//#include "kernel/proc.h"
//
#ifndef SEM_TABLE_INIT
#define SEM_TABLE_INIT
semaphore *sem_table[SEM_MAX] = {NULL};
#endif

void init_Queue(Queue * q){
  q->size = 0;
  q->head = NULL;
  q->tail = NULL;
}

void enQueue(Queue * q, int value){
  node * new_node = malloc(sizeof(node));
  if (new_node) {
    new_node->next = NULL;
    new_node->value = value;
    if (q->head == NULL) {
      q->head = new_node;
      q->tail = new_node;
    }else{
      q->tail->next = new_node;
      q->tail = new_node;
    }
    q->size++;
  }else{
    printf("ERROR en la reserva de memoria en enQueue \n");
  }
}

/** Removes node from the Queue
\param *q initialized Queue
*/
int deQueue(Queue * q){
  int process_id;
  if(q->head == NULL && q->tail == NULL){
    node *n = q->head;
  	process_id = n->value;
  	q->head = n->next;
  	if(q->head == NULL){
  		q->tail = NULL;
    }
  	free(n);
    q->size--;
  }
	return process_id;
}

/**
 * [Queue_size Devuelve el tamanio de la pila]
 * @param  q [La pila d]
 * @return   [tamanio de la pila]
 */
int Queue_size(Queue * q){
  return q->size;
}

/**
 * [is_in_Queue Verifica ]
 * @param q     [La pila d]
 * @param value [Devuelve 1 si es Verdadero, devuleve 0 si es falso]
 */
int is_in_Queue(Queue * q, int value){
  for(node *i = q->head; i!= NULL; i = i->next){
    if(i->value == value ){
      return 1;
    }
  }
  return 0;
}

int sem_create()
{
  int id = m_in.m1_i1;

  if(0 < id && id <= SEM_MAX){
    if (sem_table[id-1]==NULL) {
      semaphore *sem = malloc(sizeof(semaphore));
      if (sem) {
        sem_init(sem);
        sem_table[id -1] = sem;
        return 0;
      }
      else{
        return -1;
      }
    }else{
      return -1;
    }
  }else{
    return 1;
  }
}
int sem_terminate(){
  //printf("System call sem_terminate\n");
  int id = m_in.m1_i1;

  if(1 > id || id > SEM_MAX||sem_table[id-1]==NULL){
    return -1;
  }
  free (sem_table[id -1]);
  sem_table[id -1] = NULL;
  return 0;
}

int sem_down()
{
  int id = m_in.m1_i1;
	int pid = m_in.m1_i2;

  if(1 > id || id > SEM_MAX||sem_table[id-1]==NULL){
    return -1;
  }

	if(is_in_Queue(&(sem_table[id-1]->blocked_processes), pid) == 0)
		enQueue(&(sem_table[id-1]->blocked_processes), pid);

	if(sem_table[id-1]->value != 1 || sem_table[id-1]->blocked_processes.head->value != pid)
		return 1;

	sem_table[id-1]->value = 0;
	return 0;
}

int sem_up()
{

    int id = m_in.m1_i1;
    int pid = m_in.m1_i2;

    if(1 > id || id > SEM_MAX||sem_table[id-1]==NULL){
      return -1;
    }

    if (sem_table[id-1]->value != 0 ||
        sem_table[id-1]->blocked_processes.head->value!=pid) {
      return -1;
    }
    //se debe quitar el proceso de la cola
    deQueue(&(sem_table[id-1]->blocked_processes));
    sem_table[id-1]->value = 1;

    if (sem_table[id-1]->blocked_processes.head!=NULL) {
      printf("\nDesbloqueando proceso: %d\n",sem_table[id-1]->blocked_processes.head->value);
      check_sig(sem_table[id-1]->blocked_processes.head->value, SIGCONT, FALSE);
    }

  return 0;
}

/**
 * [sem_init Inicializar el semaforo]
 * @param sem [Semaforo que se va a inicializar]
 */
void sem_init(semaphore *sem){
  sem->value = 1;
  init_Queue(&(sem->blocked_processes));
}
