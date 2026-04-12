Compiling command - > 

g++ src/main.cpp src/glad.c \
-Iinclude \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lglfw \
-framework OpenGL \
-o app

Note :- i have use brew for glfw lib path , you should use your own path  , also ignore cmake file 

then run the app excutable file :-

./app