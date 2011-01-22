/*
** JNetLib
** Copyright (C) Joshua Teitelbaum, sergent first class, 1014 army.
** Author: Joshua Teitelbaum
** File: ircclient.cpp
** License: see jnetlib.h
*/
#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#else
#define Sleep(x) usleep((x)*1000)
#endif
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include "jnetlib.h"
#include "ircclient.h"
#include <string>
#include <algorithm>
#include <list>

bool g_settrace = false;
void dbgstr(const char *str,...);

void settrace(bool b)
{
  g_settrace = b;
}
void dbgstr(const char *format,...)
{
    char buf[8192];
    va_list va;
    int len;

    if(g_settrace == 0)
    {
      return;
    }

    FILE *fp = fopen("c:\\shupdbg.txt", "a");

    if (!fp)
    return;

    buf[sizeof(buf) - 1] = 0;
    va_start(va, format);

#ifdef NO_vsnprintf
#  ifdef HAS_vsprintf_void
    (void)vsprintf(buf, format, va);
    va_end(va);
    for (len = 0; len < sizeof(buf); len++)
        if (buf[len] == 0) break;
#  else
    len = vsprintf(buf, format, va);
    va_end(va);
#  endif
#else
#  ifdef HAS_vsnprintf_void
    (void)vsnprintf(buf, sizeof(buf), format, va);
    va_end(va);
    len = strlen(buf);
#  else
    len = vsnprintf(buf, sizeof(buf), format, va);
    va_end(va);
#  endif

  fwrite(buf, strlen(buf), 1, fp);
  fwrite("\n", 1, 1, fp);
  fclose(fp);

#endif

}

bool JNL_IRCMessage::parse_NAMES(std::string &strchannel, std::map<std::string,int> &chanusers)
{
	bool bret = false;

	if(m_params.m_params == NULL)
	{
		return false;
	}


	if(m_params.m_params->m_params == NULL)
	{
		return false;
	}

	if(m_params.m_params->m_params->middle.length() <= 0)
	{
		return false;
	}

	if(m_params.m_params->m_params->m_params->trailing.length() <= 0)
	{
		return false;
	}

	std::string struserlist;
	std::string struser;
	struserlist = m_params.m_params->m_params->m_params->trailing;
	strchannel = m_params.m_params->m_params->middle;

	int x;
	int n;
	int op;
	n = struserlist.size();

	struser = "";
	op = JNL_IRC_USERSTATUS_PRESENT;

	for(x=0; x < n; x++)
	{
		if(struserlist[x] == ' ')
		{
			if(struser.length())
			{
				if(struser[0] == '@')
				{
					op = JNL_IRC_USERSTATUS_OPER;
					struser = struser.substr(1);
				}
				if(struser.length())
				{
					chanusers[struser] = op;
				}
				struser = "";
				op = JNL_IRC_USERSTATUS_PRESENT;
			}
		}
		else
		{
			struser += struserlist[x];
		}
	}

	if(struser.length() && chanusers.find(struser) == chanusers.end())
	{
		op = JNL_IRC_USERSTATUS_PRESENT;
		if(struser[0] == '@')
		{
			op = JNL_IRC_USERSTATUS_OPER;
			struser = struser.substr(1);
		}
		if(struser.length())
		{
			chanusers[struser] = op;
		}
	}

	bret = true;
	return bret;
}
bool JNL_IRCMessage::parse_LISTEND(std::string &channame)
{
	if(m_params.m_params == NULL)
	{
		return false;
	}

	if(m_params.m_params->middle.length() <= 0)
	{
		return false;
	}
	channame = m_params.m_params->middle;

	return true;
}
bool JNL_IRCMessage::parse_LIST(std::string &channame, int &occupants, std::string &strmode, std::string &topicstring)
{
	std::string sircstring;
	int x;
	int n;
	std::string smode;
	std::string stopic;
	if(m_params.m_params == NULL)
	{
		return false;
	}
	if(m_params.m_params->m_params == NULL)
	{
		return false;
	}

	if(m_params.m_params->m_params->middle.length() <= 0)
	{
		return false;
	}

	if(m_params.m_params->m_params->m_params == NULL)
	{
		return false;
	}

	sircstring = m_params.m_params->m_params->m_params->trailing;

	smode = "";
	stopic = "";
	for(x=0; x < sircstring.length(); x++)
	{
		if(sircstring[x] != ' ')
		{
			smode += sircstring[x];
		}
		else
		{
			break;
		}
	}

	x++;

	for(; x < sircstring.length();x++)
	{
		stopic += sircstring[x];
	}

	strmode = smode;
	topicstring = stopic;

	channame = m_params.m_params->middle;
	occupants = atoi(m_params.m_params->m_params->middle.c_str());

	return true;
}
bool JNL_IRCMessage::parse_JOIN(std::string &shortname, std::string &fullname, std::string &channel)
{
	bool bret = false;

	if(m_prefix.length() <= 0)
	{
		return false;
	}
	if(m_params.trailing.length() <= 0)
	{
		return false;
	}
	if(m_prefix[0] == ':')
	{
		fullname = m_prefix.substr(1);
	}

	int n;
	int x;

	n = fullname.length();

	for(x=0; x < n; x++)
	{
		if(fullname[x] == '!')
		{
			break;
		}
	}
	
	shortname = fullname.substr(0,x);
	channel = m_params.trailing;

	bret = true;
	return bret;
}

bool JNL_IRCMessage::parse_KICK(std::string &shortnameofkicker, std::string &channel, std::string &strwhokicked)
{
	bool bret = false;
  std::string strfullname;

	if(m_prefix.length() <= 0)
	{
		return false;
	}
	if(m_params.middle.length() <= 0)
	{
		return false;
	}
	if(m_params.m_params == NULL)
	{
		return false;
	}

	if(m_params.m_params->middle.length() <= 0)
	{
		return false;
	}

	
  channel = m_params.middle;
  strwhokicked = m_params.m_params->middle;

	if(m_prefix[0] == ':')
	{
		strfullname = m_prefix.substr(1);
	}

	int n;
	int x;

	n = strfullname.length();

	for(x=0; x < n; x++)
	{
		if(strfullname[x] == '!')
		{
			break;
		}
	}
	
	shortnameofkicker = strfullname.substr(0,x);

	bret = true;
	return bret;

}

bool JNL_IRCMessage::parse_TOPIC(std::string &shortname, std::string &fullname, std::string &channel, std::string &strtopic)
{
	bool bret = false;

	if(m_prefix.length() <= 0)
	{
		return false;
	}
	if(m_params.middle.length() <= 0)
	{
		return false;
	}
	if(m_params.m_params == NULL)
	{
		return false;
	}

	if(m_params.m_params->middle.length() <= 0)
	{
		return false;
	}


	if(m_params.m_params->m_params == NULL)
	{
		return false;
	}
	
	channel = m_params.m_params->middle;

	strtopic = m_params.m_params->m_params->trailing;

	if(m_params.middle[0] == ':')
	{
		fullname = m_params.middle.substr(1);
	}

	int n;
	int x;

	n = fullname.length();

	for(x=0; x < n; x++)
	{
		if(fullname[x] == '!')
		{
			break;
		}
	}
	
	shortname = fullname.substr(0,x);

	bret = true;
	return bret;
}
bool JNL_IRCMessage::parse_PART(std::string &shortname, std::string &fullname, std::string &channel)
{
	bool bret = false;

	if(m_prefix.length() <= 0)
	{
		return false;
	}
	if(m_params.middle.length() <= 0)
	{
		return false;
	}
	if(m_prefix[0] == ':')
	{
		fullname = m_prefix.substr(1);
	}

	int n;
	int x;

	n = fullname.length();

	for(x=0; x < n; x++)
	{
		if(fullname[x] == '!')
		{
			break;
		}
	}
	
	shortname = fullname.substr(0,x);
	channel = m_params.middle;
	
	bret = true;
	return bret;
}
bool JNL_IRCMessage::parse_QUIT(std::string &shortname, std::string &message)
{
	bool bret = false;

	std::string fullname;

	if(m_prefix.length() <= 0)
	{
		return false;
	}
	if(m_prefix[0] == ':')
	{
		fullname = m_prefix.substr(1);
	}

	message = m_params.trailing;
	int n;
	int x;

	n = fullname.length();

	for(x=0; x < n; x++)
	{
		if(fullname[x] == '!')
		{
			break;
		}
	}
	
	shortname = fullname.substr(0,x);
	
	bret = true;
	return bret;
}
bool JNL_IRCMessage::parse_NOSUCHNICK(std::string &shortname)
{
	bool bret = false;
  if(m_params.m_params && m_params.m_params->middle.length())
  {
    shortname = m_params.m_params->middle;
    bret =  true;
  }
	return bret;
}
bool JNL_IRCMessage::parse_CHANNELJOINPROBLEM(std::string &channame,std::string &strreason)
{
  channame = "";
  strreason = "";
  if(m_params.m_params == NULL)
  {
    return false;
  }
  if(m_params.m_params->middle.length() <= 0)
  {
    return false;
  }

  channame = m_params.m_params->middle;

  if(m_params.m_params->m_params == NULL)
  {
    return true;
  }

  strreason = m_params.m_params->m_params->trailing;
  
  return true;
}
bool JNL_IRCMessage::parse_PRIVMSG(std::string &shortname, std::string &fullname, std::string &channel, std::string &strmsg, bool &bisaction)
{
	bool bret = false;

	if(m_prefix.length() <= 0)
	{
		return false;
	}
	if(m_params.middle.length() <= 0)
	{
		return false;
	}
	if(m_params.m_params == NULL)
	{
		return false;
	}

	if(m_params.m_params->trailing.length() <= 0)
	{
		return false;
	}

	channel = m_params.middle;

	strmsg = m_params.m_params->trailing;

	if(m_prefix[0] == ':')
	{
		fullname = m_prefix.substr(1);
	}

	int n;
	int x;

	n = fullname.length();

	for(x=0; x < n; x++)
	{
		if(fullname[x] == '!')
		{
			break;
		}
	}
	if(strmsg.length() > 6)
	{
		if(strmsg[0] == 1)
		{
			std::string straction = "\1ACTION";
			std::string sa;
			sa = strmsg.substr(0,7);
			if(sa == straction)
			{
				bisaction = true;
				strmsg = strmsg.substr(7);
				if(strmsg.length() > 0)
				{
					if(strmsg[strmsg.length()-1] == '\1')
					{
						strmsg[strmsg.length()-1]=0;
					}
				}
			}

		}
	}
	shortname = fullname.substr(0,x);

	bret = true;
	return bret;
}
JNL_IRCConnection::~JNL_IRCConnection()
{
	if(m_con)
	{
		delete m_con;
	}
	m_con = NULL;


	std::list<JNL_IRCMessage*>::iterator it;

	for(it= m_messagequeue.begin(); it != m_messagequeue.end(); it++)
	{
		JNL_IRCMessage *pm = (*it);
		delete pm;
	}

	m_messagequeue.clear();

}
JNL_IRCConnection::JNL_IRCConnection()
{
	m_con = NULL;
}

bool JNL_IRCConnection::send_message(const char *fmt,...)
{
	int nbytes;
	if(fmt == NULL)
	{
		return false;
	}
	va_list argptr;
	char str[513];
	*str = 0;
	va_start (argptr, fmt);
#ifdef _WIN32
	nbytes = _vsnprintf (str, 512,fmt,argptr);
#else
	nbytes = vsnprintf (str, 512,fmt,argptr);
#endif
	str[512] = 0;
	va_end (argptr);

	if(m_con->send_string(str) < 0)
	{
		return false;
	}
	return nbytes > 0 ;
}

bool JNL_IRCConnection::connect(char *szHost, unsigned short nPort, int insz, int outsz)
{
	m_con = new JNL_Connection(JNL_CONNECTION_AUTODNS,insz,outsz);

	if(m_con == NULL)
	{
		return false;
	}
	m_con->connect(szHost,nPort);

	return true;
}

bool JNL_IRCConnection::run(int pollwait /* = -1 */)
{

	int			nlines;
	char		buf[4096];

	if (!m_con) return false; // error
	
	m_con->run(-1,-1,NULL,NULL,pollwait);

	if (m_con->get_state()==JNL_Connection::STATE_ERROR)
	{
    dbgstr("JNL_IRCConnection::run error state, unrolling");
		return false;
	}

	if (m_con->get_state()==JNL_Connection::STATE_CLOSED) return false;

	do
	{
		nlines = m_con->recv_lines_available();

		if(nlines < 0)
		{
      dbgstr("JNL_IRCConnection::run recv_lines_available < 0 unrolling");
			return false;
		}

		if(nlines == 0)
		{
			return true;
		}

		memset(buf,0,sizeof(buf));
		m_con->recv_line(buf,4095);
		buf[4095]=0;

    dbgstr("JNL_IRCConnection::run recv_line %s",buf);
		if(buf[0] && _process_line(buf) != true)
		{
      dbgstr("JNL_IRCConnection::run _process_line casued fail %s",buf);
			m_con->close(1);
			return false;
		}


	} while(nlines > 0);

	return true;
}

bool JNL_IRCConnection::_process_line(const char *szline)
{
	if(szline == NULL || szline[0] == 0)
	{
		return false;
	}

	JNL_IRCMessage *pm;

	pm = new JNL_IRCMessage();

	std::list<char*> params;
	char *prefix=NULL;
	char *command=NULL;
	int  nparams=0;
	int n=0;
	int x=0;
	int k=0;
	int bfound = 0;

	if(szline == NULL || szline[0] == 0)
	{
		return false;
	}

	n = strlen(szline);

	if(szline[0] == ':')
	{
		/*
		**  0123456789abcdef
		**  :prefix command params
		*/
		for(x=0; x < n; x++)
		{
			if(szline[x] == ' ')
			{
				bfound = 1;
				break;
			}
		}

		if(bfound)
		{
			prefix = (char*)alloca(x+1);
			memcpy(prefix,szline,x);
			prefix[x]=0;
			pm->m_prefix = prefix;
			x++;
		}
	}
	while(isalnum(szline[x+k]))
	{
		k++;
	}

	command = (char*)alloca(k+1);
	memcpy(command,szline+x,k);
	command[k] = 0;

	pm->m_command = command;

	x+= k;

	nparams = _parse_paramlist(szline+x,pm->m_params);

	m_messagequeue.push_back(pm);

	return pm->m_command.length() > 0;
}

/*
BNF NOTATION
<message> ::=
    [':' <prefix> <SPACE> ] <command> <params> <crlf>
<prefix> ::=
    <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
<command> ::=
    <letter> { <letter> } | <number> <number> <number>
<SPACE> ::=
    ' ' { ' ' }
<params> ::=
    <SPACE> [ ':' <trailing> | <middle> <params> ]
<middle> ::=
    <Any *non-empty* sequence of octets not including SPACE or NUL or CR or LF, the first of which may not be ':'>
<trailing> ::=
    <Any, possibly *empty*, sequence of octets not including NUL or CR or LF>
<crlf> ::=
    CR LF 
*/

int JNL_IRCConnection::_parse_paramlist(const char *szline,JNL_IRCParams &p)
{

	if(szline == NULL || szline[0] == 0)
	{
		return 0;
	}
	if(szline[0] != ' ')
	{
		return 0;
	}
	int len=0;
	int x=0;
	int nparams = 0;
	char *trailing=NULL;
	std::string str;

	JNL_IRCParams *pcur;

	pcur = &p;
	
	len = strlen(szline);

parampickup:

	x++;

	if(x >= len)
	{
		return nparams;
	}
	if(szline[x] == ':')
	{
		/*
		**  0123456789abcdef
		**   :the secret of#
		**   x is @ 1, len is 16
		**   we are to copy 14 (- the space and ':' )
		*/
		if(len - (x+1) > 0)
		{
			trailing = (char*)alloca( (len - (x+1) ) + 1);
			memcpy(trailing,szline + x + 1,(len - (x+1)));
			trailing[(len - (x+1))]=0;
			x += (len - (x+1));
			pcur->trailing = trailing;
			trailing = NULL;
			nparams++;
			return nparams;
		}
	}
	else
	{
		/*
		**  We are in
		** <params> ::=
			<SPACE> [ ':' <trailing> | <middle> <params> ]
			<middle> ::=
			<Any *non-empty* sequence of octets not including SPACE or NUL or CR or LF, the first of which may not be ':'>
			<trailing> ::=
			<Any, possibly *empty*, sequence of octets not including NUL or CR or LF>

		*/

		str = "";
		while((x < len) && !(szline[x]==' ' || szline[x] == '\r' || szline[x] == '\n'))
		{
			str += szline[x];
			x++;
		}
		pcur->middle = str;
		pcur->m_params = new JNL_IRCParams();
		pcur = pcur->m_params;
		goto parampickup;
	}

	return nparams;
}

JNL_IRCMessage *JNL_IRCConnection::get_message()
{
	JNL_IRCMessage *pm;

	pm = NULL;

	if(m_messagequeue.size() <= 0)
	{
		return pm;
	}

	pm = m_messagequeue.front();

	m_messagequeue.pop_front();

	return pm;

}
/*
**  sendpass
**  0 failure
**  1 success
*/
bool JNL_IRCClient::sendpass(char *password)
{
	if(password == NULL)
	{
		return 0;
	}
	return m_con.send_message("PASS %s\r\n",password);
}
bool JNL_IRCClient::nick(char *sznick)
{
	if(sznick == NULL)
	{
		return 0;
	}
	return m_con.send_message("NICK %s\r\n",sznick);
}
bool JNL_IRCClient::oper(char *sznick, char *szpassword)
{
	if(sznick == NULL)
	{
		return 0;
	}
	return m_con.send_message("OPER %s %s\r\n",sznick,(szpassword ? szpassword : ""));
}
bool JNL_IRCClient::quit(char *szswansong)
{
	if(szswansong == NULL)
	{
		szswansong = "";
	}
	return m_con.send_message("QUIT %s\r\n",szswansong);
}

bool JNL_IRCClient::part(char *szchannel)
{
	if(szchannel == NULL || strlen(szchannel) == 0)
	{
		return 0;
	}
	return m_con.send_message("PART %s\r\n",szchannel);
}
bool JNL_IRCClient::setmode(char *channel, int isadd, char mode, char *param)
{
	if(channel == NULL || strlen(channel) == 0)
	{
		return 0;
	}

	char *prefix;

	if(isadd)
	{
		prefix = "+";
	}
	else
	{
		prefix = "-";
	}

	switch(mode)
	{
		case 'o':
		case 'p':
		case 's':
		case 'i':
		case 't':
		case 'n':
		case 'b':
		case 'v':
		case 'k':
		{
		}
		break;

		default:
		{
			/*
			**  !?Invalid protocol?!
			*/
			return 0;
		}
		break;
	}

	if(param && strlen(param))
	{
		return m_con.send_message("MODE %s %s%c %s\r\n",channel,prefix,mode,param);
	}
	else
	{
		return m_con.send_message("MODE %s %s%c\r\n",channel,prefix,mode);
	}
}


bool JNL_IRCClient::topic(char *szchannel, char *sztopic)
{
	if(szchannel == NULL || strlen(szchannel) == 0)
	{
		return 0;
	}

	if(sztopic == NULL)
	{
		return m_con.send_message("TOPIC %s\r\n",szchannel);
	}
	else
	{
		return m_con.send_message("TOPIC %s :%s\r\n",szchannel,sztopic);
	}
}

bool JNL_IRCClient::names(char *szchannel)
{

	if(szchannel == NULL)
	{
		return m_con.send_message("NAMES\r\n");
	}
	else
	{
		return m_con.send_message("NAMES %s\r\n",szchannel);
	}
}
bool JNL_IRCClient::list(char *szchannel)
{
	
	if(szchannel == NULL)
	{
		return m_con.send_message("LIST\r\n");
	}
	else
	{
		return m_con.send_message("LIST %s\r\n",szchannel);
	}
}

bool JNL_IRCClient::invite(char *sznick, char *szchannel)
{
	if(sznick == NULL || strlen(sznick) == 0 || szchannel == NULL || strlen(szchannel) == 0)
	{
		return 0;
	}

	return m_con.send_message("INVITE %s %s\r\n",sznick,szchannel);
}
bool JNL_IRCClient::userregistration(char *username, char *hostname, char *servername, char *realname)
{
	if(username == NULL || hostname == NULL || servername == NULL || realname == NULL)
	{
		return 0;
	}
	return m_con.send_message("USER %s %s %s :%s\r\n",username,hostname,servername,realname);
}

bool JNL_IRCClient::join(char *channel, char *key)
{
	if(channel == NULL)
	{
		return 0;
	}

	if(key == NULL || strlen(key) <= 0)
	{
		return m_con.send_message("JOIN %s\r\n",channel);
	}
	else
	{
		return m_con.send_message("JOIN %s %s\r\n",channel,key);
	}
}

bool JNL_IRCClient::kick(char *szchannel, char *sznick, char *szswansong)
{
	if(szchannel == NULL || strlen(szchannel) == 0 || sznick == NULL || strlen(sznick) == 0)
	{
		return 0;
	}

	if(szswansong == NULL)
	{
		szswansong = "buhbyepoopyhead";
	}
	
	return m_con.send_message("KICK %s %s :%s\r\n",szchannel,sznick,szswansong);
}

bool JNL_IRCClient::version(char *szserver)
{
	if(szserver == NULL)
	{
		return m_con.send_message("VERSION\r\n");
	}
	else
	{
		return m_con.send_message("VERSION %s\r\n",szserver);
	}
}
bool JNL_IRCClient::stats(char query,char *szserver)
{
	if(query == 0)
	{
		return m_con.send_message("STATS\r\n");
	}
	else
	{
		switch(query)
		{
			case 'c':
			case 'h':
			case 'i':
			case 'k':
			case 'l':
			case 'm':
			case 'o':
			case 'y':
			case 'u':
			{
			}
			break;

			default:
			{
				return 0;
			}
		}
		if(szserver)
		{
			return m_con.send_message("STATS %c %s\r\n",query,szserver);
		}
		else
		{
			return m_con.send_message("STATS %c\r\n",query);
		}
	}
}
bool JNL_IRCClient::links(char *szremote, char *szmask)
{
	if(szremote == NULL || strlen(szremote) == 0)
	{
		return m_con.send_message("LINKS\r\n");
	}
	else
	{
		if(szmask == NULL || strlen(szmask) == 0)
		{
			return m_con.send_message("LINKS %s\r\n",szremote);
		}
		else
		{
			return m_con.send_message("LINKS %s %s\r\n",szremote,szmask);
		}
	}
}

bool JNL_IRCClient::time(char *szserver)
{
	if(szserver == NULL)
	{
		return m_con.send_message("TIME\r\n");
	}
	else
	{
		return m_con.send_message("TIME %s\r\n",szserver);
	}
}

bool JNL_IRCClient::admin(char *szserver)
{
	if(szserver == NULL)
	{
		return m_con.send_message("ADMIN\r\n");
	}
	else
	{
		return m_con.send_message("ADMIN %s\r\n",szserver);
	}
}

bool JNL_IRCClient::info(char *szserver)
{
	if(szserver == NULL)
	{
		return m_con.send_message("INFO\r\n");
	}
	else
	{
		return m_con.send_message("INFO %s\r\n",szserver);
	}
}

int JNL_IRCClient::baseprivmsgbytesneeded()
{
  static int x = 0;
  if(x==0)
  {
    x=strlen("PRIVMSG  :1ACTION 1\r\n");
  }
  return x;
}
int JNL_IRCClient::maxprivmsgbytes(const char *szchan)
{
	/*
	**  Assume the worst :)
	**  You're going to do /me bullshit *ALL THE TIME*
	*/
	std::string s = "PRIVMSG  :1ACTION 1\r\n";

	if(strlen(szchan) >= MAX_IRC_LINELEN)
	{
		/*
		**  wtf
		*/
		return 0;
	}
	return MAX_IRC_LINELEN - s.length();
}

int JNL_IRCClient::privmsgbytesover(const char *szchan, const char *txt, bool baction)
{
	std::string s;

	if(!baction)
	{
		s = "PRIVMSG  :\r\n";
		s += szchan;
		s += txt;
	}
	else
	{
		s = "PRIVMSG  :1ACTION 1\r\n";
		s += szchan;
		s += txt;
	}
	if(s.length() > MAX_IRC_LINELEN)
	{
		return s.length() - MAX_IRC_LINELEN;
	}

	return 0;
}
bool JNL_IRCClient::privmsg(char *szuser, char *txt, bool baction /*= false */)
{
	if(szuser == NULL || strlen(szuser) == 0 || txt == NULL || strlen(txt) == 0)
	{
		return 0;
	}
	
	if(!baction)
	{
		return m_con.send_message("PRIVMSG %s :%s\r\n",szuser,txt);
	}
	else
	{
		return m_con.send_message("PRIVMSG %s :%cACTION %s%c\r\n",szuser,1,txt,1);
	}
}
bool JNL_IRCClient::notice(char *szuser, char *txt)
{
	if(szuser == NULL || strlen(szuser) == 0 || txt == NULL || strlen(txt) == 0)
	{
		return 0;
	}
	return m_con.send_message("NOTICE %s :%s\r\n",szuser,txt);
}

bool JNL_IRCClient::who(char *szuser, char o)
{
	if(szuser == NULL || strlen(szuser) == 0 )
	{
		return 0;
	}
	
	if(o == 0)
	{
		return m_con.send_message("WHO %s\r\n",szuser);
	}
	else
	{
		return m_con.send_message("WHO %s o\r\n",szuser);
	}
}

bool JNL_IRCClient::whois(char *server, char *szuser)
{
	if(szuser == NULL || strlen(szuser) == 0)
	{
		return 0;
	}
	
	if(server == NULL || strlen(server) == 0)
	{
		return m_con.send_message("WHOIS %s\r\n",szuser);
	}
	else
	{
		return m_con.send_message("WHOIS %s %s\r\n",server,szuser);
	}
}

bool JNL_IRCClient::ping(char *szserver)
{
	if(szserver == NULL || strlen(szserver) == 0)
	{
		return 0;
	}
	return m_con.send_message("PING %s\r\n",szserver);	
}
bool JNL_IRCClient::pong(char *szserver)
{
	if(szserver == NULL || strlen(szserver) == 0)
	{
		return 0;
	}

	return m_con.send_message("PONG %s\r\n",szserver);
}

bool JNL_IRCClient::run(int pollwait /*=-1*/)
{
	
	if(m_con.run(pollwait) == false)
	{
    dbgstr("JNL_IRCClient::run failed");
		return false;
	}

	JNL_IRCMessage *pm = NULL;

	while( (pm = m_con.get_message()) != NULL )
	{
		process_message(pm);

	} 

	return true;
}
JNL_IRC_RETVAL JNL_IRCClient::process_message(JNL_IRCMessage *pm, bool bdeletemessage /*=true*/)
{
	JNL_IRC_RETVAL rv;
	JNL_IRCMessage &m = *pm;

  
	rv = JNL_IRC_RETVAL_OK;

	if(m.messagenum() == 0)
	{
		if(!strcmp("JOIN",m.command()))
		{
			rv = onJoin(m);
		}
		else if(!strcmp("PART",m.command()))
		{
			rv = onPart(m);
		}
		else if(!strcmp("MODE",m.command()))
		{
			rv = onMode(m);
		}
		else if(!strcmp("TOPIC",m.command()))
		{
			rv = onTopic(m);
		}
		else if(!strcmp("INVITE",m.command()))
		{
			rv = onInvite(m);
		}
		else if(!strcmp("KICK",m.command()))
		{
			rv = onKick(m);
		}
		else if(!strcmp("STATS",m.command()))
		{
			rv = onStats(m);
		}
		else if(!strcmp("PRIVMSG",m.command()))
		{
			rv = onPrivMsg(m);
		}
		else if(!strcmp("NOTICE",m.command()))
		{
			rv = onNotice(m);
		}
		else if(!strcmp("PING",m.command()))
		{
			rv = onPing(m);
		}
		if(bdeletemessage)
		{
			delete pm;
		}
		return rv;
	}
	switch(m.messagenum())
	{
		case ERR_NOSUCHNICK:
		{
			rv = onErrNoSuckNick(m);
		}
		break;
		case ERR_NOSUCHSERVER:
		{
			rv = onErrNoSuchServer(m);
		}
		break;
		case ERR_NOSUCHCHANNEL:
		{
			rv = onErrNoSuchChannel(m);
		}
		break;
		case ERR_CANNOTSENDTOCHAN:
		{
			rv = onErrCannotSendToChan(m);
		}
		case ERR_TOOMANYCHANNELS:
		{
			rv = onErrTooManyChannels(m);
		}
		case ERR_WASNOSUCHNICK:
		{
			rv = onErrWasNoSuchNick(m);
		}
		break;
		case ERR_TOOMANYTARGETS:
		{
			rv = onErrTooManyTargets(m);
		}
		break;
		case ERR_NOORIGIN:
		{
			rv = onErrNoOrigin(m);
		}
		break;
		case ERR_NORECIPIENT:
		{
			rv = onErrNoRecipient(m);
		}
		break;
		case ERR_NOTEXTTOSEND:
		{
			rv = onErrNoTextToSend(m);
		}
		break;
		case ERR_NOTOPLEVEL:
		{
			rv = onErrNoTopLevel(m);
		}
		break;
		case ERR_WILDTOPLEVEL:
		{
			rv = onErrWildTopLevel(m);
		}
		break;
		case ERR_UNKNOWNCOMMAND:
		{
			rv = onErrUnknownCommand(m);
		}
		break;
		case ERR_NOMOTD:
		{
			rv = onErrNoMOTD(m);
		}
		break;
		case ERR_NOADMININFO:
		{
			rv = onErrNoAdminInfo(m);
		}
		break;
		case ERR_FILEERROR:
		{
			rv = onErrFileError(m);
		}
		break;
		case ERR_NONICKNAMEGIVEN:
		{
			rv = onErrNoNickNameGiven(m);
		}
		break;
		case ERR_ERRONEUSNICKNAME:
		{
			rv = onErrOneUSNickName(m);
		}
		case ERR_NICKNAMEINUSE:
		{
			rv = onErrNickNameInUse(m);
		}
		break;
		case ERR_NICKCOLLISION:
		{
			rv = onErrNickCollision(m);
		}
		case ERR_USERNOTINCHANNEL:
		{
			rv = onErrUserNotInChannel(m);
		}
		break;
		case ERR_NOTONCHANNEL:
		{
			rv = onErrNotOnChannel(m);
		}
		break;
		case ERR_USERONCHANNEL:
		{
			rv = onErrUserOnChannel(m);
		}
		break;
		case ERR_NOLOGIN:
		{
			rv = onErrNoLogin(m);
		}
		break;
		case ERR_SUMMONDISABLED:
		{
			rv = onErrSummonDisabled(m);
		}
		break;
		case ERR_USERSDISABLED:
		{
			rv = onErrUserDisabled(m);
		}
		break;
		case ERR_NOTREGISTERED:
		{
			rv = onErrNotRegistered(m);
		}
		break;
		case ERR_NEEDMOREPARAMS:
		{
			rv = onErrNeedMoreParams(m);
		}
		break;
		case ERR_ALREADYREGISTRED:
		{
			rv = onErrAlreadyRegistered(m);
		}
		break;
		case ERR_NOPERMFORHOST:
		{
			rv = onErrNoPermForHost(m);
		}
		break;
		case ERR_PASSWDMISMATCH:
		{
			rv = onErrPasswdMismatch(m);
		}
		break;
		case ERR_YOUREBANNEDCREEP:
		{
			rv = onErrYourBannedCreep(m);
		}
		break;
		case ERR_KEYSET:
		{
			rv = onErrKeySet(m);
		}
		break;
		case ERR_CHANNELISFULL:
		{
			rv = onErrChannelIsFull(m);
		}
		break;
		case ERR_UNKNOWNMODE:
		{
			rv = onErrUnknownMode(m);
		}
		break;
		case ERR_INVITEONLYCHAN:
		{
			rv = onErrInviteOnlyChan(m);
		}
		break;
		case ERR_BANNEDFROMCHAN:
		{
			rv = onErrBannedFromChan(m);
		}
		break;
		case ERR_BADCHANNELKEY:
		{
			rv = onErrBadChannelKey(m);
		}
		break;
		case ERR_NOPRIVILEGES:
		{
			rv = onErrNoPrivileges(m);
		}
		break;
		case ERR_CHANOPRIVSNEEDED:
		{
			rv = onErrChanOpPrivsNeeded(m);
		}
		break;
		case ERR_CANTKILLSERVER:
		{
			rv = onErrCantKillServer(m);
		}
		break;
		case ERR_NOOPERHOST:
		{
			rv = onErrNoOperHost(m);
		}
		break;
		case ERR_UMODEUNKNOWNFLAG:
		{
			rv = onErrUModeUnknownFlag(m);
		}
		break;
		case ERR_USERSDONTMATCH:
		{
			rv = onErrUsersDontMatch(m);
		}
		break;
		/*
		**  Replies
		*/
		case RPL_NONE:
		{
			rv = onRplNone(m);
		}
		break;
		case RPL_USERHOST:
		{
			rv = onRplUserHost(m);
		}
		break;
		case RPL_ISON:
		{
			rv = onRplIsOn(m);
		}
		break;
		case RPL_AWAY:
		{
			rv = onRplAway(m);
		}
		break;
		case RPL_UNAWAY:
		{
			rv = onRplUnAway(m);
		}
		break;
		case RPL_NOWAWAY:
		{
			rv = onRplNoAway(m);
		}
		break;
		case RPL_WHOISUSER:
		{
			rv = onRplWhoIsUser(m);
		}
		break;
		case RPL_WHOISSERVER:
		{
			rv = onRplWhoIsServer(m);
		}
		break;
		case RPL_WHOISOPERATOR:
		{
			rv = onRplWhoIsOperator(m);
		}
		break;
		case RPL_WHOISIDLE:
		{
			rv = onRplWhoIsIdle(m);
		}
		break;
		case RPL_ENDOFWHOIS:
		{
			rv = onRplEndOfWhoIs(m);
		}
		break;
		case RPL_WHOISCHANNELS:
		{
			rv = onRplWhoIsChannels(m);
		}
		break;
		case RPL_WHOWASUSER:
		{
			rv = onRplWhoWasUser(m);
		}
		break;
		case RPL_ENDOFWHOWAS:
		{
			rv = onRplEndOfWhoWas(m);
		}
		break;
		case RPL_LISTSTART:
		{
			rv = onRplListStart(m);
		}
		break;
		case RPL_LIST:
		{
			rv = onRplList(m);
		}
		break;
		case RPL_LISTEND:
		{
			rv = onRplListEnd(m);
		}
		break;
		case RPL_CHANNELMODEIS:
		{
			rv = onRplChannelModeIs(m);
		}
		break;
		case RPL_NOTOPIC:
		{
			rv = onRplNoTopic(m);
		}
		break;
		case RPL_TOPIC:
		{
			rv = onRplTopic(m);
		}
		break;
		case RPL_WHOISREALIP:
		{
			rv = onRplWhoIsRealIP(m);
		}
		break;
		case RPL_INVITING:
		{
			rv = onRplInviting(m);
		}
		break;
		case RPL_SUMMONING:
		{
			rv = onRplSummoning(m);
		}
		break;
		case RPL_VERSION:
		{
			rv = onRplVersion(m);
		}
		break;
		case RPL_WHOREPLY:
		{
			rv = onRplWhoReply(m);
		}
		break;
		case RPL_ENDOFWHO:
		{
			rv = onRplEndWho(m);
		}
		break;
		case RPL_NAMREPLY:
		{
			rv = onRplNameReply(m);
		}
		break;
		case RPL_ENDOFNAMES:
		{
			rv = onRplEndOfNames(m);
		}
		break;
		case RPL_LINKS:
		{
			rv = onRplLinks(m);
		}
		break;
		case RPL_ENDOFLINKS:
		{
			rv = onRplEndOfLinks(m);
		}
		break;
		case RPL_BANLIST:
		{
			rv = onRplBanList(m);
		}
		break;
		case RPL_ENDOFBANLIST:
		{
			rv = onRplEndOfBanList(m);
		}
		break;
		case RPL_INFO:
		{
			rv = onRplInfo(m);
		}
		break;
		case RPL_ENDOFINFO:
		{
			rv = onRplEnofOfInfo(m);
		}
		break;
		case RPL_MOTDSTART:
		{
			rv = onRplMOTDStart(m);
		}
		break;
		case RPL_MOTD:
		{
			rv = onRplMOTD(m);
		}
		break;
		case RPL_ENDOFMOTD:
		{
			rv = onRplEndOfMOTD(m);
		}
		break;
		case RPL_YOUREOPER:
		{
			rv = onRplYoureOper(m);
		}
		break;
		case RPL_REHASHING:
		{
			rv = onRplRehasing(m);
		}
		break;
		case RPL_TIME:
		{
			rv = onRplTime(m);
		}
		break;
		case RPL_USERSSTART:
		{
			rv = onRplUserStart(m);
		}
		break;
		case RPL_USERS:
		{
			rv = onRplUser(m);
		}
		break;
		case RPL_ENDOFUSERS:
		{
			rv = onRplEndOfUsers(m);
		}
		break;
		case RPL_NOUSERS:
		{
			rv = onRplNoUsers(m);
		}
		break;
		case RPL_TRACELINK:
		{
			rv = onRplTraceLink(m);
		}
		break;
		case RPL_TRACECONNECTING:
		{
			rv = onRplTraceConnecting(m);
		}
		break;
		case RPL_TRACEHANDSHAKE:
		{
			rv = onRplTraceHandleShake(m);
		}
		break;
		case RPL_TRACEUNKNOWN:
		{
			rv = onRplTraceUnknown(m);
		}
		break;
		case RPL_TRACEOPERATOR:
		{
			rv = onRplTraceOperator(m);
		}
		break;
		case RPL_TRACEUSER:
		{
			rv = onRplTraceUser(m);
		}
		break;
		case RPL_TRACESERVER:
		{
			rv = onRplTraceServer(m);
		}
		break;
		case RPL_TRACENEWTYPE:
		{
			rv = onRplTraceNewType(m);
		}
		break;
		case RPL_TRACELOG:
		{
			rv = onRplTraceLog(m);
		}
		break;
		case RPL_STATSLINKINFO:
		{
			rv = onRplStatsLinkInfo(m);
		}
		break;
		case RPL_STATSCOMMANDS:
		{
			rv = onRplStatsCommands(m);
		}
		break;
		case RPL_STATSCLINE:
		{
			rv = onRplStatsCLine(m);
		}
		break;
		case RPL_STATSNLINE:
		{
			rv = onRplStatsNLine(m);
		}
		break;
		case RPL_STATSILINE:
		{			
			rv = onRplStatsILine(m);
		}
		break;
		case RPL_STATSKLINE:
		{
			rv = onRplStatsKLine(m);
		}
		break;
		case RPL_STATSYLINE:
		{
			rv = onRplStatsYLine(m);
		}
		break;
		case RPL_ENDOFSTATS:
		{
			rv = onRplEndOfStats(m);
		}
		break;
		case RPL_STATSLLINE:
		{
			rv = onRplStatsLLine(m);
		}
		break;
		case RPL_STATSUPTIME:
		{
			rv = onRplStatsUptime(m);
		}
		break;
		case RPL_STATSOLINE:
		{
			rv = onRplStatsOLine(m);
		}
		break;
		case RPL_STATSHLINE:
		{
			rv = onRplStatsHLine(m);
		}
		break;
		case RPL_STATSULINE:
		{
			rv = onRplStatsULine(m);
		}
		break;
		case RPL_UMODEIS:
		{
			rv = onRplUModeIs(m);
		}
		break;
		case RPL_LUSERCLIENT:
		{
			rv = onRplLUserClient(m);
		}
		break;
		case RPL_LUSEROP:
		{
			rv = onRplLUserOp(m);
		}
		break;
		case RPL_LUSERUNKNOWN:
		{
			rv = onRplLUserUknown(m);
		}
		break;
		case RPL_LUSERCHANNELS:
		{
			rv = onRplLUserChannels(m);
		}
		break;
		case RPL_LUSERME:
		{
			rv = onRplLUserMe(m);
		}
		break;
		case RPL_ADMINME:
		{
			rv = onRplAdminMe(m);
		}
		break;
		case RPL_ADMINLOC1:
		{
			rv = onRplAdminLoc1(m);
		}
		break;
		case RPL_ADMINLOC2:
		{
			rv = onRplAdminLoc2(m);
		}
		break;
		case RPL_ADMINEMAIL:
		{
			rv = onRplAdminEmail(m);
		}
		break;
		case RPL_LUSERLOCAL:
		{
			rv = onRplLUserLocal(m);
		}
		break;
		case RPL_LUSERGLOBAL:
		{
			rv = onRplLUserGlobal(m);
		}
		break;
	}
	if(bdeletemessage)
	{
		delete pm;
	}

	return rv;
}
