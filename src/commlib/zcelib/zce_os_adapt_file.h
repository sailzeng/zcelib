/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_file.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011骞�7鏈�10鏃�
* @brief      鏂囦欢鎿嶄綔鐨勯�傞厤鍣ㄥ眰锛屼富瑕佽繕鏄悜LINUX涓嬮潬鎷�
*
* @details    涓轰粈涔堝湪WINDOWS骞冲彴瑕佹彁渚涗竴缁勪互WINDOWS鐨凥ANDLE鍑芥暟鍛紝鑰屼笉鏄疌鏂囦欢鎻忚堪绗︾殑鍑芥暟鍛�
*             1.鍥犱负WINDOWS鍐呴儴鐨凙PI澶ч噺浣跨敤HANDLE浣滀负鍙傛暟鑰屼笉鏄痠nt fd,鎵�浠ユ垜涓嶈兘瀹屽叏渚濊禆CRT
*             2.鍥犱负WINDOWS涓婥RT鐨勫嚱鏁板疄鐜伴兘鏈変竴浜涚憰鐤碉紝姣斿lseek,(鏂扮殑鍊掓槸鏈変簡_lseek64)
*
*             杩樻湁涓�涓棶棰樺繀椤昏鏄庝竴涓嬶紝鏂囦欢闀垮害鐨勯棶棰橈紝鏂囦欢鎿嶄綔鐨勫緢澶氬弬鏁版垜鐢ㄤ簡ssize_t杩欎釜绫诲瀷锛�
*             LINUX涓嬪線寰�鏄痮ff_t锛宱ff_t澶ч儴鍒嗘儏鍐典笅鍜宻size_t琛ㄧ幇涓�鏍凤紝锛堥櫎闈炰綘鑷繁鍘荤紪璇戝唴鏍革級
*             ssize_t鍦�32浣嶇郴缁熶笅鏈�澶у�煎氨鏄�2G锛屽綋鐒跺悇绉峅S锛堝寘鎷�32浣嶏級鐨勬枃浠惰偗瀹氭槸鍙互绐佺牬2G,
*             杩欏氨缁欎娇鐢ㄥ甫鏉ヤ簡楹荤儲锛屽叾瀹炶繖鏄疧S鑷繁鐨勯夯鐑︼紝OS澶ч儴鍒嗛兘鎻愪緵涓�缁刋XXX64鐨勫嚱鏁帮紝姣斿
*             lseek64杩欐牱鐨勫嚱鏁版潵瑙ｅ喅杩欎釜闂锛�
*             浣嗗鏋滄垜涔熻杩欐牱锛屾垜鍙堣寰楁湁鐐瑰皬楹荤儲锛屽懙鍛点�備綔涓轰竴涓湇鍔″櫒寮�鍙戜汉鍛橈紝鎴戠殑杩愯鐜鍏跺疄
*             鍩烘湰涓婇兘鏄�64浣嶇殑锛岃�宻szie_t鍦�64浣峅S涓婁篃鏄�64浣嶏紙涓�涓鍙蜂綅锛夛紝鎵�浠ュぇ閮ㄥ垎鎯呭喌涓嬫垜娌�
*             楹荤儲銆備絾濡傛灉鍝ぉ闈炶鍦�32浣嶇殑绯荤粺涓婃姌鑵撅紝浼拌杩樿鎶樿吘涓�涓嬨��
*
* @note       鎴戞浘缁忓湪鍏朵腑鐨勫緢澶氬嚱鏁伴噰鐢ㄤ簡鍏堢敤C鍑芥暟杩涜鎿嶄綔锛岀劧鍚庡緱鍒癢INDOWS鍙ユ焺鐨勬柟寮�
*             _get_osfhandle杩涜杞崲锛岄渶瑕佽浆鎹㈠洖鏉ョ殑鏃跺�欑敤_open_osfhandle锛�
*             浣嗗叾瀹炴槸瀹屽叏璇浜哶open_osfhandle鍑芥暟鐨勬剰鍥撅紝浠栦笉鏄浆鎹紝鑰屾槸鍒涘缓锛�
*             http://www.cnblogs.com/fullsail/archive/2012/10/21/2732873.html
*/

#ifndef ZCE_LIB_OS_ADAPT_FILE_H_
#define ZCE_LIB_OS_ADAPT_FILE_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

/*!
* @brief      鎵撳紑涓�涓枃浠�
* @return     ZCE_HANDLE 鎵撳紑鏂囦欢鐨勫彞鏌�
* @param      filename   鏂囦欢鍚嶇О
* @param      open_mode  鎵撳紑鏂囦欢鐨勬ā寮忥紝涓や釜骞冲彴閫氱敤鐨勫弬鏁癘_CREAT锛孫_APPEND锛孫_EXCL锛孫_TRUNC锛孫_RDONLY, O_WRONLY, and O_RDWR澶ц嚧杩欏嚑涓�
* @param      perms      鏂囦欢鐨勫叡浜ā寮忥紝WINDOWS涓嬫垜浼氭牴鎹綘鐨勮緭鍏ヨ繘琛岃浆鎹紙鍏跺疄宸埆涓嶅皬锛夛紝浣犲彲浠ヤ娇鐢↙INUX涓嬬殑鍏变韩鏂瑰紡鍙傛暟
* @note       ZCE_DEFAULT_FILE_PERMS 搴旇鏄�0660
*/
ZCE_HANDLE open(const char* filename,
                int open_mode,
                mode_t perms=ZCE_DEFAULT_FILE_PERMS);

/*!
* @brief      鍏抽棴涓�涓枃浠�
* @return     int ==0鏍囪瘑鎴愬姛
* @param      handle 鏂囦欢鍙ユ焺
*/
int close(ZCE_HANDLE handle);

/*!
* @brief      璇诲彇鏂囦欢锛學INDOWS涓嬶紝闀垮害鏃犳硶绐佺牬32浣嶇殑
* @return     ssize_t 閿欒杩斿洖-1锛岋紝姝ｇ‘杩斿洖璇诲彇鐨勫瓧鑺傞暱搴︼紙涔熷彲鑳戒负0锛夛紝errno 琛ㄧず閿欒鍘熷洜
* @param      file_handle 鏂囦欢鍙ユ焺
* @param      buf 璇诲彇鐨刡uffer鍙傛暟
* @param      count buffer鐨勯暱搴︼紝WINDOWS涓嬶紝闀垮害鏃犳硶绐佺牬32浣嶇殑锛屽綋鐒垛�︹�︼紙褰撶劧鍏跺疄鍑芥暟鐨勮涔夌湅32浣嶄笅鍙兘鏄�2G锛夛紝浣犳噦寰�
*/
ssize_t read(ZCE_HANDLE file_handle,void* buf,size_t count);

/*!
* @brief      鍐欏鏂囦欢锛�
* @return     ssize_t 閿欒杩斿洖-1锛岋紝姝ｇ‘杩斿洖璇诲彇鐨勫瓧鑺傞暱搴︼紙涔熷彲鑳戒负0锛夛紝errno 琛ㄧず閿欒鍘熷洜
* @param      file_handle 鏂囦欢鍙ユ焺
* @param      buf 璇诲彇鐨刡uffer鍙傛暟
* @param      count buffer鐨勯暱搴︼紝WINDOWS涓嬶紝闀垮害鏃犳硶绐佺牬32浣嶇殑锛屽綋鐒舵湁浜洪渶瑕佸啓鍏�4G鏁版嵁鍚楋紵
* @note       娉ㄦ剰Windows涓嬮粯璁よ皟鐢ㄧ殑WriteFile杩樻槸鏈夌紦鍐茬殑锛屾垜涓轰簡鍜孭OSIX缁熶竴锛岃繕鏄敤浜咶lushFileBuffers
*/
ssize_t write(ZCE_HANDLE file_handle,const void* buf,size_t count);

/*!
* @brief      鍦ㄦ枃浠跺唴杩涜鍋忕Щ
* @return     ssize_t 杩斿洖褰撳墠鐨勪綅缃�
* @param      file_handle
* @param      offset
* @param      whence
*/
ssize_t lseek(ZCE_HANDLE file_handle,ssize_t offset,int whence);

/*!
* @brief      鏂枃浠讹紝鍊掗湁鐨勬槸WINDOWS涓嬪張TMD 娌℃湁锛�
* @return     int ==0 鏍囪瘑鎴愬姛
* @param      file_handle 鎿嶄綔鐨勬枃浠跺彞鏌�
* @param      offset      鎴柇鐨勫ぇ灏�
*/
int ftruncate(ZCE_HANDLE file_handle,size_t  offset);


///鎴柇鏂囦欢,鏍规嵁鏂囦欢鍚嶇О
int truncate(const char *filename, size_t offset);

/*!
* @brief      閫氳繃鏂囦欢鍙ユ焺,寰楀埌鏂囦欢鐨勯暱搴︼紝鍏充簬鏂囦欢闀垮害鐨勯棶棰橈紝浣犲簲璇ョ湅鐪嬫枃浠跺紑濮嬬殑娉ㄩ噴娈佃惤
* @return     int 杩斿洖-1鏍囪瘑澶辫触锛岃繑鍥�0鏍囪瘑鎴愬姛
* @param      file_handle 鏂囦欢鍙ユ焺
* @param      file_size   鏂囦欢鐨勫ぇ灏忥紝
* @note       杩欐槸涓�涓潪鏍囧噯鍑芥暟锛屽皯閲忕郴缁熸彁渚涘嚱鏁皁ff_t filesize(int file_handle);
*             鑰冭檻鏉ヨ�冭檻鍘伙紝涓嶄娇鐢╫ff_t浣滀负杩斿洖鍊兼垨鑰呭弬鏁帮紝鍥犱负鍦╳in涓嬪彧琚畾涔夋垚long,
*             鍘熸潵鐢ㄤ綔涓鸿繑鍥炲�硷紝浣嗙敱浜庡湪WINDOWS32涓嬩笉瓒呰繃2G锛屾劅瑙変笉濂斤紝鍙堟敼浜嗘垚浜唖ize_t锛�
*             浣嗘槸鐢╯ize_t浣滀负杩斿洖鍊煎張鎭跺績,0鍒板簳鏄敊璇繕鏄昂瀵�0锛熸墍浠ュ嚱鏁拌鏀规垚浜嗚繖鏍凤紝涓涓煩
*/
int filesize(ZCE_HANDLE file_handle,size_t* file_size);

///
/*!
* @brief      閫氳繃鏂囦欢鍚嶇О,鍙栧緱鏂囦欢鐨勯暱搴︼紝
* @return     int  杩斿洖-1鏍囪瘑澶辫触锛岃繑鍥�0鏍囪瘑鎴愬姛
* @param      filename  鏂囦欢
* @param      file_size 鏂囦欢鐨勫ぇ灏�
* @note       涓嶉噸杞絝ilesize鐨勫師鍥犳槸filelen锛宖ilesize绗竴涓弬鏁伴兘鍙兘鏄寚閽�
*/
int filelen(const char* filename,size_t* file_size);

/*!
* @brief      鐢ㄦā鐗堝悕绉板缓绔嬪苟涓旀墦寮�涓�涓复鏃舵枃浠讹紝
* @return     ZCE_HANDLE 鏂囦欢鍙ユ焺
* @param      template_name 涓存椂鏂囦欢鐨勬ā鐗堝悕绉�
*/
ZCE_HANDLE mkstemp(char* template_name);


//寰楀埌鏂囦欢鐨剆tat淇℃伅锛屼綘鍙互璁や负zce_os_stat灏辨槸stat锛屽彧鏄湪WINDOWS涓媠tat64,涓昏鏄负浜嗛暱鏂囦欢鑰冭檻鐨�
int stat(const char *path, zce_os_stat *buf);
//閫氳繃鏂囦欢鍙ユ焺寰楀埌鏂囦欢stat
int fstat(ZCE_HANDLE file_handle, zce_os_stat *buf);


///璺緞鏄惁鏄竴涓洰褰曪紝濡傛灉鏄繑鍥瀟rue锛屽鏋滀笉鏄繑鍥瀎alse
bool is_directory(const char *path_name);



/*!
* @brief      鍒犻櫎鏂囦欢
* @return     int == 0鏍囪瘑鎴愬姛锛�
* @param      filename 鏂囦欢鍚嶇О
*/
int unlink(const char* filename);


///璁剧疆umask
mode_t umask (mode_t cmask);


/*!
* @brief      妫�鏌ユ枃浠舵槸鍚K锛屽惣鍚�
* @return     int       ==0 琛ㄧず姝ｇ‘锛岄敊璇繑鍥�-1
* @param      pathname  鏂囦欢璺緞鍚嶇О
* @param      mode      鏂囦欢妯″紡锛屾敮鎸丗_OK,R_OR,W_OK,鎴栬�呭嚑涓殑|锛� Windows 涓嶆敮鎸乆_OK锛孡INUX鏀寔
*/
int access(const char* pathname,int mode);

//-------------------------------------------------------------------------------------------------

/*!
* @brief      闈炴爣鍑嗗嚱鏁�
* @return     int
* @param[in]  filename 鐢ㄥ彧璇绘柟寮忚鍙栦竴涓枃浠剁殑鍐呭锛�
* @param[in]  buff     璇诲彇鐨刡uffer
* @param[in]  buf_len  buffer鐨勯暱搴�
* @param[out] read_len 杈撳嚭鍙傛暟锛岃繑鍥炶鍙栫殑闀垮害
* @param[in]  offset   寮�濮嬭鍙栫殑鍋忕Щ鏄灏戯紝浠庢枃浠跺ご浣嶇疆璁＄畻锛�
*/
int read_file_data(const char* filename,
                   char* buff,
                   size_t buf_len,
                   size_t* read_len,
                   size_t offset=0);



/*!
* @brief      鐢–++ 11鐨勬柟寮忓寘瑁呬竴涓粰浣犮��
* @return     std::pair<int,std::unique_ptr<char*>>
* @param[in]  filename
* @param[out] read_len 杈撳嚭鍙傛暟锛岃繑鍥炶鍙栫殑闀垮害
* @param[in]  offset
* @note       Windows涓嬮潰涓嶅彲鑳戒竴娆¤鍙栬秴杩�4G澶у皬鐨勬暟鎹絾锛岃秴澶ф枃浠跺埆鎸囨湜杩欎釜鍑芥暟
*/
std::pair<int,std::shared_ptr<char>> read_file_all(const char* filename,
                                                   size_t* file_len,
                                                   size_t offset=0);



/*!
* @brief     鐢ㄤ簬鏂囦欢澶勭悊杩囩▼鐨勮嚜鍔ㄩ噴鏀�
*/
inline void close_FILE_assist(FILE* to_close)
{
    ::fclose(to_close);
}


    ///鏋勯�犲嚱寰楀埌鏂囦欢鍙ユ焺
    close_assist(close_type to_close):
        to_close_(to_close)
    {
    }
    //鍒╃敤鏋愭瀯鍑芥暟鍏抽棴鏂囦欢
    ~close_assist();


protected:

    ///鑷姩澶勭悊鐨勬枃浠跺彞鏌�
    close_type    to_close_;
};


template<>
class close_assist<FILE*>
{
public:

    ///鏋勯�犲嚱寰楀埌鏂囦欢鍙ユ焺
    close_assist(FILE* to_close):
        to_close_(to_close)
    {
    }
    //鍒╃敤鏋愭瀯鍑芥暟鍏抽棴鏂囦欢
    ~close_assist()
    {
        ::fclose(to_close_);
    }
protected:

    FILE* to_close_;
};


inline void close_stdFILE_assist(std::FILE* to_close)
{

public:

    ///鏋勯�犲嚱寰楀埌鏂囦欢鍙ユ焺
    close_assist(ZCE_HANDLE to_close):
        to_close_(to_close)
    {
    }
    //鍒╃敤鏋愭瀯鍑芥暟鍏抽棴鏂囦欢
    ~close_assist()
    {
        zce::close(to_close_);
    }
protected:

    ZCE_HANDLE to_close_;
};

};



#endif //ZCE_LIB_OS_ADAPT_FILE_H_

