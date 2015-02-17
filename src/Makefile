#所有工程的一次性编译，
#感谢Kavin的优秀想法，这样我们的发布编译省力很多。
#好的东西都来自挠痒痒，

MAKEDIR=commlib/zcelib \
        commlib/soarlib \
		commsvr/zergsvrd \
		commsvr/ogre4asvrd \
		commsvr/wormholesvrd \
		test/zealot
		
		
ifeq ($(debug),1)
	MAKECMD = make mkdir; make -j4 -fMakefile debug=1
elif ($(realse),1)
	MAKECMD = make mkdir; make -j4 -fMakefile realse=1
else
	MAKECMD = make mkdir; make -j4 -fMakefile
endif

SRCPATH = $(ZCELIB_ROOT)/src
MAKERESULT="./compile_result.tmp"

all:
	@echo "" > $(MAKERESULT)
	@for dir in $(MAKEDIR); \
	do \
	cd $(SRCPATH)/$$dir; \
	$(MAKECMD); \
	echo $$dir "make done"; \
	done 2>> $(MAKERESULT);

	@echo ""
	@echo "all source file compiled, stat information is: "
	@make stat 

install:


clean:
	@for dir in $(MAKEDIR); \
	do \
	make -C $$dir clean; \
	done

mkdir:
	@for dir in $(MAKEDIR); \
	do \
	make -C $$dir mkdir; \
	done


#想加入一个统计信息
stat:
	@errors=`grep -c "[Ee]rror" $(MAKERESULT)`; \
	if [ $$errors -eq 0 ]; \
	then \
	echo "Congratulation! All source file has been maked without any errors." ; \
	else \
	grep "[Ee]rror" $(MAKERESULT); \
	fi;	
	@warnings=`grep -c "[Ww]arning" $(MAKERESULT)`; \
	if [ $$warnings -eq 0 ]; \
	then \
	echo "Congratulation! All source file has been maked without any warnings." ; \
	else \
	grep "[Ww]arning" $(MAKERESULT); \
	fi;	

	


