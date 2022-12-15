#LINUX_SRC = ${shell pwd}/../linux/linux-3.0/
LINUX_SRC = /works/rk3568/rk3568_v1_2_4/kernel
CROSS_COMPILE = /works/rk3568/rk3568_v1_2_4/buildroot/output/rockchip_rk3568/host/bin/aarch64-buildroot-linux-gnu
INST_PATH=/nfs

PWD := $(shell pwd)

EXTRA_CFLAGS+=-DMODULE

obj-m += leds-switcher.o leds-tm1681.o

modules:
	@echo ${LINUX_SRC}
	@make -C $(LINUX_SRC) M=$(PWD) modules
	@make clear

uninstall:
	rm -f ${INST_PATH}/*.ko

install: uninstall
	cp -af *.ko ${INST_PATH}

clear:
	@rm -f *.o *.cmd *.mod.c
	@rm -rf  *~ core .depend  .tmp_versions Module.symvers modules.order -f
	@rm -f .*ko.cmd .*.o.cmd .*.o.d

clean: clear
	@rm -f  *.ko

