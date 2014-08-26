#include "stdafx.h"
#include <comdef.h>
#include <taskschd.h>
#include "UACElevation.h"

//#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Taskschd.lib")

#define FAILED_IF(hr) \
	do\
	{\
		if (FAILED(hr)) \
			return FALSE;\
	} while (0)\


static BOOL RunShellExecute(BOOL bIsElevated, LPCTSTR lpszExePath, LPCTSTR lpszParameters)
{
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	if (bIsElevated)
		sei.lpVerb = _T("runas");
	sei.lpFile = lpszExePath;
	sei.lpParameters = lpszParameters;
	sei.nShow = SW_NORMAL;

	return ShellExecuteEx(&sei);
}

BOOL uac::IsAdminElevated()
{
	bool bIsElevated = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		TOKEN_ELEVATION elevationInfo;
		DWORD dwRetSize = 0;
		if (GetTokenInformation(hToken, TokenElevation, &elevationInfo, sizeof(elevationInfo), &dwRetSize))
		{
			bIsElevated = elevationInfo.TokenIsElevated > 0;
		}
		CloseHandle(hToken);
	}

	return bIsElevated;
}

BOOL uac::RunAsAdminElevated(LPCTSTR lpszExePath, LPCTSTR lpszParameters)
{
	return RunShellExecute(TRUE, lpszExePath, lpszParameters);
}

template<typename T>
class ComObjHelper
{
public:
	ComObjHelper() : m_pComObj(NULL) {}
	~ComObjHelper()
	{
		if (m_pComObj)
			m_pComObj->Release();
	}

	T *m_pComObj;
};

BOOL uac::RunAsStdUser(LPCTSTR lpszExePath, LPCTSTR lpszParameters)
{
	if (!IsAdminElevated())
		RunShellExecute(FALSE, lpszExePath, lpszParameters);

	HRESULT hr = 0;

	//  Choose a name for the task.
	LPCTSTR pszTaskName = L"RunAsStdUser Task";

	//  Create an instance of the Task Service. 
	ComObjHelper<ITaskService> iService;
	hr = CoCreateInstance(CLSID_TaskScheduler,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ITaskService,
		(void**)&(iService.m_pComObj));
	FAILED_IF(hr);

	//  Connect to the task service.
	hr = iService.m_pComObj->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	FAILED_IF(hr);

	//  Get the pointer to the root task folder.  This folder will hold the
	//  new task that is registered.
	ComObjHelper<ITaskFolder> iRootFolder;
	hr = iService.m_pComObj->GetFolder(_bstr_t(L"\\"), &iRootFolder.m_pComObj);
	FAILED_IF(hr);

	//  If the same task exists, remove it.
	iRootFolder.m_pComObj->DeleteTask(_bstr_t(pszTaskName), 0); // ignore error message, if any

	//  Create the task builder object to create the task.
	ComObjHelper<ITaskDefinition> iTask;
	hr = iService.m_pComObj->NewTask(0, &iTask.m_pComObj);
	FAILED_IF(hr);

	//  Get the registration info for setting the identification.
	ComObjHelper<IRegistrationInfo> iRegInfo;
	hr = iTask.m_pComObj->get_RegistrationInfo(&iRegInfo.m_pComObj);
	FAILED_IF(hr);

	hr = iRegInfo.m_pComObj->put_Author(L"RunAsStdUser");
	FAILED_IF(hr);

	//  Create the principal for the task
	ComObjHelper<IPrincipal> iPrincipal;
	hr = iTask.m_pComObj->get_Principal(&iPrincipal.m_pComObj);
	FAILED_IF(hr);

	//  Set up principal information: 
	hr = iPrincipal.m_pComObj->put_Id(_bstr_t(L"RunAsStdUser_Principal"));
	FAILED_IF(hr);

	hr = iPrincipal.m_pComObj->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
	FAILED_IF(hr);

	//  Run the task with the least privileges (LUA) 
	hr = iPrincipal.m_pComObj->put_RunLevel(TASK_RUNLEVEL_LUA);
	FAILED_IF(hr);

	//  Create the settings for the task
	ComObjHelper<ITaskSettings> iSettings;
	hr = iTask.m_pComObj->get_Settings(&iSettings.m_pComObj);
	FAILED_IF(hr);

	//  Set setting values for the task.
	hr = iSettings.m_pComObj->put_StartWhenAvailable(VARIANT_BOOL(true));
	FAILED_IF(hr);

	//  Get the trigger collection to insert the registration trigger.
	ComObjHelper<ITriggerCollection> iTriggerCollection;
	hr = iTask.m_pComObj->get_Triggers(&iTriggerCollection.m_pComObj);
	FAILED_IF(hr);

	//  Add the registration trigger to the task.
	ComObjHelper<ITrigger> iTrigger;
	hr = iTriggerCollection.m_pComObj->Create(TASK_TRIGGER_REGISTRATION, &iTrigger.m_pComObj);
	FAILED_IF(hr);

	ComObjHelper<IRegistrationTrigger> iRegistrationTrigger;
	hr = iTrigger.m_pComObj->QueryInterface(
		IID_IRegistrationTrigger, (void**)&iRegistrationTrigger.m_pComObj);
	FAILED_IF(hr);

	hr = iRegistrationTrigger.m_pComObj->put_Id(_bstr_t(L"RunAsStdUser_Trigger"));
	FAILED_IF(hr);

	//  Define the delay for the registration trigger.
	hr = iRegistrationTrigger.m_pComObj->put_Delay(L"PT0S"); // 0 second delay of execution; 
	FAILED_IF(hr);

	//  Add an Action to the task. This task will execute notepad.exe.
	//  Get the task action collection pointer.
	ComObjHelper<IActionCollection> iActionCollection;
	hr = iTask.m_pComObj->get_Actions(&iActionCollection.m_pComObj);
	FAILED_IF(hr);

	//  Create the action, specifying that it is an executable action.
	ComObjHelper<IAction> iAction;
	hr = iActionCollection.m_pComObj->Create(TASK_ACTION_EXEC, &iAction.m_pComObj);
	FAILED_IF(hr);

	//  QI for the executable task pointer.
	ComObjHelper<IExecAction> iExecAction;
	hr = iAction.m_pComObj->QueryInterface(
		IID_IExecAction, (void**)&iExecAction.m_pComObj);
	FAILED_IF(hr);

	//  Set the path of the executable to notepad.exe.
	hr = iExecAction.m_pComObj->put_Path(_bstr_t(lpszExePath));

	if (lpszParameters)
	{
		hr = iExecAction.m_pComObj->put_Arguments(_bstr_t(lpszParameters));
		FAILED_IF(hr);
	}

	// TODO:
	//if (pszDirectory)
	//{
	//	hr = iExecAction.m_pComObj->put_WorkingDirectory(_bstr_t(pszDirectory));
	//}

	//  Save the task in the root folder.
	ComObjHelper<IRegisteredTask> iRegisteredTask;
	hr = iRootFolder.m_pComObj->RegisterTaskDefinition(
		_bstr_t(pszTaskName),
		iTask.m_pComObj,
		TASK_CREATE_OR_UPDATE,
		_variant_t(),
		_variant_t(),
		TASK_LOGON_INTERACTIVE_TOKEN,
		_variant_t(L""),
		&iRegisteredTask.m_pComObj);
	FAILED_IF(hr);

	return TRUE;
}
