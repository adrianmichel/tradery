#include "stdafx.h"

using namespace tradery;

void au(bool silent = false, const t_string& author = _T( "" ),
        const t_string& programName = _T( "" )) {
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  HKEY hKey;
  char autoUpdater[500];
  DWORD dwBufLen = 500;
  LONG lRet;

  lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      _T( "SOFTWARE\\Thraex Software\\AutoUpdator" ), 0,
                      KEY_QUERY_VALUE, &hKey);
  if (lRet != ERROR_SUCCESS) {
    RegCloseKey(hKey);
    AfxMessageBox(_T( "Could not find the AutoUpdate application" ));
    return;
  }

  lRet = RegQueryValueEx(hKey, "Directory", NULL, NULL, (LPBYTE)autoUpdater,
                         &dwBufLen);
  RegCloseKey(hKey);
  if ((lRet != ERROR_SUCCESS) || (dwBufLen > 500)) {
    AfxMessageBox(_T( "Could not find the AutoUpdate application" ));
    return;
  }

  t_ostringstream cmdLine;
  if (silent)
    cmdLine << _T( " /silent /author \"" ) << author << _T( "\" /program \"" )
            << programName << _T( "\"" );

  t_ostringstream o;
  t_string path(autoUpdater);
  TCHAR last = path[path.length() - 1];
  o << autoUpdater;
  if (last != TCHAR('\\') && last != TCHAR('/')) o << '\\';
  o << _T( "AutoUpdator.exe" );

  try {
    runProcess(o.str(), cmdLine.str(), false);
  } catch (const RunProcessException&) {
    AfxMessageBox("Could not start AutoApdate application");
  }
}

MISCWIN_API void tradery::autoUpdateSilent(const t_string& author,
                                           const t_string& programName) {
  au(true, author, programName);
}

MISCWIN_API void tradery::autoUpdate() { au(); }
