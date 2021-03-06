
#include "PckClassIndex.h"
#include "CharsCodeConv.h"

CPckClassIndex::CPckClassIndex()
{}

CPckClassIndex::~CPckClassIndex()
{
	if(NULL != m_PckAllInfo.lpPckIndexTable)
		free(m_PckAllInfo.lpPckIndexTable);
}

void* CPckClassIndex::AllocMemory(size_t	sizeStuct)
{
	void*		lpMallocNode;

	if(NULL == (lpMallocNode = malloc(sizeStuct))) {
		return lpMallocNode;
	}
	//初始化内存
	memset(lpMallocNode, 0, sizeStuct);
	return lpMallocNode;
}

LPPCKINDEXTABLE	CPckClassIndex::AllocPckIndexTableByFileCount()
{
	return (LPPCKINDEXTABLE)AllocMemory(sizeof(PCKINDEXTABLE) * m_PckAllInfo.dwFileCount);
}

void CPckClassIndex::GenerateUnicodeStringToIndex()
{
	LPPCKINDEXTABLE lpPckIndexTable = m_PckAllInfo.lpPckIndexTable;

	for(DWORD i = 0;i < m_PckAllInfo.dwFileCount;++i) {
		//建立目录

		CAnsi2Ucs cA2U;
		cA2U.GetString(lpPckIndexTable->cFileIndex.szFilename, lpPckIndexTable->cFileIndex.szwFilename, sizeof(lpPckIndexTable->cFileIndex.szwFilename) / sizeof(wchar_t));

		++lpPckIndexTable;
	}
}

//重建时计算有效文件数量，排除重复的文件
DWORD CPckClassIndex::ReCountFiles()
{
	DWORD deNewFileCount = 0;
	LPPCKINDEXTABLE lpPckIndexTableSource = m_PckAllInfo.lpPckIndexTable;

	for(DWORD i = 0; i < m_PckAllInfo.dwFileCount; ++i) {

		if(!lpPckIndexTableSource->isInvalid) {
			++deNewFileCount;
		}
		++lpPckIndexTableSource;
	}
	return deNewFileCount;
}

//按修改后的索引数据按版本填入到结构体中并压缩好
LPPCKINDEXTABLE_COMPRESS CPckClassIndex::FillAndCompressIndexData(LPPCKINDEXTABLE_COMPRESS lpPckIndexTableComped, LPPCKFILEINDEX lpPckFileIndexToCompress)
{
	BYTE pckFileIndexBuf[MAX_INDEXTABLE_CLEARTEXT_LENGTH];
	lpPckIndexTableComped->dwIndexDataLength = MAX_INDEXTABLE_CLEARTEXT_LENGTH;
	compress(lpPckIndexTableComped->buffer, &lpPckIndexTableComped->dwIndexDataLength,
		m_PckAllInfo.lpSaveAsPckVerFunc->FillIndexData(lpPckFileIndexToCompress, pckFileIndexBuf), m_PckAllInfo.lpSaveAsPckVerFunc->dwFileIndexSize);
	//将获取的
	lpPckIndexTableComped->dwIndexValueHead = lpPckIndexTableComped->dwIndexDataLength ^ m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey1;
	lpPckIndexTableComped->dwIndexValueTail = lpPckIndexTableComped->dwIndexDataLength ^ m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey2;

	return lpPckIndexTableComped;
}