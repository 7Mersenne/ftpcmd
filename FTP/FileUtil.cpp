#include "FileUtil.h"

bool FTokenInfo::IsFolder()
{
	if (items.size() > 0)
	{
		return items[0].at(0) == 'd';
	}
	return false;
}

string FTokenInfo::GetName()
{
	if (items.size() >= 9)
	{
		return items[8];
	}

	return "";
}

string FTokenInfo::GetStyleName()
{
	if (items.size() >= 9)
	{
		if (IsFolder())
		{
			return items[8] + "/";
		}
		return items[8];
	}

	return "";
}

string FTokenInfo::GetSizeStr()
{
	if (items.size() >= 9)
	{
		if (IsFolder())
		{
			return items[4] + "/";
		}
		return items[4];
	}

	return "0";
}

FFTPInfo::FFTPInfo()
{
	ip = DEFAULT_IP;
	port = 21;
	username = DEFAULT_NAME;
	password = DEFAULT_PWD;
	clientRootPath = "C:\\FTPSyncDir";
	remoteDir = "/";
	bOverwriteFile = true;
}

FFTPInfo::FFTPInfo(const string & InIP, unsigned int InPort, const string & InName, const string & InPWD, const string & InPath, const string & InRemoteDir, bool InOverWrite)
	:ip(InIP)
	, port(InPort)
	, username(InName)
	, password(InPWD)
	, clientRootPath(InPath)
	, remoteDir(InRemoteDir)
	, bOverwriteFile(InOverWrite)
{
}
