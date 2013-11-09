

#include <rtthread.h>
#include "lwip/api.h"
#include <string.h>



ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t tcp_server_stack[ 1024 ];
static struct rt_thread tcp_server_thread;

static void server_echo(void* para)
{
    struct netbuf     *buf;
    rt_uint8_t        *rec_data;
    rt_uint16_t       rec_data_len;

    while(1)
    {
        netconn_recv(para,&buf);
        
        if(buf != NULL)
        {
            netbuf_data(buf,(void**)&rec_data,&rec_data_len);
            netconn_write(para,rec_data,rec_data_len,NETCONN_COPY);
            netbuf_delete(buf);                
        }
        else
        {
            rt_kprintf("sever close!\r\n");
            netconn_delete(para);
            break;
        }
    }
}


static void tcp_server_thread_entry(void* parameter)
{
    const rt_uint8_t welcome[] = "hello, world!\r\n";
    struct netconn    *receive_conn, *new_conn; 
    rt_thread_t       tid = RT_NULL;

    new_conn = netconn_new(NETCONN_TCP);
    netconn_bind(new_conn,NULL,56793);
    netconn_listen(new_conn);

    while(1)
    {
        netconn_accept(new_conn,&receive_conn);
        netconn_write(receive_conn,welcome,sizeof(welcome),NETCONN_COPY);
        rt_kprintf("sever open!\r\n");

        tid = rt_thread_create("connect",server_echo,(void*)receive_conn,1024,20,5);
        if(tid != RT_NULL)
        {
            rt_thread_startup(tid);
        }
        else
        {
            rt_kprintf("creat echo server failed.\r\n");
            break;
        }

    }
    netconn_delete(new_conn);
}


int tcp_server(void)
{

    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&tcp_server_thread,
        "tcp_ser",
        tcp_server_thread_entry, RT_NULL,
        (rt_uint8_t*)&tcp_server_stack[0], sizeof(tcp_server_stack), 20, 5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&tcp_server_thread);
    }
    return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(tcp_server, creat tcp server);
#endif

