# f1 : main.o game.o bgm.o car.o enemy.o rendering.o transition.o ui.o
f1 : main.o
	gcc -o $@ $^ -lSDL2 -lSDL2_image -lSDL2_mixer -lm
clean :
	rm -rf *.o f1