#include "FileUploadeRunnable.h"

#include "FTPClient.h"
using namespace embeddedmz;

CFileUploaderRunnable::CFileUploaderRunnable(const FFTPInfo & InFTPInfo, const FFTPSyncFileInfo & InSyncFileInfo)
	: ftpInfo(InFTPInfo)
	, syncFileInfo(InSyncFileInfo)
{
	bRun = true;
}

CFileUploaderRunnable::~CFileUploaderRunnable()
{
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
				return 2;
			}
		}
		else {
			return 1;
		}

		break;
	}
	return 0;
}
