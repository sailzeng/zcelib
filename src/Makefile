#所有工程的一次性编译，
#感谢Kavin的优秀想法，这样我们的发布编译省力很多。
#好的东西都来自挠痒痒，

MAKEDIR=commlib\zcelib \
        commlib\soarlib \
		commsvr\zergsvrd \
		commsvr\orge4asvrd \
		commsvr\wormholesvrd \
		test\zealot
		
		
ifeq ($(debug),1)
	MAKECMD = make mkdir; make -j4 -fmakefile debug=1
else
	MAKECMD = make mkdir; make -j4 -fmakefile
endif

SRCPATH = "/data/qqpet2/code/src"
MAKERESULT="./makefile.result"

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
	@make stat warning=1

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

#默认情况下，统计error信息，如果加入warning信息，自己加入参数吧
warning=0
#想加入一个统计信息
stat:
	@errors=`grep -c "[Ee]rror" $(MAKERESULT)`; \
	if [ $$errors -eq 0 ]; \
	then \
	echo "Congratulation! All source file has been maked without any errors." ; \
	else \
	grep "[Ee]rror" $(MAKERESULT); \
	fi;	
	@if [ "$(warning)" -eq 1 ]; \
	then \
        grep "warning:" $(MAKERESULT); \
	else \
	echo "don't show any warnings."; \
	fi;
	


