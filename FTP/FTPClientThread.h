#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include "FileUtil.h"
using namespace std;

class FTPClientThread
{
public:
	FTPClientThread();
	FTPClientThread(const string& InIP, unsigned int InPort, const string& InName, const string& InPWD,  const string& InPath, const string& InRemoteDir, bool InOverWrite);
	~FTPClientThread();

	// async style
	// return future bool if the thread is stop correct
	future<bool> Start();
	void Stop();

	atomic_bool bPressSync;
	void OnPressSyncButton();

private:
	bool RunThread();
	void Run();
	void Dir(const string& InPath, bool bLog = true);

	atomic_bool bRun;
	thread* ptr_thread;
	FFTPInfo ftpInfo;
	bool bOverwriteFile;


	map<string, size_t> m_SyncFileMap;	// map fullpath to the index of m_ftpSyncList
	vector< FFTPSyncFileInfo > m_ftpSyncList; 
	vector< future<uint32_t> > m_taskStateFutureList;
};
