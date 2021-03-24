# ftpcmd
A ftp client multi-thread uploader for fast using ftpclient-cpp to upload huge count of files.
Wrapper ftpclient-cpp.
Require lib: curl.
MIT License.
Quick Build: Unzip bin.zip and move the *.dll and *.lib to ftpcmd/ftpcmd/

# 压力测试

* [x] 中文文件名   
* [x] 大规模文件上传
* [x] 文件重命名
* [ ] 数据库接口

# FTPClient扩展功能

...

# FTP client for C++

[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)

- [ftpclient-cpp](https://github.com/embeddedmz/ftpclient-cpp/)


# require lib : libcurl

- [libcurl](http://curl.haxx.se/libcurl/)

build version: curl-7.75.0

unzip bin.zip and move the *.dll and *.lib to ftpcmd/ftpcmd/

## Thread Safety

If you are using other libraries that make use of libcurl, please take a look at this issue (https://github.com/embeddedmz/ftpclient-cpp/issues/4). In short, remove from this project libcurl's global initialization/clean-up code, and manually call these functions, at your program startup/shutdown (or let your 3rd party library handle this).

Do not share CFTPClient objects across threads as this would mean accessing libcurl handles from multiple threads
at the same time which is not allowed.

The method SetNoSignal can be used to skip all signal handling. This is important in multi-threaded applications as DNS
resolution timeouts use signals. The signal handlers quite readily get executed on other threads.
