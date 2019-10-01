# HCMD
Windows Console from another process (HCMD example).

# How it works
HCMD use pipe-files for requests and results from cmd.exe.

# Example usage
```c
CreateThread(NULL, 0, &RemoteConsole, NULL, 0, NULL);
WriteCommand("ping 127.0.0.1\r\n");
CloseSession();
```
