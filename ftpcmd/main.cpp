#include <iostream>
#include <FTP/FTPClient.h>
#include <FTP/FileUtil.h>
#include <FTP/DefaultConfig.h>
#include <FTP/FTPClientThread.h>
using namespace std;
using namespace embeddedmz;

void ftp_testcase_1(CFTPClient &FTPClient)
{
	FTPClient.InitSession(DEFAULT_IP, DEFAULT_PORT, DEFAULT_NAME,  DEFAULT_PWD );

	// create dir
	FTPClient.CreateDir("/1/test_dir3"); // if the dir is exist, return error = 21
	// remove dir
	//FTPClient.RemoveDir("/1/test_dir");

	// download
	FTPClient.DownloadFile("D:\\FTPSyncDir\\defaultLibrary.jpg", "/uploadFile/defaultPic/defaultLibrary.jpg");  // it cannot create dir in local win32 now
	//FTPClient.DownloadWildcard("D:\\FTPSyncDir\\test_alldir\\", "/uploadFile/*");// cannot create remote dir here
	//FTPClient.DownloadWildcard("D:\\FTPSyncDir\\", "/*");// download to local dir, still has too many bugs here.

	//FTPClient.DownloadFile("D:\\FTPSyncDir\\测试中文2.jpg", "/2/测试中文.jpg");  // it cannot create dir in local win32 now

	// upload
	FTPClient.UploadFile("D:\\FTPSyncDir\\defaultLibrary.jpg", "/2/defaultLibrary.jpg"); // cannot create remote dir here
	//FTPClient.UploadFile("D:\\FTPSyncDir\\测试中文.jpg", "/2/测试中文.jpg"); // cannot create remote dir here
	//FTPClient.RemoveFile("/output_test/output.txt.calculate.txt");

	cout << "list all dir :" << endl;
	string strList;
	FTPClient.List("/", strList, false); // only show file name , no dir now
	cout << strList << endl;
	cout << "======================" << endl;

	string tmptoken;
	vector<string> tokens;
	stringstream ss(strList);
	vector<FTokenInfo> tokenList;
	/*
	while (ss >> tmptoken)
	{
		tokens.push_back(tmptoken);
		cout << tmptoken << endl;
	}*/
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


	// request a remote file's size and mtime
	CFTPClient::FileInfo fileinfo = { 0,0.0 };
	FTPClient.Info("/test/hkk-1/3882-2.txt", fileinfo); // file name must  be right
	cout << "file size = " << fileinfo.dFileSize << "  mtime = " << fileinfo.tFileMTime << endl;
}

FTPClientThread*  ftp_testcase_2()
{
	FTPClientThread * ct = new FTPClientThread(DEFAULT_IP, 21, DEFAULT_NAME, DEFAULT_PWD, "D:\\FTPSyncDir", "/Sync", true);
	//FTPClientThread * ct = new FTPClientThread(DEFAULT_IP, 21, DEFAULT_NAME, DEFAULT_PWD, "D:\\专利和著作权", "/", true);

	auto _future = ct->Start();// future must return to a value, or the thread will be block here.

	this_thread::sleep_for(chrono::seconds(1));
	//cout << "main thread on press sync button" << endl;
	ct->OnPressSyncButton();

	return ct;
}

int main()
{
	cout << "FTP cmd in Windows" << endl;

	CFTPClient FTPClient([](const string& logMsg) {cout << logMsg << endl; });
	//ftp_testcase_1(FTPClient);

	FTPClientThread* ct = ftp_testcase_2();

	// ct cannot delete here. hold by another thread;


	system("pause");
}