gcc -I../lib esempio.c ../lib/obj_trace.c chess_gui_font.c chess_to_gui_interface.c chess_gui.c -o esempio `sdl2-config --cflags --libs` -lSDL2_ttf -lSDL2_image
