if  grep "__x64_sys_read" /sys/kernel/debug/tracing/kprobe_events > /dev/null ; then
  echo '-:__x64_sys_read'  >> /sys/kernel/debug/tracing/kprobe_events
fi
./load $1

