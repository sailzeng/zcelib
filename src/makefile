
#感谢Kavin的优秀想法，这样我们的发布编译省力很多。
#好的东西都来自挠痒痒，

MAKEDIR= commlib/zenlib \
         test/zeolot 


MAKERESULT="./makefile.result"

.PHONY : all clean mkdir veryclean 



all:
	@echo "====================================================================" > $(MAKERESULT);
	@echo "====================================================================";
	#编译所有的目录    
	@for one_dir in $(MAKEDIR); \
	do \
	$(MAKE) -C $(ZCELIB_ROOT)/src/$$one_dir; \
	echo $(ZCELIB_ROOT)/src/$$one_dir "make done"; \
	done 2>> $(MAKERESULT)
	@echo "====================================================================" > $(MAKERESULT);
	@echo "====================================================================";
	#打印输出告警，错误的结果        
	@echo "All source file compiled, statistics information is: " 
	@warnings=`grep -c "[Ww]arning" $(MAKERESULT)`; \
	if [ $$warnings -eq 0 ]; \
	then \
	echo "Congratulation! All source file has been maked without any warnings." ; \
	else \
	grep "[Ww]arning" $(MAKERESULT); \
	fi;
	@errors=`grep -c "[Ee]rror" $(MAKERESULT)`; \
	if [ $$errors -eq 0 ]; \
	then \
	echo "Congratulation! All source file has been maked without any errors." ; \
	else \
	grep "[Ee]rror" $(MAKERESULT); \
	fi;

mkdir:  
	@for one_dir in $(MAKEDIR); \
	do \
	$(MAKE) -C $(ZCELIB_ROOT)/src/$$one_dir mkdir; \
	echo $(ZCELIB_ROOT)/src/$$one_dir "make done"; \
	done
    
clean: 
	@for one_dir in $(MAKEDIR); \
	do \
	$(MAKE) -C $(ZCELIB_ROOT)/src/$$one_dir clean; \
	echo $(ZCELIB_ROOT)/src/$$one_dir "make done"; \
	done
    
veryclean :
	@for one_dir in $(MAKEDIR); \
	do \
	$(MAKE) -C $(ZCELIB_ROOT)/src/$$one_dir veryclean; \
	echo $(ZCELIB_ROOT)/src/$$one_dir "make done"; \
	done
