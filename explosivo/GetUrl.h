#ifndef _GETURL_SHUP_H
#define _GETURL_SHUP_H

#include <string>
#include "../../jnetlib/jnetlib.h"

class GetUrl
{
public:
  static std::string Gimme(const char *url, int use_post = 0)
  {
    JNL_HTTPGet *get = NULL;

    if (use_post)
      get = new JNL_HTTPPost();
    else
      get = new JNL_HTTPGet();

    JNL::open_socketlib();

    get->addheader("User-Agent: shup (Mozilla)");
    get->addheader("Accept:*/*");
    get->connect(url);

    std::string txt = "";

//    int headerstate=0;
//    int has_printed_headers=0;
//    int has_printed_reply=0;

    while (1)
    {
      int st=get->run();
      if (st<0)
      {
        //char tmp[1024];
        //wsprintf(tmp, "HTTP error: %s\n",get->geterrorstr());
        //MessageBox(NULL, tmp, "Error!", MB_OK);
        //throw(tmp);
        break;
      }
      if (get->get_status()>0)
      {
        if (get->get_status()==2)
        {
          int len;
          while ((len=get->bytes_available()) > 0)
          {
            char buf[4096];
            if (len > 4096) len=4096;
            len=get->get_bytes(buf,len);                  
            if (len>0) 
            {
              buf[len] = '\0';
              txt += buf; //fwrite(buf,len,1,fp);
            }
          }
        }
      }
      if (st==1) // 1 means connection closed
      {
        break;
      }
    }
    JNL::close_socketlib();
    delete get;

    return(txt);
  }
};

#endif // _GETURL_SHUP_H