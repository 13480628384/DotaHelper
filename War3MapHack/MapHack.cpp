#include "pch.h"
#include "MapHack.h"
#include <TlHelp32.h>

// ��̬������ʼ��
const string CMapHack::m_strProcName = "war3.exe";
const string CMapHack::m_strGameDllName = "game.dll";

//const string CMapHack::m_strProcName = "launchy.exe";
//const string CMapHack::m_strGameDllName = "calcy.dll";

BOOL CMapHack::Init()
{
    // ������ʼ��
    m_hProcHand = NULL;
    m_hGameHand = NULL;
    m_dwPid = 0;
    m_dwGameAddr = NULL;
    m_strProcPath = "";
    m_strGamePath = "";

    // ��Ȩ
    BOOL bRet = EnableDebugPrivileges();
    if (!bRet)
    {
        MessageBox(NULL, _T("���ó���Ȩ��ʧ��"), _T("Ȩ�޴���"), MB_ICONERROR | MB_OK);
        return bRet;
    }
    // ��ʼ��Ŀ��
    bRet = InitGameHandle();
    if (!bRet)
    {
        MessageBox(NULL, _T("��ʼ��Ŀ�����ʧ�ܣ���ȷ�ϳ���������"), _T("�Ҳ�������"), MB_ICONERROR | MB_OK);
        return bRet;
    }
    return bRet;
}

BOOL CMapHack::EnableDebugPrivileges()
{
    // �õ����̵����ƾ��
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
    {
        return FALSE;
    }
    // ��ѯ���̵�Ȩ��
    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
    // �޸Ľ���Ȩ��
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(token, FALSE, &tkp, sizeof(tkp), NULL, NULL))
    {
        return FALSE;
    }
    CloseHandle(token);
    return TRUE;
}

BOOL CMapHack::InitGameHandle()
{
    // ��ȡ����ID
    if (GetProcPidByName(m_strProcName.c_str()) <= 0)
    {
        return FALSE;
    }
    // �򿪽���
    auto hProc = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_TERMINATE | PROCESS_VM_OPERATION
        | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, m_dwPid);
    if (hProc == NULL)
    {
        return FALSE;
    }
    m_hProcHand = hProc;
    // ��ȡDLL����ַ
    if (GetDllBaseAddr(m_strGameDllName.c_str(), m_dwPid) <= 0)
    {
        return FALSE;
    }
    return TRUE;
}

DWORD CMapHack::GetProcPidByName(const char* pName)
{
    PROCESSENTRY32 pe;
    // �ο�msdn����Ҫ�ǻ��windows��ǰ�������һ��snap�����գ�
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe.dwSize = sizeof(PROCESSENTRY32);
    // ������һ����õ�windows�Ŀ��յ�ÿ�����̡�First ,next ���� 
    if (!Process32First(hSnapshot, &pe))
    {
        return 0;
    }
    CString strFindName(pName);
    strFindName.MakeUpper();
    do
    {
        pe.dwSize = sizeof(PROCESSENTRY32);
        CString strCurName = pe.szExeFile;
        strCurName.MakeUpper();
        // ���в���pe�����н�����Ϣ��name������������������濴�������֣���qq.exe 
        if (strFindName == strCurName)
        {
            // �������ID��Ҳ��ʱ����Ҫ�õ��Ľ��̵�ID 
            m_dwPid = pe.th32ProcessID;
            m_strProcPath = pe.szExeFile;
            break;
        }
        if (Process32Next(hSnapshot, &pe) == FALSE)
            break;
    } while (1);
    CloseHandle(hSnapshot);
    return m_dwPid;
}

DWORD CMapHack::GetDllBaseAddr(const char* pName, DWORD dwPid)
{
    HANDLE hSnap = NULL;
    MODULEENTRY32 me32;
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
    me32.dwSize = sizeof(MODULEENTRY32);
    CString strFindName(pName);
    strFindName.MakeUpper();
    CString strCurName;
    if (Module32First(hSnap, &me32))
    {
        do
        {
            strCurName = me32.szModule;
            strCurName.MakeUpper();
            if (strCurName == strFindName)
            {
                m_strGamePath = me32.szExePath;
                m_hGameHand = me32.hModule;
                m_dwGameAddr = (DWORD)me32.modBaseAddr;
                CloseHandle(hSnap);
                return m_dwGameAddr;
            }
        } while (Module32Next(hSnap, &me32));
    }
    CloseHandle(hSnap);
    return 0;
}
BOOL CMapHack::GameMemoryWrite(DWORD dwOffset, const char* lpAddr, DWORD nSize)
{
    auto pAddr = (LPVOID)(m_dwGameAddr + lpAddr);
    return WriteProcessMemory(m_hProcHand, pAddr, lpAddr, 1, &nSize);
}


BOOL CMapHack::Source()
{
    BOOL bRet = TRUE;
    // ���ͼ��ʾ��λ
    bRet &= GameMemoryWrite(0x74D1B9, "\xB2\x00\x90\x90\x90\x90", 6);

    // ��ʾ���ε�λ 
    bRet &= GameMemoryWrite(0x39EBBC, "\x75", 1);
    bRet &= GameMemoryWrite(0x3A2030, "\x90\x90", 2);
    bRet &= GameMemoryWrite(0x3A20DB, "\x8B\xC0", 2);
    // ��ʾ��Ʒ
    bRet &= GameMemoryWrite(0x28357C, "\x40\xC3", 2);
    // С��ͼ ȥ������
    bRet &= GameMemoryWrite(0x3A201B, "\xEB", 1);
    bRet &= GameMemoryWrite(0x40A864, "\x90\x90", 2);
    // С��ͼ��ʾ��λ
    bRet &= GameMemoryWrite(0x357065, "\x90\x90", 2);
    // �����ַ����ƹ����
    // PATCH(0x361F7C,"\x00",1);
    // �з��ź�
    bRet &= GameMemoryWrite(0x361F7C, "\xC1\x90\x90\x90", 4);
    // ������ʾ
    bRet &= GameMemoryWrite(0x43F9A6, "\x3B", 1);
    bRet &= GameMemoryWrite(0x43F9A9, "\x85", 1);
    bRet &= GameMemoryWrite(0x43F9B9, "\x3B", 1);
    bRet &= GameMemoryWrite(0x43F9BC, "\x85", 1);
    // �з�ͷ��
    bRet &= GameMemoryWrite(0x3345E9, "\x39\xC0\x0F\x85", 4);
    // ����ͷ��
    bRet &= GameMemoryWrite(0x371700, "\xE8\x3B\x28\x03\x00\x85\xC0\x0F\x85\x8F\x02\x00\x00\xEB\xC9\x90\x90\x90\x90", 19);
    // ��Դ���
    bRet &= GameMemoryWrite(0x371700, "\xE8\x3B\x28\x03\x00\x85\xC0\x0F\x84\x8F\x02\x00\x00\xEB\xC9\x90\x90\x90\x90", 19);
    // ������
    bRet &= GameMemoryWrite(0x36058A, "\x90", 1);
    bRet &= GameMemoryWrite(0x36058B, "\x90", 1);
    // ��ʾ����
    bRet &= GameMemoryWrite(0x34E8E2, "\xB8\xC8\x00\x00", 4);
    bRet &= GameMemoryWrite(0x34E8E7, "\x90", 1);
    bRet &= GameMemoryWrite(0x34E8EA, "\xB8\x64\x00\x00", 4);
    bRet &= GameMemoryWrite(0x34E8EF, "\x90", 1);
    // ����CD
    bRet &= GameMemoryWrite(0x2031EC, "\x90\x90\x90\x90\x90\x90", 6);
    bRet &= GameMemoryWrite(0x34FDE8, "\x90\x90", 2);
    // ��Դ�� Ұ����Ѫ ��Ұ����
    bRet &= GameMemoryWrite(0x28ECFE, "\xEB", 1);
    bRet &= GameMemoryWrite(0x34FE26, "\x90\x90\x90\x90", 4);
    // ����ȡ��
    bRet &= GameMemoryWrite(0x285CBC, "\x90\x90", 2);
    bRet &= GameMemoryWrite(0x285CD2, "\xEB", 1);
    // ��-MH
    bRet &= GameMemoryWrite(0x57BA7C, "\xEB", 1);
    bRet &= GameMemoryWrite(0x5B2D77, "\x03", 1);
    bRet &= GameMemoryWrite(0x5B2D8B, "\x03", 1);
    // ��-AH
    bRet &= GameMemoryWrite(0x3C84C7, "\xEB\x11", 2);
    bRet &= GameMemoryWrite(0x3C84E7, "\xEB\x11", 2);
    // �ֱ��Ӱ
    bRet &= GameMemoryWrite(0x3C6EDC, "\xB8\xFF\x00\x00\x00\xEB", 6);
    bRet &= GameMemoryWrite(0x3CC3B2, "\xEB", 1);
    bRet &= GameMemoryWrite(0x362391, "\x3B", 1);
    bRet &= GameMemoryWrite(0x362394, "\x85", 1);
    bRet &= GameMemoryWrite(0x39A51B, "\x90\x90\x90\x90\x90\x90", 6);
    bRet &= GameMemoryWrite(0x39A52E, "\x90\x90\x90\x90\x90\x90\x90\x90\x33\xC0\x40", 11);
    return bRet;
}
