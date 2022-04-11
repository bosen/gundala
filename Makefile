CC=gcc
UPX=upx
SSL=-lcrypto
INC=-I/usr/local/include 
LIB=-L/usr/local/lib -lcurl
CURL=-lcurl

all: 
        ${CC} gundala.c -o gundala ${SSL} ${INC} ${LIB} ${CURL}
        ${UPX} gundala
        
