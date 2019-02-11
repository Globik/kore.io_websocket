#include <kore/kore.h>
#include <kore/http.h>

#include <kore/tasks.h>
#include "assets.h"
#include <rawrtc.h>
//include <jansson.h>
#include "helper/utils.h"
#include "helper/handler.h"
#define DEBUG_MODULE "peer-connection-app"
#define DEBUG_LEVEL 7
#include <re_dbg.h>

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

int chao=0;
struct kore_task task;  
struct mqueue *mq = NULL;

struct me{
int argc;
char* argvi[3];
//argv[argc+1];
}mi;

// Note: Shadows struct client 
struct peer_connection_client {
    char* name;
    char** ice_candidate_types;
    size_t n_ice_candidate_types;
    bool offering;
    struct rawrtc_peer_connection_configuration* configuration;
    struct rawrtc_peer_connection* connection;
    struct data_channel_helper* data_channel_negotiated;
    struct data_channel_helper* data_channel;
    struct connection*c;
};

static void print_local_description(struct peer_connection_client* const client);
static void client_init(struct peer_connection_client* const client);
static void parse_remote_description(int flags,void* arg);
static void client_stop(struct peer_connection_client* const client);

int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*,u_int8_t,void*,size_t);


static struct tmr timer = {{0}};
struct peer_connection_client clienti = {0};
 
int	page(struct http_request *);
int init(int);
int libre_loop(struct kore_task*);
void data_available(struct kore_task*);
void mqueue_handler(int, void*, void*);
int lmain(int argc, char* argv[argc + 1]);

void kore_worker_teardown(void){
kore_log(LOG_INFO,yellow "teardown" rst);
if(chao==0){
int s=mqueue_push(mq, 1, NULL);
chao=1;
kore_log(LOG_INFO, green "mqueue push %d" rst,s);
usleep(5000);

}
}
void mqueue_handler(int f, void*data, void*arg){
kore_log(LOG_INFO, green "mqueue_handler occured" rst);
//kore_log(LOG_INFO, green "data: %s" rst,data);
struct peer_connection_client *client=(struct peer_connection_client*)arg;
if(f==1){
kore_log(LOG_INFO, yellow "F: %d" rst, f);
re_cancel();
}else if(f==2){
kore_log(LOG_INFO,yellow "f==2, client_init" rst);
struct connection*c=data;
client->c=c;
//rawrtc_thread_enter();
client_init(client);

//kore_websocket_send(c,1,"mama\0",4);
}else if(f==3){
kore_log(LOG_INFO,yellow "f==3, client_stop" rst);
client_stop(client);	
}else{}	
}

int init(int state){
if(state==KORE_MODULE_UNLOAD)return (KORE_RESULT_ERROR);
printf("state init!\n");
kore_task_create(&task, libre_loop);
kore_task_bind_callback(&task, data_available);
kore_task_run(&task, 0);
return (KORE_RESULT_OK);	
}   


int page_ws_connect(struct http_request*req){
kore_log(LOG_INFO,"websocket connected , path %s %p", req->path, req);
kore_websocket_handshake(req, "websocket_connect","websocket_message","websocket_disconnect");
return (KORE_RESULT_OK);	
}
void websocket_connect(struct connection*c){
kore_log(LOG_INFO,"websocket connected %p",c);	
//clienti.c=c;
}
void websocket_message(struct connection*c,u_int8_t op,void*data,size_t len){
kore_log(LOG_INFO,"on message %d",len);
//kore_websocket_send(c,op,data,len);	
int send_to_clients=0;
/*
json_auto_t*root=load_json_buf((const char*)data,len);
if(!root){kore_log(LOG_INFO, red "why not a root?" rst);return;}
json_t *type_f=json_object_get(root,"type");
const char*type_str=json_str_value(type_f);
if(!strcmp(type_str,"msg")){
kore_log(LOG_INFO,  green "type msg" rst);	
}else if(!strcmp(type_str,"call")){
kore_log(LOG_INFO, green "type call" rst);
// local sdp to be send, try it

}else if(!strcmp(type_str,"was")){
	
}else{
kore_log(LOG_INFO,yellow "Unknown type: %s" rst,type_str);	
send_to_clients=1;
}
*/
struct odict*dict=NULL;
char*type_str;
int err;
err=json_decode_odict(&dict,16,data,len,3);
if(err){kore_log(LOG_INFO,red "decode odict err" rst);} 
err |=dict_get_entry(&type_str,dict,"type",ODICT_STRING,true);
if(err){kore_log(LOG_INFO, red "dict get entry error." rst);}

//if(type_str)kore_log(LOG_INFO, green "type came: %s" rst, type_str);
if(!strcmp(type_str,"msg")){
kore_log(LOG_INFO,green "type msg" rst);	
}else if(!strcmp(type_str,"call")){
kore_log(LOG_INFO, green "type call to browser" rst);
mqueue_push(mq, 2, c);
}else if(!strcmp(type_str,"endi")){
kore_log(LOG_INFO,yellow "type endi" rst);
mqueue_push(mq,3,NULL);	
}else{
kore_log(LOG_INFO,yellow "unknown type %s" rst, type_str);	
}
mem_deref(dict);
if(send_to_clients==0){kore_websocket_send(c,op,data,len);}
}

void websocket_disconnect(struct connection*c){
kore_log(LOG_INFO,"websocket disconnected %p",c);	
}

int
page(struct http_request *req)
{
	http_response_header(req,"content-type","text/html");
	http_response(req, 200, asset_index_html, asset_len_index_html);
	return (KORE_RESULT_OK);
}

static void exit_with_usage(char*program){
DEBUG_WARNING("Usage: %s <0|1 (offering)> [<ice-candidate-type> ...]",program);	
}
int libre_loop(struct kore_task*t){
kore_task_channel_write(t,"mama\0",5);
lmain(3,(char*[3]){"peer-connection", "1", "host"});

printf("argc: %d\n", mi.argc);	
printf("mi.arvi[0] %s\n",mi.argvi[0]);
printf("mi.argvi[1] %s\n",mi.argvi[1]);
printf("mi.argvi[2] %s\n",mi.argvi[2]);


int err;



    char** ice_candidate_types = NULL;
    size_t n_ice_candidate_types = 0;
    enum rawrtc_ice_role role;
    struct rawrtc_peer_connection_configuration* configuration;
    char* const stun_google_com_urls[] = {"stun:stun.l.google.com:19302",
                                          "stun:stun1.l.google.com:19302"};
    char* const turn_threema_ch_urls[] = {"turn:turn.threema.ch:443"};
  struct peer_connection_client client = {0};
    (void) client.ice_candidate_types; (void) client.n_ice_candidate_types;

    // Initialise
   // err=mqueue_alloc(&mq,mqueue_handler,NULL);
//if(err){kore_log(LOG_INFO, red "mqueue_alloc allocate failed");goto out;}

    EOE(rawrtc_init());

    // Debug
    dbg_init(DBG_DEBUG, DBG_ALL);
    //DEBUG_PRINTF(" Init\n");

    // Check arguments length
    if (mi.argc < 2) {
        exit_with_usage(mi.argvi[0]);
        DEBUG_PRINTF("\nexit 1\n");
       // return ;
       goto out;
    }

    // Get role
    // Note: We handle it as an ICE role (because that is pretty close)
    DEBUG_PRINTF("ICE ROLE IS %s\n",mi.argvi[1]);
    if (get_ice_role(&role, mi.argvi[1])) {
        exit_with_usage(mi.argvi[0]);
        DEBUG_PRINTF("\nexit 2\n");
        goto out;
       // return;
    }

    // Get enabled ICE candidate types to be added (optional)
    if (mi.argc >= 3) {
		DEBUG_PRINTF("argc>= 3!\n");
        ice_candidate_types = &mi.argvi[2];
        n_ice_candidate_types = (size_t) mi.argc - 2;
    }

    // Create peer connection configuration
    EOE(rawrtc_peer_connection_configuration_create(
            &configuration, RAWRTC_ICE_GATHER_POLICY_ALL));

    // Add ICE servers to configuration
    EOE(rawrtc_peer_connection_configuration_add_ice_server(
            configuration, stun_google_com_urls, ARRAY_SIZE(stun_google_com_urls),
            NULL, NULL, RAWRTC_ICE_CREDENTIAL_TYPE_NONE));
    EOE(rawrtc_peer_connection_configuration_add_ice_server(
            configuration, turn_threema_ch_urls, ARRAY_SIZE(turn_threema_ch_urls),
            "threema-angular", "Uv0LcCq3kyx6EiRwQW5jVigkhzbp70CjN2CJqzmRxG3UGIdJHSJV6tpo7Gj7YnGB",
            RAWRTC_ICE_CREDENTIAL_TYPE_PASSWORD));

    // Set client fields
    client.name = "A";
    client.ice_candidate_types = ice_candidate_types;
    client.n_ice_candidate_types = n_ice_candidate_types;
    client.configuration = configuration;
    client.offering = role == RAWRTC_ICE_ROLE_CONTROLLING ? true : false;

    // Setup client
    
  // client_init(&client);
    clienti=client;
 err=mqueue_alloc(&mq,mqueue_handler,&client);
if(err){kore_log(LOG_INFO, red "mqueue_alloc allocate failed");goto out;}
    // Listen on stdin
  //  EOR(fd_listen(STDIN_FILENO, FD_READ, parse_remote_description, &client));

printf("***Start main loop***\n");

EOR(re_main(NULL));

printf("*** Stop client & bye ***\n");
if(chao==0)client_stop(&client);
mem_deref(mq);
before_exit();




out:
{
if(err){
printf(red "in out body\n" rst);
mem_deref(mq);
}
}
kore_log(LOG_INFO, green "***bye from a task!***" rst);
chao=1;
return (KORE_RESULT_OK);
}

int lmain(int argc, char* argv[argc + 1]){
//struct me mi;
//mi.argc=1;
mi.argc=argc;
//char*a[]=argv;
mi.argvi[0]=argv[0];//argv;
mi.argvi[1]=argv[1];
mi.argvi[2]=argv[2];
return 0;
}
void data_available(struct kore_task *t){
size_t len;
u_int8_t buf[BUFSIZ];
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,"Task finished.");
return;

}
len=kore_task_channel_read(t,buf,sizeof(buf));
if(len > sizeof(buf))printf(red "len great than buf\n" rst);
kore_log(LOG_NOTICE,"Task msg: %s", buf);
printf(yellow "LEN: %d\n" rst,len);	
}


static void timer_handler(
        void* arg
) {
    struct data_channel_helper* const channel = arg;
    struct peer_connection_client* const client = (struct peer_connection_client*) channel->client;
    struct mbuf* buffer;
    enum rawrtc_code error;

    // Compose message (16 KiB)
    buffer = mbuf_alloc(1 << 14);
    EOE(buffer ? RAWRTC_CODE_SUCCESS : RAWRTC_CODE_NO_MEMORY);
    EOR(mbuf_fill(buffer, 'M', mbuf_get_space(buffer)));
    mbuf_set_pos(buffer, 0);

    // Send message
    DEBUG_PRINTF("(%s) Sending %zu bytes\n", client->name, mbuf_get_left(buffer));
    error = rawrtc_data_channel_send(channel->channel, buffer, true);
    if (error) {
        DEBUG_WARNING("Could not send, reason: %s\n", rawrtc_code_to_str(error));
    }
    mem_deref(buffer);

    // Close if offering
    if (client->offering) {
        // Close bear-noises
        DEBUG_PRINTF("(%s) Closing channel\n", client->name, channel->label);
        EOR(rawrtc_data_channel_close(client->data_channel->channel));
    }
}


static void data_channel_open_handler(
        void* const arg
) {
    struct data_channel_helper* const channel = arg;
    struct peer_connection_client* const client = (struct peer_connection_client*) channel->client;
    struct mbuf* buffer;
    enum rawrtc_code error;

    // Print open event
    default_data_channel_open_handler(arg);

    // Send data delayed on bear-noises
    if (str_cmp(channel->label, "bear-noises") == 0) {
        tmr_start(&timer, 30000, timer_handler, channel);
        return;
    }

    // Compose message (8 KiB)
    buffer = mbuf_alloc(1 << 13);
    EOE(buffer ? RAWRTC_CODE_SUCCESS : RAWRTC_CODE_NO_MEMORY);
    EOR(mbuf_fill(buffer, 'M', mbuf_get_space(buffer)));
    mbuf_set_pos(buffer, 0);

    // Send message
    DEBUG_PRINTF("(%s) Sending %zu bytes\n", client->name, mbuf_get_left(buffer));
    error = rawrtc_data_channel_send(channel->channel, buffer, true);
    if (error) {
        DEBUG_WARNING("Could not send, reason: %s\n", rawrtc_code_to_str(error));
    }
    mem_deref(buffer);
}
static void negotiation_needed_handler(
        void* const arg
) {
    struct peer_connection_client* const client = arg;

    // Print negotiation needed
    default_negotiation_needed_handler(arg);

    // Offering: Create and set local description
    if (client->offering) {
        struct rawrtc_peer_connection_description* description;
        EOE(rawrtc_peer_connection_create_offer(&description, client->connection, false));
        EOE(rawrtc_peer_connection_set_local_description(client->connection, description));
        mem_deref(description);
    }
}




static void connection_state_change_handler(
        enum rawrtc_peer_connection_state const state, // read-only
        void* const arg
) {
    struct peer_connection_client* const client = arg;

    // Print state
    default_peer_connection_state_change_handler(state, arg);

    // Open? Create new channel
    // Note: Since this state can switch from 'connected' to 'disconnected' and back again, we
    //       need to make sure we don't re-create data channels unintended.
    // TODO: Move this once we can create data channels earlier
    if (!client->data_channel && state == RAWRTC_PEER_CONNECTION_STATE_CONNECTED) {
        struct rawrtc_data_channel_parameters* channel_parameters;
        char* const label = client->offering ? "bear-noises" : "lion-noises";

        // Create data channel helper for in-band negotiated data channel
        data_channel_helper_create(
                &client->data_channel, (struct client *) client, label);

        // Create data channel parameters
        EOE(rawrtc_data_channel_parameters_create(
                &channel_parameters, client->data_channel->label,
                RAWRTC_DATA_CHANNEL_TYPE_RELIABLE_UNORDERED, 0, NULL, false, 0));

        // Create data channel
        EOE(rawrtc_peer_connection_create_data_channel(
                &client->data_channel->channel, client->connection, channel_parameters, NULL,
                data_channel_open_handler, default_data_channel_buffered_amount_low_handler,
                default_data_channel_error_handler, default_data_channel_close_handler,
                default_data_channel_message_handler, client->data_channel));

        // Un-reference data channel parameters
        mem_deref(channel_parameters);
    }
}



static void local_candidate_handler(
        struct rawrtc_peer_connection_ice_candidate* const candidate,
        char const * const url, // read-only
        void* const arg
) {
    struct peer_connection_client* const client = arg;

    // Print local candidate
    default_peer_connection_local_candidate_handler(candidate, url, arg);

    // Print local description (if last candidate)
    if (!candidate) {
		printf(yellow "!candidate in local_candidate_handler\n" rst);
        print_local_description(client);
    }
}


static void client_init(
        struct peer_connection_client* const client
) {
    struct rawrtc_data_channel_parameters* channel_parameters;

   printf(yellow "Create peer connection\n" rst);
    EOE(rawrtc_peer_connection_create(
            &client->connection, client->configuration,
            negotiation_needed_handler, local_candidate_handler,
            default_peer_connection_local_candidate_error_handler,
            default_signaling_state_change_handler, default_ice_transport_state_change_handler,
            default_ice_gatherer_state_change_handler, connection_state_change_handler,
            default_data_channel_handler, client));

    // Create data channel helper for pre-negotiated data channel
    data_channel_helper_create(
            &client->data_channel_negotiated, (struct client *) client, "cat-noises");

    // Create data channel parameters
    EOE(rawrtc_data_channel_parameters_create(
            &channel_parameters, client->data_channel_negotiated->label,
            RAWRTC_DATA_CHANNEL_TYPE_RELIABLE_ORDERED, 0, NULL, true, 0));

    // Create pre-negotiated data channel
    EOE(rawrtc_peer_connection_create_data_channel(
           &client->data_channel_negotiated->channel, client->connection,
            channel_parameters, NULL,
            data_channel_open_handler, default_data_channel_buffered_amount_low_handler,
            default_data_channel_error_handler, default_data_channel_close_handler,
            default_data_channel_message_handler, client->data_channel_negotiated));

    // TODO: Create in-band negotiated data channel
    // TODO: Return some kind of promise that resolves once the data channel can be created
    // yeah it would be nice to have a promise here [000007196] main: long async blocking: 607>100 ms (h=0xb770f505 arg=0xb5c0f9b0)
    // о чем се говорит? а о томБ что данная библиотека игрушка и не тянет на продакшн. Она по большому счету бесполезна.
    //Нахуй ее автор писал, я хуй ее знает. Выебнуться?
    /*
     * в реальной жизни библиотеку нужно подключать к фреймворку, а не делать стендалоне. Может и заебись в главном потоке работатает
     * само по себе с ебанным либрэ луп, но совершенно не дееспособна в реальной жизни. Выбрал либрэ луп он зазря.
     * Тебе туду по идее следовало бы доки написать и промисы ко всем этим тяжеловесным синхроническим операциям. 
     * Хоть, блядЬ, садись и сам эту либу переписывай под коммерческие нужды. Причем с нуля.
     * У тебя эта либа недоработана и не жизнеспособна. Какой бы крутой либой либрэ не была, базис которой ты используешь. Не жизнеспособна.
     * Тут че, я так вижу, либрэ полность сносить нахуй и прикручивать юзрсктплиб к кор напрямую. Иначе никак. Только спиздить у тебя структуру
     * данных. Позаимствовать. Но в корне менять всё. 
     * Так. Просто мысли. У меня нет времени тоже эту библиотеку переписывать. Просто не надо ее вов се места толкать. Оная не жизнеспособнаю
     * Что доказывает моя практика.
     * 
     * /

    // Un-reference data channel parameters
    mem_deref(channel_parameters);
    
}




static void client_stop(
        struct peer_connection_client* const client
) {
    EOE(rawrtc_peer_connection_close(client->connection));

    // Un-reference & close
    client->data_channel = mem_deref(client->data_channel);
    client->data_channel_negotiated = mem_deref(client->data_channel_negotiated);
    client->connection = mem_deref(client->connection);
    client->configuration = mem_deref(client->configuration);
    //by me
   /* if(client->c !=NULL){
	printf(yellow "client->c is NOT null\n" rst);
	client->c=NULL;	
	}*/

    // Stop listening on STDIN
   // fd_close(STDIN_FILENO);
}

static void parse_remote_description(
        int flags,
        void* arg
) {
    struct peer_connection_client* const client = arg;
    enum rawrtc_code error;
    bool do_exit = false;
    struct odict* dict = NULL;
    char* type_str;
    char* sdp;
    enum rawrtc_sdp_type type;
    struct rawrtc_peer_connection_description* remote_description = NULL;
    (void) flags;

    // Get dict from JSON
    error = get_json_stdin(&dict);
    if (error) {
        do_exit = error == RAWRTC_CODE_NO_VALUE;
        goto out;
    }

    // Decode JSON
    error |= dict_get_entry(&type_str, dict, "type", ODICT_STRING, true);
    error |= dict_get_entry(&sdp, dict, "sdp", ODICT_STRING, true);
    if (error) {
        DEBUG_WARNING("Invalid remote description\n");
        goto out;
    }

    // Convert to description
    error = rawrtc_str_to_sdp_type(&type, type_str);
    if (error) {
        DEBUG_WARNING("Invalid SDP type in remote description: '%s'\n", type_str);
        goto out;
    }
    error = rawrtc_peer_connection_description_create(&remote_description, type, sdp);
    if (error) {
        DEBUG_WARNING("Cannot parse remote description: %s\n", rawrtc_code_to_str(error));
        goto out;
    }

    // Set remote description
    DEBUG_INFO("Applying remote description\n");
    EOE(rawrtc_peer_connection_set_remote_description(client->connection, remote_description));

    // Answering: Create and set local description
    if (!client->offering) {
        struct rawrtc_peer_connection_description* local_description;
        EOE(rawrtc_peer_connection_create_answer(&local_description, client->connection));
        EOE(rawrtc_peer_connection_set_local_description(client->connection, local_description));
        mem_deref(local_description);
    }

out:
    // Un-reference
    mem_deref(remote_description);
    mem_deref(dict);

    // Exit?
    if (do_exit) {
        DEBUG_NOTICE("Exiting\n");

        // Stop client & bye
        client_stop(client);
        tmr_cancel(&timer);
        before_exit();
        exit(0);
    }
}

static void print_local_description(
        struct peer_connection_client* const client
) {
    struct rawrtc_peer_connection_description* description;
    enum rawrtc_sdp_type type;
    char* sdp;
    struct odict* dict;

    // Get description
    EOE(rawrtc_peer_connection_get_local_description(&description, client->connection));

    // Get SDP type & the SDP itself
    EOE(rawrtc_peer_connection_description_get_sdp_type(&type, description));
    EOE(rawrtc_peer_connection_description_get_sdp(&sdp, description));

    // Create dict & add entries
    EOR(odict_alloc(&dict, 16));
    EOR(odict_entry_add(dict, "type", ODICT_STRING, rawrtc_sdp_type_to_str(type)));
    EOR(odict_entry_add(dict, "sdp", ODICT_STRING, sdp));

    // Print local description as JSON
    DEBUG_INFO("Local Description:\n%H\n", json_encode_odict, dict);
    struct mbuf*mb_enc=NULL;
    mb_enc=mbuf_alloc(1024);
    if(!mb_enc){printf(red "no  memory in mb_enc?\n" rst);}
    int err;
    err=mbuf_printf(mb_enc,"%H",json_encode_odict,dict);
    if(err){printf(red "error in mbuf_printf?\n" rst);}
    printf(yellow "%d\n" rst,mb_enc->end);
    printf(green "data: %s\n" rst,mb_enc->buf);
    if(client->c){printf(green "CLIENT->C!!\n" rst);
	//kore_websocket_send(client->c,1,"papa\0",5);
	//[000006048] main: long async blocking: 612>100 ms (h=0xb777d4e6 arg=0xb5c0f9b0)
	kore_websocket_send(client->c,1,mb_enc->buf,mb_enc->end);
	}
mem_deref(mb_enc);
    // Un-reference
    mem_deref(dict);
    mem_deref(sdp);
    mem_deref(description);
}

