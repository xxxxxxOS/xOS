TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

default :
	$(MAKE) xOS.img

xOS.img : xOS/ipl.bin xOS/xOS.sys Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:xOS/ipl.bin len:512 from:0 to:0 \
		copy from:xOS/xOS.sys to:@: \
		copy from:xOS/ipl.nas to:@: \
		copy from:make.bat to:@: \
		imgout:xOS.img


run :
	$(MAKE) xOS.img
	$(COPY) xOS.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) xOS.img
	$(IMGTOL) w a: xOS.img

full :
	$(MAKE) -C xOS

	$(MAKE) xOS.img

run_full :
	$(MAKE) full
	$(COPY) xOS.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install_full :
	$(MAKE) full
	$(IMGTOL) w a: xOS.img

run_os :
	$(MAKE) -C xOS
	$(MAKE) run

clean :


src_only :
	$(MAKE) clean
	-$(DEL) xOS.img

clean_full :
	$(MAKE) -C xOS		clean

src_only_full :
	$(MAKE) -C xOS		src_only
	-$(DEL) xOS.img

refresh :
	$(MAKE) full
	$(MAKE) clean_full
	-$(DEL) xOS.img
