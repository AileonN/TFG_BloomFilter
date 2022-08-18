#********************************************************#
# Author:	Noelia Oliete Escuín, 779526				 #     
#********************************************************#

CC=g++
MAIN=CP
BF=bloom
BRAIN=brain

all: ${MAIN}
#-------------------------------------------------------#
# MAIN
# Linkado
${MAIN}: ${MAIN}.o ${BF}.o ${BRAIN}.o 
	${CC}  ${MAIN}.o ${BF}.o ${BRAIN}.o -o ${MAIN} -std=c++11 -lcrypto
# Compilacion
${MAIN}.o: ${MAIN}.cpp
	${CC} -c ${MAIN}.cpp -std=c++11
#-------------------------------------------------------#
# BLOOM FILTER
# Compilacion
${BF}.o: ${BF}.cpp ${BF}.h
	${CC} -c ${BF}.cpp -o ${BF}.o -std=c++11
#-------------------------------------------------------#
# BRAIN
# Compilacion
${BRAIN}.o: ${BRAIN}.cpp ${BRAIN}.h
	${CC} -c ${BRAIN}.cpp -o ${BRAIN}.o -std=c++11
#-------------------------------------------------------#
clean :
	-rm *.o $(objects) 