//////////////////////////////////////////////////////////////////////
// PckClass.cpp: 用于解析完美世界公司的pck文件中的数据，并显示在List中
// 有关类的初始化等
//
// 此程序由 李秋枫/stsm/liqf 编写，pck结构参考若水的pck结构.txt，并
// 参考了其易语言代码中并于读pck文件列表的部分
//
// 此代码预计将会开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

#include "PckClass.h"

#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4146 )
#pragma warning ( disable : 4267 )

CPckClass::CPckClass(LPPCK_RUNTIME_PARAMS inout)
{
	m_lpPckParams = inout;
}

CPckClass::~CPckClass()
{}

BOOL CPckClass::Init(LPCTSTR	szFile)
{
	_tcscpy(m_PckAllInfo.szFilename, szFile);
	GetFileTitle(m_PckAllInfo.szFilename, m_PckAllInfo.szFileTitle, MAX_PATH);

	if(!MountPckFile(m_PckAllInfo.szFilename)) {

		ResetPckInfos();

		return FALSE;
	} else {
		return (m_PckAllInfo.isPckFileLoaded = TRUE);
	}
}
