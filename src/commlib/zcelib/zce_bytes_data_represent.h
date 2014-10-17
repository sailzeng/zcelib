#ifndef ZCE_LIB_BYTES_REPRESENT_H_
#define ZCE_LIB_BYTES_REPRESENT_H_


/*!
* @brief      
*             
* @note       
*/
class ZCE_DR_Encode
{
public:

    ///¹¹Ôìº¯Êý
    ZCE_DR_Encode(char *write_buf,size_t buf_len);

    ~ZCE_DR_Encode();

public:

    template<typename val_type>
    ZCE_DR_Encode & write(const val_type &);


    template<typename ary_type>
    ZCE_DR_Encode & write_array(const ary_type *);


    bool is_good();

protected:

    ///
    bool is_good_ = true;

    ///
    char *write_buf_;
    ///
    size_t buf_len_;

    ///
    char *end_pos_;
    ///
    char *write_pos_;
};



/*!
* @brief      
*             
* @note       
*/
class ZCE_DR_Decode
{
public:

    ZCE_DR_Decode(const char *read_buf,size_t buf_len);

    ~ZCE_DR_Decode();
public:

    template<typename val_type>
    ZCE_DR_Decode & read(const val_type &);


    template<typename ary_type>
    ZCE_DR_Decode & read(const ary_type *);

protected:

    ///
    bool is_good_;
    ///
    const char *read_buf_;
    ///
    size_t buf_len_;

    ///
    const char *end_pos_;
    ///
    const char *read_pos_;

};



#endif //ZCE_LIB_BYTES_REPRESENT_H_