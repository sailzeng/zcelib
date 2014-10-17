/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce_net_http_client.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Monday, August 25, 2014
* @brief      代码的第一版本来自libghttp，还没有任何改写，只是尝试移植。
*             有点违反他的版权（GNU），但我肯定要改写移植的。
*
* @details
*
*
*
* @note
*
*/
#ifndef _ZCE_LIB_NET_HTTP_CLIENT_H_
#define _ZCE_LIB_NET_HTTP_CLIENT_H_




/* the list of known headers */
extern const char *http_hdr_known_list[];

/* a header list */
#define HTTP_HDRS_MAX 256
typedef struct http_hdr_list_tag
{
    char *header[HTTP_HDRS_MAX];
    char *value[HTTP_HDRS_MAX];
} http_hdr_list;

/* functions dealing with headers */

/* check to see if the library knows about the header */
const char *
http_hdr_is_known(const char *a_hdr);

/* create a new list */
http_hdr_list *
http_hdr_list_new(void);

/* destroy a list */
void
http_hdr_list_destroy(http_hdr_list *a_list);

/* set a value in a list */
int
http_hdr_set_value(http_hdr_list *a_list,
                   const char *a_name,
                   const char *a_val);

/* set the value in a list from a range, not a NTS */
int
http_hdr_set_value_no_nts(http_hdr_list *a_list,
                          const char *a_name_start,
                          int a_name_len,
                          const char *a_val_start,
                          int a_val_len);

/* get a copy of a value in a list */
char *
http_hdr_get_value(http_hdr_list *a_list,
                   const char *a_name);

/* get a copy of the headers in a list */
int
http_hdr_get_headers(http_hdr_list *a_list,
                     char ***a_names,
                     int *a_num_names);

/* clear a header in a list */
int
http_hdr_clear_value(http_hdr_list *a_list,
                     const char *a_name);


typedef struct http_uri_tag
{
    char             *full;                          /* full URL */
    char             *proto;                         /* protocol */
    char             *host;                          /* copy semantics */
    unsigned short    port;
    char             *resource;                      /* copy semantics */
} http_uri;

http_uri *
http_uri_new(void);

void
http_uri_destroy(http_uri *a_uri);

int
http_uri_parse(char *a_uri,
               http_uri *a_request);


//========================================================================================


typedef enum http_trans_err_type_tag
{
    http_trans_err_type_host = 0,
    http_trans_err_type_errno
} http_trans_err_type;

typedef struct http_trans_conn_tag
{
    struct hostent      *hostinfo;
    struct sockaddr_in   saddr;
    char                *host;
    char                *proxy_host;
    int                  sock;
    short                port;
    short                proxy_port;
    http_trans_err_type  error_type;
    int                  error;
    int                  sync;              /* sync or async? */
    char                *io_buf;            /* buffer */
    int                  io_buf_len;        /* how big is it? */
    int                  io_buf_alloc;      /* how much is used */
    int                  io_buf_io_done;    /* how much have we already moved? */
    int                  io_buf_io_left;    /* how much data do we have left? */
    int                  io_buf_chunksize;  /* how big should the chunks be that get
                                            read in and out be? */
    int                  last_read;         /* the size of the last read */
    int                  chunk_len;         /* length of a chunk. */
    char                *errstr;            /* a hint as to an error */
} http_trans_conn;

http_trans_conn *
http_trans_conn_new(void);

void
http_trans_conn_destroy(http_trans_conn *a_conn);

void
http_trans_buf_reset(http_trans_conn *a_conn);

void
http_trans_buf_clip(http_trans_conn *a_conn, char *a_clip_to);

int
http_trans_connect(http_trans_conn *a_conn);

const char *
http_trans_get_host_error(int a_herror);

int
http_trans_append_data_to_buf(http_trans_conn *a_conn,
                              char *a_data,
                              int a_data_len);

int
http_trans_read_into_buf(http_trans_conn *a_conn);

int
http_trans_write_buf(http_trans_conn *a_conn);

char *
http_trans_buf_has_patt(char *a_buf, int a_len,
                        char *a_pat, int a_patlen);


//========================================================================================


typedef enum http_req_type
{
    http_req_type_get = 0,
    http_req_type_options,
    http_req_type_head,
    http_req_type_post,
    http_req_type_put,
    http_req_type_delete,
    http_req_type_trace,
    http_req_type_connect,
    http_req_type_propfind,
    http_req_type_proppatch,
    http_req_type_mkcol,
    http_req_type_copy,
    http_req_type_move,
    http_req_type_lock,
    http_req_type_unlock
} http_req_type;

typedef enum http_req_state_tag
{
    http_req_state_start = 0,
    http_req_state_sending_request,
    http_req_state_sending_headers,
    http_req_state_sending_body
} http_req_state;








/* same character representations as above. */

extern const char *http_req_type_char[];

typedef struct http_req_tag
{
    http_req_type type;
    float          http_ver;
    char          *host;
    char          *full_uri;
    char          *resource;
    char          *body;
    int            body_len;
    http_hdr_list *headers;
    http_req_state state;
} http_req;

http_req *
http_req_new(void);

void
http_req_destroy(http_req *a_req);

int
http_req_prepare(http_req *a_req);

int
http_req_send(http_req *a_req, http_trans_conn *a_conn);




//========================================================================================

typedef enum http_resp_header_state_tag
{
    http_resp_header_start = 0,
    http_resp_reading_header
} http_resp_header_state;

typedef enum http_resp_body_state_tag
{
    http_resp_body_start = 0,
    http_resp_body_read_content_length,
    http_resp_body_read_chunked,
    http_resp_body_read_standard
} http_resp_body_state;


typedef struct http_resp_tag
{
    float                                http_ver;
    int                                  status_code;
    char                                *reason_phrase;
    http_hdr_list                       *headers;
    char                                *body;
    int                                  body_len;
    int                                  content_length;
    int                                  flushed_length;
    http_resp_header_state               header_state;
    http_resp_body_state                 body_state;
} http_resp;

http_resp *
http_resp_new(void);

void
http_resp_destroy(http_resp *a_resp);

int
http_resp_read_body(http_resp *a_resp,
                    http_req *a_req,
                    http_trans_conn *a_conn);

int
http_resp_read_headers(http_resp *a_resp, http_trans_conn *a_conn);

void
http_resp_flush(http_resp *a_resp,
                http_trans_conn *a_conn);


//========================================================================================

typedef enum ghttp_proc_tag
{
    ghttp_proc_none = 0,
    ghttp_proc_request,
    ghttp_proc_response_hdrs,
    ghttp_proc_response
} ghttp_proc;

typedef struct _ghttp_request
{
    http_uri           *uri;
    http_uri           *proxy;
    http_req           *req;
    http_resp          *resp;
    http_trans_conn    *conn;
    const char         *errstr;
    int                 connected;
    ghttp_proc          proc;
    char               *username;
    char               *password;
    char               *authtoken;
    char               *proxy_username;
    char               *proxy_password;
    char               *proxy_authtoken;
} ghttp_request;

/* create a new request object */
ghttp_request *ghttp_request_new(void);

/* delete a current request object */
void ghttp_request_destroy(ghttp_request *a_request);



#endif //_ZCE_LIB_NET_HTTP_CLIENT_H_

