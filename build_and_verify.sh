make clean
make kernel/kernel 2>&1 | tee build_final.log
if grep -iE "error:|warning:" build_final.log; then
    echo "BUILD_HAS_ISSUES"
else
    echo "BUILD_IS_CLEAN"
fi
cp build_final.log COMPILATION_LOG.txt
nm kernel/kernel | grep -E "syscall_dispatch|ramdisk_init"
