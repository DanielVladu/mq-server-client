#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"

int main(int argc, char **argv)
{
  int rc;
  mqd_t mqc, mqs;
  struct mq_attr attr;

  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_SIZE;
  attr.mq_curmsgs = 0;

  char buf[MAX_SIZE];

  char *server_queue = "/server_queue";
  char client_queue[20];
  char request_message[30];
  int token;

  sprintf(client_queue,"/MQ%ld",(long)getpid());
  sprintf(request_message,"REQ;%s",client_queue);

  mqc = mq_open(client_queue, O_CREAT | O_RDONLY, 0644, &attr);
  DIE(mqc == (mqd_t)-1, "mq_open");

  printf("Client queue: %s\n",client_queue);

  mqs = mq_open(server_queue, O_RDWR);
  DIE(mqs == (mqd_t)-1, "mq_open");

  rc = mq_send(mqs, request_message, strlen(request_message), 0);
  DIE(rc == -1, "mq_send");

  rc = mq_receive(mqc, buf, MAX_SIZE, NULL);
  DIE(rc == -1, "mq_receive");

  buf[rc] = '\0';

  if (buf[0] == 'T' && buf[1] == 'O' && buf[2] == 'K')
  {
    int flag = 0, k = 0; char tmp[10];
    for (int i = 0;i < strlen(buf);i++)
    {
      if (flag == 1)
      {
        tmp[k++] = buf[i];
      }
      if (buf[i] == ';')
      {
        flag = 1;
      }
    }
    tmp[k] = '\0';
    token = atoi(tmp);
  }

  printf("Got token %d\n",token);

  rc = mq_close(mqc);
  DIE(rc == -1, "mq_close");

  rc = mq_close(mqs);
  DIE(rc == -1, "mq_close");

  rc = mq_unlink(client_queue);
  DIE(rc == -1, "mq_unlink");

  printf("\n");

  return 0;
}
