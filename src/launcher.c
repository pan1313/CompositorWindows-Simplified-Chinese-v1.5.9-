#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <strsafe.h>
#include <wchar.h>
#define CAP 32768
static WCHAR self[CAP], app[CAP], base[CAP], plugins[CAP], path[CAP], cache[CAP];
static int join(WCHAR* out,const WCHAR* a,const WCHAR* b) {
    return SUCCEEDED(StringCchPrintfW(out,CAP,L"%ls\\%ls",a,b));
}
static int regular(const WCHAR* p) {
    DWORD attr=GetFileAttributesW(p);
    return attr!=INVALID_FILE_ATTRIBUTES && !(attr&FILE_ATTRIBUTE_DIRECTORY);
}
static int supported(void) {
    if(!regular(app))return 0;
    const WCHAR* last=wcsrchr(app,L'\\');
    if(!last || _wcsicmp(last+1,L"Compositor.exe"))return 0;
    StringCchCopyW(base,CAP,app);base[last-app]=0;
    if(!join(path,base,L"Qt6Widgets.dll")||!regular(path))return 0;
    DWORD n=GetFileVersionInfoSizeW(app,NULL);
    if(!n)return 0;
    void* info=HeapAlloc(GetProcessHeap(),0,n);
    if(!info)return 0;
    VS_FIXEDFILEINFO* v=NULL;
    UINT len=0;
    int ok=GetFileVersionInfoW(app,0,n,info)&&VerQueryValueW(info,L"\\",(void**)&v,&len)
        &&len>=sizeof(*v)&&v->dwSignature==0xfeef04bd&&v->dwFileVersionMS==MAKELONG(5,1)&&v->dwFileVersionLS==MAKELONG(0,9);
    HeapFree(GetProcessHeap(),0,info);
    return ok;
}
static void issue(const WCHAR* s) {MessageBoxW(NULL,s,L"Compositor 简体中文启动器",MB_OK|MB_ICONWARNING);}
static int cached(void) {
    HANDLE f=CreateFileW(cache,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(f==INVALID_HANDLE_VALUE)return 0;
    DWORD n=0;BOOL ok=ReadFile(f,app,sizeof(app)-sizeof(WCHAR),&n,NULL);
    CloseHandle(f);app[n/sizeof(WCHAR)]=0;
    return ok&&n%sizeof(WCHAR)==0&&supported();
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE prev,PWSTR args,int show) {
    DWORD size=GetModuleFileNameW(NULL,self,CAP);
    if(!size||size>=CAP)return 1;
    WCHAR* slash=wcsrchr(self,L'\\');if(!slash)return 1;*slash=0;
    if(!join(plugins,self,L"plugins")||!join(path,plugins,L"generic\\compositorzh.dll")||!regular(path)){
        issue(L"未找到汉化插件。请先完整解压 ZIP，并保留启动器旁边的 plugins 文件夹。");return 1;
    }
    join(cache,self,L"Compositor.path");
    int found=cached();
    if(!found){
        WCHAR loc[CAP]={0};DWORD bytes=sizeof(loc);
        if(RegGetValueW(HKEY_CURRENT_USER,L"Software\\CompositorWindows\\CommunityPreview",L"InstallLocation",
                        RRF_RT_REG_SZ,NULL,loc,&bytes)==ERROR_SUCCESS)
            found=join(app,loc,L"Compositor.exe")&&supported();
    }
    if(!found)found=join(app,self,L"Compositor.exe")&&supported();
    if(!found){
        WCHAR local[MAX_PATH]={0};
        if(SUCCEEDED(SHGetFolderPathW(NULL,CSIDL_LOCAL_APPDATA,NULL,SHGFP_TYPE_CURRENT,local)))
            found=join(app,local,L"Programs\\CompositorWindows\\Compositor.exe")&&supported();
    }
    while(!found){
        OPENFILENAMEW picker={0};app[0]=0;
        picker.lStructSize=sizeof(picker);picker.lpstrFile=app;picker.nMaxFile=CAP;
        picker.lpstrFilter=L"Compositor.exe\0Compositor.exe\0\0";
        picker.lpstrTitle=L"请选择已安装的 v1.5.9 编辑器 Compositor.exe";
        picker.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
        if(!GetOpenFileNameW(&picker))return 0;
        found=supported();
        if(!found)issue(L"请选择 Cengokill Windows v1.5.9 的 Compositor.exe，所在目录应有 Qt6Widgets.dll。");
    }
    HANDLE f=CreateFileW(cache,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(f!=INVALID_HANDLE_VALUE){DWORD written;WriteFile(f,app,(DWORD)(wcslen(app)*sizeof(WCHAR)),&written,NULL);CloseHandle(f);}
    if(wcschr(plugins,L';')){issue(L"请把汉化包放到路径中不含英文分号的文件夹。");return 1;}
    if(!SetEnvironmentVariableW(L"QT_PLUGIN_PATH",plugins)){issue(L"无法设置本次启动的插件目录。");return 1;}
    WCHAR cmd[CAP];
    if(FAILED(StringCchPrintfW(cmd,CAP,L"\"%ls\" -plugin compositorzh",app)))return 1;
    STARTUPINFOW si={0};PROCESS_INFORMATION pi={0};si.cb=sizeof(si);
    if(!CreateProcessW(app,cmd,NULL,NULL,FALSE,0,NULL,base,&si,&pi)){
        issue(L"启动失败，请先确认原版 Compositor.exe 可以正常运行。");return 1;
    }
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);return 0;
}
