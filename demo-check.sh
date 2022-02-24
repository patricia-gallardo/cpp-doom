$1/build/src/crispy-doom -nographics -nosound -nograbmouse -iwad data/demos/DOOM.WAD -statdump $1/compare.txt -playdemo data/demos/m1-simple;
if ! diff --strip-trailing-cr $1/compare.txt data/demos/m1-simple.txt; then
    exit 1;
fi

$1/build/src/crispy-doom -nographics -nosound -nograbmouse -iwad data/demos/DOOM.WAD -statdump $1/compare.txt -playdemo data/demos/m1-fast;
if ! diff --strip-trailing-cr $1/compare.txt data/demos/m1-fast.txt; then
    exit 1;
fi

$1/build/src/crispy-doom -nographics -nosound -nograbmouse -iwad data/demos/DOOM.WAD -statdump $1/compare.txt -playdemo data/demos/m1-normal;
if ! diff --strip-trailing-cr $1/compare.txt data/demos/m1-normal.txt; then
    exit 1;
fi

exit 0;