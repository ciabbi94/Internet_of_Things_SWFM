#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "json_getter.c"
#include "types.h"
#include "sys/etimer.h" // Include etimer


static char dam_state[10] = "closed";
void res_event_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
void res_event_post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);


EVENT_RESOURCE(resource_example, "title=\"Resource\";rt=\"Dam\"", res_event_get_handler, res_event_post_handler, NULL, NULL, NULL);

void
res_event_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	
	/* Populate the buffer with the response payload*/
	char *message = NULL;
	char j_message[MESSAGE_SIZE];
	unsigned int accept = -1;
  	REST.get_header_accept(request, &accept);					//retrieve accepted options
	if(  accept == REST.type.TEXT_PLAIN) {						//select and create the correct format: plain tex
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN); 		//set header content format
		REST.set_response_payload(response, buffer, MESSAGE_SIZE);
	
	} else if( accept == -1 || accept == REST.type.APPLICATION_JSON) {		//select and create the correct format: X	
		sprintf(j_message,"{\"%s\":\"%s\"}", str(dam),dam_state);	
		printf("sended:%s \n",j_message);
		memcpy(buffer, j_message,strlen(j_message));

		REST.set_header_content_type(response,  REST.type.APPLICATION_JSON);	//set header content format
		REST.set_response_payload(response, buffer, strlen(j_message));
	
	} else{
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
		message = "Supporting content-types text/plain, application/xml";
		REST.set_response_payload(response, message, strlen(message));
	}
}

void jparse_and_store(struct jsonparse_state *parser ){
	json_get_string(parser, str(dam), dam_state,DAM_STATE_SIZE);
}

void res_event_post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
	int len;
	const char *val = NULL;
	char *message = NULL;	
	unsigned int accept = -1;

	REST.get_header_accept(request, &accept);							//getting request
     
  	if(accept == REST.type.APPLICATION_JSON) {							//select and create the correct format: JSON
		struct jsonparse_state	parser;				
		
		len=REST.get_post_variable(request, "json", &val);					//get post variable (json format)			
	
		if( len > 0 && val[len-1]=='}'){							//check post parameter validity
			jsonparse_setup(&parser, val, len);
			
			jparse_and_store(&parser);
			printf("state:%s",dam_state);
		 }else 
		     REST.set_response_status(response, REST.status.BAD_REQUEST);
				
		memcpy(buffer, message, MESSAGE_SIZE);
		REST.set_header_content_type(response,  REST.type.TEXT_PLAIN);				//set header content format
		REST.set_response_payload(response, buffer, MESSAGE_SIZE);
	}
}

/*---------------------------------------------------------------------------*/
PROCESS(server, "Server process");
AUTOSTART_PROCESSES(&server);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(server, ev, data)
{
 
	PROCESS_BEGIN();
	static struct etimer et;									       // Declare an etime
	etimer_set(&et, CLOCK_SECOND*5); 								       // Set the timer
	rest_init_engine();
	rest_activate_resource(&resource_example, "Dam");

	while(1)
		PROCESS_WAIT_EVENT();
	


	PROCESS_END();
}