// Author: WangZhan -> wangzhan.1985@gmail.com
#pragma once


namespace uac
{
	// ��ǰ�����Ƿ��Ȩ��
	BOOL IsAdminElevated();

	// ʹ�ù���Ա��Ȩ������
	BOOL RunAsAdminElevated(LPCTSTR lpszExePath, LPCTSTR lpszParameters);

	// ʹ����ͨ�û�Ȩ������
	BOOL RunAsStdUser(LPCTSTR lpszExePath, LPCTSTR lpszParameters);
}

