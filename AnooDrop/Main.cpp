#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <iterator>
#include "direct.h"
#include "Words.h"
using namespace std;

void toClipboard(HWND hwnd, const std::string &s);

#define HOST "anooserve.com"
#define USERNAME "USERNAME"
#define PASSWORD "PASSWORD"

string genRandom()
{
	string sentence;
	sentence = verbs[rand() % verbs.size()];
	sentence += adjectives[rand() % adjectives.size()];
	sentence += nouns[rand() % nouns.size()];
	return sentence;
}

void toClipboard(HWND hwnd, const std::string &s)
{
	OpenClipboard(hwnd);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
	if (!hg)
	{
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

HKEY OpenKey(HKEY hRootKey, char* strKey)
{
	HKEY hKey;
	LONG nError = RegOpenKeyEx(hRootKey, strKey, NULL, KEY_ALL_ACCESS, &hKey);

	if (nError == ERROR_FILE_NOT_FOUND)
	{
		cout << "Creating registry key: " << strKey << endl;
		nError = RegCreateKeyEx(hRootKey, strKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	}

	if (nError)
		cout << "Error: " << nError << " Could not find or create " << strKey << endl;

	return hKey;
}

bool install()
{
	char data[256];
	_getcwd(data, sizeof(data));
	string cwd = data;
	cwd = "\"" + cwd + "\\Anoodrop.exe\" upload \"%1\"";
	strcpy(data, cwd.c_str());
	HKEY hKey = OpenKey(HKEY_CLASSES_ROOT, "*\\shell\\anooserve\\command");

	LONG nError = RegSetValueEx(hKey, NULL, NULL, REG_SZ, (LPBYTE)data, sizeof(data));

	if (nError)
		cout << "Error: " << nError << " Could not set registry value: " << "command" << endl;

	RegCloseKey(hKey);

	hKey = OpenKey(HKEY_CLASSES_ROOT, "*\\shell\\anooserve");

	char data2[120] = { "Upload to Anooserve" };
	nError = RegSetValueEx(hKey, NULL, NULL, REG_SZ, (LPBYTE)data2, sizeof(data2));

	if (nError)
		cout << "Error: " << nError << " Could not set registry value: " << "anooserve" << endl;

	return true;
}

int main(int argc, char *argv[])
{
	switch (argc)
	{
		case(1) :
		case(2) : {
			std::cout << "Installing registry key...\r\n";
			if (install())
			{
				std::cout << "Registry key installed!\r\n";
			}
			else {
				std::cout << "Registry key not installed, try running as admin\r\n";
			}
			return 0;
		}
		case(3) : {
			if (std::string(argv[1]) == "upload") {
				break;
			}
			return 0;
		}
		default:
			return 0;
	}
	string total = argv[2];
	HINTERNET hInternet;
	HINTERNET hFtp;
	cout << "(1/4) Internet: ";
	hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet)
	{
		cerr << "Error " << GetLastError() << endl;
		return 2;
	}
	cout << "Connected" << endl << "(2/4) Anooserve: ";

	hFtp = InternetConnect(hInternet, HOST, INTERNET_DEFAULT_FTP_PORT, USERNAME, PASSWORD, INTERNET_SERVICE_FTP, 0, 0);
	if (!hFtp)
	{
		cerr << "Error " << GetLastError() << endl;
		return 2;
	}
	cout << "Connected" << endl << "(3/4) Generating name:  ";

	string extension = total;
	int index = extension.find_last_of(".");
	if (index == -1)
	{
		extension = "";
	}
	else
	{
		extension = extension.substr(index);
	}
	string what;
	srand(time(0));
	what = genRandom();
	what += extension;
	LPCTSTR uploadc = what.c_str();
	cout << what << endl << "(4/4) Uploading: ";
	if (!FtpPutFile(hFtp, total.c_str(), uploadc, FTP_TRANSFER_TYPE_BINARY, 0))
	{
		if (GetLastError() == 12002)
		{
			cout << "Timed Out" << endl;
			return 1;
		}
		else
		{
			cout << "Failed to upload: " << GetLastError() << endl;
			return 1;
		}
	}
	cout << " Success!" << endl << "Link copied to clipboard" << endl;
	string link = "https://anooserve.com/s/" + what;
	HWND hwnd = GetDesktopWindow();
	toClipboard(hwnd, link);
	InternetCloseHandle(hFtp);
	InternetCloseHandle(hInternet);
	return 0;
}