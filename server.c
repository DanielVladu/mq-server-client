#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "utils.h"

mqd_t mqs, mqc;
char *server_queue = "/server_queue";
char client_queue[20];

void cleanup()
{
  int rc;
  rc = mq_close(mqs);
  DIE(rc == -1, "mq_close");

  rc = mq_unlink(server_queue);
  DIE(rc == -1, "mq_unlink");

  printf("\nExiting ");
  exit(1);
}

int main(int argc, char **argv)
{
  int rc;
  struct mq_attr attr;

  attr.mq_flags = 0;
  attr.mq_maxmsg = 100;
  attr.mq_msgsize = MAX_SIZE;
  attr.mq_curmsgs = 0;

  char buf[MAX_SIZE];

  char token_message[20];
  int token = 0;

  mqs = mq_open(server_queue, O_CREAT | O_RDONLY, 0644, &attr);
  DIE(mqs == (mqd_t)-1, "mq_open");

  signal(SIGINT, cleanup);
  atexit(cleanup);

  printf("Server started %s\n",server_queue);

  while (1)
  {
    rc = mq_receive(mqs, buf, MAX_SIZE, NULL);
    DIE(rc == -1, "mq_receive");

    buf[rc] = '\0';

    if (buf[0] == 'R' && buf[1] == 'E' && buf[2] == 'Q')
    {
      int flag = 0, k = 0;
      for (int i = 0;i < strlen(buf);i++)
      {
        if (flag == 1)
        {
          client_queue[k] = buf[i];
          k++;
        }
        if (buf[i] == ';')
        {
          flag = 1;
        }
      }
      client_queue[k] = '\0';
    }

    printf("Received token request from %s\n",client_queue);

    mqc = mq_open(client_queue, O_RDWR);
    DIE(mqc == (mqd_t)-1, "mq_open");

    sprintf(token_message,"TOK;%d",token++);

    rc = mq_send(mqc, token_message, strlen(token_message), 0);
    DIE(rc == -1, "mq_send");

    printf("Sent token %d to %s\n",token-1,client_queue);

    rc = mq_close(mqc);
    DIE(rc == -1, "mq_close");
  }

  rc = mq_close(mqs);
  DIE(rc == -1, "mq_close");

  rc = mq_unlink(server_queue);
  DIE(rc == -1, "mq_unlink");

  printf("\n");

  return 0;
}
