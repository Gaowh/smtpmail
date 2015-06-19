vpath %.c srcfile

vpath %.h myapi

obj = main.o  smtp_standalone.o  smtp_process.o decode.o semopt.o 

smtp_mailagent: $(obj)
	$(CC) -o smtp_mailagent $(obj) 

clean:
	rm -rf smtp_mailagent $(obj)

