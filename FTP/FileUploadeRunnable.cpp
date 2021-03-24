#include "FileUploadeRunnable.h"

#include "FTPClient.h"
using namespace embeddedmz;

CFileUploaderRunnable::CFileUploaderRunnable()
{
	bRun = false;
}

CFileUploaderRunnable::CFileUploaderRunnable(const FFTPInfo & InFTPInfo, const FFTPSyncFileInfo & InSyncFileInfo)
	: ftpInfo(InFTPInfo)
	, syncFileInfo(InSyncFileInfo)
{
	bRun = false;
}

CFileUploaderRunnable::~CFileUploaderRunnable()
{
	bRun = false;
}

void CFileUploaderRunnable::Setting(const FFTPInfo & InFTPInfo, const FFTPSyncFileInfo & InSyncFileInfo)
{
	if (!bRun)
	{
		ftpInfo = InFTPInfo;
		syncFileInfo = InSyncFileInfo;
	}
}

uint32_t CFileUploaderRunnable::Run()
{
	while (bRun)
	{
#if FTP_CLIENT_DEBUG
		CFTPClient FTPClient([](const string& logMsg) {cout << logMsg << endl; });
#else
		CFTPClient FTPClient([](const string& logMsg) { });
#endif
		bool bSuccessInit = FTPClient.InitSession(ftpInfo.ip, ftpInfo.port, ftpInfo.username, ftpInfo.password);

		if (bSuccessInit)
		{
			// upload
			bool bSuccessUploaded = FTPClient.UploadFile(syncFileInfo.fullpath, syncFileInfo.remotepath, true);
			
			if (!bSuccessUploaded)
			{
				break;
			}
			else {
				bRun = false;
				return 2u;
			}
		}
		else {
			bRun = false;
			return 1u;
		}

		FTPClient.CleanupSession();

		break;
	}

	bRun = false;
	return 0;
}

future<uint32_t> CFileUploaderRunnable::Start()
{
	bRun = true;
	return async(std::bind(&CFileUploaderRunnable::Run, this));
}

void CFileUploaderRunnable::Stop()
{
	bRun = false;

}
