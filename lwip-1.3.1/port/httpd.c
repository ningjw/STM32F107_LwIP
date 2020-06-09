#include "httpd.h"
#include "lwip/tcp.h"
#include "fsdata.c"
#include "main.h"
#include <string.h>

struct http_state
{
  char *file;
  u32_t left;
};


/*-----------------------------------------------------------------------------------*/
static void
conn_err(void *arg, err_t err)
{
  struct http_state *hs;

  hs = arg;
  mem_free(hs);
}
/*-----------------------------------------------------------------------------------*/
static void
close_conn(struct tcp_pcb *pcb, struct http_state *hs)
{

  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  mem_free(hs);
  tcp_close(pcb);
}
/*-----------------------------------------------------------------------------------*/
static void
send_data(struct tcp_pcb *pcb, struct http_state *hs)
{
  err_t err;
  u16_t len;

  /* We cannot send more data than space avaliable in the send
     buffer. */
  if (tcp_sndbuf(pcb) < hs->left)
  {
    len = tcp_sndbuf(pcb);
  }
  else
  {
    len = hs->left;
  }

  err = tcp_write(pcb, hs->file, len, 0);

  if (err == ERR_OK)
  {
    hs->file += len;
    hs->left -= len;
  }
}

/*-----------------------------------------------------------------------------------*/
static err_t
http_poll(void *arg, struct tcp_pcb *pcb)
{
  if (arg == NULL)
  {
    tcp_close(pcb);
  }
  else
  {
    send_data(pcb, (struct http_state *)arg);
  }

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  struct http_state *hs;

  hs = arg;

  if (hs->left > 0)
  {
    send_data(pcb, hs);
  }
  else
  {
    close_conn(pcb, hs);
  }

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  int i;
  char *data;
  struct fs_file file = {0, 0};
  struct http_state *hs;

  hs = arg;

  if (err == ERR_OK && p != NULL)
  {

    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);

    if (hs->file == NULL)
    {
      data = p->payload;

      if (strstr(data, "GET /?led=") != NULL)
      {
        i = 10;

		switch(data[i]){
		case '1':
			LED_ON();
			flag_LedFlicker = RESET;
			fs_open("/LedOn.html", &file);
			break;
		case '2':
			LED_OFF();
			flag_LedFlicker = RESET;
			fs_open("/LedOff.html", &file);
			break;
		case '3':
			flag_LedFlicker = SET;
			fs_open("/LedFlicker.html", &file);
			break;
		default:
			fs_open("/LedFlicker.html", &file);
			break;
		}

        pbuf_free(p);

        hs->file = file.data;
        hs->left = file.len;

        send_data(pcb, hs);

        /* Tell TCP that we wish be to informed of data that has been
           successfully sent by a call to the http_sent() function. */
        tcp_sent(pcb, http_sent);
      }
	  else if(strstr(data,"GET /image/LedOn.jpg") != NULL){
		pbuf_free(p);
		fs_open("/image/LedOn.jpg", &file);
        hs->file = file.data;
        hs->left = file.len;
        send_data(pcb, hs);
        tcp_sent(pcb, http_sent);
	  }
	  else if(strstr(data,"GET /image/LedOff.jpg") != NULL){
		pbuf_free(p);
		fs_open("/image/LedOff.jpg", &file);
        hs->file = file.data;
        hs->left = file.len;
        send_data(pcb, hs);
        tcp_sent(pcb, http_sent);
	  }
	  else if(strstr(data,"GET /image/LedFlicker.gif") != NULL){
		pbuf_free(p);
		fs_open("/image/LedFlicker.gif", &file);
        hs->file = file.data;
        hs->left = file.len;
        send_data(pcb, hs);
        tcp_sent(pcb, http_sent);
	  }
	  else if(strstr(data, "GET /") != NULL)
	  {
		pbuf_free(p);
		if(flag_LedFlicker == SET){
			fs_open("/LedFlicker.html", &file);
		}else if(GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_13)){
			fs_open("/LedOff.html", &file);
		}else{
			fs_open("/LedOn.html", &file);
		}
        hs->file = file.data;
        hs->left = file.len;

        send_data(pcb, hs);

        /* Tell TCP that we wish be to informed of data that has been
           successfully sent by a call to the http_sent() function. */
        tcp_sent(pcb, http_sent);
	  }
      else
      {
        close_conn(pcb, hs);
      }
    }
    else
    {
      pbuf_free(p);
    }
  }

  if (err == ERR_OK && p == NULL)
  {

    close_conn(pcb, hs);
  }
  
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct http_state *hs;

  /* Allocate memory for the structure that holds the state of the
     connection. */
  hs = mem_malloc(sizeof(struct http_state));

  if (hs == NULL)
  {
    return ERR_MEM;
  }

  /* Initialize the structure. */
  hs->file = NULL;
  hs->left = 0;

  /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
  tcp_arg(pcb, hs);

  /* Tell TCP that we wish to be informed of incoming data by a call
     to the http_recv() function. */
  tcp_recv(pcb, http_recv);

  tcp_err(pcb, conn_err);

  tcp_poll(pcb, http_poll, 10);
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
httpd_init(void)
{
  struct tcp_pcb *pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 80);
  pcb = tcp_listen(pcb);
  tcp_accept(pcb, http_accept);
}
/*-----------------------------------------------------------------------------------*/
int
fs_open(char *name, struct fs_file *file)
{
  struct fsdata_file_noconst *f;

  for (f = (struct fsdata_file_noconst *)FS_ROOT; f != NULL; f = (struct fsdata_file_noconst *)f->next)
  {
    if (!strcmp(name, f->name))
    {
      file->data = f->data;
      file->len = f->len;
      return 1;
    }
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/

