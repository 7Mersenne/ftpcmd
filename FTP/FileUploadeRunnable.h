#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <chrono>
#include <string>
#include "FileUtil.h"
using namespace std;

/*
*	error code:
*	0	:	success
*	1	:	init ftp session failed
*	2	:	upload runtime failed
*/
class CFileUploaderRunnable
{
public:
	CFileUploaderRunnable();
	CFileUploaderRunnable(const FFTPInfo& InFTPInfo, const FFTPSyncFileInfo& InSyncFileInfo);
	~CFileUploaderRunnable();

	void Setting(const FFTPInfo& InFTPInfo, const FFTPSyncFileInfo& InSyncFileInfo);

	future<uint32_t> Start();

	void Stop();

	atomic_bool bRun;

private: 
	/*
	* async usage example:
	```
	CFileUploaderRunnable uploader_obj(ftpInfo, syncFileInfo);
	future<uint32_t> _future = async(std::bind(&CFileUploaderRunnable::Run, uploader_obj));
	```
	* return if the upload task is success. 0 means success,  >0 means error code
	*	error code:
	*	0	:	success
	*	1	:	init ftp session failed
	*	2	:	upload runtime failed
	*/
	uint32_t Run();
	
private:
	FFTPInfo ftpInfo;
	FFTPSyncFileInfo syncFileInfo;
};