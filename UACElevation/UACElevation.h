// Author: WangZhan -> wangzhan.1985@gmail.com
#pragma once


namespace uac
{
	// 当前进程是否高权限
	BOOL IsAdminElevated();

	// 使用管理员高权限运行
	BOOL RunAsAdminElevated(LPCTSTR lpszExePath, LPCTSTR lpszParameters);

	// 使用普通用户权限运行
	BOOL RunAsStdUser(LPCTSTR lpszExePath, LPCTSTR lpszParameters);

	BOOL Am_I_In_Admin_Group(BOOL bCheckAdminMode = FALSE);

	BOOL EnablePrivilege(HANDLE hToken, LPCTSTR lpszPrivilegeName);
}

