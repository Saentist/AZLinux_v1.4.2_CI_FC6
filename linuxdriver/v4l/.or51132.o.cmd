cmd_/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.o := gcc -m32 -I/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l -Wp,-MD,/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/.or51132.o.d  -nostdinc -isystem /usr/lib/gcc/i386-redhat-linux/4.1.1/include -I/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/../linux/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h  -I/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Wstrict-prototypes -Wundef -Werror-implicit-function-declaration -Os -pipe -msoft-float -fno-builtin-sprintf -fno-builtin-log2 -fno-builtin-puts  -mpreferred-stack-boundary=2  -march=i686 -mtune=generic -mtune=generic -mregparm=3 -ffreestanding -Iinclude/asm-i386/mach-generic -Iinclude/asm-i386/mach-default -fomit-frame-pointer -fasynchronous-unwind-tables -g  -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign -DCONFIG_VIDEO_V4L1_COMPAT -DHAVE_VIDEO_BUF_DVB=1 -DHAVE_VIDEO_BUF_DVB=1 -DHAVE_MANTIS_CI=1 -DCONFIG_VIDEO_V4L1_COMPAT -DHAVE_VIDEO_BUF_DVB=1 -DHAVE_VIDEO_BUF_DVB=1 -DHAVE_MANTIS_CI=1 -DCONFIG_PWC_DEBUG=0 -Isound -DCONFIG_VIDEO_V4L1_COMPAT -DHAVE_VIDEO_BUF_DVB=1 -DHAVE_VIDEO_BUF_DVB=1 -DHAVE_MANTIS_CI=1 -g   -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(or51132)"  -D"KBUILD_MODNAME=KBUILD_STR(or51132)" -c -o /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/.tmp_or51132.o /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.c

deps_/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.o := \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.c \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
  /usr/lib/gcc/i386-redhat-linux/4.1.1/include/stdarg.h \
  include/linux/linkage.h \
  include/asm/linkage.h \
    $(wildcard include/config/x86/alignment/16.h) \
  include/linux/stddef.h \
  include/linux/compiler.h \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/resources/64bit.h) \
  include/linux/posix_types.h \
  include/asm/posix_types.h \
  include/asm/types.h \
    $(wildcard include/config/highmem64g.h) \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/lsf.h) \
  include/linux/bitops.h \
  include/asm/bitops.h \
  include/asm/alternative.h \
    $(wildcard include/config/smp.h) \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  include/asm/byteorder.h \
    $(wildcard include/config/x86/bswap.h) \
  include/linux/byteorder/little_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/minix.h \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
  include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
  include/linux/sched.h \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/utrace.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/ptrace.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/pm.h) \
  include/linux/auxvec.h \
  include/asm/auxvec.h \
  include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/capability.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/asm/thread_info.h \
    $(wildcard include/config/4kstacks.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/asm/page.h \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/highmem4g.h) \
    $(wildcard include/config/page/offset.h) \
    $(wildcard include/config/physical/start.h) \
    $(wildcard include/config/flatmem.h) \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/out/of/line/pfn/to/page.h) \
  include/asm-generic/page.h \
  include/asm/processor.h \
    $(wildcard include/config/x86/ht.h) \
    $(wildcard include/config/mk8.h) \
    $(wildcard include/config/mk7.h) \
  include/asm/vm86.h \
    $(wildcard include/config/vm86.h) \
  include/asm/math_emu.h \
  include/asm/sigcontext.h \
  include/asm/segment.h \
  include/asm/cpufeature.h \
  include/asm/msr.h \
  include/asm/system.h \
    $(wildcard include/config/x86/cmpxchg64.h) \
    $(wildcard include/config/x86/cmpxchg.h) \
    $(wildcard include/config/x86/oostore.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/x86.h) \
  include/asm/irqflags.h \
  include/linux/cache.h \
  include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/cpumask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
  include/asm/string.h \
  include/linux/stringify.h \
  include/linux/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/generic/hardirqs.h) \
    $(wildcard include/config/prove/locking.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  include/linux/stacktrace.h \
    $(wildcard include/config/stacktrace.h) \
  include/asm/spinlock_types.h \
  include/asm/spinlock.h \
    $(wildcard include/config/x86/ppro/fence.h) \
  include/asm/atomic.h \
    $(wildcard include/config/m386.h) \
  include/asm-generic/atomic.h \
  include/asm/rwlock.h \
  include/linux/spinlock_api_smp.h \
  include/asm/current.h \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
  include/linux/time.h \
  include/linux/seqlock.h \
  include/asm/timex.h \
    $(wildcard include/config/x86/elan.h) \
  include/asm/tsc.h \
    $(wildcard include/config/x86/tsc.h) \
    $(wildcard include/config/x86/generic.h) \
  include/linux/jiffies.h \
  include/linux/calc64.h \
  include/asm/div64.h \
  include/linux/rbtree.h \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/nodemask.h \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/asm/semaphore.h \
  include/linux/wait.h \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/asm/rwsem.h \
  include/asm/ptrace.h \
    $(wildcard include/config/frame/pointer.h) \
  include/asm/mmu.h \
  include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/smp.h \
  include/asm/smp.h \
    $(wildcard include/config/x86/local/apic.h) \
    $(wildcard include/config/x86/io/apic.h) \
  include/asm/fixmap.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/x86/visws/apic.h) \
    $(wildcard include/config/x86/f00f/bug.h) \
    $(wildcard include/config/x86/cyclone/timer.h) \
    $(wildcard include/config/acpi.h) \
    $(wildcard include/config/pci/mmconfig.h) \
  include/asm/acpi.h \
    $(wildcard include/config/acpi/sleep.h) \
  include/acpi/pdc_intel.h \
  include/asm/apicdef.h \
  include/asm/kmap_types.h \
    $(wildcard include/config/debug/highmem.h) \
  include/asm/mpspec.h \
  include/asm/mpspec_def.h \
  include/asm-i386/mach-generic/mach_mpspec.h \
  include/asm/io_apic.h \
    $(wildcard include/config/pci/msi.h) \
  include/asm/apic.h \
    $(wildcard include/config/x86/good/apic.h) \
    $(wildcard include/config/xen.h) \
  include/linux/pm.h \
  include/asm-i386/mach-generic/mach_apicdef.h \
  include/asm/genapic.h \
  include/linux/sem.h \
    $(wildcard include/config/sysvipc.h) \
  include/linux/ipc.h \
  include/asm/ipcbuf.h \
  include/asm/sembuf.h \
  include/linux/signal.h \
  include/asm/signal.h \
  include/asm-generic/signal.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/pid.h \
  include/linux/rcupdate.h \
  include/linux/percpu.h \
  include/linux/slab.h \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/gfp.h \
    $(wildcard include/config/dma/is/dma32.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
  include/linux/init.h \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/acpi/hotplug/memory.h) \
    $(wildcard include/config/acpi/hotplug/memory/module.h) \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/kmalloc_sizes.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/large/allocs.h) \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/futex.h \
    $(wildcard include/config/futex.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/param.h \
  include/linux/resource.h \
  include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/timer.h \
  include/linux/hrtimer.h \
    $(wildcard include/config/no/idle/hz.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/aio.h \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/linux/sysdev.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
    $(wildcard include/config/sysfs.h) \
  include/linux/kref.h \
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/elf.h \
  include/linux/elf-em.h \
  include/asm/elf.h \
    $(wildcard include/config/compat/vdso.h) \
  include/asm/user.h \
  include/linux/utsname.h \
  include/asm/desc.h \
  include/asm/ldt.h \
  include/linux/moduleparam.h \
  include/asm/local.h \
  include/asm/module.h \
    $(wildcard include/config/m486.h) \
    $(wildcard include/config/m586.h) \
    $(wildcard include/config/m586tsc.h) \
    $(wildcard include/config/m586mmx.h) \
    $(wildcard include/config/m686.h) \
    $(wildcard include/config/mpentiumii.h) \
    $(wildcard include/config/mpentiumiii.h) \
    $(wildcard include/config/mpentiumm.h) \
    $(wildcard include/config/mpentium4.h) \
    $(wildcard include/config/mk6.h) \
    $(wildcard include/config/mcrusoe.h) \
    $(wildcard include/config/mefficeon.h) \
    $(wildcard include/config/mwinchipc6.h) \
    $(wildcard include/config/mwinchip2.h) \
    $(wildcard include/config/mwinchip3d.h) \
    $(wildcard include/config/mcyrixiii.h) \
    $(wildcard include/config/mviac3/2.h) \
    $(wildcard include/config/mgeodegx1.h) \
    $(wildcard include/config/mgeode/lx.h) \
    $(wildcard include/config/regparm.h) \
  include/linux/delay.h \
  include/asm/delay.h \
  include/linux/hardirq.h \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/smp_lock.h \
    $(wildcard include/config/lock/kernel.h) \
  include/asm/hardirq.h \
  include/linux/irq.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/irq/per/cpu.h) \
    $(wildcard include/config/irq/release/method.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/irqbalance.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/auto/irq/affinity.h) \
  include/linux/irqreturn.h \
  include/asm/irq.h \
  include/asm-i386/mach-default/irq_vectors.h \
  include/asm-i386/mach-default/irq_vectors_limits.h \
  include/asm/hw_irq.h \
  include/linux/profile.h \
    $(wildcard include/config/profiling.h) \
  include/asm/sections.h \
  include/asm-generic/sections.h \
  include/linux/irq_cpustat.h \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/dvb_frontend.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/i2c.h \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/../linux/include/linux/i2c-id.h \
  include/linux/mod_devicetable.h \
  include/linux/device.h \
  include/linux/ioport.h \
  include/linux/klist.h \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/../linux/include/linux/dvb/frontend.h \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/dvbdev.h \
    $(wildcard include/config/dvb/core/attach.h) \
  include/linux/poll.h \
  include/asm/poll.h \
  include/linux/mm.h \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/debug/pagealloc.h) \
  include/linux/prio_tree.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
    $(wildcard include/config/security.h) \
  include/linux/limits.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/radix-tree.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
    $(wildcard include/config/64bit.h) \
  include/linux/err.h \
  include/linux/backing-dev.h \
  include/asm/pgtable.h \
    $(wildcard include/config/highpte.h) \
  include/asm/pgtable-2level-defs.h \
  include/asm/pgtable-2level.h \
  include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/swap.h) \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
    $(wildcard include/config/dma/is/normal.h) \
  include/asm/uaccess.h \
    $(wildcard include/config/x86/intel/usercopy.h) \
    $(wildcard include/config/x86/wp/works/ok.h) \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/compat.h \
  include/linux/version.h \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/config-compat.h \
    $(wildcard include/config/compat/h//.h) \
    $(wildcard include/config/video/bwqcam.h) \
    $(wildcard include/config/video/saa7146.h) \
    $(wildcard include/config/dvb/tda826x.h) \
    $(wildcard include/config/video/saa7146/vv.h) \
    $(wildcard include/config/video/dpc.h) \
    $(wildcard include/config/dvb/ttusb/budget.h) \
    $(wildcard include/config/video/em28xx.h) \
    $(wildcard include/config/video/cx88/vp3054.h) \
    $(wildcard include/config/dvb/cinergyt2.h) \
    $(wildcard include/config/video/wm8775.h) \
    $(wildcard include/config/dvb/usb/digitv.h) \
    $(wildcard include/config/dvb/mantis.h) \
    $(wildcard include/config/dvb/mantis/module.h) \
    $(wildcard include/config/video/cpia/usb.h) \
    $(wildcard include/config/video/bt848.h) \
    $(wildcard include/config/video/usbvideo.h) \
    $(wildcard include/config/dvb/ttusb/dec.h) \
    $(wildcard include/config/dvb/sp8870.h) \
    $(wildcard include/config/dvb/sp8870/module.h) \
    $(wildcard include/config/usb/ov511.h) \
    $(wildcard include/config/video/pvrusb2/24xxx.h) \
    $(wildcard include/config/radio/maestro.h) \
    $(wildcard include/config/dvb/s5h1420.h) \
    $(wildcard include/config/radio/miropcm20.h) \
    $(wildcard include/config/dvb/nxt200x.h) \
    $(wildcard include/config/dvb/tda10021.h) \
    $(wildcard include/config/dvb/core.h) \
    $(wildcard include/config/dvb/core/module.h) \
    $(wildcard include/config/video/pvrusb2/sysfs.h) \
    $(wildcard include/config/usb/sn9c102.h) \
    $(wildcard include/config/video/upd64083.h) \
    $(wildcard include/config/usb/dabusb.h) \
    $(wildcard include/config/dvb/dib3000mc.h) \
    $(wildcard include/config/dvb/pluto2.h) \
    $(wildcard include/config/radio/gemtek/pci.h) \
    $(wildcard include/config/dvb/tda1004x.h) \
    $(wildcard include/config/video/saa711x.h) \
    $(wildcard include/config/dvb/bt8xx.h) \
    $(wildcard include/config/video/cqcam.h) \
    $(wildcard include/config/video/zoran/avs6eyes.h) \
    $(wildcard include/config/tuner/3036.h) \
    $(wildcard include/config/usb/se401.h) \
    $(wildcard include/config/dvb/l64781.h) \
    $(wildcard include/config/video/buf/dvb.h) \
    $(wildcard include/config/video/buf/dvb/module.h) \
    $(wildcard include/config/dvb/usb/dtt200u.h) \
    $(wildcard include/config/radio/trust.h) \
    $(wildcard include/config/video/hexium/orion.h) \
    $(wildcard include/config/video/v4l1/compat.h) \
    $(wildcard include/config/video/cx88/alsa.h) \
    $(wildcard include/config/dvb/tuner/mt2060.h) \
    $(wildcard include/config/dvb/sp887x.h) \
    $(wildcard include/config/dvb/sp887x/module.h) \
    $(wildcard include/config/dvb/stv0297.h) \
    $(wildcard include/config/dvb/cx22700.h) \
    $(wildcard include/config/dvb/cx22700/module.h) \
    $(wildcard include/config/video/saa7134.h) \
    $(wildcard include/config/video/wm8739.h) \
    $(wildcard include/config/dvb/pll.h) \
    $(wildcard include/config/dvb/pll/module.h) \
    $(wildcard include/config/video/videobuf.h) \
    $(wildcard include/config/dvb/mt352.h) \
    $(wildcard include/config/dvb/mt352/module.h) \
    $(wildcard include/config/usb/vicam.h) \
    $(wildcard include/config/usb/stv680.h) \
    $(wildcard include/config/video/msp3400.h) \
    $(wildcard include/config/video/msp3400/module.h) \
    $(wildcard include/config/video/saa5249.h) \
    $(wildcard include/config/dvb/usb/gp8psk.h) \
    $(wildcard include/config/video/stradis.h) \
    $(wildcard include/config/video/zoran/dc10.h) \
    $(wildcard include/config/dvb/mt312.h) \
    $(wildcard include/config/video/tuner.h) \
    $(wildcard include/config/video/tuner/module.h) \
    $(wildcard include/config/video/planb.h) \
    $(wildcard include/config/video/adv/debug.h) \
    $(wildcard include/config/video/zoran/lml33r10.h) \
    $(wildcard include/config/dvb/b2c2/flexcop/debug.h) \
    $(wildcard include/config/video/cx88/blackbird.h) \
    $(wildcard include/config/video/v4l2.h) \
    $(wildcard include/config/video/pms.h) \
    $(wildcard include/config/dvb/usb/dibusb/mb/faulty.h) \
    $(wildcard include/config/dvb/cx22702.h) \
    $(wildcard include/config/dvb/cx22702/module.h) \
    $(wildcard include/config/video/meye.h) \
    $(wildcard include/config/video/hexium/gemini.h) \
    $(wildcard include/config/video/cx2341x.h) \
    $(wildcard include/config/radio/maxiradio.h) \
    $(wildcard include/config/radio/aztech.h) \
    $(wildcard include/config/dvb/isl6421.h) \
    $(wildcard include/config/dvb/isl6421/module.h) \
    $(wildcard include/config/dvb/or51211.h) \
    $(wildcard include/config/dvb/or51211/module.h) \
    $(wildcard include/config/dvb/nxt6000.h) \
    $(wildcard include/config/dvb/nxt6000/module.h) \
    $(wildcard include/config/dvb/stb0899.h) \
    $(wildcard include/config/dvb/stb0899/module.h) \
    $(wildcard include/config/dvb/usb/nova/t/usb2.h) \
    $(wildcard include/config/usb/ibmcam.h) \
    $(wildcard include/config/dvb/lnbp21.h) \
    $(wildcard include/config/usb/zc0301.h) \
    $(wildcard include/config/dvb/mantis/ci.h) \
    $(wildcard include/config/dvb/mantis/ci/module.h) \
    $(wildcard include/config/dvb/usb/vp702x.h) \
    $(wildcard include/config/video/saa7127.h) \
    $(wildcard include/config/dvb/zl10353.h) \
    $(wildcard include/config/dvb/zl10353/module.h) \
    $(wildcard include/config/video/saa7134/alsa.h) \
    $(wildcard include/config/dvb/b2c2/flexcop/usb.h) \
    $(wildcard include/config/dvb/budget/patch.h) \
    $(wildcard include/config/dvb/cinergyt2/enable/rc/input/device.h) \
    $(wildcard include/config/dvb/budget.h) \
    $(wildcard include/config/dvb/ves1820.h) \
    $(wildcard include/config/video/cx88/dvb.h) \
    $(wildcard include/config/video/cx88/dvb/module.h) \
    $(wildcard include/config/dvb/bcm3510.h) \
    $(wildcard include/config/dvb/av7110/firmware.h) \
    $(wildcard include/config/video/cs53l32a.h) \
    $(wildcard include/config/dvb/tda8083.h) \
    $(wildcard include/config/dvb/fe/customise.h) \
    $(wildcard include/config/video/zoran/lml33.h) \
    $(wildcard include/config/usb/dsbr.h) \
    $(wildcard include/config/video/saa7134/dvb.h) \
    $(wildcard include/config/usb/et61x251.h) \
    $(wildcard include/config/dvb/b2c2/flexcop.h) \
    $(wildcard include/config/usb/konicawc.h) \
    $(wildcard include/config/radio/cadet.h) \
    $(wildcard include/config/video/w9966.h) \
    $(wildcard include/config/usb/w9968cf.h) \
    $(wildcard include/config/video/ir.h) \
    $(wildcard include/config/video/ir/module.h) \
    $(wildcard include/config/video/m32r/ar.h) \
    $(wildcard include/config/dvb/usb/debug.h) \
    $(wildcard include/config/dvb/cx24110.h) \
    $(wildcard include/config/dvb/cx24110/module.h) \
    $(wildcard include/config/dvb/mb86a16.h) \
    $(wildcard include/config/dvb/mb86a16/module.h) \
    $(wildcard include/config/dvb/budget/ci.h) \
    $(wildcard include/config/dvb/stv0299.h) \
    $(wildcard include/config/dvb/stv0299/module.h) \
    $(wildcard include/config/dvb/cinergyt2/tuning.h) \
    $(wildcard include/config/video/cpia2.h) \
    $(wildcard include/config/video/ovcamchip.h) \
    $(wildcard include/config/dvb/cu1216.h) \
    $(wildcard include/config/dvb/cu1216/module.h) \
    $(wildcard include/config/video/vivi.h) \
    $(wildcard include/config/radio/gemtek.h) \
    $(wildcard include/config/radio/sf16fmi.h) \
    $(wildcard include/config/dvb/budget/av.h) \
    $(wildcard include/config/dvb/av7110.h) \
    $(wildcard include/config/dvb/b2c2/flexcop/pci.h) \
    $(wildcard include/config/radio/sf16fmr2.h) \
    $(wildcard include/config/dvb/usb.h) \
    $(wildcard include/config/video/vino.h) \
    $(wildcard include/config/dvb/or51132.h) \
    $(wildcard include/config/dvb/or51132/module.h) \
    $(wildcard include/config/video/kernel/version.h) \
    $(wildcard include/config/dvb/usb/umt/010.h) \
    $(wildcard include/config/video/tveeprom.h) \
    $(wildcard include/config/video/tveeprom/module.h) \
    $(wildcard include/config/radio/terratec.h) \
    $(wildcard include/config/radio/zoltrix.h) \
    $(wildcard include/config/radio/miropcm20/rds.h) \
    $(wildcard include/config/video/zoran.h) \
    $(wildcard include/config/video/cx88.h) \
    $(wildcard include/config/video/cx88/module.h) \
    $(wildcard include/config/radio/rtrack2.h) \
    $(wildcard include/config/usb/quickcam/messenger.h) \
    $(wildcard include/config/dvb/usb/dibusb/mc.h) \
    $(wildcard include/config/radio/typhoon/proc/fs.h) \
    $(wildcard include/config/dvb/dib3000mb.h) \
    $(wildcard include/config/dvb/usb/vp7045.h) \
    $(wildcard include/config/video/v4l1.h) \
    $(wildcard include/config/dvb/usb/cxusb.h) \
    $(wildcard include/config/video/btcx.h) \
    $(wildcard include/config/video/btcx/module.h) \
    $(wildcard include/config/video/zr36120.h) \
    $(wildcard include/config/video/tlv320aic23b.h) \
    $(wildcard include/config/video/cpia/pp.h) \
    $(wildcard include/config/usb/pwc/debug.h) \
    $(wildcard include/config/dvb/usb/a800.h) \
    $(wildcard include/config/dvb.h) \
    $(wildcard include/config/video/cpia.h) \
    $(wildcard include/config/dvb/usb/dibusb/mb.h) \
    $(wildcard include/config/video/buf.h) \
    $(wildcard include/config/video/buf/module.h) \
    $(wildcard include/config/video/bt848/dvb.h) \
    $(wildcard include/config/video/saa6588.h) \
    $(wildcard include/config/dvb/cx24123.h) \
    $(wildcard include/config/dvb/cx24123/module.h) \
    $(wildcard include/config/radio/typhoon.h) \
    $(wildcard include/config/video/dev.h) \
    $(wildcard include/config/video/dev/module.h) \
    $(wildcard include/config/usb/pwc.h) \
    $(wildcard include/config/dvb/ves1x93.h) \
    $(wildcard include/config/video/upd64031a.h) \
    $(wildcard include/config/video/saa7134/oss.h) \
    $(wildcard include/config/dvb/lgdt330x.h) \
    $(wildcard include/config/video/zoran/buz.h) \
    $(wildcard include/config/dvb/av7110/osd.h) \
    $(wildcard include/config/video/pvrusb2/debugifc.h) \
    $(wildcard include/config/video/m32r/ar/m64278.h) \
    $(wildcard include/config/video/zoran/dc30.h) \
    $(wildcard include/config/video/mxb.h) \
    $(wildcard include/config/video/saa5246a.h) \
    $(wildcard include/config/video/cx25840.h) \
    $(wildcard include/config/video/pvrusb2.h) \
    $(wildcard include/config/radio/rtrack.h) \
    $(wildcard include/config/dvb/tda10086.h) \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/dvb-pll.h \
  /root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.h \
  include/linux/firmware.h \

/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.o: $(deps_/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.o)

$(deps_/root/AZLinux_v1.4.2_CI_full_src/linuxdriver/v4l/or51132.o):
