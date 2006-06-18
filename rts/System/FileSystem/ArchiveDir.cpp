/* Author: Tobi Vollebregt */

#include "StdAfx.h"
#include "ArchiveDir.h"
#include "Platform/filefunctions.h"

inline CFileHandler* CArchiveDir::GetFileHandler(int handle)
{
	std::map<int, CFileHandler*>::iterator it = fileHandles.find(handle);
	assert(it != fileHandles.end());
	return it->second;
}

inline std::vector<std::string>::iterator& CArchiveDir::GetSearchHandle(int handle)
{
	std::map<int, std::vector<std::string>::iterator>::iterator it = searchHandles.find(handle);
	assert(it != searchHandles.end());
	return it->second;
}

CArchiveDir::CArchiveDir(const string& archivename) :
		CArchiveBase(archivename),
		archiveName(archivename + '/'),
		curFileHandle(0),
		curSearchHandle(0)
{
	std::vector<fs::path> found = find_files(archiveName, "*", true);

	// because spring expects the contents of archives to be case independent,
	// we convert and report all names in lowercase, and keep a std::map
	// lcNameToOrigName to convert back from lowercase to original case.
	for (std::vector<fs::path>::iterator it = found.begin(); it != found.end(); it++) {
		// strip our own name off..
		std::string origName(it->string(), archiveName.length());
		// convert to lowercase and store
		std::string lcName(origName);
		std::transform(lcName.begin(), lcName.end(), lcName.begin(), (int (*)(int))tolower);
		searchFiles.push_back(lcName);
		lcNameToOrigName[lcName] = origName;
	}
}

CArchiveDir::~CArchiveDir(void)
{
}

bool CArchiveDir::IsOpen()
{
	return true;
}

int CArchiveDir::OpenFile(const std::string& fileName)
{
	CFileHandler* f = new CFileHandler(archiveName + lcNameToOrigName[fileName]);

	if (!f || !f->FileExists())
		return 0;

	++curFileHandle;
	fileHandles[curFileHandle] = f;
	return curFileHandle;
}

int CArchiveDir::ReadFile(int handle, void* buffer, int numBytes)
{
	CFileHandler* f = GetFileHandler(handle);
	f->Read(buffer, numBytes);
	return 0;
}

void CArchiveDir::CloseFile(int handle)
{
	delete GetFileHandler(handle);
	fileHandles.erase(handle);
}

void CArchiveDir::Seek(int handle, int pos)
{
	GetFileHandler(handle)->Seek(pos);
}

int CArchiveDir::Peek(int handle)
{
	return GetFileHandler(handle)->Peek();
}

bool CArchiveDir::Eof(int handle)
{
	return GetFileHandler(handle)->Eof();
}

int CArchiveDir::FileSize(int handle)
{
	return GetFileHandler(handle)->FileSize();
}

int CArchiveDir::FindFiles(int cur, string* name, int* size)
{
	if (cur == 0) {
		cur = ++curSearchHandle;
		searchHandles[cur] = searchFiles.begin();
	}
	if (searchHandles[cur] == searchFiles.end()) {
		searchHandles.erase(cur);
		return 0;
	}

	std::vector<std::string>::iterator& it = GetSearchHandle(cur);

	*name = *searchHandles[cur];
	CFileHandler f(archiveName + lcNameToOrigName[*name]);
	*size = f.FileSize();

	++searchHandles[cur];
	return cur;
}
