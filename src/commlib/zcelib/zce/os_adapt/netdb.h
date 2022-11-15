#pragma once

namespace zce
{
//======================================================================================================
//域名解析，转换IP地址的几个函数
//基础函数部分，第一部分函数详细间《UNIX网络编程》卷1的第9章

/*!
* @brief      根据域名解析得到对应的IP地址，这个函数已经不是特别推荐，因为种种不如意，
*             比如不能重入，如果一个域名同时有IPV4和IPV6，那么就只有返回IPV4，
*             推荐使用getaddrinfo， gethostbyname的使用可以参考gethostbyname_inary函数
* @return     hostent*  返回的IP地址信息
* @param      hostname  查询的域名
*/
hostent* gethostbyname(const char* hostname);

/*!
* @brief      GNU extensions ,根据协议簇，得到解析的地址
* @return     hostent*
* @param      hostname
* @param      af
*/
hostent* gethostbyname2(const char* hostname,
                        int af);

//相对而言，下面这2个函数更加使用更加简单一点，

/*!
* @brief      非标准函数,得到某个域名的IPV4的地址数组，使用起来比较容易和方便，内部使用gethostbyname
* @return        int           0成功，-1失败
* @param[in]     hostname      域名
* @param[in]     service_port  端口号，会和查询到的地址信息一起设置到sockaddr_in
* @param[in,out] ary_addr_num  输入时数组的长度，输出返回实际获得的数组长度
* @param[out]    ary_sock_addr 域名+端口对应的sockaddr_in 数组信息
*/
int gethostbyname_inary(const char* hostname,
                        uint16_t service_port,
                        size_t* ary_addr_num,
                        sockaddr_in ary_sock_addr[]);

/*!
* @brief      非标准函数,得到某个域名的IPV6的地址数组，使用起来比较容易和方便
* @return        int            0成功，-1失败
* @param[in]     hostname       域名
* @param[in]     service_port   端口号，会和查询到的地址信息一起设置到sockaddr_in
* @param[in,out] ary_addr6_num  输入时数组的长度，输出返回实际获得的地址个数
* @param[out]    ary_sock_addr6 域名+端口对应的sockaddr_in6 数组
* @note
*/
int gethostbyname_in6ary(const char* hostname,
                         uint16_t service_port,
                         size_t* ary_addr6_num,
                         sockaddr_in6 ary_sock_addr6[]);

/*!
* @brief      通过IP地址信息，反查域名，不可重入，不推荐使用，推荐使用getnameinfo
* @return     hostent* 查询到的域名
* @param[in]  addr     指向地址的指针，注意地址是in_addr，in6_addr等等
* @param[in]  len      地址的长度
* @param[in]  family   地址协议族
*/
hostent* gethostbyaddr(const void* addr,
                       socklen_t len,
                       int family);

/*!
* @brief      非标准函数，通过IPV4地址取得域名
* @return     int        0成功，-1失败
* @param[in]  sock_addr  IPV4的sockaddr
* @param[out] host_name  返回的域名信息
* @param[int] name_len   域名buffer的长度
*/
int gethostbyaddr_in(const sockaddr_in* sock_addr,
                     char* host_name,
                     size_t name_len);

/*!
* @brief      非标准函数，通过IPV6地址取得域名
* @return     int        0成功，-1失败
* @param[in]  sock_addr6 IPV6的sockaddr
* @param[out] host_name  返回的域名信息
* @param[int] name_len   域名buffer的长度
*/
int gethostbyaddr_in6(const sockaddr_in6* sock_addr6,
                      char* host_name,
                      size_t name_len);

//几个更加高级一些的的函数，Posix 1g的函数，推荐使用他们，他们可能可以重入
// （要看底层实现，如果底层就是gethostbyname，那么……）
//高级部分函数详细间《UNIX网络编程》卷1的第11章

/*!
* @brief      通过域名得到服务器地址信息，可以同时得到IPV4，和IPV6的地址
* @return     int        0成功，非0表示失败，以及错误ID
* @param[in]  hostname   域名信息
* @param[in]  service    服务的名称，比如"http","ftp"等，用于决定服务的端口号
* @param[in]  hints      调用者所需要的地址的信息，相应的参数说明：
*                        如果要同时得到IPV4，IPV6的地址，那么hints.ai_family =  AF_UNSPEC
*                        ai_socktype参数最好还是填写一个值，否则可能返回SOCK_DGRAM,SOCK_STREAM各一个，
*                        ai_flags 填0一般就OK，（蛋疼的一个参数）
*                          AI_CANONNAME表示返回的addrinfo结果的第一个节点会有ai_canoname参数（见addrinfo 结构），
*                          AI_PASSIVE表示返回的地址用于bind（hostname为nullptr时，让IP地址信息返回0，）否则用于connect
*                          AI_NUMERICHOST 阻止域名解析，认为hostname是数值格式地址
*                          AI_ALL 新选项，返回IPV4和IPV6地址，IPV4的地址也会MAP成IPV6返回（也返回IPV4的地址）。
*                          AI_V4MAPPED 如果没有IPV6的地址返回，用IPV4的地址MAPIPV6的地址返回。
*                        ai_protocol，填0把
* @param[out] result     返回的结果数组
*/
int getaddrinfo(const char* hostname,
                const char* service,
                const addrinfo* hints,
                addrinfo** result);

/*!
* @brief      释放getaddrinfo得到的结果
* @param      result  getaddrinfo返回的结果
*/
void freeaddrinfo(struct addrinfo* result);

/*!
* @brief      辅助函数，从getaddrinfo的结果中提取一个sockaddr结果,
* @return     int
* @param[in]  result getaddrinfo返回的结果
* @param[out] addr   根据你输入的addr_len的 确定是sockaddr_in,还是sockaddr_in6
* @param[in]  addr_len 地址的长度
*/
int getaddrinfo_result_to_oneaddr(addrinfo* result,
                                  sockaddr* addr,
                                  socklen_t addr_len);

/*!
* @brief         辅助函数，将getaddrinfo的结果进行加工处理，处理成数组
* @param[in]     result
* @param[in,out] ary_addr_num   ary_addr的数组数量，转换后，返回数量
* @param[out]    ary_addr
* @param[in,out] ary_addr6_num  ary_addr6的数组数量，转换后，返回数量
* @param[out]    ary_addr6
*/
void getaddrinfo_result_to_addrary(addrinfo* result,
                                   size_t* ary_addr_num,
                                   sockaddr_in ary_addr[],
                                   size_t* ary_addr6_num,
                                   sockaddr_in6 ary_addr6[]);

void getaddrinfo_result_to_addrvector(addrinfo* result,
                                      std::vector<sockaddr_in> &vector_addr,
                                      std::vector <sockaddr_in6>& vector_addr6);

/*!
* @brief         非标准函数,得到某个域名的IPV4和IPV6的地址数组，使用起来比较容
*                易和方便,底层使用getaddrinfo
* @return        int            0成功，其他失败
* @param[in]     notename       域名
* @param[in]     service        服务名称，可以是"HTTP"，也可以是"80"
* @param[in,out] ary_addr_num   输入是ary_sock_addr数组的长度，输出返回实际获得的数组长度
* @param[out]    ary_sock_addr  域名对应的sockaddr_in 数组信息
* @param[in,out] ary_addr6_num  输入是ary_sock_addr6数组的长度，输出返回实际获得的地址个数
* @param[out]    ary_sock_addr6 域名对应的sockaddr_in6 数组
*/
int getaddrinfo_addrary(const char* hostname,
                        const char* service,
                        size_t* ary_addr_num,
                        sockaddr_in ary_addr[],
                        size_t* ary_addr6_num,
                        sockaddr_in6 ary_addr6[]);

/*!
 * @brief 得到某个域名的得到某个域名的IPV4和IPV6的地址数组
 * @param[out] vector_addr
 * @param[out] vector_addr6
*/
int getaddrinfo_addrary(const char* hostname,
                        const char* service,
                        std::vector<sockaddr_in>& vector_addr,
                        std::vector <sockaddr_in6>& vector_addr6);

/*!
* @brief
* @return     int      0成功，其他失败
* @param[in]  notename 域名OR数值地址格式，内部会先进行数值地址转换。避免耗时，
*                      不成功再进行域名解析
* @param[out] addr     返回的地址
* @param[in]  addr_len 地址的长度
*/
int getaddrinfo_to_addr(const char* hostname,
                        const char* service,
                        sockaddr* addr,
                        socklen_t addr_len);

/*!
* @brief      通过IP地址信息，反查域名.服务名，（可能）可以重入函数（要看底层实现），
* @return     int   0成功，非0表示失败，以及错误ID
* @param[in]  sa      socket地址的指针，通过这个查询域名
* @param[in]  salen   scket地址的长度
* @param[out] host    返回的域名
* @param[in]  hostlen 域名的buffer的长度
* @param[out] serv    返回的服务名称buffer
* @param[in]  servlen 服务名称buffer的长度
* @param[in]  flags   flags 参数，可以使用的值包括
*                     NI_NOFQDN       对于本地主机, 仅返回完全限定域名的节点名部分.比如bear.qq.com，返回点bear，而不返回.qq.com
*                     NI_NUMERICHOST  host参数返回数字的IP地址信息信息
*                     NI_NAMEREQD     如果IP地址不能解析为域名，返回一个错误
*                     NI_NUMERICSERV  serv参数返回数值的字符串信息
*                     NI_DGRAM        服务基于数据报而非基于流，（比如同一个端口号，UDP和TCP的服务类型是不一样的）
*/
int getnameinfo(const struct sockaddr* sa,
                socklen_t salen,
                char* host,
                size_t hostlen,
                char* serv,
                size_t servlen,
                int flags);

/*!
* @brief      非标准函数，通过IPV4地址取得域名
* @return     int  0成功，非0表示失败，以及错误ID
* @param[in]  sock_addr  sockaddr的地址
* @param[out] host_name  返回的域名信息
* @param[int] name_len   域名buffer的长度
*/
int getnameinfo_sockaddr(const sockaddr* sock_addr,
                         char* host_name,
                         size_t name_len);

/*!
* @brief      返回当前机器的主机名称,2个平台应该都支持这个函数
* @return     int        0成功
* @param[out] name       返回的主机名称，
* @param[in]  name_len   主机名称的buffer长度
*/
inline int gethostname(char* name,
                       size_t name_len);
}