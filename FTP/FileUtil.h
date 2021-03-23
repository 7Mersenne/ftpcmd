#pragma once

#include <string>
#include <vector>
using namespace std;

#include "DefaultConfig.h"

struct FTokenInfo
{
	vector<string> items;
	bool IsFolder();
	string GetName();
	string GetStyleName();
	string GetSizeStr();
};

struct FFTPSyncFileInfo
{
	string filename;
	string fullpath;
	string remotepath;
	size_t filesize;
	size_t uploadid;
	int32_t stat;// 0=ready; 1=upload, 2=success, 3=failed

	FFTPSyncFileInfo();
	FFTPSyncFileInfo(const FFTPSyncFileInfo& InInfo);
};

struct FFTPInfo
{
	string ip;
	unsigned int port;
	string username;
	string password;
	string clientRootPath;
	string remoteDir;
	bool bOverwriteFile;

	FFTPInfo();
	FFTPInfo(const string & InIP, unsigned int InPort, const string & InName, const string & InPWD, const string& InPath, const string& InRemoteDir, bool InOverWrite);
	FFTPInfo(const FFTPInfo& InInfo);
};
