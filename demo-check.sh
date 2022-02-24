ls -lart build/src/;
./build/src/crispy-doom -nographics -nosound -nograbmouse -iwad data/demos/DOOM.WAD -statdump data/demos/compare.txt -playdemo data/demos/m1-simple;
diff --strip-trailing-cr data/demos/compare.txt data/demos/m1-simple.txt;