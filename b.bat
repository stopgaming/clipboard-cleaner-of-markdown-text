
windres my.rc -O coff -o my.res
g++ clipboard_cleaner.cpp my.res -DUNICODE -o clipboard_cleaner.exe -luser32 -lgdi32 -lshell32 -mwindows -static
