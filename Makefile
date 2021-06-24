TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

default :
	$(MAKE) xOS.img

xOS.img : xOS/ipl.bin xOS/xOS.sys Makefile \
			nbt/nbt.hrb \
			color/color.hrb 
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:xOS/ipl.bin len:512 from:0 to:0 \
		copy from:xOS/xOS.sys to:@: \
		copy from:xOS/ipl.nas to:@: \
		copy from:make.bat to:@: \
		copy from:nbt/nbt.hrb to:@: \
		copy from:color/color.hrb to:@: \
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
	$(MAKE) -C apilib
	$(MAKE) -C nbt
	$(MAKE) -C color
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
	$(MAKE) -C apilib		clean
	$(MAKE) -C nbt		clean
	$(MAKE) -C color		clean

src_only_full :
	$(MAKE) -C xOS			src_only
	$(MAKE) -C apilib		src_only
	$(MAKE) -C nbt		src_only
	$(MAKE) -C color		src_only
	-$(DEL) xOS.img

refresh :
	$(MAKE) full
	$(MAKE) clean_full
	-$(DEL) xOS.img
