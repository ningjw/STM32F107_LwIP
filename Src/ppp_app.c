#include "ppp.h"
#include "dns.h"
#include "netif/ppp/pppoe.h"
/* The PPP control block */
ppp_pcb *ppp;

/* The PPP IP interface */
struct netif ppp_netif;
extern struct netif gnetif;

/*
	  char *data = p->payload;
	  struct fs_file file = {0, 0};
	  extern uint8_t flag_LedFlicker;
	  if (strstr(data, "GET /?led=") != NULL)
      {
		uint8_t  i = 10;

		switch(data[i]){
		case '1':
			LED_ON();
			flag_LedFlicker = RESET;
			fs_open(&file,"/LedOn.html");
			break;
		case '2':
			LED_OFF();
			flag_LedFlicker = RESET;
			fs_open(&file,"/LedOff.html");
			break;
		case '3':
			flag_LedFlicker = SET;
			fs_open(&file,"/LedFlicker.html");
			break;
		default:
			fs_open(&file,"/LedFlicker.html");
			break;
		}
	  }else if(strstr(data,"GET /image/LedOn.jpg") != NULL){
		  fs_open(&file,"/image/LedOn.jpg");
	  }else if(strstr(data,"GET /image/LedOff.jpg") != NULL){
		  fs_open(&file,"/image/LedOff.jpg");
	  }else if(strstr(data,"GET /image/LedFlicker.gif") != NULL){
		  fs_open(&file,"/image/LedFlicker.gif");
	  }else if(strstr(data, "GET") != NULL){
		  if(flag_LedFlicker == SET){
			fs_open(&file,"/LedFlicker.html");
		}else if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)){
			fs_open(&file,"/LedOff.html");
		}else{
			fs_open(&file,"/LedOn.html");
		}
	  }
	  hs->file = file.data;
      hs->left = file.len;
*/

/*
 * PPP status callback
 * ===================
 *
 * PPP status callback is called on PPP status change (up, down, бн) from lwIP
 * core thread
 */

/* PPP status callback example */
void status_cb(ppp_pcb *pcb, int err_code, void *ctx) {
  struct netif *pppif = ppp_netif(pcb);
  LWIP_UNUSED_ARG(ctx);

  switch(err_code) {
    case PPPERR_NONE: {
#if LWIP_DNS
      const ip_addr_t *ns;
#endif /* LWIP_DNS */
      printf("status_cb: Connected\n");
#if PPP_IPV4_SUPPORT
      printf("   our_ipaddr  = %s\n", ipaddr_ntoa(&pppif->ip_addr));
      printf("   his_ipaddr  = %s\n", ipaddr_ntoa(&pppif->gw));
      printf("   netmask     = %s\n", ipaddr_ntoa(&pppif->netmask));
#if LWIP_DNS
      ns = dns_getserver(0);
      printf("   dns1        = %s\n", ipaddr_ntoa(ns));
      ns = dns_getserver(1);
      printf("   dns2        = %s\n", ipaddr_ntoa(ns));
#endif /* LWIP_DNS */
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
      printf("   our6_ipaddr = %s\n", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
#endif /* PPP_IPV6_SUPPORT */
      break;
    }
    case PPPERR_PARAM: {
      printf("status_cb: Invalid parameter\n");
      break;
    }
    case PPPERR_OPEN: {
      printf("status_cb: Unable to open PPP session\n");
      break;
    }
    case PPPERR_DEVICE: {
      printf("status_cb: Invalid I/O device for PPP\n");
      break;
    }
    case PPPERR_ALLOC: {
      printf("status_cb: Unable to allocate resources\n");
      break;
    }
    case PPPERR_USER: {
      printf("status_cb: User interrupt\n");
      break;
    }
    case PPPERR_CONNECT: {
      printf("status_cb: Connection lost\n");
      break;
    }
    case PPPERR_AUTHFAIL: {
      printf("status_cb: Failed authentication challenge\n");
      break;
    }
    case PPPERR_PROTOCOL: {
      printf("status_cb: Failed to meet protocol\n");
      break;
    }
    case PPPERR_PEERDEAD: {
      printf("status_cb: Connection timeout\n");
      break;
    }
    case PPPERR_IDLETIMEOUT: {
      printf("status_cb: Idle Timeout\n");
      break;
    }
    case PPPERR_CONNECTTIME: {
      printf("status_cb: Max connect time reached\n");
      break;
    }
    case PPPERR_LOOPBACK: {
      printf("status_cb: Loopback detected\n");
      break;
    }
    default: {
      printf("status_cb: Unknown error code %d\n", err_code);
      break;
    }
  }

/*
 * This should be in the switch case, this is put outside of the switch
 * case for example readability.
 */

  if (err_code == PPPERR_NONE) {
    return;
  }

  /* ppp_close() was previously called, don't reconnect */
  if (err_code == PPPERR_USER) {
    /* ppp_free(); -- can be called here */
    return;
  }

  /*
   * Try to reconnect in 30 seconds, if you need a modem chatscript you have
   * to do a much better signaling here ;-)
   */
  ppp_connect(pcb, 30);
  /* OR ppp_listen(pcb); */
}

/***************************************************************************************
  * @brief   
  * @input   
  * @return  
***************************************************************************************/
void PPPoE_Init(void)
{
/*
 * Create a new PPPoE interface
 *
 * ppp_netif, netif to use for this PPP link, i.e. PPP IP interface
 * ethif, already existing and setup Ethernet interface to use
 * service_name, PPPoE service name discriminator (not supported yet)
 * concentrator_name, PPPoE concentrator name discriminator (not supported yet)
 * status_cb, PPP status callback, called on PPP status change (up, down, бн)
 * ctx_cb, optional user-provided callback context pointer
 */
	ppp = pppoe_create(&ppp_netif, &gnetif,
       " ", "",
       status_cb, NULL);//ctx_cb
	
	/* Set this interface as default route */
	ppp_set_default(ppp);
	
	/*
 * Basic PPP client configuration. Can only be set if PPP session is in the
 * dead state (i.e. disconnected). We don't need to provide thread-safe
 * equivalents through PPPAPI because those helpers are only changing
 * structure members while session is inactive for lwIP core. Configuration
 * only need to be done once.
 */

	/* Ask the peer for up to 2 DNS server addresses. */
	ppp_set_usepeerdns(ppp, 1);

	/* Auth configuration, this is pretty self-explanatory */
	ppp_set_auth(ppp, PPPAUTHTYPE_ANY, "login", "password");

	/*
	 * Initiate PPP negotiation, without waiting (holdoff=0), can only be called
	 * if PPP session is in the dead state (i.e. disconnected).
	 */
	u16_t holdoff = 0;
	ppp_connect(ppp, holdoff);
}


