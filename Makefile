.PHONY: all
all: opt

opt:
	$(MAKE) -C libraries/EightBit/src opt
	$(MAKE) -C libraries/EightBit/LR35902/src opt
	$(MAKE) -C libraries/Gb_Snd_Emu opt
	$(MAKE) -C src opt

debug:
	$(MAKE) -C libraries/EightBit/src debug
	$(MAKE) -C libraries/EightBit/LR35902/src debug
	$(MAKE) -C libraries/Gb_Snd_Emu debug
	$(MAKE) -C src debug

coverage:
	$(MAKE) -C libraries/EightBit/src coverage
	$(MAKE) -C libraries/EightBit/LR35902/src coverage
	$(MAKE) -C libraries/Gb_Snd_Emu coverage
	$(MAKE) -C src coverage

.PHONY: clean
clean:
	$(MAKE) -C libraries/EightBit/src clean
	$(MAKE) -C libraries/EightBit/LR35902/src clean
	$(MAKE) -C libraries/Gb_Snd_Emu clean
	$(MAKE) -C src clean
