######################################################
#            PROJECT  : MALT                         #
#            VERSION  : 1.1.5                        #
#            DATE     : 06/2023                      #
#            AUTHOR   : Valat Sébastien              #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
RONN=ronn
TARGETS=numaprof-passwd.1.bz2 numaprof-webview.1.bz2 numaprof.1.bz2 numaprof-qt5.1.bz2 numaprof-passwd.1.html numaprof-webview.1.html numaprof.1.html numaprof-qt5.1.html
RM=rm -f
BZIP2=bzip2

######################################################
all: $(TARGETS)

######################################################
clean:
	$(RM) $(TARGETS)

######################################################
%.1.bz2: %.ronn
	$(RONN) -r --pipe $< | $(BZIP2) > $@

######################################################
%.1.html: %.ronn
	$(RONN) -5 $<

######################################################
.PHONY: all clean
