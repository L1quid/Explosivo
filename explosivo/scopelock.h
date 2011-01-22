#ifndef SCOPE_LOCK_H
#define SCOPE_LOCK_H

class CShupMutexLock
{
protected:
  void *m_ptr;
  int m_nwaittimems;
  bool m_havelocked;

public:

#ifdef _WIN32
  CShupMutexLock(HANDLE ptr, int waittimems = INFINITE)
#else
   CShupMutexLock(void *ptr, int waittimems=-1)
#endif
   {
     m_havelocked = false;
     m_nwaittimems = waittimems;
    m_ptr = (void*)ptr;

    m_havelocked = Lock(m_nwaittimems);
   }

   ~CShupMutexLock()
   {
     if(m_havelocked)
     {
       UnLock();
     }
   }

  bool Lock(int waitms)
  {
    if(m_havelocked)
    {
      return true;
    }
    bool b = false;
#ifdef _WIN32
    //EnterCriticalSection((CRITICAL_SECTION *)m_ptr);
    //b = true;
    b =  (WaitForSingleObject((HANDLE)m_ptr,waitms) == WAIT_OBJECT_0);
#endif
    m_havelocked = b;
    return b;

  }

  bool HaveLocked()
  {
    return m_havelocked;
  }
  void UnLock()
  {
    if(m_havelocked)
    {
#ifdef _WIN32
      //LeaveCriticalSection((CRITICAL_SECTION *)m_ptr);
      ReleaseMutex((HANDLE)m_ptr);
#endif
      m_havelocked = false;

      return;
    }
  }
};

#endif