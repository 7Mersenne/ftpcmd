#include "FTPClientThread.h"
#include <iostream>
#include "FTPClient.h"
#include "DefaultConfig.h"
#include <io.h>
using namespace embeddedmz;


FTPClientThread::FTPClientThread()
{
	ptr_thread = nullptr;
	bRun = false;
	bPressSync = false;
	m_MaxUploaderCnt = 4;
	concurrentCount = 0;
}

FTPClientThread::FTPClientThread(const string & InIP, unsigned int InPort, const string & InName, const string & InPWD, const string& InPath, const string& InRemoteDir, bool InOverWrite)
	: ftpInfo(InIP, InPort, InName, InPWD, InPath, InRemoteDir, InOverWrite)
{
	ptr_thread = nullptr;
	bRun = false;
	bPressSync = false;
	m_MaxUploaderCnt = 4;
	concurrentCount = 0;
}

FTPClientThread::~FTPClientThread()
{
	Stop();
	if (ptr_thread) {
		//delete ptr_thread;
		//ptr_thread = nullptr;
	}

	for (auto itr = uploaderThreads.begin(); itr != uploaderThreads.end(); itr++)
	{
		delete *itr;
		*itr = nullptr;
	}

	for (auto elem: uploaderThreads)
	{
		elem->bRun = false;
	}
}

future<bool> FTPClientThread::Start()
{
	uploaderThreads.clear();
	for (auto i = 0; i < m_MaxUploaderCnt; ++i)
	{
		uploaderThreads.push_back(new CFileUploaderRunnable());
	}
	return async(std::bind(&FTPClientThread::RunThread, this));
}

void FTPClientThread::Run()
{
	CFTPClient FTPClient([](const string& logMsg) {cout << logMsg << endl; });
	bool bSuccessInit = FTPClient.InitSession(ftpInfo.ip, ftpInfo.port, ftpInfo.username, ftpInfo.password);
	string strList;

	if (!bSuccessInit)
	{
		// error
		cout << "login ftp failed, error username or password" << endl;
		return;
	}

	cout << "client thread start..." << endl;

	while (bRun)
	{
		{
			if (FTPClientThread::bFTPThreadLog)
				cout << "check local file list: " << endl;
			std::lock_guard<std::mutex> lock(m_mutexFileCards);
			// init cards
			InitLocalFileInfo(ftpInfo.clientRootPath, ftpInfo.remoteDir);
		}

		while (bPressSync) {
			cout << "on press sync button" << endl;
			bPressSync = false;

			{
				cout << "upload all" << endl;
				for (auto elem : m_ftpSyncList)
				{
					string _remote_path = elem.remotepath;
					if (0 == elem.stat || 2==elem.stat) {
						CFTPClient::FileInfo _remoteFInfo;

						if (bOverwriteFile) {
							if (FTPClient.Info(_remote_path, _remoteFInfo))
							{
								if ((size_t)_remoteFInfo.dFileSize == elem.filesize) continue;
							}
						}
						else {
							bool bSynced = false;
							int i = 1;
							while (FTPClient.Info(_remote_path, _remoteFInfo))
							{
								if ((size_t)_remoteFInfo.dFileSize == elem.filesize)
								{
									bSynced = true;
									break;
								}
								else {
									_remote_path = elem.remotepath + ".copy" + to_string(i);
								}
							}

							if (bSynced) continue;
						}
					}

					elem.stat = 1;

					FFTPSyncFileInfo tmpSyncFileInfo = elem;
					tmpSyncFileInfo.remotepath = _remote_path;

					future<int32_t> _future = async([](const FFTPInfo& info, const FFTPSyncFileInfo& e)
					{
						CFTPClient _FTPClient([](const string& logMsg) {cout << logMsg << endl; });
						_FTPClient.InitSession(info.ip, info.port, info.username, info.password);
						if (FTPClientThread::bFTPThreadLog)
						{
							cout << "upload begin:" << e.fullpath << " to " << e.remotepath <<"\n" << endl;
						}
						bool bSuccess = _FTPClient.UploadFile(e.fullpath, e.remotepath, true);
						if (FTPClientThread::bFTPThreadLog)
						{
							cout << "upload end:" << e.fullpath << " to " << e.remotepath << "\n" << endl;
						}
						_FTPClient.CleanupSession();

						if (bSuccess)
							return 2;
						return 3;
					}, ftpInfo, tmpSyncFileInfo);

					_future.wait();

					elem.stat = _future.get();
					this_thread::sleep_for(chrono::milliseconds(1));
				}
			}
		}

		this_thread::sleep_for(chrono::seconds(DEFAULT_SLEEP_SECOND));
	}

	FTPClient.CleanupSession();

	return;
}

void FTPClientThread::RunWithAsyncLambda()
{
	CFTPClient FTPClient([](const string& logMsg) {cout << logMsg << endl; });
	bool bSuccessInit = FTPClient.InitSession(ftpInfo.ip, ftpInfo.port, ftpInfo.username, ftpInfo.password);

	if (!bSuccessInit)
	{
		// error
		cout << "login ftp failed, error username or password" << endl;
		return;
	}

	cout << "client thread start..." << endl;

	while (bRun)
	{
		{
			if (FTPClientThread::bFTPThreadLog)
				cout << "check local file list: " << endl;
			std::lock_guard<std::mutex> lock(m_mutexFileCards);
			// init cards
			InitLocalFileInfo(ftpInfo.clientRootPath, ftpInfo.remoteDir);
		}

		while (bPressSync) {
			cout << "on press sync button" << endl;
			bPressSync = false;
			// list ftp dir begin====================================
			string strList;
			FTPClient.List("/", strList, false);
			string tmptoken;
			vector<string> tokens;
			stringstream ss(strList);
			vector<FTokenInfo> tokenList;

			while (getline(ss, tmptoken))
			{
				tokens.push_back(tmptoken);
				stringstream tmp_ss(tmptoken);
				string tmp_item;
				FTokenInfo tmp_tokeninfo;
				while (tmp_ss >> tmp_item)
				{
					tmp_tokeninfo.items.push_back(tmp_item);
				}
				tokenList.push_back(tmp_tokeninfo);
			}

			for (auto elem : tokenList)
			{
				cout << elem.GetStyleName() << endl;
			}

			// list ftp dir end====================================

			// dir sync path
			//Dir(ftpInfo.clientRootPath);

			{
				cout << "upload all" << endl;
				for (auto elem : m_ftpSyncList)
				{
					async([](FFTPInfo info, FFTPSyncFileInfo e)
					{
						CFTPClient _FTPClient([](const string& logMsg) {cout << logMsg << endl; });
						_FTPClient.InitSession(info.ip, info.port, info.username, info.password);
						if (FTPClientThread::bFTPThreadLog)
						{
							cout << "upload begin:" << e.fullpath << " to " << e.remotepath << endl;
						}
						_FTPClient.UploadFile(e.fullpath, e.remotepath, true);
						if (FTPClientThread::bFTPThreadLog)
						{
							cout << "upload end:" << e.fullpath << " to " << e.remotepath << endl;
						}
						_FTPClient.CleanupSession();
					}, ftpInfo, elem);
					this_thread::sleep_for(chrono::seconds(DEFAULT_SLEEP_SECOND));
				}
			}
		}

		this_thread::sleep_for(chrono::seconds(DEFAULT_SLEEP_SECOND));
	}

	FTPClient.CleanupSession();

	return;
}

void FTPClientThread::Dir(const string & InPath, bool bLog/* = true*/)
{
	intptr_t hFile = 0;
	struct _finddata_t fileInfo;
	string pathName, exdName;

	if ((hFile = _findfirst(pathName.assign(InPath).
		append("\\*").c_str(), &fileInfo)) == -1) {
		return;
	}
	do {
		if (fileInfo.attrib&_A_SUBDIR) {
			string fname = string(fileInfo.name);
			if (fname != ".." && fname != ".") {
				Dir(InPath + "\\" + fname);
			}
		}
		else {
			if(bLog)
				cout << InPath << "\\" << fileInfo.name << endl;
		}
	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);

	return;
}

void FTPClientThread::InitLocalFileInfo(const string & InPath, const string& InRemotePath)
{
	intptr_t hFile = 0;
	struct _finddata_t fileInfo;
	string pathName, exdName;

	if ((hFile = _findfirst(pathName.assign(InPath).
		append("\\*").c_str(), &fileInfo)) == -1) {
		return;
	}
	do {
		if (fileInfo.attrib&_A_SUBDIR) {
			string fname = string(fileInfo.name);
			if (fname != ".." && fname != ".") {
				InitLocalFileInfo(InPath + "\\" + fname, InRemotePath + "/" + fname);
			}
		}
		else {
			FFTPSyncFileInfo _info;
			_info.filename = string(fileInfo.name);
			_info.fullpath = InPath + "\\" + _info.filename;
			if (InRemotePath[InRemotePath.length() - 1] == '/')
			{
				_info.remotepath = InRemotePath + _info.filename;
			}
			else {
				_info.remotepath = InRemotePath + "/" + _info.filename;
			}
			
			_info.filesize = fileInfo.size;
			_info.uploadid = m_ftpSyncList.size();

			auto itr = m_syncFileMap.find(_info.fullpath);
			if (itr == m_syncFileMap.end())
			{
				m_ftpSyncList.push_back(_info);
				m_syncFileMap.insert(pair<string, size_t>(_info.fullpath, _info.uploadid));
			}
			else {
				_info.uploadid = itr->second;
				if (m_ftpSyncList.size() > _info.uploadid && _info.filesize != m_ftpSyncList[_info.uploadid].filesize) {
					// the file had been modified, need upload again.
					m_ftpSyncList[_info.uploadid] = _info;
				}
			}
			
			if (FTPClientThread::bFTPThreadLog)
				cout << InPath << "\\" << fileInfo.name << " -->"<< _info.stat << endl;
		}
	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);

	return;
}

const CFileUploaderRunnable * FTPClientThread::GetIdleUploader()
{
	for (size_t i = 0; i < uploaderThreads.size(); ++i)
	{
		if (!uploaderThreads[i]->bRun)
			return uploaderThreads[i];
	}
	return nullptr;
}

void FTPClientThread::Stop()
{
	bRun = false;
}

void FTPClientThread::OnPressSyncButton()
{
	bPressSync.store(true);
}

bool FTPClientThread::RunThread()
{
	if (bRun)
		return false;
	bRun = true;
	ptr_thread = new thread(std::bind(&FTPClientThread::Run, this));
	if (ptr_thread && ptr_thread->joinable())
	{
		ptr_thread->join();
	}
	else {
		// create thread failed
		return false;
	}

	return true;
}

bool FTPClientThread::bFTPThreadLog = true;
