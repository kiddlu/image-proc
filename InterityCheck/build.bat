::gcc main.c process.c -o main -D UNICODE -D _UNICODE -lcomdlg32

g++ short.cpp -o InterityCheckForShort -D UNICODE -D _UNICODE ^
-I..\..\..\..\3rdparty\opencv\include\ -L..\..\..\..\3rdparty\opencv\lib\Release ^
-lcomdlg32 -lopencv_core248 -lopencv_highgui248

::g++ long.cpp -o InterityCheckForLong -D UNICODE -D _UNICODE ^
::-I..\..\..\..\3rdparty\opencv\include\ -L..\..\..\..\3rdparty\opencv\lib\Release ^
::-lcomdlg32 -lopencv_core248 -lopencv_highgui248