/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_shm_cache_chunk.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2005��12��16��
* @brief      һ���򵥵Ĵ�ű䳤���ݵ�Cache�����ڴ�С��̫�̶������ݵĻ��壬
*             ����С���ڴ棬����������ݣ��ռ��˷�С��
*             ���ڴ�ָ�����ɸ���С��Ͱ�� ÿ��������N(N>=1)��Ͱ��ţ�
*
* @details    Ͱ�Ĵ�С��ƱϾ��Ƚ��н�����̫�󲻺ã����ǻ������ϡ�˷ѣ�
*             ̫СҲ���ã��ᵼ��һ�����ݴ����̫���Ͱ�ڡ�
*             ���رȣ�ƽ�����ݴ�С��Ͱ�Ĺ�ϵ��һ�����������2-4�ȽϺã�
*
* @note       �������˼·��ʵ��FAT����һ�£�����Ȥ��ȥ����һ�£�
*             ��������ڵ���С�ܵ����������汻��һ�γ���ʹ�ã���ʱ��jovi
*             Ӧ�ð������æ��
*
*/
#ifndef ZCE_LIB_SHM_CACHE_CHUNK_H_
#define ZCE_LIB_SHM_CACHE_CHUNK_H_

#include "zce_shm_predefine.h"

namespace zce
{

/*!
@brief      NODE����Ϣ��NODE����һ�����ݣ�NODE�ڲ������ʼ��CHUNK��
            �������CHUNK���γ�һ������
            NODE�Ĵ�С������������֪�����ݵ�ʵ�ʳ��ȣ�
*/
class cachechunk_node_index
{
public:
    ///NODE�������ʼͰ����
    size_t     chunk_index_;
    ///NODE��ʵ�ʳߴ�
    size_t     size_of_node_;
};

/*!
@brief      �����ڴ��ͷ�������������ڱ�ʶ���ж��ٿ��е�NODE��CHUNK��
            ��Ϣ��Ҳ���¼FREENODE��FREECHUNK����ʼ�ڵ�ȡ�
            �ڲ��ṹ���ⲿ��Ҫʹ��
*/
class _shm_cachechunk_head
{
    ///ͨ����Ԫ��������ʹ��
    friend class shm_cachechunk;
private:
    ///���캯��
    _shm_cachechunk_head():
        size_of_mmap_(0),
        num_of_node_(0),
        usable_of_node_(0),
        num_of_chunk_(0),
        usable_of_chunk_(0),
        size_of_chunk_(0),
        free_node_head_(0),
        free_chunk_head_(0)
    {
    }
    ///��������
    ~_shm_cachechunk_head()
    {
    }

private:
    ///�ڴ����ĳ���
    size_t               size_of_mmap_;

    ///NODE������
    size_t               num_of_node_;
    ///������ʹ�õ�NODE��������ע���ǿ��ã������Ѿ�ʹ��
    size_t               usable_of_node_;

    ///Chunk������
    size_t               num_of_chunk_;
    ///������ʹ�õ�CHUNK������
    size_t               usable_of_chunk_;

    ///chunk�ߴ�
    size_t               size_of_chunk_;

    ///FREE NODE����ʼ�ڵ�
    size_t               free_node_head_;
    ///FREE CHUNK����ʼ�ڵ�
    size_t               free_chunk_head_;

};

/*!
* @brief      һ���򵥵Ĵ�ű䳤���ݵ�Cache�����ڴ�С��̫�̶������ݵĻ��壬
*             ����С���ڴ棬����������ݣ��ռ��˷�С��
*
*/
class shm_cachechunk : public _shm_memory_base
{

protected:
    //���캯��,
    shm_cachechunk();
    //��������,
    ~shm_cachechunk();

protected:

    /*!
    * @brief      ��������Ŀռ�,����һ��NODE,
    * @return     bool    �Ƿ�ɹ�����
    * @param[in]  size_t    ϣ����������NODE�ĳ���
    * @param[out] nodeindex ���ز��������뵽��NODE������
    */
    bool create_node(size_t, size_t &nodeindex);

    /*!
    * @brief      �ͷ�һ��NODE,����黹��FREELIST
    * @param      nodeindex �ͷŵ�NODE����
    */
    void destroy_node(const size_t nodeindex);

public:

    ///@brief      �������е�����,��Cache��ԭ�ɳ�ʼ����û���κ����ݵ�����
    void clear();

    /*!
    * @brief      ȡʣ��ռ�Ĵ�С
    * @param[out] free_node    ʣ���NODE�����������ԷŶ��ٸ�����
    * @param[out] free_chunk   ʣ���Ͱ��������
    * @param[out] max_room     ʣ��Ŀռ䣬�����ԷŶ�������
    */
    void free_size(size_t &free_node,
                   size_t &free_chunk,
                   size_t &max_room);

    /*!
    * @brief      ����Ƿ����㹻�ռ���һ������
    * @return     bool   ����ֵ���Ƿ���Է��룬
    * @param[in]  szdata Ҫ��������ݴ�С
    */
    bool check_enough(size_t szdata);

    /*!
    * @brief      ����һ��NODE���ݣ�
    * @return     bool       �Ƿ�ɹ�����
    * @param[in]  szdata     ���ݵĴ�С
    * @param[in]  indata     ����ָ��
    * @param[out] nodeindex  NODE�����NODE��������������������ҵ����NODE
    */
    bool set_node(const size_t szdata,
                  const char *indata,
                  size_t &nodeindex);

    /*!
    * @brief      �õ�ĳ��NODE�ĳߴ�
    * @return     size_t    ����NODE�ĳߴ�
    * @param[in]  nodeindex NODE�������������㴫��һ����ȷ�Ĳ�����������Ϊδ����
    */
    size_t nodesize(const size_t nodeindex);

    /*!
    * @brief      �õ�ĳ��NODE�ĳߴ�,Ͱ����������������һ������ֵ��ʾ�Ƿ�ȡֵ�ɹ��ģ�
    *             �������������������±����������������֤�����ɡ�
    * @param[in]  nodeindex  NODE�������������㴫��һ����ȷ�Ĳ�����������Ϊδ����
    * @param[out] nodesize   ���ز�����NODE�ĳߴ�
    * @param[out] chunknum   ���ز�����������õ�CHUNK������
    */
    void nodesize(const size_t nodeindex,
                  size_t &nodesize,
                  size_t &chunknum);

    /*!
    * @brief      ȡ��һ���ڵ������
    * @param[in]  nodeindex  ���NODE������
    * @param[out] szdata     ����NODE�Ĵ�С
    * @param[out] outdata    ���ص�NODE�������ݿռ�ĳߴ���Ҫ�Լ���֤ร�
    */
    void get_node(const size_t nodeindex,
                  size_t &szdata,
                  char *outdata);

    /*!
    * @brief      ����Ҫһ����CHUNKȡ������ʱ���õ�һ��NODE�ĵ�N��CHUNK������
    * @param[in]  nodeindex  NODE������
    * @param[in]  chunk_no   �ڼ���CHUNK����0��ʼѽ����ע���ⲻ���±꣬���ǵڼ���Ͱ��
    * @param[out] szdata     ���ز��������ص����ݳ��ȣ���С�ڵ���Ͱ���ȣ�
    * @param[out] outdata    ���ز��������Ͱ�����ݣ����ݿռ�Ҫ����Ͱ������
    */
    void get_chunk(const size_t nodeindex,
                   size_t chunk_no,
                   size_t &szdata,
                   char *outdata);

    /*!
    * @brief      �������ݵ���ʼλ�ã�ȡ�����λ������CHUNK������,��ע��ֻ����һ��CHUNK�����ݣ�
    *             ����������ݵ���ʼλ�ÿ�ʼ��������CHUNK�м䣨����0�����������������ݴ�data_start
    *             ��ʼ
    * @param[in]  nodeindex   NODE������
    * @param[in]  data_start  ���ݵ���ʼλ��
    * @param[out] chunk_no    ���ز����������ʼλ�ã�λ�ڵڼ���Ͱ��
    * @param[out] szdata      ���ز��������ص����ݳ���
    * @param[out] outdata     ���ز��������Ͱ�Ĵ�data_start��ʼ��Ͱ����λ�õ����ݣ�
    */
    void get_chunkdata(const size_t nodeindex,
                       const size_t data_start,
                       size_t &chunk_no,
                       size_t &szdata,
                       char *outdata);

    /*!
    * @brief      �ͷ�ĳ��NODE�ڵ�
    * @param[in]  nodeindex  �ͷŵ�NODE������
    */
    void freenode(const size_t nodeindex);

    /*!
    * @brief      �õ�CHUNK�Ķ����С,ע������CHUNK�����������������ݵĴ�С
    * @return     size_t CHUNK�Ķ����С
    */
    size_t chunksize();

    /*!
    * @brief      ����ÿ��ȡһ��CHUNK��ָ�����������NODE�������ڼ���CHUNK,����
    *             CHUNK��ָ���Լ���Ӧ�ĳ���,ע��ָ�����������,���߳��������ע�����
    *             ������һЩΪ��׷�����ٶȣ���ϣ���������ݵĵط�
    * @param[in]  nodeindex   NODE����
    * @param[in]  chunk_no    �ڼ���CHUNK��
    * @param[out] szdata      ���CHUNK�����ݵĳ���
    * @param[out] chunk_point ���CHUNK��ʼ��ָ��
    */
    void get_chunk_point(const size_t nodeindex,
                         size_t chunk_no,
                         size_t &szdata,
                         char *&chunk_point);

    /*!
    * @brief      ���ڸ������ݵ���ʼλ�ã�ȡ�����λ������CHUNK��ָ��,�Լ�ȡ��
    *             �����CHUNK�����ʣ�������ʱ
    *             ������һЩΪ��׷�����ٶȣ���ϣ���������ݵĵط�
    * @param[in]  nodeindex        NODE����
    * @param[in]  data_start       ��ѯ�����ݵ���ʼλ��
    * @param[out] chunk_no         ���ز����������ʼλ�ã�λ�ڵڼ���Ͱ��
    * @param[out] szdata           ���ز��������ص����ݳ��ȣ���data_startλ�ÿ�ʼ�������CHUNK�����������ݳ���
    * @param[out] chunk_data_point ���ز��������data_startλ����CHUNK�е�λ��ָ��
    * @note       ע��ָ����������ڣ�����jovi������������һ�����ü������Ǻ�
    */
    void get_chunkdata_point(const size_t nodeindex,
                             const size_t data_start,
                             size_t &chunk_no,
                             size_t &szdata,
                             char *&chunk_data_point);

public:
    /*!
    * @brief      �õ���ʼ��������ڴ�ߴ磬��λ�ֽڣ���Ӧ�ø����������ȥ�����ڴ�
    * @return     size_t     ������ڴ��С��
    * @param[in]  numnode    NODE�ڵ�ĸ���
    * @param[in]  numchunk   CHUNK�ڵ�ĸ��������Ͱ���һ��NODE
    * @param[in]  szchunk    CHUNK�ĳߴ��С
    */
    static size_t getallocsize(const size_t numnode,
                               const size_t numchunk,
                               const size_t szchunk);

    /*!
    * @brief      ���ݲ�����ʼ����Ӧ���ڴ棬
    * @return     shm_cachechunk* �����ʼ���ɹ�������ָ������ʹ��
    * @param[in]  numnode     NODE�ڵ�ĸ���
    * @param[in]  numchunk    CHUNK�ڵ�ĸ���
    * @param[in]  szchunk     CHUNK�ĳߴ��С
    * @param[in]  pmmap       ������ڴ�ָ��
    * @param[in]  if_restore  �Ƿ�ָ�ԭ���ڴ�������
    */
    static shm_cachechunk *initialize(const size_t numnode,
                                      const size_t numchunk,
                                      const size_t szchunk,
                                      char *pmmap,
                                      bool if_restore = false);

protected:

    ///CACHE��ͷ����
    _shm_cachechunk_head     *cachechunk_head_ = nullptr;

    ///Cache NODE ��BASEָ��,NODE��ʾʹ�õ�
    cachechunk_node_index    *cachenode_base_ = nullptr;

    ///CHUNK INDEX��BASEָ��,
    size_t                   *chunkindex_base_ = nullptr;

    ///CHUNK DATA��������BASEָ��
    char                     *chunkdata_base_ = nullptr;
};

};

#endif //ZCE_LIB_SHM_CACHE_CHUNK_H_

