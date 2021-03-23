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
}

FTPClientThread::FTPClientThread(const string & InIP, unsigned int InPort, const string & InName, const string & InPWD, const string& InPath, const string& InRemoteDir, bool InOverWrite)
	: ftpInfo(InIP, InPort, InName, InPWD, InPath, InRemoteDir, InOverWrite)
{
	ptr_thread = nullptr;
	bRun = false;
	bPressSync = false;
}

FTPClientThread::~FTPClientThread()
{
	Stop();
	if (ptr_thread) {
		//delete ptr_thread;
		//ptr_thread = nullptr;
	}
}

future<bool> FTPClientThread::Start()
{
	return async(std::bind(&FTPClientThread::RunThread, this));
}

void FTPClientThread::Run()
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
		cout << "running" << endl;
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
				if (FTPClientThread::bFTPThreadLog)
					cout << "local file list: " << endl;
				const std::lock_guard<std::mutex> lock(m_mutexFileCards);
				// init cards
				InitLocalFileInfo(ftpInfo.clientRootPath, ftpInfo.remoteDir);
			}

			{
				for (auto elem : m_ftpSyncList)
				{
					FTPClient.UploadFile(elem.fullpath, elem.remotepath, true);
				}
			}
			
		}

		this_thread::sleep_for(chrono::seconds(DEFAULT_SLEEP_SECOND));
	}

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
