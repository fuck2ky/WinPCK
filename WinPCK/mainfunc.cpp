//////////////////////////////////////////////////////////////////////
// mainfunc.cpp: WinPCK 界面线程部分
// 打开并显示pck文件、查找文件、记录浏览位置 
//
// 此程序由 李秋枫/stsm/liqf 编写
//
// 此代码预计将会开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2012.4.10
// 2012.10.10.OK
//////////////////////////////////////////////////////////////////////

#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4005 )

#include "tlib.h"
#include "resource.h"
#include "winmain.h"
//#include <strsafe.h>
#include <shlwapi.h>
#include "OpenSaveDlg.h"
#include "CharsCodeConv.h"
#include <stdio.h>
#include "StopWatch.h"

LPPCK_PATH_NODE	TInstDlg::GetLastShowNode()
{
	LPPCK_PATH_NODE lpCurrentNode = m_cPckCenter.m_lpPckRootNode->child;
	LPPCK_PATH_NODE lpCurrentNodeToFind = lpCurrentNode;

	TCHAR	**lpCurrentDir = m_PathDirs.lpszDirNames;

	for(int i = 0;i < m_PathDirs.nDirCount;i++) {

		if(0 == **lpCurrentDir)return lpCurrentNode;

		BOOL isDirFound = FALSE;

		while(NULL != lpCurrentNodeToFind) {

#ifdef UNICODE
			if(0 == _tcscmp(lpCurrentNodeToFind->szName, *lpCurrentDir)) {
#else
			CAnsi2Ucs cA2U;
			if(0 == wcscmp(lpCurrentNodeToFind->szName, cA2U.GetString(*lpCurrentDir))) {
#endif
				lpCurrentNode = lpCurrentNodeToFind = lpCurrentNodeToFind->child;
				isDirFound = TRUE;

				break;
			}


			lpCurrentNodeToFind = lpCurrentNodeToFind->next;
		}

		if(!isDirFound)
			return lpCurrentNode;

		lpCurrentDir++;

	}
	return lpCurrentNode;

}

BOOL TInstDlg::OpenPckFile(TCHAR *lpszFileToOpen, BOOL isReOpen)
{

	CStopWatch	timer;
	TCHAR	szString[64];
	int		iListTopView;

	m_currentNodeOnShow = NULL;

	if(0 != *lpszFileToOpen && lpszFileToOpen != m_Filename) {
		_tcscpy_s(m_Filename, MAX_PATH, lpszFileToOpen);
	}

	if(!isReOpen) {
		memset(&m_PathDirs, 0, sizeof(m_PathDirs));
		*m_PathDirs.lpszDirNames = m_PathDirs.szPaths;
	} else {
		//记录位置
		iListTopView = ListView_GetTopIndex(GetDlgItem(IDC_LIST));
	}

	if(0 != *lpszFileToOpen || OpenSingleFile(hWnd, m_Filename, TEXT_FILE_FILTER)) {
		timer.start();

		//转换文件名格式 
		if(m_cPckCenter.Open(m_Filename)) {
			timer.stop();
			_stprintf_s(szString, 64, GetLoadStr(IDS_STRING_OPENOK), timer.getElapsedTime());
			SetStatusBarText(4, szString);

			SetStatusBarText(0, _tcsrchr(m_Filename, TEXT('\\')) + 1);
			SetStatusBarText(3, TEXT(""));

			_stprintf_s(szString, 64, GetLoadStr(IDS_STRING_OPENFILESIZE), m_cPckCenter.GetPckSize());
			SetStatusBarText(1, szString);

			_stprintf_s(szString, 64, GetLoadStr(IDS_STRING_OPENFILECOUNT), m_cPckCenter.GetPckFileCount());
			SetStatusBarText(2, szString);

			if(isReOpen) {
				ShowPckFiles(GetLastShowNode());
				ListView_EnsureVisible(GetDlgItem(IDC_LIST), iListTopView, NULL);
				ListView_EnsureVisible(GetDlgItem(IDC_LIST), iListTopView + ListView_GetCountPerPage(GetDlgItem(IDC_LIST)) - 1, NULL);
			} else
				ShowPckFiles(m_cPckCenter.m_lpPckRootNode->child);

			return TRUE;
		} else {
			SetStatusBarText(4, GetLoadStr(IDS_STRING_PROCESS_ERROR));
			m_cPckCenter.Close();
			return FALSE;
		}

	}
	SetStatusBarText(4, GetLoadStr(IDS_STRING_OPENFAIL));
	return FALSE;

}
VOID TInstDlg::SearchPckFiles()
{
	DWORD	dwFileCount = m_cPckCenter.GetPckFileCount();
	if(0 == dwFileCount)return;
	HWND	hList = GetDlgItem(IDC_LIST);
	LPPCKINDEXTABLE	lpPckIndexTable;
	lpPckIndexTable = m_cPckCenter.GetPckIndexTable();

	wchar_t	szClearTextSize[CHAR_NUM_LEN], szCipherTextSize[CHAR_NUM_LEN];
	wchar_t	szCompressionRatio[CHAR_NUM_LEN];

	LPPCK_PATH_NODE		lpNodeToShowPtr = m_cPckCenter.m_lpPckRootNode;

	if(NULL == lpNodeToShowPtr)return;

	m_cPckCenter.SetListInSearchMode(TRUE);

	//显示查找文字
	char	szPrintf[64];
	sprintf_s(szPrintf, 64, GetLoadStrA(IDS_STRING_SEARCHING), m_szStrToSearch);
	SendDlgItemMessageA(IDC_STATUS, SB_SETTEXTA, 4, (LPARAM)szPrintf);

	ListView_DeleteAllItems(hList);

	//清除浏览记录
	memset(&m_PathDirs, 0, sizeof(m_PathDirs));
	*m_PathDirs.lpszDirNames = m_PathDirs.szPaths;
	m_PathDirs.nDirCount = 0;

	::SendMessage(hList, WM_SETREDRAW, FALSE, 0);

	int	iListIndex = 1;

	m_currentNodeOnShow = m_cPckCenter.m_lpPckRootNode->child;

	InsertList(hList, 0, LVIF_PARAM | LVIF_IMAGE, 0, m_cPckCenter.m_lpPckRootNode, 1,
		L"..");

	for(DWORD i = 0;i < dwFileCount;i++) {
		if(NULL != strstr(lpPckIndexTable->cFileIndex.szFilename, m_szStrToSearch)) {
			if(0 == lpPckIndexTable->cFileIndex.dwFileClearTextSize)
				wcscpy(szCompressionRatio, L"-");
			else
				swprintf_s(szCompressionRatio, CHAR_NUM_LEN, L"%.1f%%", lpPckIndexTable->cFileIndex.dwFileCipherTextSize / (float)lpPckIndexTable->cFileIndex.dwFileClearTextSize * 100.0);

			InsertList(hList, iListIndex, LVIF_PARAM | LVIF_IMAGE, 1, lpPckIndexTable, 4,
				lpPckIndexTable->cFileIndex.szwFilename,
				StrFormatByteSizeW(lpPckIndexTable->cFileIndex.dwFileClearTextSize, szClearTextSize, CHAR_NUM_LEN),
				StrFormatByteSizeW(lpPckIndexTable->cFileIndex.dwFileCipherTextSize, szCipherTextSize, CHAR_NUM_LEN),
				szCompressionRatio);

			iListIndex++;
		}
		lpPckIndexTable++;

	}

	::SendMessage(hList, WM_SETREDRAW, TRUE, 0);

	sprintf_s(szPrintf, 64, GetLoadStrA(IDS_STRING_SEARCHOK), m_szStrToSearch, iListIndex - 1);
	SendDlgItemMessageA(IDC_STATUS, SB_SETTEXTA, 4, (LPARAM)szPrintf);

}

VOID TInstDlg::ShowPckFiles(LPPCK_PATH_NODE		lpNodeToShow)
{
	HWND	hList = GetDlgItem(IDC_LIST);
	m_cPckCenter.SetListInSearchMode(FALSE);
	LPPCKINDEXTABLE	lpPckIndexTable;

	wchar_t	szClearTextSize[CHAR_NUM_LEN], szCipherTextSize[CHAR_NUM_LEN];
	wchar_t	szCompressionRatio[CHAR_NUM_LEN];

	LPPCK_PATH_NODE		lpNodeToShowPtr = lpNodeToShow;

	if(NULL == lpNodeToShowPtr)return;

	m_currentNodeOnShow = lpNodeToShow;

	ListView_DeleteAllItems(hList);

	::SendMessage(hList, WM_SETREDRAW, FALSE, 0);

	int i = 0;

	while(NULL != lpNodeToShowPtr && 0 != *lpNodeToShowPtr->szName) {
		//这里先显示文件夹
		if(NULL == lpNodeToShowPtr->lpPckIndexTable) {
			if(NULL != lpNodeToShowPtr->child) {

				if(0 == lpNodeToShowPtr->child->qdwDirClearTextSize)
					wcscpy(szCompressionRatio, L"-");
				else
					swprintf_s(szCompressionRatio, CHAR_NUM_LEN, L"%.1f%%", lpNodeToShowPtr->child->qdwDirCipherTextSize / (float)lpNodeToShowPtr->child->qdwDirClearTextSize * 100.0);

				InsertList(hList, i, LVIF_PARAM | LVIF_IMAGE, 0, lpNodeToShowPtr, 4,
					lpNodeToShowPtr->szName,
					StrFormatByteSizeW(lpNodeToShowPtr->child->qdwDirClearTextSize, szClearTextSize, CHAR_NUM_LEN),
					StrFormatByteSizeW(lpNodeToShowPtr->child->qdwDirCipherTextSize, szCipherTextSize, CHAR_NUM_LEN),
					szCompressionRatio);
			} else
				InsertList(hList, i, LVIF_PARAM | LVIF_IMAGE, 0, lpNodeToShowPtr, 1,
					lpNodeToShowPtr->szName);
			i++;
		}
		lpNodeToShowPtr = lpNodeToShowPtr->next;
	}

	lpNodeToShowPtr = lpNodeToShow;
	while(NULL != lpNodeToShowPtr && 0 != *lpNodeToShowPtr->szName) {
		if(NULL != lpNodeToShowPtr->lpPckIndexTable) {
			lpPckIndexTable = lpNodeToShowPtr->lpPckIndexTable;

			if(0 == lpPckIndexTable->cFileIndex.dwFileClearTextSize)
				wcscpy(szCompressionRatio, L"-");
			else
				swprintf_s(szCompressionRatio, CHAR_NUM_LEN, L"%.1f%%", lpPckIndexTable->cFileIndex.dwFileCipherTextSize / (float)lpPckIndexTable->cFileIndex.dwFileClearTextSize * 100.0);

			InsertList(hList, i, LVIF_PARAM | LVIF_IMAGE, 1, lpNodeToShowPtr, 4,
				lpNodeToShowPtr->szName,
				StrFormatByteSizeW(lpPckIndexTable->cFileIndex.dwFileClearTextSize, szClearTextSize, CHAR_NUM_LEN),
				StrFormatByteSizeW(lpPckIndexTable->cFileIndex.dwFileCipherTextSize, szCipherTextSize, CHAR_NUM_LEN),
				szCompressionRatio);
			i++;
		}

		lpNodeToShowPtr = lpNodeToShowPtr->next;

	}

	::SendMessage(hList, WM_SETREDRAW, TRUE, 0);

}

