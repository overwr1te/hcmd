#include <windows.h>

#define BUFSIZE 1024 * 5

PROCESS_INFORMATION pi;

HANDLE hReadPipe1;
HANDLE hWritePipe1;
HANDLE hReadPipe2;
HANDLE hWritePipe2;

void WriteLine(char* out)
{
	HANDLE stdOut = GetStdHandle(-11);
	WriteConsoleA(stdOut, out, strlen(out), NULL, NULL);
}

int ReadFromPipe(HANDLE pipe, char * buf, unsigned long max_len)
{
	unsigned long lpBytesRead, readed,i;
	char *p;

	lpBytesRead = 0;
	while( lpBytesRead == 0 && PeekNamedPipe( pipe, buf, max_len - 1, &readed, &lpBytesRead, 0 )  ) ;

	if( lpBytesRead > 0 )
	{
		lpBytesRead = readed > max_len ? max_len : readed;
		p = buf;
		
		while( lpBytesRead > 0 )
		{
			if( ! ReadFile( pipe, p, lpBytesRead , &readed, NULL ) ) return 0;
			
			if( readed == 0 ) break;
			
			lpBytesRead -= readed;
			p += readed;
		}
		*p = 0;
	}
	else
	{
		return 0;
	}
	
	return 1;
}

void WriteCommand(LPSTR command)
{
	unsigned long lBytesWrite;
	WriteFile(hWritePipe2, command, strlen(command), &lBytesWrite, NULL);
}

void CloseSession()
{
	TerminateProcess(pi.hProcess, 0);
	WaitForSingleObject( pi.hProcess, INFINITE );
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hReadPipe1);
	CloseHandle(hWritePipe1);
	CloseHandle(hReadPipe2);
	CloseHandle(hWritePipe2);
}

DWORD WINAPI RemoteConsole(LPVOID lpParam)
{
	char buff[BUFSIZE] = {0};

	SECURITY_ATTRIBUTES sa;
	STARTUPINFO si;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = TRUE;

	ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));

	if(CreatePipe(&hReadPipe1,&hWritePipe1,&sa,0))
	{
		if(CreatePipe(&hReadPipe2,&hWritePipe2,&sa,0))
		{
			ZeroMemory(&si,sizeof(STARTUPINFO));
			GetStartupInfo(&si);

			si.cb = sizeof( si );
			si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
			si.wShowWindow = SW_HIDE;
			si.hStdInput = hReadPipe2;
			si.hStdOutput = si.hStdError = hWritePipe1;

			if(CreateProcess(NULL, "cmd.exe", NULL, NULL, 1, 0, NULL, NULL, &si, &pi))
			{
				unsigned long lBytesWrite;

				while(TRUE)
				{
					if(ReadFromPipe(hReadPipe1, buff, BUFSIZE - 1 ))
					{
						WriteLine(buff);
					}
				}
			}
		}
	}

	return 0;
}

void Init()
{
	CreateThread(NULL, 0, &RemoteConsole, NULL, 0, NULL);

	Sleep(6000);
	WriteCommand("route\r\n");

	Sleep(6000);
	WriteCommand("systeminfo\r\n");

	Sleep(6000);
	WriteCommand("ping 127.0.0.1\r\n");

	Sleep(6000);
	WriteCommand("ipconfig\r\n");

	Sleep(6000);
	CloseSession();
	
	ExitProcess(0);
}
