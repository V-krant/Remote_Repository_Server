cd Debug

start server.exe

timeout /t 2
cd "../ClientGUI/bin/x86/Debug"

start ClientGUI.exe

timeout /t 100
start ClientGUI.exe -port 8083