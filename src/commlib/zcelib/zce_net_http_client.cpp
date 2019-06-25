#include "zce_predefine.h"
#include "zce_trace_debugging.h"
#include "zce_net_http_client.h"


#if 0

//================================================================================================

/* entity headers */

const char http_hdr_Allow[] = "Allow";
const char http_hdr_Content_Encoding[] = "Content-Encoding";
const char http_hdr_Content_Language[] = "Content-Language";
const char http_hdr_Content_Length[] = "Content-Length";
const char http_hdr_Content_Location[] = "Content-Location";
const char http_hdr_Content_MD5[] = "Content-MD5";
const char http_hdr_Content_Range[] = "Content-Range";
const char http_hdr_Content_Type[] = "Content-Type";
const char http_hdr_Expires[] = "Expires";
const char http_hdr_Last_Modified[] = "Last-Modified";

/* general headers */

const char http_hdr_Cache_Control[] = "Cache-Control";
const char http_hdr_Connection[] = "Connection";
const char http_hdr_Date[] = "Date";
const char http_hdr_Pragma[] = "Pragma";
const char http_hdr_Transfer_Encoding[] = "Transfer-Encoding";
const char http_hdr_Update[] = "Update";
const char http_hdr_Trailer[] = "Trailer";
const char http_hdr_Via[] = "Via";

/* request headers */

const char http_hdr_Accept[] = "Accept";
const char http_hdr_Accept_Charset[] = "Accept-Charset";
const char http_hdr_Accept_Encoding[] = "Accept-Encoding";
const char http_hdr_Accept_Language[] = "Accept-Language";
const char http_hdr_Authorization[] = "Authorization";
const char http_hdr_Expect[] = "Expect";
const char http_hdr_From[] = "From";
const char http_hdr_Host[] = "Host";
const char http_hdr_If_Modified_Since[] = "If-Modified-Since";
const char http_hdr_If_Match[] = "If-Match";
const char http_hdr_If_None_Match[] = "If-None-Match";
const char http_hdr_If_Range[] = "If-Range";
const char http_hdr_If_Unmodified_Since[] = "If-Unmodified-Since";
const char http_hdr_Max_Forwards[] = "Max-Forwards";
const char http_hdr_Proxy_Authorization[] = "Proxy-Authorization";
const char http_hdr_Range[] = "Range";
const char http_hdr_Referrer[] = "Referrer";
const char http_hdr_TE[] = "TE";
const char http_hdr_User_Agent[] = "User-Agent";

/* response headers */

const char http_hdr_Accept_Ranges[] = "Accept-Ranges";
const char http_hdr_Age[] = "Age";
const char http_hdr_ETag[] = "ETag";
const char http_hdr_Location[] = "Location";
const char http_hdr_Retry_After[] = "Retry-After";
const char http_hdr_Server[] = "Server";
const char http_hdr_Vary[] = "Vary";
const char http_hdr_Warning[] = "Warning";
const char http_hdr_WWW_Authenticate[] = "WWW-Authenticate";

/* Other headers */

const char http_hdr_Set_Cookie[] = "Set-Cookie";

/* WebDAV headers */

const char http_hdr_DAV[] = "DAV";
const char http_hdr_Depth[] = "Depth";
const char http_hdr_Destination[] = "Destination";
const char http_hdr_If[] = "If";
const char http_hdr_Lock_Token[] = "Lock-Token";
const char http_hdr_Overwrite[] = "Overwrite";
const char http_hdr_Status_URI[] = "Status-URI";
const char http_hdr_Timeout[] = "Timeout";




//================================================================================================


typedef enum uri_parse_state_tag
{
    parse_state_read_host = 0,
    parse_state_read_port,
    parse_state_read_resource
} uri_parse_state;


int http_uri_parse(char *a_string, http_uri *a_uri)
{
    /* Everyone chant... "we love state machines..." */
    uri_parse_state l_state = parse_state_read_host;
    char *l_start_string = NULL;
    char *l_end_string = NULL;
    char  l_temp_port[6];

    /* init the array */
    memset(l_temp_port, 0, 6);
    /* check the parameters */
    if (a_string == NULL)
    {
        goto ec;
    }
    if (a_uri)
    {
        a_uri->full = strdup(a_string);
    }
    l_start_string = strchr(a_string, ':');
    /* check to make sure that there was a : in the string */
    if (!l_start_string)
    {
        goto ec;
    }
    if (a_uri)
    {
        a_uri->proto = (char *)malloc(l_start_string - a_string + 1);
        memcpy(a_uri->proto, a_string, (l_start_string - a_string));
        a_uri->proto[l_start_string - a_string] = '\0';
    }
    /* check to make sure it starts with "http://" */
    if (strncmp(l_start_string, "://", 3) != 0)
    {
        goto ec;
    }
    /* start at the beginning of the string */
    l_start_string = l_end_string = &l_start_string[3];
    while (*l_end_string)
    {
        if (l_state == parse_state_read_host)
        {
            if (*l_end_string == ':')
            {
                l_state = parse_state_read_port;
                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }
                /* allocate space */
                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }
                /* only do this if a uri was passed in */
                if (a_uri)
                {
                    a_uri->host = (char *)malloc(l_end_string - l_start_string + 1);
                    /* copy the data */
                    memcpy(a_uri->host, l_start_string, (l_end_string - l_start_string));
                    /* terminate */
                    a_uri->host[l_end_string - l_start_string] = '\0';
                }
                /* reset the counters */
                l_end_string++;
                l_start_string = l_end_string;
                continue;
            }
            else if (*l_end_string == '/')
            {
                l_state = parse_state_read_resource;
                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }
                if (a_uri)
                {
                    a_uri->host = (char *)malloc(l_end_string - l_start_string + 1);
                    memcpy(a_uri->host, l_start_string, (l_end_string - l_start_string));
                    a_uri->host[l_end_string - l_start_string] = '\0';
                }
                l_start_string = l_end_string;
                continue;
            }
        }
        else if (l_state == parse_state_read_port)
        {
            if (*l_end_string == '/')
            {
                l_state = parse_state_read_resource;
                /* check to make sure we're not going to overflow */
                if (l_end_string - l_start_string > 5)
                {
                    goto ec;
                }
                /* check to make sure there was a port */
                if ((l_end_string - l_start_string) == 0)
                {
                    goto ec;
                }
                /* copy the port into a temp buffer */
                memcpy(l_temp_port, l_start_string, l_end_string - l_start_string);
                /* convert it. */
                if (a_uri)
                {
                    a_uri->port = (uint16_t) atoi(l_temp_port);
                }
                l_start_string = l_end_string;
                continue;
            }
            else if (isdigit(*l_end_string) == 0)
            {
                /* check to make sure they are just digits */
                goto ec;
            }
        }
        /* next.. */
        l_end_string++;
        continue;
    }

    if (l_state == parse_state_read_host)
    {
        if ((l_end_string - l_start_string) == 0)
        {
            goto ec;
        }
        if (a_uri)
        {
            a_uri->host = (char *)malloc(l_end_string - l_start_string + 1);
            memcpy(a_uri->host, l_start_string, (l_end_string - l_start_string));
            a_uri->host[l_end_string - l_start_string] = '\0';
            /* for a "/" */
            a_uri->resource = strdup("/");
        }
    }
    else if (l_state == parse_state_read_port)
    {
        if (strlen(l_start_string) == 0)
            /* oops.  that's not a valid number */
        {
            goto ec;
        }
        if (a_uri)
        {
            a_uri->port = (uint16_t)atoi(l_start_string);
            a_uri->resource = strdup("/");
        }
    }
    else if (l_state == parse_state_read_resource)
    {
        if (strlen(l_start_string) == 0)
        {
            if (a_uri)
            {
                a_uri->resource = strdup("/");
            }
        }
        else
        {
            if (a_uri)
            {
                a_uri->resource = strdup(l_start_string);
            }
        }
    }
    else
    {
        /* uhh...how did we get here? */
        goto ec;
    }
    return 0;

ec:
    return -1;
}

http_uri *http_uri_new(void)
{
    http_uri *l_return = NULL;

    l_return = (http_uri *)malloc(sizeof(http_uri));
    l_return->full = NULL;
    l_return->proto = NULL;
    l_return->host = NULL;
    l_return->port = 80;
    l_return->resource = NULL;
    return l_return;
}

void http_uri_destroy(http_uri *a_uri)
{
    if (a_uri->full)
    {
        free(a_uri->full);
        a_uri->full = NULL;
    }
    if (a_uri->proto)
    {
        free(a_uri->proto);
        a_uri->proto = NULL;
    }
    if (a_uri->host)
    {
        free(a_uri->host);
        a_uri->host = NULL;
    }
    if (a_uri->resource)
    {
        free(a_uri->resource);
        a_uri->resource = NULL;
    }
    free(a_uri);
}

//================================================================================================

ghttp_request *ghttp_request_new(void)
{
    struct _ghttp_request *l_return = NULL;

    /* create everything */
    l_return = (_ghttp_request *)malloc(sizeof(struct _ghttp_request));
    if (!l_return)
    {
        return NULL;
    }
    memset(l_return, 0, sizeof(struct _ghttp_request));
    l_return->uri = http_uri_new();
    l_return->proxy = http_uri_new();
    l_return->req = http_req_new();
    l_return->resp = http_resp_new();
    l_return->conn = http_trans_conn_new();
    return l_return;
}

void ghttp_request_destroy(ghttp_request *a_request)
{
    if (!a_request)
    {
        return;
    }
    /* make sure that the socket was shut down. */
    if (a_request->conn->sock >= 0)
    {
        close(a_request->conn->sock);
        a_request->conn->sock = -1;
    }
    /* destroy everything else */
    if (a_request->uri)
    {
        http_uri_destroy(a_request->uri);
    }
    if (a_request->proxy)
    {
        http_uri_destroy(a_request->proxy);
    }
    if (a_request->req)
    {
        http_req_destroy(a_request->req);
    }
    if (a_request->resp)
    {
        http_resp_destroy(a_request->resp);
    }
    if (a_request->conn)
    {
        http_trans_conn_destroy(a_request->conn);
    }
    /* destroy username info. */
    if (a_request->username)
    {
        free(a_request->username);
        a_request->username = NULL;
    }
    if (a_request->password)
    {
        free(a_request->password);
        a_request->password = NULL;
    }
    if (a_request->authtoken)
    {
        free(a_request->authtoken);
        a_request->authtoken = NULL;
    }
    /* destroy proxy authentication */
    if (a_request->proxy_username)
    {
        free(a_request->proxy_username);
        a_request->proxy_username = NULL;
    }
    if (a_request->proxy_password)
    {
        free(a_request->proxy_password);
        a_request->proxy_password = NULL;
    }
    if (a_request->proxy_authtoken)
    {
        free(a_request->proxy_authtoken);
        a_request->proxy_authtoken = NULL;
    }
    if (a_request)
    {
        free(a_request);
    }
    return;
}



int ghttp_set_uri(ghttp_request *a_request, char *a_uri)
{
    int l_rv = 0;
    http_uri *l_new_uri = NULL;

    if ((!a_request) || (!a_uri))
    {
        return -1;
    }
    /* set the uri */
    l_new_uri = http_uri_new();
    l_rv = http_uri_parse(a_uri, l_new_uri);
    if (l_rv < 0)
    {
        http_uri_destroy(l_new_uri);
        return -1;
    }
    if (a_request->uri)
    {
        /* check to see if this has been set yet. */
        if (a_request->uri->host &&
            a_request->uri->port &&
            a_request->uri->resource)
        {
            /* check to see if we just need to change the resource */
            if ((!strcmp(a_request->uri->host, l_new_uri->host)) &&
                (a_request->uri->port == l_new_uri->port))
            {
                free(a_request->uri->resource);
                /* make a copy since we're about to destroy it */
                a_request->uri->resource = strdup(l_new_uri->resource);
                http_uri_destroy(l_new_uri);
            }
            else
            {
                http_uri_destroy(a_request->uri);
                a_request->uri = l_new_uri;
            }
        }
        else
        {
            http_uri_destroy(a_request->uri);
            a_request->uri = l_new_uri;
        }
    }
    return 0;
}


void ghttp_set_header(ghttp_request *a_request,
                      const char *a_hdr, const char *a_val)
{
    http_hdr_set_value(a_request->req->headers,
                       a_hdr, a_val);
}


int ghttp_prepare(ghttp_request *a_request)
{
    /* only allow http requests if no proxy has been set */
    if (!a_request->proxy->host && a_request->uri->proto &&
        strcmp(a_request->uri->proto, "http"))
    {
        return 1;
    }
    /* check to see if we have to set up the
    host information */
    if ((a_request->conn->host == NULL) ||
        (a_request->conn->host != a_request->uri->host) ||
        (a_request->conn->port != a_request->uri->port) ||
        (a_request->conn->proxy_host != a_request->proxy->host) ||
        (a_request->conn->proxy_port != a_request->proxy->port))
    {
        /* reset everything. */
        a_request->conn->host = a_request->uri->host;
        a_request->req->host = a_request->uri->host;
        a_request->req->full_uri = a_request->uri->full;
        a_request->conn->port = a_request->uri->port;
        a_request->conn->proxy_host = a_request->proxy->host;
        a_request->conn->proxy_port = a_request->proxy->port;
        a_request->conn->hostinfo = NULL;
        /* close the socket if it looks open */
        if (a_request->conn->sock >= 0)
        {
            close(a_request->conn->sock);
            a_request->conn->sock = -1;
            a_request->connected = 0;
        }
    }
    /* check to see if we need to change the resource. */
    if ((a_request->req->resource == NULL) ||
        (a_request->req->resource != a_request->uri->resource))
    {
        a_request->req->resource = a_request->uri->resource;
        a_request->req->host = a_request->uri->host;
    }
    /* set the authorization header */
    if ((a_request->authtoken != NULL) &&
        (strlen(a_request->authtoken) > 0))
    {
        http_hdr_set_value(a_request->req->headers,
                           http_hdr_Authorization,
                           a_request->authtoken);
    }
    else
    {
        http_hdr_set_value(a_request->req->headers,
                           http_hdr_WWW_Authenticate,
                           NULL);
    }
    /* set the proxy authorization header */
    if ((a_request->proxy_authtoken != NULL) &&
        (strlen(a_request->proxy_authtoken) > 0))
    {
        http_hdr_set_value(a_request->req->headers,
                           http_hdr_Proxy_Authorization,
                           a_request->proxy_authtoken);
    }
    http_req_prepare(a_request->req);
    return 0;
}

#endif
