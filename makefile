all:
	gcc ./src/*.c -I./include -o Hotline-Recife.out -lm

run:
	./Hotline-Recife.out

clean:
	rm Hotline-Recife.out
