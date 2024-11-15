all:
	gcc ./src/*.c -I./include -o Hotline-Recife.out -lm -lSDL2 -lSDL2_mixer

run:
	./Hotline-Recife.out

clean:
	rm Hotline-Recife.out
