all: 
        gcc gundala.c -o gundala -lcrypto -I/usr/local/include -L/usr/local/lib -lcurl
        upx gundala
        
