#include "stdafx.h"
#include "SRBand.h"
#include "resource.h"
#include <algorithm>

INT_PTR CALLBACK CDeskBand::OptionsDlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			CDeskBand * pThis = (CDeskBand*)lParam;
			HWND hwndOwner; 
			RECT rc, rcDlg, rcOwner;

			SetLastError(0);
			if ((SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pThis)==0)&&(GetLastError()))
				EndDialog(hwndDlg, IDCANCEL);	// if we could not set the user data, get out of here immediately!

			hwndOwner = ::GetParent(hwndDlg);
			if (hwndOwner == NULL)
				hwndOwner = ::GetDesktopWindow();

			GetWindowRect(hwndOwner, &rcOwner); 
			GetWindowRect(hwndDlg, &rcDlg); 
			CopyRect(&rc, &rcOwner); 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
			OffsetRect(&rc, -rc.left, -rc.top); 
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

			SetWindowPos(hwndDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0,	SWP_NOSIZE); 
			//HICON hIcon = (HICON)::LoadImage(g_hInst, MAKEINTRESOURCE(IDI_RENDLG), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_SHARED);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			SendMessage(GetDlgItem(hwndDlg, IDC_SHOWTEXT), BM_SETCHECK, DWORD(pThis->m_regShowBtnText) ? BST_CHECKED : BST_UNCHECKED, 0);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				CDeskBand * pThis = (CDeskBand*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				pThis->m_regShowBtnText = SendMessage(GetDlgItem(hwndDlg, IDC_SHOWTEXT), BM_GETCHECK, 0, 0);
			}
			// fall through
		case IDCANCEL:
			EndDialog(hwndDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		case IDC_EDITCONFIG:
			{
				// find custom commands ini file
				TCHAR szPath[MAX_PATH] = {0};
				if (SUCCEEDED(SHGetFolderPath(NULL, 
					CSIDL_APPDATA|CSIDL_FLAG_CREATE, 
					NULL, 
					SHGFP_TYPE_CURRENT, 
					szPath))) 
				{
					PathAppend(szPath, TEXT("StExBar"));
					CreateDirectory(szPath, NULL);
					PathAppend(szPath, TEXT("CustomCommands.ini"));
					// if the file does not exist (yet), we create one and
					// fill it with helpful comments
					if (!PathFileExists(szPath))
					{
						FILE * stream;
						// Open for write 
						errno_t err = _tfopen_s(&stream, szPath, _T("w"));
						if (err == 0)
						{
							fputs("# configuration file\n"
								"#\n"
								"# the format of a custom command is as follows:\n"
								"# [name of the command]\n"
								"# name = text shown in the toolbar for the button\n"
								"# icon = path to an icon file with 16x16 size\n"
								"# commandline = the command line to execute\n"
								"#\n"
								"# example configuration for a new custom command:\n"
								"# [Notepad]\n"
								"# name = Editor\n"
								"# icon = c:\\icons\\editor.ico\n"
								"# commandline = c:\\windows\\system32\\notepad.exe %selpaths\n"
								"#\n"
								"# some special placeholders are available\n"
								"# %selpaths : will be replaced with the paths of the selected items\n"
								"#             note: if too many items are selected, the command line\n"
								"#             could get too long and get truncated!\n"
								"# %selnames : will be replaced with the names of the selected items\n"
								"# %curdir   : will be replaced with the path of the currently shown\n"
								"#             directory.\n"
								"#             note: not all views in explorer have a defined directory\n"
								"#             for example the search window does not have a 'current\n"
								"#             directory', and there the %curdir is not defined\n"
								"#\n"
								"#\n"
								, stream);
						}

						// Close stream if it is not NULL 
						if (stream)
							fclose(stream);
					}
					PathQuoteSpaces(szPath);
					// now start the default text editor with our config file
					wstring viewer;
					CRegStdString txt = CRegStdString(_T(".txt\\"), _T(""), FALSE, HKEY_CLASSES_ROOT);
					viewer = (wstring)txt;
					if (!viewer.empty())
					{
						viewer = viewer + _T("\\Shell\\Open\\Command\\");
						CRegStdString txtexe = CRegStdString(viewer, _T(""), FALSE, HKEY_CLASSES_ROOT);
						viewer = (wstring)txtexe;
						DWORD len = ExpandEnvironmentStrings(viewer.c_str(), NULL, 0);
						TCHAR * buf = new TCHAR[len+1];
						ExpandEnvironmentStrings(viewer.c_str(), buf, len);
						viewer = buf;
						delete buf;
					}
					else
					{
						// no viewer for txt files???
						// use hard coded path to notepad.exe
						TCHAR buf[MAX_PATH] = {0};
						SHGetFolderPath(hwndDlg, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, buf);
						PathAppend(buf, _T("Notepad.exe"));
						viewer = buf;
					}
					// replace "%1" with the path to our config file
					string tag("%1");
					// get the range of tag in viewer
					wstring::iterator it_begin = search(viewer.begin(), viewer.end(), tag.begin(), tag.end());

					if (it_begin != viewer.end())
					{
						wstring::iterator it_end= it_begin + tag.size();
						viewer.replace(it_begin, it_end, wstring(szPath));
					}
					else
					{
						viewer += _T(" ");
						viewer += szPath;
					}
					// finally start the application
					STARTUPINFO startup;
					PROCESS_INFORMATION process;
					memset(&startup, 0, sizeof(startup));
					startup.cb = sizeof(startup);
					memset(&process, 0, sizeof(process));

					if (CreateProcess(NULL, const_cast<TCHAR*>(viewer.c_str()), NULL, NULL, FALSE, 0, 0, 0, &startup, &process)==0)
					{
						LPVOID lpMsgBuf;
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_SYSTEM | 
							FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							GetLastError(),
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
							(LPTSTR) &lpMsgBuf,
							0,
							NULL 
							);
						MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
						LocalFree( lpMsgBuf );
					}
					else
					{
						CloseHandle(process.hThread);
						CloseHandle(process.hProcess);
					}
				}
			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
