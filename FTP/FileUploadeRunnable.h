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
	CFileUploaderRunnable(const FFTPInfo& InFTPInfo, const FFTPSyncFileInfo& InSyncFileInfo);
	~CFileUploaderRunnable();

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
	atomic_bool bRun;
	FFTPInfo ftpInfo;
	FFTPSyncFileInfo syncFileInfo;
};