Compiling command - > 

g++ -std=c++17 src/main.cpp src/glad.c \
-Iinclude \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lglfw \
-framework OpenGL \
-o app


Note :- i have use brew for glfw lib path , you should use your own path  , also ignore cmake file 

then run the app excutable file :-

./app


I have also have the reference code so you can play around with lighter version of this full codeblock
