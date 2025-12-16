
export SOURCE_PORT=/usr/games/chocolate-doom

OUTPUTS=output/av.txt \
        output/cydreams.txt \
        output/d2twid.txt \
        output/eternal.txt \
        output/hr.txt \
        output/hr2.txt \
        output/mm.txt \
        output/mm2.txt \
        output/pl2.txt

check: $(OUTPUTS)
	diff -x .gitignore -ur expected output

clean:
	rm -rf $(OUTPUTS)

output/av.txt: $(SOURCE_PORT)
	./testrunner pwads/av_new.zip demos/avall-41337.zip -- \
	     -merge AV.WAD -timedemo 30av-25337.lmp >$@

output/cydreams.txt: $(SOURCE_PORT)
	./testrunner pwads/cydreams.zip demos/cyallx1904.zip -- \
	     -deh Cyber110.deh -merge Cyber110.wad -timedemo 30cyx1904.lmp >$@

output/d2twid.txt: $(SOURCE_PORT)
	./testrunner pwads/d2twid.zip demos/30id-4525.zip -- \
	     -nodehlump -merge D2TWID.wad -timedemo 30id-4525.lmp >$@

output/eternal.txt: $(SOURCE_PORT)
	./testrunner pwads/eternal.zip demos/etall-21854.zip -- \
	     -merge ETERNALL.WAD -timedemo 30et-13854.lmp >$@

output/hr.txt: $(SOURCE_PORT)
	./testrunner pwads/hr.zip demos/hrall-4339.zip -- \
	     -merge HR.WAD -timedemo HQR-4339.LMP >$@

output/hr2.txt: $(SOURCE_PORT)
	./testrunner pwads/hr2final.zip demos/h2all-22944.zip -- \
	     -merge hr2final.wad -timedemo h2alls2-22944.lmp >$@

output/mm.txt: $(SOURCE_PORT)
	./testrunner pwads/mm_allup.zip demos/30mm8356.zip -- \
	     -merge MM.WAD -timedemo 30mm8356.lmp >$@

output/mm2.txt: $(SOURCE_PORT)
	./testrunner pwads/mm2.zip demos/m2allo4938.zip -- \
	     -merge MM2.WAD -timedemo mm2allnomo4938.lmp >$@

output/pl2.txt: $(SOURCE_PORT)
	./testrunner pwads/pl2.zip demos/p2all-6504.zip -- \
	     -gameversion final -merge PL2.WAD -timedemo pl2all1.lmp >$@

